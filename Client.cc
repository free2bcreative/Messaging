#include "Client.h"

Client::Client(string host, int port) {
    // setup variables
    host_ = host;
    port_ = port;
    buflen_ = 1024;
    buf_ = new char[buflen_+1];

    // connect to the server and run echo program
    create();
    msg();
}

Client::~Client() {
}

void
Client::create() {
    struct sockaddr_in server_addr;

    // use DNS to get IP address
    struct hostent *hostEntry;
    hostEntry = gethostbyname(host_.c_str());
    if (!hostEntry) {
        cout << "No such host name: " << host_ << endl;
        exit(-1);
    }

    // setup socket address structure
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    memcpy(&server_addr.sin_addr, hostEntry->h_addr_list[0], hostEntry->h_length);

    // create socket
    server_ = socket(PF_INET,SOCK_STREAM,0);
    if (!server_) {
        perror("socket");
        exit(-1);
    }

    // connect to server
    if (connect(server_,(const struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
        perror("connect");
        exit(-1);
    }
}

void
Client::msg() {
    while(1){
        cout << "% ";

        string line;
        
        getline(cin,line);

        vector<string> request = divideString(line);

        if (request.size() == 0)
            continue;

        string protocolMessage;

        string firstWord = request.at(0);
        if (firstWord == "send" && request.size() == 3)
        {
            request.push_back(getUserMessage());

            ostringstream os;
            os << "put " << request.at(1) << " " << request.at(2) << " ";
            os << request.at(3).size() << "\n" << request.at(3);

            //cout << os.str();
            protocolMessage = os.str();

        }
        else if (firstWord == "list" && request.size() == 2)
        {
            //implement list
            ostringstream os;
            os << "list " << request.at(1) << "\n";

            //cout << os.str();
            protocolMessage = os.str();
        }
        else if (firstWord == "read" && request.size() == 3)
        {
            //implement read
            int index = atoi(request.at(2).c_str());

            ostringstream os;
            os << "get " << request.at(1) << " ";
            os << index << "\n";

            //cout << os.str();
            protocolMessage = os.str();
        }
        else if (firstWord == "quit")
        {
            return;
        }
        else
        {
            cout << "I don't recognize that command." << endl;
            continue;
        }
        
        
        // send request
        bool success = send_request(protocolMessage);
        // break if an error occurred
        if (not success){
            cout << "No success after send_request" << endl;
            continue;
        }
        // get a response
        success = get_response();
        // break if an error occurred
        if (not success){
            cout << "No response was sent from the server" << endl;
            continue;
        }
    }
    
    close(server_);
}

string
Client::getUserMessage(){
    
    cout << "- Type your message.  End with a blank line -" << endl;

    vector<string> message;
    string line;

    while(getline(cin, line)){

        if (line.size() == 0)
        {
            break;
            
        }
        else{
            message.push_back(line);
        }
        
    }

    ostringstream os;

    for (size_t i = 0; i < message.size(); ++i)
    {
        os << message.at(i) << "\n";
    }

    return os.str();
}

vector<string>
Client::divideString(string s){
    vector<string> v;
    istringstream ss (s);
    string token;

    while(ss >> token){
        //cout << "Token: " << token << endl;
        v.push_back (token);
    }

    return v;
}

bool
Client::send_request(string request) {
    // prepare to send request
    const char* ptr = request.c_str();
    int nleft = request.length();
    int nwritten;
    // loop to be sure it is all sent
    while (nleft) {
        if ((nwritten = send(server_, ptr, nleft, 0)) < 0) {
            if (errno == EINTR) {
                // the socket call was interrupted -- try again
                cout << "the socket call was interrupted -- try again" << endl;
                continue;
            } else {
                // an error occurred, so break out
                perror("write");
                cout << "an error occurred, so break out" << endl;
                return false;
            }
        } else if (nwritten == 0) {
            // the socket is closed
            cout << "The socket is closed" << endl;
            return false;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return true;
}

bool
Client::get_response() {
    string response = "";
    // read until we get a newline
    while (response.find("\n") == string::npos) {
        int nread = recv(server_,buf_,1024,0);
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
        response.append(buf_,nread);
    }
    // a better client would cut off anything after the newline and
    // save it in a cache

    cout << handleResponse(response);
    return true;
}

string
Client::handleResponse(string response){
    ostringstream os;

    //Get first word
    size_t spacePos = response.find(" ");
    size_t newLinePos = response.find("\n");
    size_t pos;

    if (spacePos < newLinePos)
    {
        pos = spacePos;
    }
    else
    {
        pos = newLinePos;
    }
    string firstWord = response.substr(0, pos);

    //Decided what to do from firstWord
    if (firstWord == "error")
    {
        os <<  "Server returned bad message: " << response;
    }
    else if (firstWord == "OK")
    {
        os << "";
    }
    else if (firstWord == "list")
    {
        int messageCount = atoi(response.substr(spacePos, newLinePos - spacePos).c_str());
        if (messageCount == 0)
        {
            os << "";
        }
        else{
            os << response.substr(newLinePos+1);
        }
    }
    else if (firstWord == "message")
    {
        //get subject (second word)
        istringstream is(response);
        string subject;
        is >> subject;
        is >> subject;

        os << subject << response.substr(newLinePos+1);
    }

    return os.str();
}

