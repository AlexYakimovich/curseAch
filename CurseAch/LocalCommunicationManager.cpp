#include "LocalCommunicationManager.h"

using namespace std;

int LocalCommunicationManager::broadcast(int value)
{
  Message message(id, -1, value);
  send(ClientSocket, (char *)&message, sizeof(message), 0);
  return SUCCESS;
}

int LocalCommunicationManager::recieveValue(int timeout)
{
	Message message;
	fd_set set;
	struct timeval tv;
	FD_ZERO(&set);
	FD_SET(ClientSocket, &set);
	tv.tv_usec = timeout % 1000;
	tv.tv_sec = timeout / 1000;
	int rv = select(ClientSocket, &set, NULL, NULL, &tv);
	if(rv == 0)
		return TIMEOUT_REACHED;
	recv(ClientSocket, (char *)&message, sizeof(message), 0);
	return message.value;
}

LocalCommunicationManager::~LocalCommunicationManager()
{
}


LocalCommunicationManager::LocalCommunicationManager(int id)
{
	srand(time(0));
	this->id = id;
	WSADATA wsa;
	if (WSAStartup(1, &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return;
	}

	ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(1337);
	SockAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	connect(ClientSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));
}