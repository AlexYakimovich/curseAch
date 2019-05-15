#include "Message.h"
#include "Message.h"



Message::Message(int sender, int reciever, int value)
{
	senderID = sender;
	recieverID = reciever;
	this->value = value;
}

bool Message::operator<(const Message & msg) const
{
	return recieveTime < msg.recieveTime;
}

bool Message::operator==(const Message & msg) const
{
	return senderID == msg.senderID && recieverID == msg.recieverID && value == msg.value;
}

Message::Message()
{
}


Message::~Message()
{
}
