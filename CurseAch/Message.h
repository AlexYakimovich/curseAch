#pragma once
class Message
{
public:
	int senderID, recieverID, value;
	Message(int sender, int reciever, int value);
	Message();
	~Message();
};

