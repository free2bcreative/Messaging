#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

class Client {
public:
    Client(string, int, bool);
    ~Client();

private:

    void create();
    void msg();
    vector<string> divideString(string);
    vector<string> divideResponse(string);
    bool send_request(string);
    bool get_response();
    string getUserMessage();
    string handleResponse(string);
    void printDebugMessage(string);

    int port_;
    string host_;
    bool debug_;
    int server_;
    int buflen_;
    char* buf_;
};
