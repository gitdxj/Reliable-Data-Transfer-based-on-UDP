#include "rdtSocket.h"
#include <iostream>
#include <thread>
#include "timer.h"
using namespace std;

rdtSocket::rdtSocket()
{
	this->m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
	{
		printf("socket error !");
		exit(0);
	}

	this->recv_status = RecvStatus::wait_recv_0;
	this->send_status = SendStatus::wait_send_0;
}

int rdtSocket::rdt_send(char * package)
{
	char *new_package = NULL;
	//wait_ack状态代表重传，wait_send状态代表第一次发送
	if (send_status == wait_send_0 || send_status == wait_ack_0) 
		new_package = make_package(package, 0);
	else if (send_status == wait_send_1 || send_status == wait_ack_1) 
		new_package = make_package(package, 1);

	cout << "现在发送端状态为" << this->send_status << endl;

	sendto(m_socket, new_package, DEFAULT_BUFFER_SIZE+4, 0, (sockaddr*)&m_remote_addr, sizeof(m_remote_addr));
	Sleep(100);

	//发送完状态跳变
	switch (this->send_status)
	{
	case wait_send_0:
		send_status = wait_ack_0; break;
	case wait_send_1:
		send_status = wait_ack_1; break;
	//如果本身是等待ack状态的话，意味着上面的发送是重传，状态不需要跳变
	default: 
		break;
	}
	cout << "现在发送端状态为" << this->send_status << endl;


	//接收ACK，一定时间接收不到ACK就重传，接收到ACK则发送成功
	//如果等待的是ACK0而发来了ACK1就啥也不做
	Timer t;
	char ackBuffer[1024];
	memset(ackBuffer, 0, 1024);

	//根据状态，设置等待的ack值是多少
	int waitACK;
	if (send_status == wait_ack_0)
		waitACK = 0;
	else if (send_status == wait_ack_1)
		waitACK = 1;
	int ret = recvfrom(m_socket, ackBuffer, sizeof(ackBuffer), 0, NULL, NULL);
	if (isACK(ackBuffer, waitACK) && t.elapsed_milli() < 3000)
	{
		if (send_status == wait_ack_0)
			send_status = wait_send_1;
		else if (send_status == wait_ack_1)
			send_status = wait_send_0;
		cout << "现在发送端状态为" << this->send_status << endl;

		return 1;

	}
	else rdt_send(package); //不是则重传
	//int ret = recvfrom(m_socket, ackBuffer, sizeof(ackBuffer), 0, NULL, NULL);
	//while ((ret == -1 || !isACK(ackBuffer, 0)) && t.elapsed_milli() < DEFAULT_TIMEOUT) //没有收到任何数据或者收到的数据不是ACK0， 并且时间未超出timeout时间
	//	{
	//		int ret = recvfrom(m_socket, ackBuffer, sizeof(ackBuffer), 0, NULL, NULL);
	//	}
	return -1;
	
}

int rdtSocket::rdt_recv(char *package)
{

	char new_buffer[DEFAULT_BUFFER_SIZE + 4];
	memset(new_buffer, 0, DEFAULT_BUFFER_SIZE + 4);
	int ret = recvfrom(m_socket, new_buffer, DEFAULT_BUFFER_SIZE+4, 0, NULL, NULL);
	int waitSEQ;
	if (recv_status == wait_recv_0)
		waitSEQ = 0;
	else if (recv_status == wait_recv_1)
		waitSEQ = 1;

	cout << "接收端的状态：" << recv_status << endl;

	//如果等待的seq号和来的一样，就回复相应的ACK，
	//若不一样，就回复上一个ACK
	if (has_seq(new_buffer, waitSEQ))
	{
		//发送ACK
		send_ACK(waitSEQ);
		//if(waitSEQ == 0)
		//	sendto(m_socket, ACK0, DEFAULT_BUFFER_SIZE, 0, (sockaddr*)&m_remote_addr, sizeof(m_remote_addr));
		//else sendto(m_socket, ACK1, DEFAULT_BUFFER_SIZE, 0, (sockaddr*)&m_remote_addr, sizeof(m_remote_addr));
		//状态跳变
		if (recv_status == wait_recv_0)
			recv_status = wait_recv_1;
		else if (recv_status == wait_recv_1)
			recv_status = wait_recv_0;
		cout << "接收端的状态："<<recv_status << endl;
		char *extracted = extract_package(new_buffer);
		for (int i = 0; i < DEFAULT_BUFFER_SIZE; i++)
			package[i] = extracted[i];
		return 1;
	}
	else send_ACK((waitSEQ ? 0 : 1)); //若等待的是seq1则发送ack0，反之类似
	return -1;
}

