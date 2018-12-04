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

int recieveValue(HANDLE hNamedPipe)
{
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
	return recievedValue;
}

DWORD WINAPI marker(LPVOID args)
{
	bool * execute = (bool *)args;
	int recievedValue;
	while (*execute)
	{
		if (!ConnectNamedPipe(hNamedPipe,   // ���������� ������    
			(LPOVERLAPPED)NULL // ����� ����������   
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
	SECURITY_ATTRIBUTES sa;  // �������� ������  
	SECURITY_DESCRIPTOR sd;  // ���������� ������ 
	HANDLE  hConnectedPipe;
	char *  lpszInMessage = new char[80];  // ��� ��������� �� �������  
	char * lpszOutMessage = new char[80]; // �������� ���������  
	DWORD  dwBytesWritten;  // ��� ����� ���������� ������
	DWORD  dwBytesRead;  // ��� ����� ����������� ������  
	DWORD threadID;
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
		cout << "Press any char to finish server: ";
		cin.get();
		return 0;
	}
	cout << "Pipe succesfully created" << endl;



	if (broadcast(NEW_MACHINE_ADDED) == 0)
		currentValue = 0;
	else
	{
		if (!ConnectNamedPipe(hNamedPipe,   // ���������� ������    
			(LPOVERLAPPED)NULL // ����� ����������   
		)) {
			cerr << "The connection failed." << endl << "The last error code: " << GetLastError() << endl;
		}
		currentValue = recieveValue(hNamedPipe);
		if (!DisconnectNamedPipe(hNamedPipe)) {
			cerr << "The disconnection failed." << endl << "The last error code: " << GetLastError() << endl;
		}
	}
	cout << "Current value = " << currentValue << endl;
	CreateThread(NULL, // �������� ������ 
		0,							// ������ ����� ������ � ������ 
		marker,						// ����� ����������� ������� 
		&execute,		// ����� ��������� 
		0,							// ����� �������� ������
		&threadID				// ������������� ������ 
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
			currentValue++;
			cout << "Current value = " << currentValue << endl;
			broadcast(currentValue);
			break;
		default:
			cout << "Current value = " << currentValue << endl;
			break;
		}
	}

	return 0;
}