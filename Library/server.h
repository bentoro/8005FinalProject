#ifndef SERVER_H
#define SERVER_H

#include "socketwrappers.h"
#include "client.h"
#include <netinet/in.h>
#include <iostream>
#include <strings.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

#define CONNECTIONS 5

class Server{

    public:
        Server(int listenPort);
        ~Server(){
            printf("Closing server\n");
        }
        int GetSocket();
    private:
        int listenfd, connectfd;
        struct sockaddr_in servaddr,adr;

};

#endif
