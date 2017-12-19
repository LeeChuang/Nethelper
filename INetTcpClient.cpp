#include "INetTcpClient.h"
#include <iostream>
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
		
		m_event = WSACreateEvent();
		if (m_event == WSA_INVALID_EVENT)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return false;
		}
		if (WSAEventSelect(m_socket, m_event, FD_WRITE | FD_READ) == SOCKET_ERROR)
			return false;

		//设置socket缓冲区大小
		int temp_buf = INET_BUFF_LEN;
		setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&temp_buf, sizeof(int));
		setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&temp_buf, sizeof(int));

		if (connect(m_socket, (sockaddr*)&server_address, sizeof(SOCKADDR_IN))
			== SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				closesocket(m_socket);
				WSACloseEvent(m_event);
				m_socket = INVALID_SOCKET;
				m_event = WSA_INVALID_EVENT;
				return false;
			}

			DWORD d_error = WSAWaitForMultipleEvents(1, &m_event, FALSE, SEND_RECEIVE_TIMEOUT, FALSE);	
			WSAResetEvent(m_event);		//复位事件为无信号状态
			if (d_error == WSA_WAIT_FAILED || d_error == WSA_WAIT_TIMEOUT)
			{
				if (m_event != WSA_INVALID_EVENT)
					WSACloseEvent(m_event);
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				m_event = WSA_INVALID_EVENT;
				return false;
			}
		}

	}
	catch (...)
	{
		closesocket(m_socket);
		WSACloseEvent(m_event);
		m_socket = INVALID_SOCKET;
		m_event = WSA_INVALID_EVENT;
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
			if (!Connect())	return -2;
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
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return -1;
	}
	return send_size;
}

int CINetTcpClient::ReceiveData(char * str_data)
{
	int recive_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect()) return -2;
		}
		
		DWORD d_error = WSAWaitForMultipleEvents(1, &m_event, FALSE, SEND_RECEIVE_TIMEOUT, FALSE);
		if (d_error == WSA_WAIT_FAILED || d_error == WSA_WAIT_TIMEOUT)
		{
			closesocket(m_socket);
			WSACloseEvent(m_event);
			m_socket = INVALID_SOCKET;
			m_event = WSA_INVALID_EVENT;
			return -3;
		}

		recive_size = recv(m_socket, (char*)str_data, INET_BUFF_LEN, 0);

	}
	catch (...)
	{
		closesocket(m_socket);
		WSACloseEvent(m_event);
		m_socket = INVALID_SOCKET;
		m_event = WSA_INVALID_EVENT;
		return -1;
	}

	return recive_size;
}