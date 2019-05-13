#pragma once
#include <vector>
#include <string>
#include <conio.h>
#include <iostream>
#include <winsock.h>
#include <ctime>
#include "Message.h"
#include "CommunicationManager.h"

#pragma comment(lib,"ws2_32.lib")

class LocalCommunicationManager :
	public CommunicationManager
{
private:
	int id;
	int ClientSocket;
public:
	int broadcast(int value);
	int recieveValue(int timeout);
	LocalCommunicationManager(int id);
	~LocalCommunicationManager();
};

