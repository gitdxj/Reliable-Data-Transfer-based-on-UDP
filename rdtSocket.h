#ifndef RDT_SOCKET
#define RDT_SOCKET
#include <WinSock2.h>
#include <string>
#pragma comment(lib, "ws2_32.lib") 

#define DEFAULT_BUFFER_SIZE 1024
#define DEFAULT_TIMEOUT 800 //默认的超时时间，以millisecond为单位

//接收端和发送端的状态 根据rdt3.0
enum RecvStatus  { wait_recv_0, wait_recv_1 };
enum SendStatus  { wait_send_0, wait_ack_0, wait_send_1, wait_ack_1 };

class rdtSocket
{
private:
	RecvStatus recv_status;
	SendStatus send_status;
	char sendBuffer[DEFAULT_BUFFER_SIZE];
	char recvBuffer[DEFAULT_BUFFER_SIZE];
	SOCKET m_socket;
	sockaddr_in m_remote_addr;
public :
	rdtSocket();
	int rdt_send(char *package);
	int rdt_recv(char *package);
	int rdt_recv(char *package, sockaddr_in& remote, int& len);
	bool isACK(char *package, int ack); //ack只能取0或者1
	bool has_seq(char *package, int seq); //seq只能取0或1
	char* make_package(char* content, int seq); //在buffer前面加上seq组成数据包
	char* extract_package(char* package); //对有把数据包前面的seq删除
	void send_ACK(int ack);
	void bindLocalAddress(int port);
	void setRemoteAddress(std::string IP, int port);

	void reset_status();
};


#endif
