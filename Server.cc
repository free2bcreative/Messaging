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

        string response = get_response(request);

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
Server::get_response(string request) {
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
        string message = request.substr(pos);
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

    User * user = allUsers.getUser(name);
    user->addMessage(m);

    return "OK\n";
}

string
Server::list(string name) {
    if(debug_) printDebugMessage("Got into list");

    User * user = allUsers.getUser(name);
    return user->getListOfSubjects();
}

string
Server::get(string name, int index) {
    if(debug_) printDebugMessage("Got into get");

    User * user = allUsers.getUser(name);
    Message * message = user->getMessage(index);

    if (message == 0)
    {
        return errorMessage("Message does not exist for " + name);
    }

    ostringstream os;
    os << "message " << message->getSubject() << " " << message->getMessageLength() << message->getMessage();

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

    allUsers.reset();
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
