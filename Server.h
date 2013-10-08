#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <queue>

#include <string>
#include <vector>
#include <sstream>

#include <semaphore.h>

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

    queue<int> clientQ_;
    sem_t buffer_sem; //ensures that we don't have more than BUFFER_SIZE connections
    sem_t queue_sem; //protects my queue when accessing it by many threads
    sem_t queue_signal; //signals "listener" threads when I have placed something in queue

    AllUsers allUsers;
};
