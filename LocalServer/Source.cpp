#include <iostream>
#include <string>
#include <conio.h>
#include <vector>
#include <winsock.h>
#include <chrono>
#include <set>
#include <ctime>
#include <algorithm>
#include "../CurseAch/Message.h"

#pragma comment(lib,"ws2_32.lib")

#define MAX_CLIENTS 512
#define BROADCAST -1
#define ERROR INT_MIN
using namespace std;

int currentID = 0;
int MasterSocket;
int maxSocket;
vector <int> sockets;
fd_set socketsSet, recievedSet;
int delay = 10;
double percentage = 1;

set<Message> msgQ;

//<DIV>

Message recieveValue()
{
	Message recievedValue;
	recievedSet = socketsSet;
	select(maxSocket + 1, &recievedSet, NULL, NULL, NULL);

	for (int ActiveSocket = 0; ActiveSocket <= maxSocket; ActiveSocket++)
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
				cout << "Client #" << sockets.size() << " connected on " << ActiveSocket << endl;
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

void addNewProcess()
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
}

void sendMsg(Message msg)
{
  if (msg.recieverID == msg.senderID)
	return;
  cout << "Sending message to client #" << msg.recieverID << endl;
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
		cout << "R";
		if (value.recieverID == value.senderID)
			continue;
		cout << "ecieved message from client #" << value.senderID << " to #" << value.recieverID << " value: " << value.value << endl;
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

	while (true)
	{
		auto sym = _getch();
		if(sym == ' ')
			addNewProcess();
	}
	WaitForSingleObject(hRecvThread, INFINITE);
	system("pause");
}
