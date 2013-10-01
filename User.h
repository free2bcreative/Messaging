/*
 * User.h
 *
 *  Created on: Sep 16, 2013
 *      Author: free2run
 */

#pragma once
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#include "Message.h"

#ifndef USER_H_
#define USER_H_

class User {
public:
	User(string name);
	~User();

	string getName();
	string getListOfSubjects();
	Message * getMessage(unsigned int index);
	void addMessage(Message m);

	string toString();

private:
	string name;
	vector<Message> * messages;
};

#endif /* USER_H_ */
