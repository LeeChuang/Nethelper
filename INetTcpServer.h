#pragma once
#ifndef INET_TCP_SERVER_H
#define INET_TCP_SERVER_H
#include "INet.h"
class CINetTcpServer :
	public CINet
{
public:
	CINetTcpServer();
	~CINetTcpServer();
	//连接
	bool Connect();

	//发送数据
	int SendData(char *str_data);

	//接收数据
	int ReciveData(char *str_data) { return 1; };	

private:

	static void TCPAcceptThread(WSAEVENT &wsa_event, SOCKET &server_socket,
		std::unordered_map <std::string, SOCKET> *client_map);
	
};

#endif