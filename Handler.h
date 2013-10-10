#include <string>
#include <iostream>
#include <semaphore.h>
#include "AllUsers.h"

using namespace std;


class Handler{
public:
	Handler(int, AllUsers *, bool);
	~Handler();

private:

	void handle(int);
	string get_request(int);
    string get_rest_of_request(int, int, int);
    bool send_response(int, string);
    string get_response(string, int);
    string put(string, string, int, string);
    string list(string);
    string get(string, int);
    string reset();

    int server_;
    int buflen_;
    char* buf_;

    bool debug_;

    AllUsers * allUsers_;
    int client_;
	sem_t * userData_sem_; // protects userData...might need to be a shared thing for all threads

};