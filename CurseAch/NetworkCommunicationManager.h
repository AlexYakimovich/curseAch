#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <conio.h>
#include <iostream>
#include "CommunicationManager.h"


#define NEW_MACHINE_ADDED -1
#define NETWORK_ENABLED -2
#define NETWORK_DISABLED -3
#define NETWORK_ERROR -4

class NetworkCommunicationManager : CommunicationManager 
{
private:
	HANDLE  hNamedPipe;
	std::vector<std::string> machineNames;
	const int id = 0;
public:
	int broadcast(int value);
	int recieveValue();
	NetworkCommunicationManager();
	~NetworkCommunicationManager();
};

