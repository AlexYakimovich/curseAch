#pragma once
#include <vector>
#include <string>
#include <Windows.h>
#include <conio.h>
#include <iostream>
#include "Message.h"
#include "CommunicationManager.h"

class NetworkCommunicationManager : public CommunicationManager
{
private:
  HANDLE  hNamedPipe;
  std::vector<std::string> machineNames;
  const int id = 0;
public:
  int broadcast(int value);
  int sendValue(int value, int to);
  Message recieveValue(int timeout);
  NetworkCommunicationManager();
  ~NetworkCommunicationManager();
};

