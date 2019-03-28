#include "Message.h"



Message::Message(int sender, int reciever, int value)
{
	senderID = sender;
	recieverID = reciever;
	this->value = value;
}

Message::Message()
{
}


Message::~Message()
{
}
