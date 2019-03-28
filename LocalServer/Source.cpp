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
	if (!ConnectNamedPipe(serverPipe,   // дескриптор канала    
		(LPOVERLAPPED)NULL // связь синхронная   
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return Message(ERROR, ERROR, ERROR);
	}
	Message recievedValue;
	DWORD dwBytesRead;
	if (!ReadFile(serverPipe,   // дескриптор канала    
		&recievedValue,   // адрес буфера для ввода данных    
		sizeof(recievedValue),  // число читаемых байтов    
		&dwBytesRead,   // число прочитанных байтов    
		(LPOVERLAPPED)NULL // передача данных синхронная   
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
	SECURITY_ATTRIBUTES sa;  // атрибуты защиты  
	SECURITY_DESCRIPTOR sd;  // дескриптор защиты  
	// инициализация атрибутов защиты  
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE; // дескриптор канала ненаследуемый   // инициализируем дескриптор защиты  
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// устанавливаем атрибуты защиты, разрешая доступ всем пользователям  
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &sd;   // создаем именованный канал для чтения  
	HANDLE hNamedPipe = CreateNamedPipe(name,   // имя канала  
		PIPE_ACCESS_DUPLEX,  // читаем из канала и пишем в канал   
		PIPE_TYPE_MESSAGE | PIPE_WAIT, // синхронная передача сообщений   
		1,  // максимальное количество экземпляров канала    
		0,  // размер выходного буфера по умолчанию   
		0,  // размер входного буфера по умолчанию   
		INFINITE, // клиент ждет связь 500 мс   
		&sa  // доступ для всех пользователей  
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
	if (!ConnectNamedPipe(clientPipes[msg.recieverID],   // дескриптор канала    
		(LPOVERLAPPED)NULL // связь синхронная   
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return;
	}
	int recievedValue;
	DWORD dwBytesWritten;
	if (!WriteFile(
		clientPipes[msg.recieverID],  // дескриптор канала
		&msg,  // данные 
		sizeof(msg), // размер данных 
		&dwBytesWritten, // количество записанных байтов
		(LPOVERLAPPED)NULL // синхронная запись 
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
	HANDLE hThread = CreateThread(NULL, // атрибуты защиты 
		0,							// размер стека потока в байтах 
		reciever,						// адрес исполняемой функции 
		NULL,		// адрес параметра 
		0,							// флаги создания потока
		&threadID				// идентификатор потока 
	);
	return hThread;
}

int main()
{
	char serverPipeName[] = "\\\\.\\pipe\\serverPipe";
	serverPipe = createPipe(serverPipeName);
	cin.get();
	addNewProcess();
	HANDLE hThread = createRecievingThread();
	WaitForSingleObject(hThread, INFINITE);
	system("pause");
}
