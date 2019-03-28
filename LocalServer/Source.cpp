#include <iostream>
#include <Windows.h>
#include <string>
#include <conio.h>
#include <vector>
#include "../CurseAch/Message.h"
#define BROADCAST -1
#define ERROR INT_MIN
using namespace std;

vector<HANDLE> clientPipes;
HANDLE serverPipe;
int currentID = 0;

Message recieveValue()
{
	if (!ConnectNamedPipe(serverPipe,  
		(LPOVERLAPPED)NULL  
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return Message(ERROR, ERROR, ERROR);
	}
	Message recievedValue;
	DWORD dwBytesRead;
	if (!ReadFile(serverPipe,  
		&recievedValue,      
		sizeof(recievedValue),     
		&dwBytesRead,   
		(LPOVERLAPPED)NULL 
	))
	{
		cerr << "Data reading from the named pipe failed." << endl << "The last error code: " << GetLastError() << endl;
	}
	if (!DisconnectNamedPipe(serverPipe)) {
		cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		return Message(ERROR, ERROR, ERROR);
	}
	return recievedValue;
}

HANDLE createPipe(char * name)
{
	SECURITY_ATTRIBUTES sa; 
	SECURITY_DESCRIPTOR sd;
	 
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;  
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	 
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &sd;  
	HANDLE hNamedPipe = CreateNamedPipe(name,  
		PIPE_ACCESS_DUPLEX,  
		PIPE_TYPE_MESSAGE | PIPE_WAIT,   
		1,   
		0,  
		0,  
		INFINITE, 
		&sa  
	);
	if (hNamedPipe == INVALID_HANDLE_VALUE)
		cout << "Error creating pipe";
	return hNamedPipe;
}

void addNewProcess()
{
	STARTUPINFO info = { sizeof(info) };
	PROCESS_INFORMATION processInfo;
	char * clientPipeName = new char[80];
	char * cmd = new char[80];

	wsprintf(clientPipeName, "\\\\.\\pipe\\clientPipe#%d", currentID);
	HANDLE hClientPipe = createPipe(clientPipeName);
	clientPipes.push_back(hClientPipe);
	wsprintf(cmd, "CurseAch.exe %d", currentID);
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
	if (!ConnectNamedPipe(clientPipes[msg.recieverID],   
		(LPOVERLAPPED)NULL   
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return;
	}
	int recievedValue;
	DWORD dwBytesWritten;
	if (!WriteFile(
		clientPipes[msg.recieverID],  
		&msg, 
		sizeof(msg), 
		&dwBytesWritten, 
		(LPOVERLAPPED)NULL 
	)) {
		return;
	}
	if (!DisconnectNamedPipe(clientPipes[msg.recieverID])) {
		cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		return;
	}
}


DWORD WINAPI reciever(LPVOID args)
{
	while (true)
	{
		auto value = recieveValue();
		if (value.senderID == ERROR)
			return ERROR;
		if (value.recieverID == BROADCAST)
			for (int i = 0; i < currentID; i++)
			{
				value.recieverID = i;
				sendMsg(value);
			} 
		else
			sendMsg(value);
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

int main()
{
	char serverPipeName[] = "\\\\.\\pipe\\serverPipe";
	serverPipe = createPipe(serverPipeName);
  _getch();
	addNewProcess();
	HANDLE hThread = createRecievingThread();
	WaitForSingleObject(hThread, INFINITE);
	system("pause");
}
