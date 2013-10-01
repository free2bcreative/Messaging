/*
 * Message.h
 *
 *  Created on: Sep 17, 2013
 *      Author: free2run
 */

#pragma once
#include <string>
using namespace std;

#ifndef MESSAGE_H_
#define MESSAGE_H_



class Message {

public:
	Message(string subject, string message, int messageLength);
	~Message();

	string getSubject();
	string getMessage();
	int getMessageLength();

private:
	string subject;
	string message;
	int messageLength;
};


#endif /* MESSAGE_H_ */
