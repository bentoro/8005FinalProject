/*------------------------------------------------------------------------------
# SOURCE FILE: 		client.cpp
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
# NOTES:		Client class that creates a connection to a listening socket
#
------------------------------------------------------------------------------*/
#include "client.h"

using namespace std;

/*------------------------------------------------------------------------------
# FUNCTIONS: Client
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: const char* ip - IP address of the listening workstation
             int port - Port to be listened to
#
# NOTES:		Created a client object and creates a connection
#
------------------------------------------------------------------------------*/
Client::Client(const char* ip, int port) {
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    ClientConfig(&servaddr, ip, port);
    Connect(sockfd, servaddr);
}

/*------------------------------------------------------------------------------
# FUNCTIONS: GetSocket
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER:
#
# NOTES:		Gets the socket of the connected client
#
------------------------------------------------------------------------------*/
int Client::GetSocket(){
    return sockfd;
}
