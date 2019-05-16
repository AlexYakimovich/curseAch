#include <chrono>
#pragma once

enum MessageType {AppendEntry, RequestVote, Vote, NewValue, Error};

class Message
{
public:
	int senderID, recieverID, value;
	std::chrono::time_point<std::chrono::system_clock> recieveTime;
	Message(int sender, int reciever, int value);
	bool operator <(const Message &msg) const;
	bool operator ==(const Message &msg) const;
	Message();
	~Message();
};

