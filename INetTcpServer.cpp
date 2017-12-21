#include "INetTcpServer.h"

CINetTcpServer::CINetTcpServer()
{

}


CINetTcpServer::~CINetTcpServer()
{
	Shutdown();
}

bool CINetTcpServer::Connect()
{
	try
	{
		if (m_socket != INVALID_SOCKET)
			return true;

		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			return false;

		SOCKADDR_IN local_address;
		local_address.sin_family = AF_INET;
		local_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		local_address.sin_port = htons(m_port);
		memset(local_address.sin_zero, 0, sizeof(local_address.sin_zero));

		//设置缓冲区大小
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

		if (listen(m_socket, m_max_client_number) == SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			return false;
		}

		m_event = WSACreateEvent();
		if (m_event == WSA_INVALID_EVENT)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			WSACloseEvent(m_event);
			m_event = WSA_INVALID_EVENT;
			return false;
		}

		if (WSAEventSelect(m_socket, m_event, FD_ACCEPT)
			== SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			WSACloseEvent(m_event);
			m_event = WSA_INVALID_EVENT;
			return false;
		}
		m_accept_running = true;
		if (m_accept_thread.joinable())
		{
			m_accept_thread.detach();
		}
		m_accept_thread = std::thread(&CINetTcpServer::TCPAcceptThread,this);

	/*	if (m_accept_thread.joinable())
			m_accept_thread.detach();*/

	}
	catch (...)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return false;
	}
	return true;
}

int CINetTcpServer::SendData(char *str_data)
{
	int send_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect()) return -1;
			Sleep(500);
			SendData(str_data);
		}

		{
			std::lock_guard<std::mutex> guard(g_mutex);
			if (m_client_umap.size() <= 0)
				return -2;

			for (auto client_umap : m_client_umap)
			{
				while (send_size != strlen(str_data))
				{
					int temp_send_size = 0;
					temp_send_size = send(client_umap.second,
						str_data + send_size,
						strlen(str_data) - send_size,
						0);
					if (temp_send_size == SOCKET_ERROR)
					{
						send_size = -1;
						closesocket(client_umap.second);
						client_umap.second = INVALID_SOCKET;
						m_client_umap.erase(client_umap.first);
						break;
					}
					send_size += temp_send_size;
				}
			}

		}
	}
	catch (...)
	{
		return -5;
	}
	return send_size;
}

