#include "INetTcpServer.h"
#include <thread>
std::mutex g_mutex;
CINetTcpServer::CINetTcpServer()
{
	m_max_client_number = DEFAULT_LINK_NUMBER;
}


CINetTcpServer::~CINetTcpServer()
{
	Shutdown();
}

bool CINetTcpServer::Connect()
{
	try
	{
		if ((m_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
			return false;

		SOCKADDR_IN local_address;
		local_address.sin_family = AF_INET;
		local_address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		local_address.sin_port = htons(m_port);
		memset(local_address.sin_zero, 0, sizeof(local_address.sin_zero));

		if (bind(m_socket, (struct sockaddr*)&local_address, sizeof(SOCKADDR_IN))
			== SOCKET_ERROR)
		{
			int ierror = GetLastError();

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

		std::thread temp_thread(TCPAcceptThread, m_event, m_socket, &m_client_umap);

		if (temp_thread.joinable())
			temp_thread.detach();

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
		return -1;
	}
	return send_size;
}

void CINetTcpServer::TCPAcceptThread(WSAEVENT &wsa_event, SOCKET &server_socket,
	std::unordered_map <std::string, SOCKET> *client_map)
{
	SOCKADDR_IN accept_addr;
	int addr_len = sizeof(accept_addr);
	SOCKET sock_accept = INVALID_SOCKET;
	DWORD wsa_error = 0;
	while (true)
	{

		wsa_error = WSAWaitForMultipleEvents(1, &wsa_event, FALSE, WAIT_CLIENT_LINK_TIME, FALSE);
		if (wsa_error == WSA_WAIT_TIMEOUT)
			continue;
		else if (wsa_error == WSA_WAIT_FAILED)
		{
			closesocket(server_socket);
			server_socket = INVALID_SOCKET;
			break;
		}

		WSAResetEvent(wsa_event);

		ZeroMemory(&accept_addr, sizeof(SOCKADDR_IN));
		sock_accept = WSAAccept(server_socket, (struct sockaddr *)&accept_addr, &addr_len, NULL, 0);
		if (sock_accept == INVALID_SOCKET) continue;

		std::string temp_ip = inet_ntoa(*((struct  in_addr*)(&accept_addr.sin_addr)));

		{
			std::lock_guard<std::mutex> guard(g_mutex);
			(*client_map)[temp_ip] = sock_accept;
		}

		Sleep(50);

	}
}
