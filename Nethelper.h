#ifndef NET_HELPER_H
#define NET_HELPER_H

class Nethelper
{
	
public:	
	//
	bool Init();

	//
	int Sendmsg();

	//
	int Recvmsg();

	//
	int Checkstat();

	//
	int CloseNet();

private:
	SOCKET m_socket;

}

#endif
