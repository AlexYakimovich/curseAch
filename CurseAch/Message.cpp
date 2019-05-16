#include "Message.h"
#include "Message.h"



Message::Message(long long sender, long long reciever, long long value = 0)
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
