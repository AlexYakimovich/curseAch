#include "NetworkCommunicationManager.h"
#include "LocalCommunicationManager.h"
using namespace std;

int currentValue;

CommunicationManager * manager;

DWORD WINAPI marker(LPVOID args)
{
	bool * execute = (bool *)args;
	int recievedValue;
	while (*execute)
	{
		recievedValue = manager->recieveValue();

		switch (recievedValue)
		{
		case NEW_MACHINE_ADDED:
			manager->broadcast(currentValue);
			break;
		case NETWORK_ENABLED:
			manager->setNetworkEnabled(true);
			cout << "Network enabled" << endl;
			break;
		case NETWORK_DISABLED:
			manager->setNetworkEnabled(false);
			cout << "Network disabled" << endl;
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
	if(argc < 2)
		manager = (CommunicationManager *)(new NetworkCommunicationManager());
	else
	{
		int id = atoi(argv[1]);
		manager = (CommunicationManager *)(new LocalCommunicationManager(id));
	}
	if (manager->getNetworkEnabled())
		cout << "Connection succesfully created" << endl;
	else
		return -1;

	if (manager->broadcast(NEW_MACHINE_ADDED) == 0)
		currentValue = 0;
	else
		currentValue = manager->recieveValue();
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
			manager->setNetworkEnabled(true);
			cout << "Network enabled" << endl;
			manager->broadcast(NETWORK_ENABLED);
			break;
		case '-':
			cout << "Network disabled" << endl;
			manager->broadcast(NETWORK_DISABLED);
			manager->setNetworkEnabled(false);
			break;
		case ' ':
			if (manager->getNetworkEnabled())
			{
				currentValue++;
				cout << "Current value = " << currentValue << endl;
				manager->broadcast(currentValue);
				break;
			}
			else
				cout << "No network. Unable to increase value" << endl;
			break;
		default:
			if (manager->getNetworkEnabled())
				cout << "Current value = " << currentValue << endl;
			else
				cout << "No network. Unable to get value" << endl;
			break;
		}
	}

	return 0;
}