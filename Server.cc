#include "Server.h"
#include <iostream>
#include <stdlib.h>


string errorMessage(string);

Server::Server(int port, bool debug) {
    // setup variables
    port_ = port;
    debug_ = debug;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];

    sem_t buffer_sem; //ensures that we don't have more than BUFFER_SIZE connections
    sem_t queue_sem; //protects my queue when accessing it by many threads
    sem_t queue_signal; //signals "listener" threads when I have placed something in queue



    // setup semaphores (for server)
    sem_init(&buffer_sem, 0, 1000); // buffer size 1000
    sem_init(&queue_signal, 0, 0); // signals when I've placed client in Q
    sem_init(&queue_sem, 0, 1); // protects my queue from other accessing

    /*
    Do I need Worker class?  Probably not.
    Just need to do the following:

    vector<pthread_t> threads
    for (# of threads to make){
        pthread_t temp;
        threads.push(temp);
    }

    for (# of threads){
        pthread_create(threads[i], NULL, &threadTask, void * arg) //may need to pass data structure?
    }
    */

    if (debug_)
    {
        ostringstream debugOS;
        debugOS << "Port: " << port_ << endl;
        debugOS << "Debug: " << debug_ << endl;
        printDebugMessage(debugOS.str());

    }

    // create and run the server
    create();
    serve();
}

Server::~Server() {
    delete buf_;
}

void
Server::create() {
    struct sockaddr_in server_addr;

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // set socket to immediately reuse port when the application closes
    int reuse = 1;
    if (setsockopt(server_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        exit(-1);
    }

    // call bind to associate the socket with our local address and
    // port
    if (bind(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("bind");
        exit(-1);
    }

      // convert the socket to listen for incoming connections
    if (listen(server_,SOMAXCONN) < 0) {
        perror("listen");
        exit(-1);
    }
}

void
Server::serve() {
    // setup client
    int client;
    struct sockaddr_in client_addr;
    socklen_t clientlen = sizeof(client_addr);

    if (debug_)
    {
        printDebugMessage("Server is ready to serve.");
    }
    

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        if (debug_)
        {
            printDebugMessage("Client has connected.  Starting to handle request");
        }

        /*
        This is probably where I add the client to the queue.
        The worker threads should be checking the queue..maybe,
        or perhaps have a signal semaphore to alert them.
        
        sem_wait(&buffer_sem);
        sem_wait(&queue_sem);
        clientQ.push(client);
        sem_post(&queue_sem);
        sem_post(queue_signal);

        */
        handle(client);
        close(client);
    }

    close(server_);

}

void
Server::handle(int client) {
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request(client);
        if (debug_)
        {
            printDebugMessage("Request: " + request);
        }

        // break if client is done or an error occurred
        if (request.empty())
            break;

        string response = get_response(request, client);

        // send response
        if(debug_) printDebugMessage("Beginning to send_response");
        bool success = send_response(client,response);
        // break if an error occurred
        if (not success)
            break;
    }
}

string
Server::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buf_,nread);
    }
    // a better server would cut off anything after the newline and
    // save it in a cache
    return request;
}

string
Server::get_rest_of_request(int messageLength, int currentMessageLength, int client){
    
    int nleft = messageLength - currentMessageLength;
    
    if (debug_)
    {
        ostringstream debugOS;
        debugOS << "Entering get_rest_of_request:" << endl;
        debugOS << "messageLength needed: " << messageLength << endl;
        debugOS << "currentMessageLength: " << currentMessageLength << endl;
        debugOS << "nleft: " << nleft << endl;
        printDebugMessage(debugOS.str());
    }

    string request = "";

    //read until nleft gets to 0
    while (nleft){
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR)
                // the socket call was interrupted -- try again
                continue;
            else
                // an error occurred, so break out
                return "";
        } else if (nread == 0) {
            // the socket is closed
            return "";
        }
        // be sure to use append in case we have binary data
        request.append(buf_,nread);

        nleft -= nread;
    }

    if (debug_)
    {
        ostringstream debugOS;
        debugOS << "Finished looping to get rest of message:" << endl;
        debugOS << "Rest of Message: \"" << request << "\"" << endl;
        debugOS << "Rest of Message length: " << request.size() << endl;
        printDebugMessage(debugOS.str());
    }

    return request;
}

