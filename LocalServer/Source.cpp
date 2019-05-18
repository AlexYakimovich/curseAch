#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <vector>
#include <winsock.h>
#include <chrono>
#include <set>
#include <ctime>
#include <algorithm>
#include "../CurseAch/Message.h"
#include "../CurseAch/LocalCommunicationManager.h"

#pragma comment(lib,"ws2_32.lib")

long long makeMessage(MessageType type, long long proposalId, long long acceptedId, long long value)
{
  return type | (proposalId << 16) | (acceptedId << 32) | (value << 48);
}


#define MAX_CLIENTS 512
#define BROADCAST -1
#define ERROR INT_MIN
#define SERVER 1000
using namespace std;

long long success = 0;
long long expectedValue;
long long currentID = 0;
long long MasterSocket;
long long maxSocket;
vector <long long> sockets;
fd_set socketsSet, recievedSet;
long long delay = 10;
double percentage = 1;

set<Message> msgQ;

//<DIV>

Message recieveValue()
{
	Message recievedValue;
	recievedSet = socketsSet;
	select(maxSocket + 1, &recievedSet, NULL, NULL, NULL);

	for (long long ActiveSocket = 0; ActiveSocket <= maxSocket; ActiveSocket++)
	{
		if (FD_ISSET(ActiveSocket, &recievedSet))
		{
			if (ActiveSocket != MasterSocket)
			{
				size_t msg_size = recv(ActiveSocket, (char *)&recievedValue, sizeof(recievedValue), 0);
				if (msg_size == -1)
				  FD_CLR(ActiveSocket, &socketsSet);
				return recievedValue;
			}
			else
			{
				ActiveSocket = accept(MasterSocket, 0, 0);
				//cout << "Client #" << sockets.size() << " connected on " << ActiveSocket << endl;
				sockets.push_back(ActiveSocket);
				FD_SET(ActiveSocket, &socketsSet);
				if (ActiveSocket > maxSocket)
					maxSocket = ActiveSocket;
				return recievedValue;
			}
		}
	}

	return recievedValue;
}

HANDLE addNewProcess()
{
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	char * cmd = new char[80];
	wsprintf(cmd, "CurseAch.exe %d %d", currentID);
	currentID++;
	if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &info, &processInfo))
		cout << "Error creating process." << endl;
	/*char buff[512];
	cout << "Hey " << pipe.first;
	DWORD cbread;
	ReadFile(pipe.first, buff, 5, &cbread, NULL);
	cout << "Recieved:" << buff;*/
	return processInfo.hProcess;
}

void sendMsg(Message msg)
{
  if (msg.recieverID == msg.senderID)
	return;
  //cout << "Sending message to client #" << msg.recieverID << endl;
  send(sockets[msg.recieverID], (char *)&msg, sizeof(Message), 0);
}

DWORD WINAPI sender(LPVOID args)
{

	chrono::time_point<chrono::system_clock> currentTime;
	while (TRUE)
	{
		currentTime = chrono::system_clock::now();
		if (msgQ.size() != 0 && (*msgQ.begin()).recieveTime <= currentTime)
		{
			Message current = *msgQ.begin();
			sendMsg(current);
			msgQ.erase(current);
		}
		Sleep(1);
	}
	
}

DWORD WINAPI reciever(LPVOID args)
{
	while (true)
	{
		auto value = recieveValue();
		auto recieveTime = chrono::system_clock::now();
		//cout << "R";
		if (value.recieverID == value.senderID)
			continue;
		//cout << "ecieved message from client #" << value.senderID << " to #" << value.recieverID << " value: " << value.value << endl;
		if (value.recieverID == SERVER)
		{
		  if (expectedValue == value.value)
		  {
			success++;
			cout << "Success" << endl;
		  }
		  else
		  {
			cout << "Failure " << value.value << " / " << expectedValue << endl;
		  }
		  expectedValue = -1;
		  continue;
		}
		if (value.senderID == ERROR)
			return ERROR;
		Message sendingValue;
		if (value.recieverID == BROADCAST)
			for (auto i = 0; i < sockets.size(); i++)
			{
				if (double(rand()) / RAND_MAX < percentage)
				{
					recieveTime = chrono::system_clock::now();
					sendingValue = Message(value.senderID, i, value.value);
					sendingValue.recieveTime = recieveTime + chrono::milliseconds(delay);
					msgQ.insert(sendingValue);
				}
			}
		else
		{
			sendingValue = Message(value.senderID, value.recieverID, value.value);
			sendingValue.recieveTime = recieveTime + chrono::milliseconds(delay);
			msgQ.insert(sendingValue);
		}
	}
	return 0;
}

HANDLE createRecievingThread()
{
	DWORD threadID;
	HANDLE hThread = CreateThread(NULL, 
		0,						
		reciever,				
		NULL,		
		0,					
		&threadID		
	);
	return hThread;
}

HANDLE createSendingThread()
{
	DWORD threadID;
	HANDLE hThread = CreateThread(NULL,
		0,
		sender,
		NULL,
		0,
		&threadID
	);
	return hThread;
}



int main()
{
	srand(time(0));
	WSADATA wsa;
	if (WSAStartup(1, &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	maxSocket = MasterSocket;
	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(1337);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));

	listen(MasterSocket, SOMAXCONN);

	FD_ZERO(&socketsSet);
	FD_ZERO(&recievedSet);
	FD_SET(MasterSocket, &socketsSet);

	HANDLE hRecvThread = createRecievingThread();
	HANDLE hSendThread = createSendingThread();

	int machinesCount, inputCount, input;
	Message sendingValue;

	ifstream in("tests.txt");
	for (int i = 0; i < 100; i++)
	{
	  currentID = 0;
	  sockets.clear();
	  msgQ.clear();
	  vector<HANDLE> hProc;
	  in >> machinesCount;
	  in >> inputCount;
	  for(int g = 0; g < machinesCount; g++)
		hProc.push_back(addNewProcess());
	  Sleep(500);
	  for (int value = 1; value <= inputCount; value++)
	  {
		in >> input;
		sendingValue = Message(1000, input, makeMessage(ServerNewValue, 0,0,value));
		sendingValue.recieveTime = chrono::system_clock::now();
		sendMsg(sendingValue);
		Sleep(300);
	  }
	  expectedValue = inputCount;
	  sendingValue = Message(1000, input, makeMessage(ServerRequestValue, 0, 0, 0));
	  sendingValue.recieveTime = chrono::system_clock::now();
	  sendMsg(sendingValue);
	  while (expectedValue != -1)
		Sleep(100);
	  for (auto it = hProc.begin(); it != hProc.end(); it++)
		TerminateProcess(*it, 2);
	  hProc.clear();
	}
	WaitForSingleObject(hRecvThread, INFINITE);
	system("pause");
}
