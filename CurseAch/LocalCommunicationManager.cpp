#include "LocalCommunicationManager.h"

using namespace std;

int LocalCommunicationManager::broadcast(int value)
{
	DWORD dwBytesWritten;
	char * serverPipeName = new char[80];
  Message message(id, -1, value);
	wsprintf(serverPipeName, "\\\\.\\pipe\\serverPipe", id);
	HANDLE hConnectedPipe = CreateFile(serverPipeName,   
		GENERIC_READ | GENERIC_WRITE, 
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING,   
		FILE_ATTRIBUTE_NORMAL,  
		(HANDLE)NULL   
	);
	if (hConnectedPipe == INVALID_HANDLE_VALUE)
		return NETWORK_ERROR;
	if (!WriteFile(
		hConnectedPipe, 
		&message, 
		sizeof(message), 
		&dwBytesWritten,
		(LPOVERLAPPED)NULL 
	)) {
		return NETWORK_ERROR;
	}
  return SUCCESS;
}

int LocalCommunicationManager::recieveValue(int timeout)
{
  DWORD dwIgnore;
	DWORD dwBytesRead;
	char * clientPipeName = new char[80];
	Message value;
	wsprintf(clientPipeName, "\\\\.\\pipe\\clientPipe#%d", id);
	HANDLE hClientPipe = CreateFile(clientPipeName,  
		GENERIC_READ | GENERIC_WRITE,  
		FILE_SHARE_READ | FILE_SHARE_WRITE, 
		(LPSECURITY_ATTRIBUTES)NULL, 
		OPEN_EXISTING,  
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,  
		(HANDLE)NULL   
	);

	if (hClientPipe == INVALID_HANDLE_VALUE)
		cout << "KAK ZHE ZAEBALO";

  cout << "Hey";
  LPOVERLAPPED ol = { 0 };
  cout << "Hey";
  ol->hEvent = CreateEvent(NULL, TRUE, FALSE, "Hey");
  cout << "Hey";
	if (!ReadFile(
		hClientPipe,
		&value, 
		sizeof(value),
		&dwBytesRead, 
		ol 
	)) {
    cout << "Hey";
    WaitForSingleObject(ol->hEvent, timeout);
    if (GetOverlappedResult(hClientPipe, ol, &dwIgnore, FALSE) == 0)
      return TIMEOUT_REACHED;
  }
  cout << "Here";
	return value.value;
}

LocalCommunicationManager::LocalCommunicationManager(int id)
{
	this->id = id;
}


LocalCommunicationManager::~LocalCommunicationManager()
{
}
