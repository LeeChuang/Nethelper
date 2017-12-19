#pragma once
#ifndef INET_TCP_CLIENT_H
#define INET_TCP_CLIENT_H

#include "INet.h"
class CINetTcpClient :
	public CINet
{
public:
	CINetTcpClient();
	~CINetTcpClient();
	//连接
	bool Connect();

	//发送数据
	int SendData(char *str_data);

	//接收数据
	int ReceiveData(char *str_data);

};

#endif