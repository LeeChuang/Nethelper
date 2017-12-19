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
	//����
	bool Connect();

	//��������
	int SendData(char *str_data);

	//��������
	int ReciveData(char *str_data) { return 1; };	

private:

	static void TCPAcceptThread(WSAEVENT &wsa_event, SOCKET &server_socket,
		std::unordered_map <std::string, SOCKET> *client_map);
	
};

#endif