int CINetTcpServer::SendData(char *str_data, std::string destination_ip)
{
	int send_size = 0;
	try
	{
		if (m_socket == INVALID_SOCKET)
		{
			if (!Connect()) return -1;
			Sleep(500);
			SendData(str_data);
		}

		{
			std::lock_guard<std::mutex> guard(g_mutex);
			if (m_client_umap.size() <= 0)
				return -2;

			auto client_umap = m_client_umap.find(destination_ip);
			if (client_umap == m_client_umap.end())
				return -4;

			while (send_size != strlen(str_data))
			{
				int temp_send_size = 0;
				temp_send_size = send(client_umap->second,
					str_data + send_size,
					strlen(str_data) - send_size,
					0);
				if (temp_send_size == SOCKET_ERROR)
				{
					send_size = -1;
					closesocket(client_umap->second);
					client_umap->second = INVALID_SOCKET;
					m_client_umap.erase(client_umap->first);
					break;
				}
				send_size += temp_send_size;
			}
		}
	}
	catch (...)
	{
		std::lock_guard<std::mutex> guard(g_mutex);
		if (m_client_umap.find(destination_ip) != m_client_umap.end())
		{
			closesocket(m_client_umap[destination_ip]);
			m_client_umap[destination_ip] = INVALID_SOCKET;
			m_client_umap.erase(destination_ip);
		}
		if (m_last_client_data.find(destination_ip) != m_last_client_data.end())
		{
			m_last_client_data[destination_ip].clear();
			m_last_client_data.erase(destination_ip);
		}
		if (m_client_running_map.find(destination_ip) != m_client_running_map.end())
		{
			m_client_running_map[destination_ip] = false;
			m_client_running_map.erase(destination_ip);
		}
		if (m_recevie_thread_map.find(destination_ip) != m_recevie_thread_map.end())
		{
			if (m_recevie_thread_map[destination_ip]->joinable())
				m_recevie_thread_map[destination_ip]->join();
			m_last_client_data.erase(destination_ip);
		}

		WSACloseEvent(m_event);
		m_event = WSA_INVALID_EVENT;
		send_size = -1;
	}
	return send_size;
}
void CINetTcpServer::TCPAcceptThread()
{
	SOCKADDR_IN accept_addr;
	int addr_len = sizeof(accept_addr);
	SOCKET sock_accept = INVALID_SOCKET;
	DWORD wsa_error = 0;
	while (m_accept_running)
	{

		wsa_error = WSAWaitForMultipleEvents(1, &m_event, FALSE, WAIT_CLIENT_LINK_TIME, FALSE);
		if (wsa_error == WSA_WAIT_TIMEOUT)
			continue;
		else if (wsa_error == WSA_WAIT_FAILED)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			break;
		}

		WSAResetEvent(m_event);

		ZeroMemory(&accept_addr, sizeof(SOCKADDR_IN));
		sock_accept = WSAAccept(m_socket, (struct sockaddr *)&accept_addr, &addr_len, NULL, 0);
		if (sock_accept == INVALID_SOCKET) continue;

		std::string temp_ip = inet_ntoa(accept_addr.sin_addr);
		int temp_port = ntohs(accept_addr.sin_port);
		//std::tuple<std::string, int> temp_inet_data = std::make_tuple(temp_ip,temp_port);
		{
			std::lock_guard<std::mutex> guard(g_mutex);
			m_client_umap[temp_ip] = sock_accept;
			if (m_recevie_thread_map.find(temp_ip) != m_recevie_thread_map.end())
			{
				m_client_running_map[temp_ip] = false;
				if (m_recevie_thread_map[temp_ip]->joinable())
					m_recevie_thread_map[temp_ip]->join();  //

				m_recevie_thread_map.erase(temp_ip);
			}
			m_client_running_map[temp_ip] = true;
			m_recevie_thread_map[temp_ip] = std::make_shared<std::thread>(std::bind(&CINetTcpServer::ReceiveDataThread, this, temp_ip));
	
			/*		if (m_recevie_thread_map[temp_ip]->joinable())
				m_recevie_thread_map[temp_ip]->detach();*/
		}

		Sleep(50);

	}
}

int CINetTcpServer::ReceiveData(char *str_data)
{
	int recive_size = 0;
	std::string str_map_key = "";
	try
	{
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

			recive_size = temp_str.length();
			memcpy(str_data, temp_str.c_str(), recive_size);

		}


	}
	catch (...)
	{
		std::lock_guard<std::mutex> guard(g_mutex);
		if (m_client_umap.find(str_map_key) != m_client_umap.end())
		{
			closesocket(m_client_umap[str_map_key]);
			m_client_umap[str_map_key] = INVALID_SOCKET;
			m_client_umap.erase(str_map_key);
		}
		if (m_last_client_data.find(str_map_key) != m_last_client_data.end())
		{
			m_last_client_data[str_map_key].clear();
			m_last_client_data.erase(str_map_key);
		}
		if (m_client_running_map.find(str_map_key) != m_client_running_map.end())
		{
			m_client_running_map[str_map_key] = false;
			m_client_running_map.erase(str_map_key);
		}
		if (m_recevie_thread_map.find(str_map_key) != m_recevie_thread_map.end())
		{
			if (m_recevie_thread_map[str_map_key]->joinable())
				m_recevie_thread_map[str_map_key]->join();
			m_last_client_data.erase(str_map_key);
		}

		WSACloseEvent(m_event);
		m_event = WSA_INVALID_EVENT;
		return -1;

		WSACloseEvent(m_event);
		m_event = WSA_INVALID_EVENT;
		return -1;
	}

	return recive_size;

}

