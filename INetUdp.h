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
	bool Connect(){ return true; };

	//发送数据
	int SendData(char *str_data);

	//接收数据
	int ReciveData(char *str_data){ return 0; };
private:
	//UDP本地监视端口
	//UINT m_udp_local_port;
};

#endif