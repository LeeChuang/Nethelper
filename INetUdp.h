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
	bool Connect(){ return true; };

	//��������
	int SendData(char *str_data);

	//��������
	int ReciveData(char *str_data){ return 0; };
private:
	//UDP���ؼ��Ӷ˿�
	//UINT m_udp_local_port;
};

#endif