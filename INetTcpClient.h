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
	int ReciveData(char *str_data){ return 1; };

};

#endif