int CINetTcpServer::ReceiveData(char *str_data, std::string destination_ip)
{
	int recive_size = 0;
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

			recive_size = temp_str.length();
			memcpy(str_data, temp_str.c_str(), recive_size);

		}

	}
	catch (...)
	{
		std::lock_guard<std::mutex> guard(g_mutex);
		if (m_client_umap.find(destination_ip) != m_client_umap.end())
		{
			closesocket(m_client_umap[destination_ip]);
			m_client_umap[destination_ip] = INVALID_SOCKET;
			m_client_umap.erase(destination_ip);
		}
		if (m_last_client_data.find(destination_ip) != m_last_client_data.end())
		{
			m_last_client_data[destination_ip].clear();
			m_last_client_data.erase(destination_ip);
		}
		if (m_client_running_map.find(destination_ip) != m_client_running_map.end())
		{
			m_client_running_map[destination_ip] = false;
			m_client_running_map.erase(destination_ip);
		}
		if (m_recevie_thread_map.find(destination_ip) != m_recevie_thread_map.end())
		{
			if(m_recevie_thread_map[destination_ip]->joinable())
				m_recevie_thread_map[destination_ip]->join();
			m_last_client_data.erase(destination_ip);
		}
			
		WSACloseEvent(m_event);
		m_event = WSA_INVALID_EVENT;
		return -1;
	}

	return recive_size;

}

void CINetTcpServer::ReceiveDataThread(
	std::string &client_ip/*
	WSAEVENT &wsa_event,
	std::unordered_map <std::string, SOCKET> *client_map,
	std::unordered_map <std::string, std::string> *client_data*/)
{
	SOCKET recevie_socket = INVALID_SOCKET;
	WSAEVENT temp_event = WSA_INVALID_EVENT;
	{
		std::lock_guard<std::mutex> guard(g_mutex);
		if (m_client_running_map.find(client_ip) == m_client_running_map.end())
			return;

		auto it_client_map = m_client_umap.find(client_ip);
		if (it_client_map == m_client_umap.end())
			return;
		recevie_socket = it_client_map->second;
		temp_event = WSACreateEvent();
		if (temp_event == WSA_INVALID_EVENT)
		{
			closesocket(recevie_socket);
			recevie_socket = INVALID_SOCKET;
			WSACloseEvent(temp_event);
			temp_event = WSA_INVALID_EVENT;
			m_client_umap.erase(client_ip);
			return;
		}
		if (WSAEventSelect(recevie_socket, temp_event, FD_READ)
			== SOCKET_ERROR)
		{
			closesocket(recevie_socket);
			recevie_socket = INVALID_SOCKET;
			WSACloseEvent(temp_event);
			temp_event = WSA_INVALID_EVENT;
			m_client_umap.erase(client_ip);
			return;
		}
	}

	int control_number = 0;
	while (m_client_running_map[client_ip])
	{
		int temp_recevie_size = 0;
		DWORD d_error = WSAWaitForMultipleEvents(1, &temp_event, FALSE, SERVER_RECEIVE_TIMEOUT, FALSE);
		if (d_error == WSA_WAIT_FAILED )
		{
			std::lock_guard<std::mutex> guard(g_mutex);
			closesocket(recevie_socket);
			WSACloseEvent(m_event);
			recevie_socket = INVALID_SOCKET;
			m_client_umap.erase(client_ip);
			m_last_client_data[client_ip].clear();
			m_last_client_data.erase(client_ip);
			m_event = WSA_INVALID_EVENT;
			break;
		}
		else if (d_error == WSA_WAIT_TIMEOUT)
		{
			if (control_number == 120)
			{
				std::lock_guard<std::mutex> guard(g_mutex);
				closesocket(recevie_socket);
				WSACloseEvent(m_event);
				recevie_socket = INVALID_SOCKET;
				m_client_umap.erase(client_ip);
				m_last_client_data[client_ip].clear();
				m_last_client_data.erase(client_ip);
				m_event = WSA_INVALID_EVENT;
				break;
			}
			++control_number;
			continue;
		}

		WSAResetEvent(temp_event);

		char str_data[INET_BUFF_LEN] = "\0";
		temp_recevie_size = recv(recevie_socket, (char*)str_data, INET_BUFF_LEN, 0);
			
		control_number = 0;
		{
			std::lock_guard<std::mutex> guard(g_mutex);
			m_last_client_data[client_ip].push_front(str_data);
		}

	}
	
}