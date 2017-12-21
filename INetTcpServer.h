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
	int ReceiveData(char *str_data);	

	//��������
	int SendData(char *str_data, std::string destination_ip);

	//��������
	int ReceiveData(char *str_data, std::string destination_ip);

private:

	void TCPAcceptThread();
	
	void ReceiveDataThread(std::string &client_ip);

	
	
};

#endif