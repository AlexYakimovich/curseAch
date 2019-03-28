#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <conio.h>
#include <iostream>
#include "CommunicationManager.h"

class LocalCommunicationManager :
	public CommunicationManager
{
private:
	int id;
public:
	int broadcast(int value);
	int recieveValue();
	LocalCommunicationManager(int id);
	~LocalCommunicationManager();
};

