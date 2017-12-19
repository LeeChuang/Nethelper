#include "INetUdp.h"


CINetUdp::CINetUdp()
{

}

CINetUdp::~CINetUdp()
{
	Shutdown();
}

int CINetUdp::SendData(char *str_data)
{
	int send_size = 0;
	SOCKET temp_socket = INVALID_SOCKET;
	try
	{
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