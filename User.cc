/*
 * User.cpp
 *
 *  Created on: Sep 16, 2013
 *      Author: free2run
 */

#include "User.h"

User::User(string name) {
	this->name = name;

	messages = new vector<Message>();
}

User::~User() {
	// TODO Auto-generated destructor stub
}

string User::getName(){
	return name;
}

/**
 * Gets a message at a certain index and returns it as a string
 * If the index is out of bounds, then NULL is returned
 */
Message * User::getMessage(unsigned int index){

	Message * result = 0;

	if (index >= messages->size()){
		return result;
	}
	else{
		result = &(messages->at(index));
	}

	return result;

}

void User::addMessage(Message m){
	messages->push_back(m);
}

/*
	This fulfills the protocol that is needed for the "list" command.
	Format is this:  [number]\n[index] [subject]\n...[index] [subject]\n
*/
string User::getListOfSubjects() {
	ostringstream os;
	os << "list " << messages->size() << "\n";

	for (size_t i = 0; i < messages->size(); ++i)
	{

		os << i+1 << " " << messages->at(i).getSubject() << "\n";
	}

	return os.str();
}

string User::toString(){
	ostringstream ss;

	ss << getName() << "\nMessages:";

	for (size_t i = 0; i < messages->size(); i++){
		Message currentMessage = messages->at(i);
		ss << "\n\nIndex[" << i << "] Subject: " << currentMessage.getSubject();
		ss << " Length: " << currentMessage.getMessageLength() << "\n";
		ss << currentMessage.getMessage();
	}

	return ss.str();


}