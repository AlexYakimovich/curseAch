#include <chrono>
#pragma once

enum MessageType {Prepare, Promise, Accept, Error, ServerNewValue, ServerRequestValue};

class Message
{
public:
	long long senderID, recieverID, value;
	std::chrono::time_point<std::chrono::system_clock> recieveTime;
	Message(long long sender, long long reciever, long long value);
	bool operator <(const Message &msg) const;
	bool operator ==(const Message &msg) const;
	Message();
	~Message();
};

