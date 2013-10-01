/*
 * msgd.cpp
 *
 *  Created on: Sep 23, 2013
 *      Author: free2run
 */

#include "Server.h"
#include <iostream>

using namespace std;

int
main(int argc, char **argv)
{
    // setup default arguments
    int option;
    int port = 3000;
    bool debug = false;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"p:d")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'd':
                debug = true;
                break;
            default:
                cout << "server [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }

	Server server(port, debug);
}


