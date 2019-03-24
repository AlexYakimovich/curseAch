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
		wsprintf(pipeName, "\\\\%s\\pipe\\demo_pipe", machineNames[machineNumber].c_str());     // ����������� � ����������� �������
		hConnectedPipe = CreateFile(pipeName,    // ��� ������  
			GENERIC_READ | GENERIC_WRITE,  // ������ � ���������� � ����� 
			FILE_SHARE_READ | FILE_SHARE_WRITE, // ��������� ������ � ������ � ����� 
			(LPSECURITY_ATTRIBUTES)NULL,  // ������ �� ���������
			OPEN_EXISTING,   // ��������� ������������ �����
			FILE_ATTRIBUTE_NORMAL,   // �������� �� ���������  
			(HANDLE)NULL    // �������������� ��������� ���
		);
		// ��������� ����� � �������  
		if (hConnectedPipe == INVALID_HANDLE_VALUE)
			continue;
		if (!WriteFile(
			hConnectedPipe,  // ���������� ������
			&value,  // ������ 
			sizeof(value), // ������ ������ 
			&dwBytesWritten, // ���������� ���������� ������
			(LPOVERLAPPED)NULL // ���������� ������ 
		)) {
			continue;
		}
		sendingCount++;
	}
	return sendingCount;
}

int NetworkCommunicationManager::recieveValue()
{
	if (!ConnectNamedPipe(hNamedPipe,   // ���������� ������    
		(LPOVERLAPPED)NULL // ����� ����������   
	)) {
		cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		return NETWORK_ERROR;
	}
	int recievedValue;
	DWORD dwBytesRead;
	if (!ReadFile(hNamedPipe,   // ���������� ������    
		&recievedValue,   // ����� ������ ��� ����� ������    
		sizeof(recievedValue),  // ����� �������� ������    
		&dwBytesRead,   // ����� ����������� ������    
		(LPOVERLAPPED)NULL // �������� ������ ����������   
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
	
	SECURITY_ATTRIBUTES sa;  // �������� ������  
	SECURITY_DESCRIPTOR sd;  // ���������� ������  
	// ������������� ��������� ������  
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE; // ���������� ������ �������������   // �������������� ���������� ������  
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	// ������������� �������� ������, �������� ������ ���� �������������  
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);
	sa.lpSecurityDescriptor = &sd;   // ������� ����������� ����� ��� ������  
	hNamedPipe = CreateNamedPipe("\\\\.\\pipe\\demo_pipe",   // ��� ������  
		PIPE_ACCESS_DUPLEX,  // ������ �� ������ � ����� � �����   
		PIPE_TYPE_MESSAGE | PIPE_WAIT, // ���������� �������� ���������   
		1,  // ������������ ���������� ����������� ������    
		0,  // ������ ��������� ������ �� ���������   
		0,  // ������ �������� ������ �� ���������   
		INFINITE, // ������ ���� ����� 500 ��   
		&sa  // ������ ��� ���� �������������  
	);
	// ��������� �� �������� ��������  
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