bool
Server::send_response(int client, string response) {
    // prepare to send response
    const char* ptr = response.c_str();
    int nleft = response.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if (debug_)
        {
            ostringstream debugOS;
            debugOS << "nleft: " << nleft << endl;
            debugOS << "nwritten: " << nwritten << endl;
            printDebugMessage(debugOS.str());
        }
        
        if ((nwritten = send(client, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;

        
    }
    return true;
}

string
Server::get_response(string request, int client) {
    string invalidRequest = "error Did not recognize the request you made.\n";

    if (request.size() == 0)
    {
        return invalidRequest;
    }

    size_t pos = request.find('\n');
    string protocol = request.substr(0, pos);

    std::vector<string> v;
    istringstream ss (protocol);
    string token;

    while(ss >> token){
        v.push_back (token);
    }

    if (debug_)
    {
        ostringstream debugOS;
        debugOS << "Request from client: \"" << request << "\"" << endl;
        debugOS << "After chopping up protocol message:" << endl;
        for (std::vector<string>::iterator i = v.begin(); i != v.end(); ++i)
        {
            debugOS << "[" << *i << "]";
        }
        printDebugMessage(debugOS.str());
    }

    string task = v.at(0);

    if (task == "put" && v.size() == 4)
    {
        int length = atoi(v.at(3).c_str());
        string message = request.substr(pos + 1);

        if (length != (int)message.size())
        {
            message.append(get_rest_of_request(length, message.size(), client));
        }

        return put(v.at(1), v.at(2), length, message);
    }
    else if (task == "list" && v.size() == 2)
    {
        return list(v.at(1));
    }
    else if (task == "get" && v.size() == 3)
    {
        int index = atoi(v.at(2).c_str()) - 1;
        return get(v.at(1), index);
    }
    else if (task == "reset" && v.size() == 1)
    {
        return reset();
    }

    return invalidRequest;

    

}

string
Server::put(string name, string subject, int length, string message){
    if(debug_) printDebugMessage("Got into put");
    Message m (subject, message, length);

    // This is where I need a sem_wait(&s)
    User * user = allUsers.getUser(name);
    user->addMessage(m);
    // This is where I need a sem_post(&s)

    return "OK\n";
}

string
Server::list(string name) {
    if(debug_) printDebugMessage("Got into list");

    // This is where I need a sem_wait(&s)
    User * user = allUsers.getUser(name);
    string listOfSubjects = user->getListOfSubjects();
    // This is where I need a sem_post(&s)

    return listOfSubjects;
}

string
Server::get(string name, int index) {
    if(debug_) printDebugMessage("Got into get");

    // This is where I need a sem_wait(&s)
    User * user = allUsers.getUser(name);
    Message * message = user->getMessage(index);

    if (message == 0)
    {
        // This is where I need a sem_post(&s)
        return errorMessage("Message does not exist for " + name);
    }

    ostringstream os;
    os << "message " << message->getSubject() << " ";
    os << message->getMessageLength() << "\n";
    os << message->getMessage();
    // This is where I need a sem_post(&s)

    if (debug_)
    {
        ostringstream debugOS;
        debugOS << "Got message from data structure:" << endl;
        debugOS << "\"" << os.str() << "\"";
        printDebugMessage(debugOS.str());
    }

    return os.str();
}

string
Server::reset() {
    if(debug_) printDebugMessage("Got into reset");

    // This is where I need a sem_wait(&s)
    allUsers.reset();
    // This is where I need a sem_post(&s)
    return "OK\n";
}

string
errorMessage(string message) {
    return "error " + message + "\n";
}

void
Server::printDebugMessage(string message){
    cout << message << "\n" << endl;
}
