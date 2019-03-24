#pragma once
class CommunicationManager
{
protected:
	bool networkEnabled = true;
public:
	virtual int broadcast(int value) = 0;
	virtual int recieveValue() = 0;
	void setNetworkEnabled(bool value);
	bool getNetworkEnabled();
	CommunicationManager();
	~CommunicationManager();
};

