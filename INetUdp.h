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
	//����
	bool Connect();

	//��������
	int SendData(char *str_data);

	//��������
	int ReceiveData(char *str_data);

	//��������
	int SendData(char *str_data, std::string destination_ip){ return 1; };

	//��������
	int ReceiveData(char *str_data, std::string destination_ip);
private:
	void ReceiveThread();

};

#endif