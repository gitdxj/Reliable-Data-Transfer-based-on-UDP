#include "rdtSocket.h"
#include "util.h"
#include <iostream>
#include <fstream>
#include <thread>
using namespace std;
int available_port = 8888;
void setMessage(char *buffer, string message);
void newConnection(int local_port, string IP, int remote_port);
int main()
{
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}
	rdtSocket socket;
	socket.bindLocalAddress(available_port);
	
	char send_buffer[DEFAULT_BUFFER_SIZE];
	char recv_buffer[DEFAULT_BUFFER_SIZE];
	memset(send_buffer, 0, DEFAULT_BUFFER_SIZE);
	memset(recv_buffer, 0, DEFAULT_BUFFER_SIZE);

	sockaddr_in remote_addr;
	int addr_len = sizeof(remote_addr);
	
	//socket.setRemoteAddress("192.168.139.1", 8080);
	//socket.send_ACK(1);

	while (true) {
		int a = socket.rdt_recv(recv_buffer, remote_addr, addr_len);
		cout << recv_buffer << endl;
		//char b[1024] = "hello";
		//socket.rdt_send(b);
		memset(recv_buffer, 0, DEFAULT_BUFFER_SIZE);
		
		//获取Clinet端的IP地址和端口号
		string remote_ip = inet_ntoa(remote_addr.sin_addr);
		int remote_port = ntohs(remote_addr.sin_port);
		cout << "IP address: " << remote_ip << endl
			<< "port: " << remote_port << endl;

		socket.setRemoteAddress(remote_ip, remote_port);
		socket.send_ACK(0);

		//创建新的socket和客户端通信
		int next_port = ++available_port; //新的socket端口号为next_port
		cout << "连接的端口号为" << next_port << endl;
		string str_port = to_string(next_port);
		for (int i = 0; i < str_port.length(); i++)
			send_buffer[i] = str_port[i];
		socket.rdt_send(send_buffer); //将新的端口号发给客户端
		memset(send_buffer, 0, DEFAULT_BUFFER_SIZE);
		socket.reset_status();
		socket.setRemoteAddress("1.1.1.1", 111);
		cout << "线程建立，状态重置" << endl;
		
		thread new_thread(newConnection, next_port, remote_ip, remote_port);
		new_thread.detach();

	}

	this_thread::sleep_for(std::chrono::hours(1));
	system("pause");
	return 0;
}

void newConnection(int local_port, string IP, int remote_port)
{
	rdtSocket socket;
	socket.bindLocalAddress(local_port);
	socket.setRemoteAddress(IP, remote_port);
	char send_buffer[DEFAULT_BUFFER_SIZE];
	char recv_buffer[DEFAULT_BUFFER_SIZE];
	memset(send_buffer, 0, DEFAULT_BUFFER_SIZE);
	memset(recv_buffer, 0, DEFAULT_BUFFER_SIZE);
	while (true)
	{
		int ret = socket.rdt_recv(recv_buffer);
		string recv = recv_buffer;
		string para;
		int findColon = recv.find(":");
		para = recv.substr(findColon + 1, recv.length() - findColon);
		memset(recv_buffer, 0, DEFAULT_BUFFER_SIZE);

		Command command = Command(parse_command(recv));
		switch (command)
		{
		case LS: 
		{
			string names;
			names = get_all_files_names_within_folder("C:/ftp");
			memset(send_buffer, 0, 1024);
			setMessage(send_buffer, names);
			socket.rdt_send(send_buffer);
			memset(send_buffer, 0, 1024);
			break;
		}
		case DOWN:
		{
			ifstream read;
			read.open(("C:/ftp/" + para), ios::in | ios::binary);
			memset(send_buffer, 0, 1024);
			while (read.read(send_buffer, 1024))
			{
				socket.rdt_send(send_buffer);
				memset(send_buffer, 0, 1024);
			}
			memset(send_buffer, 0, 1024);
			send_buffer[0] = '.';
			send_buffer[1] = '\r';
			send_buffer[2] = '\n';
			socket.rdt_send(send_buffer);
			memset(send_buffer, 0, 1024);
			read.close();
			break;
		}
		case UP:
		{
			ofstream write;
			write.open(("C:/ftp/") + para, ios::out | ios::binary);
			memset(recv_buffer, 0, 1024);
			socket.rdt_recv(recv_buffer);
			while (!(recv_buffer[0] == '.' &&
				recv_buffer[1] == '\r' &&
				recv_buffer[2] == '\n')) {
				write.write(recv_buffer, 1024);
				memset(recv_buffer, 0, 1024);
				socket.rdt_recv(recv_buffer);
			}
			cout << "收到文件" << endl;
			write.close();
			break;
		}
		case DATA:
		case QUIT:
			break;
		}
	}
}


void setMessage(char *buffer, string message)
{
	for (int i = 0; i < message.length(); i++)
		buffer[i] = message[i];
}