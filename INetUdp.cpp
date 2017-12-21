#include "INetUdp.h"
#include <iostream>

CINetUdp::CINetUdp()
{

}

CINetUdp::~CINetUdp()
{
	Shutdown();
}

bool CINetUdp::Connect()
{
	try
	{
		if (m_socket != INVALID_SOCKET)
			return true;
		if ((m_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
			return false;

		SOCKADDR_IN local_address;
		local_address.sin_family = AF_INET;
		local_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		local_address.sin_port = htons(m_udp_local_port);
		memset(local_address.sin_zero, 0, sizeof(local_address.sin_zero));

		int temp_buf = INET_BUFF_LEN;
		setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&temp_buf, sizeof(int));
		setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&temp_buf, sizeof(int));

		if (bind(m_socket, (struct sockaddr*)&local_address, sizeof(SOCKADDR_IN))
			== SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return false;
		}

		m_accept_running = true;
		if (m_accept_thread.joinable())
		{
			m_accept_thread.join();
		}
		m_accept_thread = std::thread(&CINetUdp::ReceiveThread,this);
	}
	catch (...)
	{
		return false;
	}
	return true;
}

int CINetUdp::SendData(char *str_data)
{
	int send_size = 0;
	SOCKET temp_socket = INVALID_SOCKET;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect()) return -2;
		}

		if ((temp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
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

int CINetUdp::ReceiveData(char *str_data, std::string destination_ip)
{
	int receive_size = 0;

	try
	{
		Sleep(50);
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect()) return -2;
		}
		{
			std::lock_guard<std::mutex> guard(g_mutex);
			auto it_data_map = m_last_client_data.find(destination_ip);
			if (it_data_map == m_last_client_data.end())
				return -10;

			if (m_last_client_data[destination_ip].size() < 0)
				return -9;

			std::string temp_str = "";
			temp_str = m_last_client_data[destination_ip].back();
			if (m_last_client_data[destination_ip].size() > 1)  //保证有数据
				m_last_client_data[destination_ip].pop_back();

			receive_size = temp_str.length();
			memcpy(str_data, temp_str.c_str(), receive_size);

		}

	}
	catch (...)
	{
		std::lock_guard<std::mutex> guard(g_mutex);
		
		if (m_last_client_data.find(destination_ip) != m_last_client_data.end())
		{
			m_last_client_data[destination_ip].clear();
			m_last_client_data.erase(destination_ip);
		}

		WSACloseEvent(m_event);
		m_event = WSA_INVALID_EVENT;
		return -1;
	}

	return receive_size;
}

void CINetUdp::ReceiveThread()
{
	try
	{
		DWORD wsa_error = 0;
		while (m_accept_running)
		{
			struct timeval tv;
			fd_set readfds;
			tv.tv_sec = UDP_RECEIVE_TIMEOUT;
			tv.tv_usec = 0;
			FD_ZERO(&readfds);
			FD_SET(m_socket, &readfds);

			if (select(1, &readfds, NULL, NULL, &tv) <= 0)
			{
				continue;
			}
			char temp_data[INET_BUFF_LEN] = "\0";
			SOCKADDR_IN temp_client_addr;
			int temp_len = sizeof(SOCKADDR_IN);
			if (recvfrom(m_socket, temp_data, INET_BUFF_LEN, 0, (sockaddr *)&temp_client_addr, (int *)&temp_len)
				== SOCKET_ERROR)
			{
				closesocket(m_socket);
				m_socket = INVALID_SOCKET;
				break;
			}

			std::string temp_ip = inet_ntoa(temp_client_addr.sin_addr);
			{
				std::lock_guard<std::mutex> gurad(g_mutex);
				m_last_client_data[temp_ip].push_front(temp_data);
			}
		}
	}
	catch (...)
	{
		if (m_socket != INVALID_SOCKET)
			closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		std::cout << " 发生异常，进行重连" << std::endl;
		return;
	}
}

int CINetUdp::ReceiveData(char *str_data)
{
	int receive_size = 0;
	std::string str_map_key = "";
	try
	{
		Sleep(50);
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect()) return -2;
		}
		{
			std::lock_guard<std::mutex> guard(g_mutex);
			
			auto it_data_map = m_last_client_data.begin();
			if (it_data_map == m_last_client_data.end())
				return -10;
			str_map_key = it_data_map->first;
			if (m_last_client_data[str_map_key].size() < 0)
				return -9;

			std::string temp_str = "";
			temp_str = m_last_client_data[str_map_key].back();
			if (m_last_client_data[str_map_key].size() > 1)  //保证有数据
				m_last_client_data[str_map_key].pop_back();

			receive_size = temp_str.length();
			memcpy(str_data, temp_str.c_str(), receive_size);

		}

	}
	catch (...)
	{
		std::lock_guard<std::mutex> guard(g_mutex);
		
		if (m_last_client_data.find(str_map_key) != m_last_client_data.end())
		{
			m_last_client_data[str_map_key].clear();
			m_last_client_data.erase(str_map_key);
		}

		WSACloseEvent(m_event);
		m_event = WSA_INVALID_EVENT;
		return -1;
	}

	return receive_size;

}
