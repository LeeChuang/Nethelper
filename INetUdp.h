#pragma once
#ifndef INET_UDP_H
#define INET_UDP_H
#include "INet.h"
class CINetUdp :
	public CINet
{
public:
	CINetUdp();
	~CINetUdp();
	//连接
	bool Connect();

	//发送数据
	int SendData(char *str_data);

	//接收数据
	int ReceiveData(char *str_data);

	//发送数据
	int SendData(char *str_data, std::string destination_ip){ return 1; };

	//接收数据
	int ReceiveData(char *str_data, std::string destination_ip);
private:
	void ReceiveThread();

};

#endif