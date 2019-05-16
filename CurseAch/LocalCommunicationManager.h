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
  long long id;
  long long ClientSocket;
public:
  static long long makeMessage(MessageType type, long long proposalId, long long acceptedId, long long value);
  long long getID();
  long long broadcast(long long value);
  long long sendValue(long long value, long long to);
  Message recieveValue(long long timeout);
  LocalCommunicationManager(long long id);
  ~LocalCommunicationManager();
};

