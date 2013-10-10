#include "Handler.h"
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>

void printDebugMessage(string);
string errorMessage(string message);

Handler::Handler(int client, AllUsers * allUsers, bool debug){
    debug_ = debug;

    buflen_ = 1024;
    buf_ = new char[buflen_+1];

	allUsers_ = allUsers;
	client_ = client;
	userData_sem_ = allUsers->getAllUsersSemaphore();

    if (debug_) 
    {
        ostringstream debugOS;
        debugOS << "In Handler constructor...about to handle(client)\n";
        printDebugMessage(debugOS.str());
    }

	handle(client);
}

Handler::~Handler(){
}

void
Handler::handle(int client) {
    // loop to handle all requests
    while (1) {
        // get a request
        string request = get_request(client);
        if (debug_)
        {
            printDebugMessage("Request: \"" + request + "\"");
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

    cout << "End of Handler::handle" << endl;
}

string
Handler::get_request(int client) {
    string request = "";
    // read until we get a newline
    while (request.find("\n") == string::npos) {
        int nread = recv(client,buf_,1024,0);
        if (nread < 0) {
            if (errno == EINTR){
                // the socket call was interrupted -- try again
                cout << "The socket call was interrupted -- try again" << endl;
                continue;
            }
            else{
                // an error occurred, so break out
                cout << "an error occurred, so break out" << endl;
                return "";
            }
        } else if (nread == 0) {
            // the socket is closed
            cout << "the socket is closed" << endl;
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
Handler::get_rest_of_request(int messageLength, int currentMessageLength, int client){
    
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
Handler::send_response(int client, string response) {
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
Handler::get_response(string request, int client) {
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
Handler::put(string name, string subject, int length, string message){
    if(debug_) printDebugMessage("Got into put");
    Message m (subject, message, length);

    // This is where I need a sem_wait(&s)
    sem_wait(userData_sem_);
    User * user = allUsers_->getUser(name);
    user->addMessage(m);
    // This is where I need a sem_post(&s)
    sem_post(userData_sem_);

    return "OK\n";
}

string
Handler::list(string name) {
    if(debug_) printDebugMessage("Got into list");

    // This is where I need a sem_wait(&s)
    sem_wait(userData_sem_);
    User * user = allUsers_->getUser(name);
    string listOfSubjects = user->getListOfSubjects();
    // This is where I need a sem_post(&s)
    sem_post(userData_sem_);

    return listOfSubjects;
}

string
Handler::get(string name, int index) {
    if(debug_) printDebugMessage("Got into get");

    // This is where I need a sem_wait(&s)
    sem_wait(userData_sem_);
    User * user = allUsers_->getUser(name);
    Message * message = user->getMessage(index);

    if (message == 0)
    {
        // This is where I need a sem_post(&s)
        sem_post(userData_sem_);
        return errorMessage("Message does not exist for " + name);
    }

    ostringstream os;
    os << "message " << message->getSubject() << " ";
    os << message->getMessageLength() << "\n";
    os << message->getMessage();
    // This is where I need a sem_post(&s)
    sem_post(userData_sem_);

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
Handler::reset() {
    if(debug_) printDebugMessage("Got into reset");

    // This is where I need a sem_wait(&s)
    sem_wait(userData_sem_);
    allUsers_->reset();
    // This is where I need a sem_post(&s)
    sem_post(userData_sem_);
    return "OK\n";
}

string
errorMessage(string message) {
    return "error " + message + "\n";
}

void
printDebugMessage(string message){
    cout << message << "\n" << endl;
}