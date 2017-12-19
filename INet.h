#pragma once
#ifndef INET_H
#define INET_H

#include <WinSock2.h>
#include <string>
#include <mutex>
#include <unordered_map>

#pragma comment(lib,"ws2_32.lib")

#define INET_BUFF_LEN 1024*20
#define DEFAULT_LINK_NUMBER 5
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 45678
#define UDP_LOCAL_PORT 51515
#define WAIT_CLIENT_LINK_TIME 5*1000

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
	CINet()
	{
		m_socket = INVALID_SOCKET;
		m_event = WSA_INVALID_EVENT;
		m_net_type = NetType::TCP_CLIENT_ASK_ANSWER;
		m_strIP = DEFAULT_IP;
		m_port = DEFAULT_PORT;
		m_max_client_number = DEFAULT_LINK_NUMBER;
		m_udp_local_port = UDP_LOCAL_PORT;
	};
	~CINet()
	{
		Shutdown();
	};

	//����
	virtual bool Connect() = 0;

	//��������
	virtual int SendData(char *str_data) = 0;

	//��������
	virtual int ReciveData(char *str_data) = 0;

	//�ر�����
	void Shutdown()
	{
		if (m_client_umap.size() > 0)
		{
			for (auto temp_map : m_client_umap)
			{
				if (temp_map.second != INVALID_SOCKET)
					closesocket(temp_map.second);
			}
			m_client_umap.clear();
		}
		if (m_socket != INVALID_SOCKET)
			closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	};

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

public:
	WSAEVENT m_event;
	SOCKET m_socket;
	NetType m_net_type;

	//Ŀ��ip���˿�
	std::string m_strIP;
	UINT m_port;

	//TCP�����ʱ  ʹ������
	UINT m_max_client_number;
	std::unordered_map <std::string, SOCKET> m_client_umap;
	
	//UDP���ؼ��Ӷ˿�
	UINT m_udp_local_port;
};

#endif
