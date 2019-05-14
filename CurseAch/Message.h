#include <chrono>
#pragma once
class Message
{
public:
	int senderID, recieverID, value;
	std::chrono::time_point<std::chrono::system_clock> recieveTime;
	Message(int sender, int reciever, int value);
	//bool operator <(const Message &msg);
	Message();
	~Message();
};

