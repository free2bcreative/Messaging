/*
 * AllUsers.cpp
 *
 *  Created on: Sep 18, 2013
 *      Author: free2run
 */

#include "AllUsers.h"
#include <iostream>

using namespace std;


AllUsers::AllUsers() {
	sem_t allUsers_sem_;
	sem_init(&allUsers_sem_, 0, 1);
}

AllUsers::~AllUsers() {
	// TODO Auto-generated destructor stub
}

sem_t * AllUsers::getAllUsersSemaphore(){
	return &allUsers_sem_;
}

User * AllUsers::createUser(string name){
	allUsers.insert(pair<string,User>(name, User(name)));

	return &allUsers.at(name);
}

User * AllUsers::getUser(string name){


	if (allUsers.find(name) == allUsers.end()){
		return createUser(name);
	}

	return &allUsers.at(name);

}

void AllUsers::reset(){
	allUsers.clear();
}

void AllUsers::printAllUsers(){

	cout << "AllUsers:" << endl;

	map<string,User>::iterator it;

	for (it=allUsers.begin(); it!=allUsers.end(); ++it)
	    cout << it->first << " => " << it->second.toString() << '\n';

}



