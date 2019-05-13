#include "NetworkCommunicationManager.h"
#include "LocalCommunicationManager.h"
#define TIMEOUT 1000
using namespace std;

int currentValue;

CommunicationManager * manager;

DWORD WINAPI marker(LPVOID args)
{
	bool * execute = (bool *)args;
	int recievedValue;
	while (*execute)
	{
		recievedValue = manager->recieveValue(INT_MAX);
		switch (recievedValue)
		{
		case NEW_MACHINE_ADDED:
			manager->broadcast(currentValue);
			break;
		case NETWORK_ERROR:
			return NETWORK_ERROR;
		default:
			currentValue = recievedValue;
			cout << "Current value = " << currentValue << endl;
			break;
		}
	}
	return 0;
}

int main(int argc, char ** argv) {
	bool execute = true;
	DWORD threadID;
	if (argc < 2)
	{
		manager = (CommunicationManager *)(new NetworkCommunicationManager());
		cout << "Network manager created" << endl;
	}
	else
	{
		int id = atoi(argv[1]);
		manager = (CommunicationManager *)(new LocalCommunicationManager(id));
		cout << "Local manager #" << id << " created" << endl;
	}
	if (manager->getNetworkEnabled())
		cout << "Connection succesfully created" << endl;
	else
		return -1;
	if (manager->broadcast(NEW_MACHINE_ADDED) != SUCCESS)
		return -2;
	currentValue = manager->recieveValue(TIMEOUT);
	if (currentValue == TIMEOUT_REACHED)
		currentValue = 0;
	else if (currentValue == NETWORK_ERROR)
		return -3;
	cout << "Current value = " << currentValue << endl;
	CreateThread(NULL,
		0,
		marker,
		&execute,
		0,
		&threadID
	);

	char command;
	while (true)
	{
		command = _getch();
		//cin >> command;
		if (command == 0)
			continue;
		if (manager->getNetworkEnabled())
		{
			currentValue++;
			cout << "Current value = " << currentValue << endl;
			manager->broadcast(currentValue);
		}

	}

	return 0;
}