int rdtSocket::rdt_recv(char * package, sockaddr_in& remote, int& len)
{
	char new_buffer[DEFAULT_BUFFER_SIZE + 4];
	memset(new_buffer, 0, DEFAULT_BUFFER_SIZE + 4);
	int ret = recvfrom(m_socket, new_buffer, DEFAULT_BUFFER_SIZE + 4, 0, (sockaddr *)&remote, &len);
	int waitSEQ;
	if (recv_status == wait_recv_0)
		waitSEQ = 0;
	else if (recv_status == wait_recv_1)
		waitSEQ = 1;

	cout << "接收端的状态：" << recv_status << endl;

	//如果等待的seq号和来的一样，就回复相应的ACK，
	//若不一样，就回复上一个ACK
	if (has_seq(new_buffer, waitSEQ))
	{
		//发送ACK
		send_ACK(waitSEQ);
		//状态跳变
		if (recv_status == wait_recv_0)
			recv_status = wait_recv_1;
		else if (recv_status == wait_recv_1)
			recv_status = wait_recv_0;
		cout << "接收端的状态：" << recv_status << endl;
		char *extracted = extract_package(new_buffer);
		for (int i = 0; i < DEFAULT_BUFFER_SIZE; i++)
			package[i] = extracted[i];
		return 1;
	}
	else send_ACK((waitSEQ ? 0 : 1)); //若等待的是seq1则发送ack0，反之类似
	return -1;
}


//判断一个包前四位是不是ACK1或者0
bool rdtSocket::isACK(char * package, int ack)
{
	char ACK;
	switch (ack)
	{
	case 0:
		ACK = '0';
		break;
	case 1:
	default:
		ACK = '1';
		break;
	}
	if (strlen(package) >= 4)
	{
		if (package[0] == 'A' &&
			package[1] == 'C' &&
			package[2] == 'K' &&
			package[3] == ACK)
			return true;
		else return false;
	}
	else return false;
}

bool rdtSocket::has_seq(char * package, int seq)
{
	char Seq;
	if (seq == 0)
		Seq = '0';
	else Seq = '1';
	if (strlen(package) >= 4)
	{
		if (package[0] == 's' &&
			package[1] == 'e' &&
			package[2] == 'q' &&
			package[3] == Seq)
			return true;
		else return false;
	}
	else return false;
}

char * rdtSocket::make_package(char * content, int seq)
{
	char Seq;
	if (0 == seq)
		Seq = '0';
	else Seq = '1';
	int content_length = DEFAULT_BUFFER_SIZE;
	int package_length = content_length + 4;
	char *package = new char[package_length];
	memset(package, 0, package_length);
	package[0] = 's'; package[1] = 'e'; package[2] = 'q'; package[3] = Seq;
	for (int i = 0; i < content_length; i++)
		package[i + 4] = char(content[i]);
	//strcpy(package, Seq);
	//strcat(package, content);
	//int content_len = strlen(content);
	//char *package = new char[content_len + 4];
	//return nullptr;


	return package;
}

char * rdtSocket::extract_package(char * package)
{
	if (has_seq(package, 0) || has_seq(package, 1)) //判断是否是有sequence标号
	{
		int length = DEFAULT_BUFFER_SIZE;
		char *new_package = new char[length];
		memset(new_package, 0, length);
		for (int i = 0; i < length; i++)
			new_package[i] = package[i + 4];
		return new_package;
	}
	else return package;
}

void rdtSocket::send_ACK(int ack)
{
	char ack_buffer[1024];
	memset(ack_buffer, 0, 1024);
	ack_buffer[0] = 'A';
	ack_buffer[1] = 'C';
	ack_buffer[2] = 'K';
	if (0 == ack)
		ack_buffer[3] = '0';
	else ack_buffer[3] = '1';
	sendto(m_socket, ack_buffer, 1024, 0, (sockaddr*)&m_remote_addr, sizeof(m_remote_addr));
	cout << "发送ACK" << ack << endl;
}

void rdtSocket::bindLocalAddress(int port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = INADDR_ANY;

	::bind(m_socket, (sockaddr *)&addr, sizeof(addr));
}

void rdtSocket::setRemoteAddress(std::string IP, int port)
{
	m_remote_addr.sin_family = AF_INET;
	m_remote_addr.sin_port = htons(port);
	m_remote_addr.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
}

void rdtSocket::reset_status()
{
	this->send_status = wait_send_0;
	this->recv_status = wait_recv_0;
}
