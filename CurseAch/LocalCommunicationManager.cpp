#include "LocalCommunicationManager.h"

using namespace std;

int LocalCommunicationManager::broadcast(int value)
{
	DWORD dwBytesWritten;
	char * serverPipeName = new char[80];
	pair<int, int> p = make_pair(value, -1);
	wsprintf(serverPipeName, "\\\\.\\pipe\\clientPipe#%d", id);
	HANDLE hConnectedPipe = CreateFile(serverPipeName,    // имя канала  
		GENERIC_READ | GENERIC_WRITE,  // читаем и записываем в канал 
		FILE_SHARE_READ | FILE_SHARE_WRITE, // разрешаем чтение и запись в канал 
		(LPSECURITY_ATTRIBUTES)NULL,  // защита по умолчанию
		OPEN_EXISTING,   // открываем существующий канал
		FILE_ATTRIBUTE_NORMAL,   // атрибуты по умолчанию  
		(HANDLE)NULL    // дополнительных атрибутов нет
	);
	// проверяем связь с каналом  
	if (hConnectedPipe == INVALID_HANDLE_VALUE)
		return NETWORK_ERROR;
	if (!WriteFile(
		hConnectedPipe,  // дескриптор канала
		&p,  // данные 
		sizeof(p), // размер данных 
		&dwBytesWritten, // количество записанных байтов
		(LPOVERLAPPED)NULL // синхронная запись 
	)) {
		return NETWORK_ERROR;
	}
}

int LocalCommunicationManager::recieveValue()
{
	DWORD dwBytesRead;
	char * clientPipeName = new char[80];
	int value;
	wsprintf(clientPipeName, "\\\\.\\pipe\\clientPipe#%d", id);
	HANDLE hClientPipe = CreateFile(clientPipeName,    // имя канала  
		GENERIC_READ | GENERIC_WRITE,  // читаем и записываем в канал 
		FILE_SHARE_READ | FILE_SHARE_WRITE, // разрешаем чтение и запись в канал 
		(LPSECURITY_ATTRIBUTES)NULL,  // защита по умолчанию
		OPEN_EXISTING,   // открываем существующий канал
		FILE_ATTRIBUTE_NORMAL,   // атрибуты по умолчанию  
		(HANDLE)NULL    // дополнительных атрибутов нет
	);

	if (hClientPipe == INVALID_HANDLE_VALUE)
		cout << "KAK ZHE ZAEBALO";
	cout << "Recv" << id << endl;
	if (!ReadFile(
		hClientPipe,  // дескриптор канала
		&value,  // данные 
		sizeof(value), // размер данных 
		&dwBytesRead, // количество записанных байтов
		(LPOVERLAPPED)NULL // синхронная запись 
	)) {
		cout << "Error while reading pipe. " << GetLastError();
	}
	return value;
}

LocalCommunicationManager::LocalCommunicationManager(int id)
{
	this->id = id;
}


LocalCommunicationManager::~LocalCommunicationManager()
{
}
