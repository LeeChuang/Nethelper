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
	//����
	bool Connect();

	//��������
	int SendData(char *str_data);

	//��������
	int ReceiveData(char *str_data);

	//��������
	virtual int SendData(char *str_data, std::string destination_ip) { return 1; };

	//��������
	virtual int ReceiveData(char *str_data, std::string destination_ip) { return 1; };

};

#endif