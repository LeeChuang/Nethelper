#include "INetTcpClient.h"

CINetTcpClient::CINetTcpClient()
{
}


CINetTcpClient::~CINetTcpClient()
{
	Shutdown();
}

bool CINetTcpClient::Connect()
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
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	return true;
}

int CINetTcpClient::SendData(char *str_data)
{
	int send_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect())
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