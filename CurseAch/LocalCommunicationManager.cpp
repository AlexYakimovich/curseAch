#include "LocalCommunicationManager.h"

using namespace std;

long long LocalCommunicationManager::makeMessage(MessageType type, long long proposalId, long long acceptedId, long long value)
{
  return type | (proposalId << 16) | (acceptedId << 32) | (value << 48);
}

long long LocalCommunicationManager::getID()
{
  return id;
}

long long LocalCommunicationManager::broadcast(long long value)
{
  Message message(id, -1, value);
  send(ClientSocket, (char *)&message, sizeof(message), 0);
  return SUCCESS;
}

long long LocalCommunicationManager::sendValue(long long value, long long to)
{
  Message message(id, to, value);
  send(ClientSocket, (char *)&message, sizeof(message), 0);
  return SUCCESS;
}

Message LocalCommunicationManager::recieveValue(long long timeout)
{
	Message message;
	fd_set set;
	struct timeval tv;
	FD_ZERO(&set);
	FD_SET(ClientSocket, &set);
	tv.tv_usec = (timeout % 1000)*1000;
	tv.tv_sec = timeout / 1000;
	long long rv = select(ClientSocket, &set, NULL, NULL, &tv);
	if (rv == 0) {
	  return Message(0, 0, makeMessage(Error, 0, 0, TIMEOUT_REACHED));
	}
	recv(ClientSocket, (char *)&message, sizeof(message), 0);
	return message;
}

LocalCommunicationManager::~LocalCommunicationManager()
{
  closesocket(ClientSocket);
}


LocalCommunicationManager::LocalCommunicationManager(long long id)
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