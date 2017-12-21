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
	int ReceiveData(char *str_data);	

	//发送数据
	int SendData(char *str_data, std::string destination_ip);

	//接收数据
	int ReceiveData(char *str_data, std::string destination_ip);

private:

	void TCPAcceptThread();
	
	void ReceiveDataThread(std::string &client_ip);

	
	
};

#endif