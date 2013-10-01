#include "Server.h"
#include <iostream>
#include <stdlib.h>


string errorMessage(string);

Server::Server(int port) {
    // setup variables
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];

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

    cout << "Server is ready to serve." << endl;

      // accept clients
    while ((client = accept(server_,(struct sockaddr *)&client_addr,&clientlen)) > 0) {

        cout << "Client has connected.  Starting to handle request" << endl;
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
        cout << "Request: " << request << endl;

        // break if client is done or an error occurred
        if (request.empty())
            break;

        string response = get_response(request);

        // send response
        cout << "Beginning to send_response" << endl;
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
        cout << "nleft: " << nleft << endl;
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
        cout << "Token: " << token << endl;
        v.push_back (token);
    }

    string task = v.at(0);

    if (task == "put")
    {
        int length = atoi(v.at(3).c_str());
        string message = request.substr(pos);
        cout << "Length: " << length << "\nMessage: " << message << endl;
        return put(v.at(1), v.at(2), length, message);
    }
    else if (task == "list")
    {
        return list(v.at(1));
    }
    else if (task == "get")
    {
        int index = atoi(v.at(2).c_str()) - 1;
        return get(v.at(1), index);
    }
    else if (task == "reset")
    {
        return reset();
    }

    return invalidRequest;

    

}

string
Server::put(string name, string subject, int length, string message){
    cout << "Got into put" << endl;
    Message m (subject, message, length);

    User * user = allUsers.getUser(name);
    user->addMessage(m);

    return "OK\n";
}

string
Server::list(string name) {
    cout << "Got into list" << endl;

    User * user = allUsers.getUser(name);
    return user->getListOfSubjects();
}

string
Server::get(string name, int index) {
    cout << "Got into get" << endl;

    User * user = allUsers.getUser(name);
    Message * message = user->getMessage(index);

    if (message == 0)
    {
        return errorMessage("Message does not exist for " + name);
    }

    ostringstream os;
    os << "message " << message->getSubject() << " " << message->getMessageLength() << "\n" << message->getMessage();

    return os.str();
}

string
Server::reset() {
    cout << "Got into reset" << endl;

    allUsers.reset();
    return "OK\n";
}

string
errorMessage(string message) {
    return "error " + message + "\n";
}
