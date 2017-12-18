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
// ��������
enum NetType{
	TCP_CLIENT_ASK_ANSWER,	//TCP�ͻ��� �ʴ�ģʽ
	TCP_CLIENT_RECEIVE,		//TCP�ͻ��� �Զ�����ģʽ
	TCP_SERVER,				//TCP�����ģʽ
	UDP						//UDPģʽ
};
// ��������
enum LinkType{
	KEEP_ALIVE,				//������
	SHORT_CONNECTION		//������
};
class CINet
{
public:
	//���졢����
	CINet();
	~CINet();

	//��ʼ������
	bool InitializeNet(NetType net_type, LinkType link_type, std::string ip, UINT port);
	
	//����
	bool connecting();

	//��������
	int SendData(char *str_data);

	//��������
	int ReciveData(char *str_data);

	//�ر�����
	void Shutdown();

	//����TCP��������������
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

	//TCP�����ʱ  ʹ������
	UINT m_max_client_number;
	std::unordered_map <std::string, SOCKET> m_client_umap;
};

#endif
