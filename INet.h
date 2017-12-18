#pragma once
#ifndef INET_H
#define INET_H

#include <WinSock2.h>
#include <string>
#include <unordered_map>

#define INET_BUFF_LEN 1024*20
#define DEFAULT_LINK_NUMBER 5
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 45678
// 网络类型
enum NetType{
	TCP_CLIENT_ASK_ANSWER,	//TCP客户端 问答模式
	TCP_CLIENT_RECEIVE,		//TCP客户端 自动接收模式
	TCP_SERVER,				//TCP服务端模式
	UDP						//UDP模式
};
// 连接类型
enum LinkType{
	KEEP_ALIVE,				//长连接
	SHORT_CONNECTION		//短连接
};
class CINet
{
public:
	//构造、析构
	CINet();
	~CINet();

	//初始化网络
	bool InitializeNet(NetType net_type, LinkType link_type, std::string ip, UINT port);
	
	//连接
	bool connecting();

	//发送数据
	int SendData(char *str_data);

	//接收数据
	int ReciveData(char *str_data);

	//关闭连接
	void Shutdown();

	//设置TCP服务端最大连接数
	void SetMaxClientNumber(int num){ m_max_client_number = num; };
private:
	int UDPSendData(char *str_data);
	int TCPClientSendData(char *str_data);
	int TCPServerSendData(char *str_data);

private:
	WSAEVENT m_event;
	SOCKET m_socket;
	NetType m_net_type;
	LinkType m_link_type;
	std::string m_strIP;
	UINT m_port;

	//TCP服务端时  使用数据
	UINT m_max_client_number;
	std::unordered_map <std::string, SOCKET> m_client_umap;
};

#endif
