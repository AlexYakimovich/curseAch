#include "NetworkCommunicationManager.h"

using namespace std;

int NetworkCommunicationManager::broadcast(int value)
{
	if (!networkEnabled)
		return 0;
	char * pipeName = new char[80];
	HANDLE hConnectedPipe;
	DWORD dwBytesWritten;
	int sendingCount = 0;
	for (int machineNumber = 0; machineNumber < machineNames.size(); machineNumber++)
	{
		if (machineNumber == id)
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

int NetworkCommunicationManager::recieveValue()
{
	if (!ConnectNamedPipe(hNamedPipe,   // дескриптор канала    
		(LPOVERLAPPED)NULL // связь синхронная   
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return NETWORK_ERROR;
	}
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
	if (!DisconnectNamedPipe(hNamedPipe)) {
		cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		return NETWORK_ERROR;
	}
	return recievedValue;
}

NetworkCommunicationManager::NetworkCommunicationManager()
{
	machineNames.push_back("DESKTOP-CO3NPLJ");
	machineNames.push_back("DESKTOP-QE9Q318");
	
	SECURITY_ATTRIBUTES sa;  // атрибуты защиты  
	SECURITY_DESCRIPTOR sd;  // дескриптор защиты  
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
		networkEnabled = false;
		system("pause");
		return;
	}
}


NetworkCommunicationManager::~NetworkCommunicationManager()
{
}
