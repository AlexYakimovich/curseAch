#include "LocalCommunicationManager.h"

using namespace std;

int LocalCommunicationManager::broadcast(int value)
{
	DWORD dwBytesWritten;
	char * serverPipeName = new char[80];
	pair<int, int> p = make_pair(value, -1);
	wsprintf(serverPipeName, "\\\\.\\pipe\\clientPipe#%d", id);
	HANDLE hConnectedPipe = CreateFile(serverPipeName,    // ��� ������  
		GENERIC_READ | GENERIC_WRITE,  // ������ � ���������� � ����� 
		FILE_SHARE_READ | FILE_SHARE_WRITE, // ��������� ������ � ������ � ����� 
		(LPSECURITY_ATTRIBUTES)NULL,  // ������ �� ���������
		OPEN_EXISTING,   // ��������� ������������ �����
		FILE_ATTRIBUTE_NORMAL,   // �������� �� ���������  
		(HANDLE)NULL    // �������������� ��������� ���
	);
	// ��������� ����� � �������  
	if (hConnectedPipe == INVALID_HANDLE_VALUE)
		return NETWORK_ERROR;
	if (!WriteFile(
		hConnectedPipe,  // ���������� ������
		&p,  // ������ 
		sizeof(p), // ������ ������ 
		&dwBytesWritten, // ���������� ���������� ������
		(LPOVERLAPPED)NULL // ���������� ������ 
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
	HANDLE hClientPipe = CreateFile(clientPipeName,    // ��� ������  
		GENERIC_READ | GENERIC_WRITE,  // ������ � ���������� � ����� 
		FILE_SHARE_READ | FILE_SHARE_WRITE, // ��������� ������ � ������ � ����� 
		(LPSECURITY_ATTRIBUTES)NULL,  // ������ �� ���������
		OPEN_EXISTING,   // ��������� ������������ �����
		FILE_ATTRIBUTE_NORMAL,   // �������� �� ���������  
		(HANDLE)NULL    // �������������� ��������� ���
	);

	if (hClientPipe == INVALID_HANDLE_VALUE)
		cout << "KAK ZHE ZAEBALO";
	cout << "Recv" << id << endl;
	if (!ReadFile(
		hClientPipe,  // ���������� ������
		&value,  // ������ 
		sizeof(value), // ������ ������ 
		&dwBytesRead, // ���������� ���������� ������
		(LPOVERLAPPED)NULL // ���������� ������ 
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
