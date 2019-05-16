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
  const long long id = 0;
public:
  long long broadcast(long long value);
  long long sendValue(long long value, long long to);
  Message recieveValue(long long timeout);
  NetworkCommunicationManager();
  ~NetworkCommunicationManager();
};

