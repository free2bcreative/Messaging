#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <sstream>

#include "AllUsers.h"
#include "User.h"
#include "Message.h"

using namespace std;

class Server {
public:
    Server(int, bool);
    ~Server();

private:

    void create();
    void serve();
    void handle(int);
    string get_request(int);
    string get_rest_of_request(int, int, int);
    bool send_response(int, string);
    string get_response(string, int);
    string put(string, string, int, string);
    string list(string);
    string get(string, int);
    string reset();
    void printDebugMessage(string);

    int port_;
    bool debug_;
    int server_;
    int buflen_;
    char* buf_;
    AllUsers allUsers;
};
