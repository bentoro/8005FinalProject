#ifndef CLIENT_H
#define CLIENT_H

#include "socketwrappers.h"
#include <netinet/in.h>
#include <iostream>
#include <strings.h>
#include <stdlib.h>
#include <cstring>
#include <fstream>

using namespace std;

class Client {
    public:
        Client(const char* ip, int port);
        int GetSocket();
        ~Client(){
            printf("Closing Client\n");
        }
    private:
        int sockfd;
        struct sockaddr_in servaddr;
};

#endif
