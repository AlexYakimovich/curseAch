#define SUCCESS 0
#define NEW_MACHINE_ADDED -1
#define NETWORK_ENABLED -2
#define NETWORK_DISABLED -3
#define NETWORK_ERROR -4
#define TIMEOUT_REACHED -5


#pragma once
class CommunicationManager
{
protected:
	bool networkEnabled = true;
public:
	virtual int broadcast(int value) = 0;
	virtual int recieveValue(int timeout) = 0;
	void setNetworkEnabled(bool value);
	bool getNetworkEnabled();
	CommunicationManager();
	~CommunicationManager();
};

