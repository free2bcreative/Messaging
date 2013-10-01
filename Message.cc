/*
 * Message.cpp
 *
 *  Created on: Sep 17, 2013
 *      Author: free2run
 */

#include "Message.h"



Message::Message(string subject, string message, int messageLength) {

	this->subject = subject;
	this->message = message;
	this->messageLength = messageLength;

}

Message::~Message() {
	// TODO Auto-generated destructor stub
}

string Message::getSubject(){
	return subject;
}

string Message::getMessage(){
	return message;
}

int Message::getMessageLength(){
	return messageLength;
}

