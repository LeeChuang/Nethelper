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
#define UDP_LOCAL_PORT 51515
// ��������
enum NetType{
	TCP_CLIENT_ASK_ANSWER,	//TCP�ͻ��� �ʴ�ģʽ
	TCP_CLIENT_RECEIVE,		//TCP�ͻ��� �Զ�����ģʽ
	TCP_SERVER,				//TCP�����ģʽ
	UDP						//UDPģʽ
};
//// ��������
//enum LinkType{
//	KEEP_ALIVE,				//������
//	SHORT_CONNECTION		//������
//};
class CINet
{
public:
	//���졢����
	CINet();
	~CINet();

	//����
	bool connecting();

	//��������
	int SendData(char *str_data);

	//��������
	int ReciveData(char *str_data);

	//�ر�����
	void Shutdown();

	//��ʼ������
	inline void InitializeNet(NetType net_type, std::string ip, UINT port)
	{
		m_net_type = net_type;
		m_strIP = ip;
		m_port = port;
	};

	//����TCP��������������
	inline void SetMaxClientNumber(int num){ m_max_client_number = num; };

	//����UDP���ؼ��Ӷ˿�
	inline void SetUDPLocalPort(UINT port){ m_udp_local_port = port; }

private:
	int UDPSendData(char *str_data);
	int TCPClientSendData(char *str_data);
	int TCPServerSendData(char *str_data);

	bool TCPClientConnect();
	bool TCPServerConnect();

	int UDPReciveData(char *str_data);
	int TCPClientReciveData(char *str_data);
	int TCPServerReciveData(char *str_data);
private:
	WSAEVENT m_event;
	SOCKET m_socket;
	NetType m_net_type;

	//Ŀ��ip���˿�
	std::string m_strIP;
	UINT m_port;

	//UDP���ؼ��Ӷ˿�
	UINT m_udp_local_port;

	//TCP�����ʱ  ʹ������
	UINT m_max_client_number;
	std::unordered_map <std::string, SOCKET> m_client_umap;
};

#endif
