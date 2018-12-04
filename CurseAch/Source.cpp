#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
using namespace std;

#define NEW_MACHINE_ADDED -1
#define NETWORK_ENABLED -2
#define NETWORK_DISABLED -3

int currentValue;
bool networkEnabled = true;
HANDLE  hNamedPipe;
vector <string> machineNames;
string currentMachineName = "DESKTOP-CO3NPLJ";

int broadcast(int value)
{
	char * pipeName = new char[80];
	HANDLE hConnectedPipe;
	DWORD dwBytesWritten;
	int sendingCount = 0;
	for (int machineNumber = 0; machineNumber < machineNames.size(); machineNumber++)
	{
		if (machineNames[machineNumber] == currentMachineName)
			continue;
		wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineNames[machineNumber].c_str());     // связываемся с именованным каналом
		hConnectedPipe = CreateFile(pipeName,    // имя канала  
			GENERIC_READ | GENERIC_WRITE,  // читаем и записываем в канал 
			FILE_SHARE_READ | FILE_SHARE_WRITE, // разрешаем чтение и запись в канал 
			(LPSECURITY_ATTRIBUTES)NULL,  // защита по умолчанию
			OPEN_EXISTING,   // открываем существующий канал
			FILE_ATTRIBUTE_NORMAL,   // атрибуты по умолчанию  
			(HANDLE)NULL    // дополнительных атрибутов нет
		);
		// проверяем связь с каналом  
		if (hConnectedPipe == INVALID_HANDLE_VALUE)
			continue;
		if (!WriteFile(
			hConnectedPipe,  // дескриптор канала
			&value,  // данные 
			sizeof(value), // размер данных 
			&dwBytesWritten, // количество записанных байтов
			(LPOVERLAPPED)NULL // синхронная запись 
		)) {
			continue;
		}
		sendingCount++;
	}
	return sendingCount;
}

int recieveValue(HANDLE hNamedPipe)
{
	int recievedValue;
	DWORD dwBytesRead;
	if (!ReadFile(hNamedPipe,   // дескриптор канала    
		&recievedValue,   // адрес буфера для ввода данных    
		sizeof(recievedValue),  // число читаемых байтов    
		&dwBytesRead,   // число прочитанных байтов    
		(LPOVERLAPPED)NULL // передача данных синхронная   
	))
	{
		cerr << "Data reading from the named pipe failed." << endl << "The last error code: " << GetLastError() << endl;
	}
	return recievedValue;
}

DWORD WINAPI marker(LPVOID args)
{
	bool * execute = (bool *)args;
	int recievedValue;
	while (*execute)
	{
		if (!ConnectNamedPipe(hNamedPipe,   // дескриптор канала    
			(LPOVERLAPPED)NULL // связь синхронная   
		)) {
			cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
			continue;
		}
		recievedValue = recieveValue(hNamedPipe);
		if (!DisconnectNamedPipe(hNamedPipe)) {
			cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		}

		switch (recievedValue)
		{
		case NEW_MACHINE_ADDED:
			broadcast(currentValue);
			break;
		case NETWORK_ENABLED:
			networkEnabled = true;
			cout << "Network enabled" << endl;
			break;
		case NETWORK_DISABLED:
			networkEnabled = false;
			cout << "Network disabled" << endl;
			break;
		default:
			currentValue = recievedValue;
			cout << "Current value = " << currentValue << endl;
			break;
		}
	}
	return 0;
}


int main() {

	machineNames.push_back("DESKTOP-CO3NPLJ");
	machineNames.push_back("DESKTOP-QE9Q318");
	bool execute = true;
	int recievedValue;
	int sendingValue;
	char pipeName[80];
	SECURITY_ATTRIBUTES sa;  // атрибуты защиты  
	SECURITY_DESCRIPTOR sd;  // дескриптор защиты 
	HANDLE  hConnectedPipe;
	char *  lpszInMessage = new char[80];  // для сообщения от клиента  
	char * lpszOutMessage = new char[80]; // обратное сообщение  
	DWORD  dwBytesWritten;  // для числа записанных байтов
	DWORD  dwBytesRead;  // для числа прочитанных байтов  
	DWORD threadID;
	// инициализация атрибутов защиты  
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE; // дескриптор канала ненаследуемый   // инициализируем дескриптор защиты  
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// устанавливаем атрибуты защиты, разрешая доступ всем пользователям  
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &sd;   // создаем именованный канал для чтения  
	hNamedPipe = CreateNamedPipe("\\\\.\\pipe\\demo_pipe",   // имя канала  
		PIPE_ACCESS_DUPLEX,  // читаем из канала и пишем в канал   
		PIPE_TYPE_MESSAGE | PIPE_WAIT, // синхронная передача сообщений   
		1,  // максимальное количество экземпляров канала    
		0,  // размер выходного буфера по умолчанию   
		0,  // размер входного буфера по умолчанию   
		INFINITE, // клиент ждет связь 500 мс   
		&sa  // доступ для всех пользователей  
	);
	// проверяем на успешное создание  
	if (hNamedPipe == INVALID_HANDLE_VALUE)
	{
		cerr << "Creation of the named pipe failed." << endl << "The last error code: " << GetLastError() << endl;
		cout << "Press any char to finish server: ";
		cin.get();
		return 0;
	}
	cout << "Pipe succesfully created" << endl;



	if (broadcast(NEW_MACHINE_ADDED) == 0)
		currentValue = 0;
	else
	{
		if (!ConnectNamedPipe(hNamedPipe,   // дескриптор канала    
			(LPOVERLAPPED)NULL // связь синхронная   
		)) {
			cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		}
		currentValue = recieveValue(hNamedPipe);
		if (!DisconnectNamedPipe(hNamedPipe)) {
			cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		}
	}
	cout << "Current value = " << currentValue << endl;
	CreateThread(NULL, // атрибуты защиты 
		0,							// размер стека потока в байтах 
		marker,						// адрес исполняемой функции 
		&execute,		// адрес параметра 
		0,							// флаги создания потока
		&threadID				// идентификатор потока 
	);

	char command;
	while (true)
	{
		command = _getch();
		//cin >> command;
		if (command == 0)
			continue;
		switch (command)
		{
		case '+':
			networkEnabled = true;
			cout << "Network enabled" << endl;
			broadcast(NETWORK_ENABLED);
			break;
		case '-':
			networkEnabled = false;
			cout << "Network disabled" << endl;
			broadcast(NETWORK_DISABLED);
			break;
		case ' ':
			if (networkEnabled) {
				currentValue++;
				cout << "Current value = " << currentValue << endl;
				broadcast(currentValue);
				break;
			}
			else
				cout << "No network. Unable to increase value" << endl;
			break;
		default:
			if (networkEnabled)
				cout << "Current value = " << currentValue << endl;
			else
				cout << "No network. Unable to get value" << endl;
			break;
		}
	}

	return 0;
}