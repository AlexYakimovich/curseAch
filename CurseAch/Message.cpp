#include "Message.h"



Message::Message(int sender, int reciever, int value)
{
	senderID = sender;
	recieverID = reciever;
	this->value = value;
}

bool Message::operator<(const Message & msg)
{
	return recieveTime < msg.recieveTime;
}

Message::Message()
{
}


Message::~Message()
{
}
