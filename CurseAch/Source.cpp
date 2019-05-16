#include "NetworkCommunicationManager.h"
#include "LocalCommunicationManager.h"
#include <set>
#include <Windows.h>
#define APPEND_TIMEOUT 100
#define ELECTION_TIMEOUT_MIN 350
#define ELECTION_TIMEOUT_MAX 700
using namespace std;
enum NodeState { leader, candidate, follower };

typedef struct {
  int term;
  MessageType type;
  int value;
} MessageValue;

set<int> ids;
int currentLeader;
int currentValue;
int currentTerm = 0;
int nodesCount;
bool voted = false;
NodeState currentNodeState = follower;

CommunicationManager * manager;

void changeValue(int newValue)
{
  if (newValue > currentValue)
  {
	currentValue = newValue;
	cout << "Current value: " << currentValue << endl;
  }
}

MessageValue parseMessage(int msg)
{
  MessageValue val;
  val.term = msg & 0xf;
  val.type = (MessageType)((msg >> 8) & 0xf);
  val.value = (msg >> 16);
  return val;
}

DWORD WINAPI marker(LPVOID args)
{
  bool * execute = (bool *)args;
  Message recievedMessage;
  MessageValue recievedValue;
  int votesCount;
  int timeout;
  chrono::time_point<chrono::system_clock> lastRecieve = chrono::system_clock::now();
  while (*execute)
  {
	timeout = ELECTION_TIMEOUT_MIN + ((ELECTION_TIMEOUT_MAX - ELECTION_TIMEOUT_MIN)*rand()) / RAND_MAX;
	switch (currentNodeState)
	{
	case leader:
	  manager->broadcast(LocalCommunicationManager::makeMessage(currentTerm, AppendEntry, currentValue));
	  recievedMessage = manager->recieveValue(APPEND_TIMEOUT);
	  recievedValue = parseMessage(recievedMessage.value);
	  if (recievedValue.term > currentTerm)
	  {
		currentTerm = recievedValue.term;
		changeValue(recievedValue.value);
		currentNodeState = follower;
		break;
	  }
	  if (recievedValue.type == NewValue)
	  {
		changeValue(recievedValue.value);
		manager->broadcast(LocalCommunicationManager::makeMessage(currentTerm, NewValue, currentValue));
	  }
	  break;
	case candidate:
	  votesCount = 1;
	  voted = true;
	  lastRecieve = chrono::system_clock::now();
	  while (votesCount < nodesCount / 2 + 1)
	  {
		recievedMessage = manager->recieveValue(timeout);
		recievedValue = parseMessage(recievedMessage.value);
		if (recievedValue.type == Error && recievedValue.value == TIMEOUT_REACHED)
		{
		  currentTerm++;
		  manager->broadcast(LocalCommunicationManager::makeMessage(currentTerm, RequestVote, currentValue));
		  break;
		}
		if (recievedValue.term < currentTerm)
		  break;
		else
		  currentTerm = recievedValue.term;
		if (recievedValue.type == Vote)
		  votesCount++;
		else
		{
		  currentNodeState = follower;
		  break;
		}
	  }
	  if (votesCount >= nodesCount / 2 + 1)
	  {
		cout << "lead" << endl;
		currentNodeState = leader;
	  }
	  break;
	case follower:
	  recievedMessage = manager->recieveValue(timeout);
	  recievedValue = parseMessage(recievedMessage.value);
	  if (recievedValue.type == Error && recievedValue.value == TIMEOUT_REACHED)
	  {
		cout << "candidate" << endl;
		currentNodeState = candidate;
		currentTerm++;
		manager->broadcast(LocalCommunicationManager::makeMessage(currentTerm, RequestVote, currentValue));
		break;
	  }
	  if (recievedValue.term < currentTerm)
		break;
	  else
		currentTerm = recievedValue.term;
	  if (recievedValue.type == RequestVote) {
		manager->sendValue(LocalCommunicationManager::makeMessage(currentTerm, Vote, currentValue), recievedMessage.senderID);
		voted = true;
		break;
	  }
	  if (recievedValue.type == AppendEntry)
		currentLeader = recievedMessage.senderID;
	  voted = false;
	  changeValue(recievedValue.value);
	  break;
	default:
	  break;
	}
  }
  return 0;
}

int main(int argc, char ** argv) {
  srand(time(0));
  HWND console = GetConsoleWindow();
  RECT r;
  GetWindowRect(console, &r); //stores the console's current dimensions

  //MoveWindow(window_handle, x, y, width, height, redraw_window);
  MoveWindow(console, r.left, r.top, 400, 200, TRUE);
  bool execute = true;
  DWORD threadID, exitCode = 0;
  HANDLE threadHandle;
  if (argc < 2)
  {
	manager = (CommunicationManager *)(new NetworkCommunicationManager());
	cout << "Network manager created" << endl;
  }
  else
  {
	int id = atoi(argv[1]);
	nodesCount = id + 1;
	manager = (CommunicationManager *)(new LocalCommunicationManager(id));
	cout << "Local manager #" << id << " created" << endl;
  }
  if (manager->getNetworkEnabled())
	cout << "Connection succesfully created" << endl;
  cout << "follow" << endl;
  currentValue = 0;
  cout << "Current value = " << currentValue << endl;
  threadHandle = CreateThread(NULL,
	0,
	marker,
	&execute,
	0,
	&threadID
  );

  char command;
  while (true)
  {
	command = _getch();
	if (command == 0)
	  continue;
	if (command != 13)
	{
	  if (manager->getNetworkEnabled())
	  {
		changeValue(currentValue + 1);
		if (currentNodeState == leader)
		  manager->broadcast(LocalCommunicationManager::makeMessage(currentTerm, NewValue, currentValue));
		else
		  manager->sendValue(LocalCommunicationManager::makeMessage(currentTerm, NewValue, currentValue), currentLeader);
	  }
	}
	else
	  break;
  }
  
  TerminateThread(threadHandle, exitCode);
  delete manager;
  return 0;
}