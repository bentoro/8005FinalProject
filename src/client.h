/*------------------------------------------------------------------------------
# HEADER FILE: 		client.h
#
# PROGRAM:  		COMP8005 - Assignment 3
#
# FUNCTIONS: GetSocket()
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# FUNCTIONS:
#            void NewData(int fd, bool flush);
#            void NewConnection(int socket, const int epollfd);
#            int GetConfig();
#            void ForwardTraffic();
#            bool newConnectionFound(int fd, int epollfd);
#            void close_fd (int signo);
#            int echo(int recvfd, int sendfd, int pipe[2]);
#            void FlushOut(int fd, int pipe[2]);
#            int GetPort(int socket);
#
#
------------------------------------------------------------------------------*/
#ifndef CLIENT_H
#define CLIENT_H

#include "library.h"
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
