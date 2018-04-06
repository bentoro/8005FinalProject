/*------------------------------------------------------------------------------
# SOURCE FILE: 		server.cpp
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
# NOTES:		Server class that creates a connection to a client
#
------------------------------------------------------------------------------*/
#include "server.h"

using namespace std;

/*------------------------------------------------------------------------------
# FUNCTIONS: Server
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int listenport - Port to be connected to
#
# NOTES:		Create a Server object and connects to a client
#
------------------------------------------------------------------------------*/
Server::Server(int listenport){

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    connectfd = listenfd;
    ServerConfig(&servaddr, listenport);
    Reuse(listenfd);
    Bind(listenfd, &servaddr);
    Listen(listenfd,CONNECTIONS);
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
int Server::GetSocket(){
    return connectfd;
}
