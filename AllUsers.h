/*
 * AllUsers.h
 *
 *  Created on: Sep 18, 2013
 *      Author: free2run
 */

#ifndef ALLUSERS_H_
#define ALLUSERS_H_

#include <map>
 #include <semaphore.h>
#include "User.h"

class AllUsers {
private:
	map<string,User> allUsers;
	User * createUser(string name);
	sem_t allUsers_sem_;

public:
	AllUsers();
	virtual ~AllUsers();

	User * getUser(string name);
	sem_t * getAllUsersSemaphore();
	void reset();
	void printAllUsers();

};


#endif /* ALLUSERS_H_ */
