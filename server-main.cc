/*
 * msgd.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: free2run
 */

#include "Server.h"

int main(){
	Server * server = new Server(3000);

	delete server;

	return 0;
}


