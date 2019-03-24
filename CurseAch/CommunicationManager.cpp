#include "CommunicationManager.h"



void CommunicationManager::setNetworkEnabled(bool value)
{
	networkEnabled = value;
}

bool CommunicationManager::getNetworkEnabled()
{
	return networkEnabled;
}

CommunicationManager::CommunicationManager()
{
}


CommunicationManager::~CommunicationManager()
{
}
