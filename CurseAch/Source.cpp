#include "NetworkCommunicationManager.h"
#include "LocalCommunicationManager.h"
#include <set>
#include <Windows.h>
#define TIMEOUT 100
using namespace std;
enum NodeState { proposer, acceptor};

typedef struct {
  MessageType type;
  long long proposalId;
  long long acceptedId;
  long long value;
} MessageValue;

long long nextProposalNumber = 0;
long long currentValue;
long long nodesCount;
NodeState currentNodeState = acceptor;

CommunicationManager * manager;

void changeValue(long long newValue)
{
  if (newValue > currentValue)
  {
	currentValue = newValue;
	cout << "Current value: " << currentValue << endl;
  }
}

MessageValue parseMessage(long long msg)
{
  MessageValue val;
  val.type = (MessageType)((msg) & 0xff);
  val.proposalId = (msg >> 16) & 0xff;
  val.acceptedId = (msg >> 32) & 0xff;
  val.value = msg >> 48;
  return val;
}

DWORD WINAPI marker(LPVOID args)
{
  bool * execute = (bool *)args;
  Message recievedMessage;
  MessageValue recievedValue;
  long long votesCount;

  long long lastAcceptedId = 0;
  long long promicesRecieved = 0;
  long long proposalId = 0;

  long long acceptedId = 0;
  long long promisedId = 0;

  chrono::time_point<chrono::system_clock> lastRecieve = chrono::system_clock::now();
  while (*execute)
  {
	switch (currentNodeState)
	{
	case proposer:
	  cout << "proposer" << endl;
	  promicesRecieved = 1;
	  proposalId = nextProposalNumber;
	  nextProposalNumber++;
	  manager->broadcast(LocalCommunicationManager::makeMessage(Prepare, proposalId, proposalId, currentValue));

	  do
	  {
		recievedMessage = manager->recieveValue(TIMEOUT);
		recievedValue = parseMessage(recievedMessage.value);
		if (recievedValue.type == Promise)
		{
		  if (recievedValue.proposalId != proposalId)
			continue;
		  if (recievedValue.acceptedId > lastAcceptedId)
		  {
			lastAcceptedId = recievedValue.acceptedId;
			changeValue(recievedValue.value);
		  }
		  promicesRecieved++;
		}
		else if (recievedValue.type == Error)
		{
		  //cout << "Timeout" << endl;
		  break;
		}
	  } while (promicesRecieved * 2 <= nodesCount);
	  //cout << "Total promices: " << promicesRecieved << endl;
	  if (promicesRecieved * 2 > nodesCount)
	  {
		//cout << "Sending accept " << proposalId << " " << proposalId << " " << currentValue << endl;
		//for(int i = 0; i < 3; )
		manager->broadcast(LocalCommunicationManager::makeMessage(Accept, proposalId, proposalId, currentValue));
		manager->broadcast(LocalCommunicationManager::makeMessage(Accept, proposalId, proposalId, currentValue));
		manager->broadcast(LocalCommunicationManager::makeMessage(Accept, proposalId, proposalId, currentValue));
	  }
	  currentNodeState = acceptor;
	  cout << "acceptor" << endl;
	  break;
	case acceptor:
	  recievedMessage = manager->recieveValue(TIMEOUT / 10);
	  recievedValue = parseMessage(recievedMessage.value);
	  if (recievedValue.type == ServerNewValue)
	  {
		changeValue(currentValue + 1);
		currentNodeState = proposer;
		break;
	  }
	  if (recievedValue.type == ServerRequestValue)
	  {
		manager->sendValue(currentValue, recievedMessage.senderID);
		break;
	  }
	  if (recievedValue.type == Prepare)
	  {
		//cout << "Recv prepare" << endl;
		if (recievedValue.proposalId == promisedId)
		{
		  //cout << "Sending promise " << recievedValue.proposalId << " " << acceptedId << " " << currentValue << endl;
		  manager->sendValue(LocalCommunicationManager::makeMessage(Promise, recievedValue.proposalId, acceptedId, currentValue), recievedMessage.senderID);
		}
		else if (recievedValue.proposalId > promisedId)
		{
		  promisedId = recievedValue.proposalId;
		  //cout << "Sending promise " << recievedValue.proposalId << " " << acceptedId << " " << currentValue << endl;
		  manager->sendValue(LocalCommunicationManager::makeMessage(Promise, recievedValue.proposalId, acceptedId, currentValue), recievedMessage.senderID);
		}
	  }
	  else if(recievedValue.type == Accept)
	  {
		//cout << "Recv accept" << endl;
		if (recievedValue.proposalId >= promisedId)
		{
		  promisedId = recievedValue.proposalId;
		  nextProposalNumber = promisedId + 1;
		  acceptedId = recievedValue.proposalId;
		  changeValue(recievedValue.value);
		}
	  }
	  /*votesCount = 1;
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
	  }*/
	  break;
	default:
	  break;
	}
  }
  return 0;
}

int main(long long argc, char ** argv) {
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
	long long id = atoi(argv[1]);
	nodesCount = id + 1;
	manager = (CommunicationManager *)(new LocalCommunicationManager(id));
	cout << "Local manager #" << id << " created" << endl;
  }
  if (manager->getNetworkEnabled())
	cout << "Connection succesfully created" << endl;
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
		currentNodeState = proposer;
	  }
	}
	else
	  break;
  }
  
  TerminateThread(threadHandle, exitCode);
  delete manager;
  return 0;
}