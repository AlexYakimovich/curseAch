#include <iostream>
#include <Windows.h>
#include <string>
#include <conio.h>
#include <vector>

using namespace std;

vector <HANDLE> clientPipes;
HANDLE serverPipe;
int currentID = 0;

int recieveValue()
{
	if (!ConnectNamedPipe(serverPipe,   // дескриптор канала    
		(LPOVERLAPPED)NULL // св€зь синхронна€   
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return -1;
	}
	int recievedValue;
	DWORD dwBytesRead;
	if (!ReadFile(serverPipe,   // дескриптор канала    
		&recievedValue,   // адрес буфера дл€ ввода данных    
		sizeof(recievedValue),  // число читаемых байтов    
		&dwBytesRead,   // число прочитанных байтов    
		(LPOVERLAPPED)NULL // передача данных синхронна€   
	))
	{
		cerr << "Data reading from the named pipe failed." << endl << "The last error code: " << GetLastError() << endl;
	}
	if (!DisconnectNamedPipe(serverPipe)) {
		cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		return -1;
	}
	return recievedValue;
}

HANDLE createPipe(char * name)
{
	SECURITY_ATTRIBUTES sa;  // атрибуты защиты  
	SECURITY_DESCRIPTOR sd;  // дескриптор защиты  
	// инициализаци€ атрибутов защиты  
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE; // дескриптор канала ненаследуемый   // инициализируем дескриптор защиты  
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// устанавливаем атрибуты защиты, разреша€ доступ всем пользовател€м  
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &sd;   // создаем именованный канал дл€ чтени€  
	HANDLE hNamedPipe = CreateNamedPipe(name,   // им€ канала  
		PIPE_ACCESS_DUPLEX,  // читаем из канала и пишем в канал   
		PIPE_TYPE_MESSAGE | PIPE_WAIT, // синхронна€ передача сообщений   
		1,  // максимальное количество экземпл€ров канала    
		0,  // размер выходного буфера по умолчанию   
		0,  // размер входного буфера по умолчанию   
		INFINITE, // клиент ждет св€зь 500 мс   
		&sa  // доступ дл€ всех пользователей  
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

int main()
{
	char serverPipeName[] = "\\\\.\\pipe\\serverPipe";
	serverPipe = createPipe(serverPipeName);
	cin.get();
	addNewProcess();
	cout << recieveValue();
	system("pause");
}
