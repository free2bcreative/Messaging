/*
 * Client.cpp
 *
 *  Created on: Sep 19, 2013
 *      Author: free2run
 */
#include "Client.h"
#include <iostream>

using namespace std;

int
main(int argc, char **argv)
{
    // setup default arguments
    int option;
    int port = 3000;
    string host = "localhost";
    bool debug = false;

    // process command line options using getopt()
    // see "man 3 getopt"
    while ((option = getopt(argc,argv,"h:p:d")) != -1) {
        switch (option) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'h':
                host = optarg;
                break;
            case 'd':
                debug = true;
                break;
            default:
                cout << "client [-s IP address] [-p port]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    

    Client client(host, port, debug);

    
}

