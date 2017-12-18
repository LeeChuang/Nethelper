#include "INet.h"

#pragma comment(lib,"ws2_32.lib")

CINet::CINet()
{
	m_socket = INVALID_SOCKET;
	m_net_type = NetType::TCP_CLIENT_ASK_ANSWER;
	m_max_client_number = DEFAULT_LINK_NUMBER;
	m_strIP = DEFAULT_IP;
	m_port = DEFAULT_PORT;
}
CINet::CINet(NetType net_type, std::string ip, UINT port)
{
	InitializeNet(net_type, ip, port);
}

CINet::~CINet()
{
	Shutdown();
}

void CINet::Shutdown()
{
	if (m_client_umap.size() > 0)
	{
		for (auto tempmap : m_client_umap)
		{
			if (tempmap.second != INVALID_SOCKET)
				closesocket(tempmap.second);
		}
		m_client_umap.clear();
	}

	if (m_socket != INVALID_SOCKET)
		closesocket(m_socket);
	m_socket = INVALID_SOCKET;
}

bool CINet::connecting()
{
	if (m_net_type == UDP)
		return true;
	else if (m_net_type == TCP_SERVER)
		return TCPServerConnect();
	else
		return TCPClientConnect();
}

int CINet::SendData(char* str_data)
{
	if (m_net_type == UDP)
		return UDPSendData(str_data);
	else if (m_net_type == TCP_SERVER)
		return TCPServerSendData(str_data);
	else
		return TCPClientSendData(str_data);
}

int CINet::ReciveData(char* str_data)
{
	if (m_net_type == UDP)
		return UDPReciveData(str_data);
	else if (m_net_type == TCP_SERVER)
		return TCPServerReciveData(str_data);
	else
		return TCPClientReciveData(str_data);
}

int CINet::UDPSendData(char* str_data)
{
	int send_size = 0;
	SOCKET temp_socket = INVALID_SOCKET;
	try
	{
		if ((m_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
			return -1;

		SOCKADDR_IN temp_addr;
		temp_addr.sin_family = AF_INET;
		temp_addr.sin_addr.S_un.S_addr = inet_addr(m_strIP.c_str());
		temp_addr.sin_port = htons(m_port);
		memset(temp_addr.sin_zero, 0, sizeof(temp_addr.sin_zero));
		while (send_size != strlen(str_data))
		{
			int temp_send_size = 0;
			temp_send_size = sendto(temp_socket, 
				str_data + send_size, 
				strlen(str_data) - send_size, 
				0, 
				(sockaddr*)&temp_addr, 
				sizeof(SOCKADDR_IN));
			if (temp_send_size == SOCKET_ERROR)
			{
				send_size = -1;
				closesocket(temp_socket);
				temp_socket = INVALID_SOCKET;
				break;
			}
			send_size += temp_send_size;
		}
		Sleep(20);
		closesocket(temp_socket);
		temp_socket = INVALID_SOCKET;
	}
	catch (...)
	{
		return -1;
	}
	return send_size;
}

int CINet::TCPClientSendData(char* str_data)
{
	int send_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!connecting())
				return -1;
		}
		
		while (send_size != strlen(str_data))
		{
			int temp_send_size = 0;
			temp_send_size = send(m_socket, 
				str_data + send_size, 
				strlen(str_data) - send_size, 
				0);
			if (temp_send_size == SOCKET_ERROR)
			{
				send_size = -1;
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				break;
			}
			send_size += temp_send_size;
		}
	}	
	catch (...)
	{
		return -1;
	}
	return send_size;
}

int CINet::TCPServerSendData(char* str_data)
{
	int send_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!connecting()) return -1;
		}

		while (send_size != strlen(str_data))
		{
			int temp_send_size = 0;
			temp_send_size = send(m_socket, 
				str_data + send_size, 
				strlen(str_data) - send_size, 
				0);
			if (temp_send_size == SOCKET_ERROR)
			{
				send_size = -1;
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				break;
			}
			send_size += temp_send_size;
		}
	}
	catch (...)
	{
		return -1;
	}
	return send_size;
}
bool CINet::TCPServerConnect()
{
	try
	{
		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
}

bool CINet::TCPClientConnect()
{
	try
	{
		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			return false;

		SOCKADDR_IN server_address;
		server_address.sin_family = AF_INET;
		server_address.sin_addr.S_un.S_addr = inet_addr(m_strIP.c_str());
		server_address.sin_port = htons(m_port);
		memset(server_address.sin_zero, 0, sizeof(server_address.sin_zero));

		if (connect(m_socket, (sockaddr*)&server_address, sizeof(SOCKADDR_IN)) 
			== SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return false;
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}

int CINet::UDPReciveData(char* str_data)
{
	int recv_size = 0;

	return recv_size;
}

int CINet::TCPServerReciveData(char* str_data)
{
	int recv_size = 0;

	return recv_size;
}

int CINet::TCPClientReciveData(char* str_data)
{
	int recv_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!connecting()) return -1;
		}		
	}
	catch (...)
	{
		return -1;
	}
	return recv_size;
}