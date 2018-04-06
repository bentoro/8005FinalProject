/*------------------------------------------------------------------------------
# SOURCE FILE: 		library.cpp
#
# PROGRAM:  		COMP8005 - Assignment 3
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# NOTES:		A library of wrappers used for client and server
#
------------------------------------------------------------------------------*/
#include "library.h"

using namespace std;

/*------------------------------------------------------------------------------
# FUNCTIONS: Socket
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int family - family of the socket
#            int type - type of socket
#            int protocol - protocol of the connection
#
# RETURNS: socket int value
#
# NOTES:		Create a socket
#
------------------------------------------------------------------------------*/
int Socket(int family, int type, int protocol){
    int sockfd;

    if((sockfd = (socket(family, type, protocol))) < 0){
        printf("socket error\n");
        return -1;
    }
    return sockfd;
}

/*------------------------------------------------------------------------------
# FUNCTIONS: Bind
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int family - family of the socket
#            struct sockaddr_in * addr - sockaddr of the connection
#
# RETURNS: socket int value
#
# NOTES:		Binds the sockets
#
------------------------------------------------------------------------------*/
int Bind(int socket, struct sockaddr_in *addr){
    int sockfd;

    if((sockfd = bind(socket,(struct sockaddr*)addr, sizeof(struct sockaddr_in))) < 0){
        perror("binding error");
        close(socket);
        return -1;
    }

    return sockfd;
}

/*------------------------------------------------------------------------------
# FUNCTIONS: Connect
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int socket - socket of the connection
#            struct sockaddr_in sockaddr - sockaddr of the connection
#
# RETURNS:  returns true of sucessful
#
# NOTES:		Connects to the specified socket
#
------------------------------------------------------------------------------*/
bool Connect(int socket, struct sockaddr_in sockaddr){

    if(connect(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1){
        printf("Client: can't connect to the server \n");
        return false;
    }

    printf("Client: Connected!\n");

    return true;
}

/*------------------------------------------------------------------------------
# FUNCTIONS: Listen
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int socket - socket to listen to
#            int size - size
#
# RETURNS:   int value
#
# NOTES:		Listen to the specified socket
#
------------------------------------------------------------------------------*/
int Listen(int socket, int size){
    int sockfd;

    if ((sockfd = listen(socket,size)) < 0){
        printf("Listening error\n");
        close(socket);
        return -1;

    }

    return 0;
}

/*------------------------------------------------------------------------------
# FUNCTIONS: SetNonBlocking
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int socket - socket to set to non blocking
#
# RETURNS: void
#
# NOTES:		set specified socket as non-blocking
#
------------------------------------------------------------------------------*/
void SetNonBlocking(int socket){
    int sockfd = fcntl(socket, F_SETFL, O_NONBLOCK);
    if (sockfd < 0){
        perror("Error changing socket to non-blocking");
        exit(3);
    }
}

/*------------------------------------------------------------------------------
# FUNCTIONS: Accept
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int socket - socket to set to non blocking
#            struct sockaddr * addr - sockaddr to be Accepted
#            socklen_t *addrlen - length of the sockaddr
#
# RETURNS: socket of the accepted socket
#
# NOTES:		Accept the specified socket
#
------------------------------------------------------------------------------*/
int Accept(int socket, struct sockaddr *addr, socklen_t *addrlen){
    int sockfd;

    printf("Waiting for connection\n");
    if((sockfd = accept(socket,addr, addrlen)) < 0){
        printf("Accept error");
        close(socket);
        return -1;
    }

    printf("Connection Accepted\n");
    return sockfd;
}

/*------------------------------------------------------------------------------
# FUNCTIONS: ServerConfig
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: struct sockaddr * servaaddr - servaaddr to be configured
#            int port - port to be ocnfigured to the sockaddr
#
# RETURNS: void
#
# NOTES:		Setsup the servaddr for the server
#
------------------------------------------------------------------------------*/
void ServerConfig(struct sockaddr_in *servaddr, int port){
    bzero((char*)servaddr, sizeof(struct sockaddr_in));
    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr->sin_port = htons (port);
}

/*------------------------------------------------------------------------------
# FUNCTIONS: ClientConfig
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: struct sockaddr_in *servaddr - servaaddr to be configured
#            const char* ip - ip to be connected to
#            int port - port to be configured to the sockaddr
#
# RETURNS: void
#
# NOTES:		Setsup the client to be connected
#
------------------------------------------------------------------------------*/
void ClientConfig(struct sockaddr_in *servaddr, const char* ip, int port){
    hostent* hp;

    if((hp = gethostbyname(ip)) == NULL){
        printf("Unknown server address %s \n", ip);
    }

    bzero((char*)servaddr, sizeof(struct sockaddr_in));
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons (port);
    bcopy(hp->h_addr, (char*) &servaddr->sin_addr, hp->h_length);

}

/*------------------------------------------------------------------------------
# FUNCTIONS: Reuse
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: int socket - socket to be reused
#
# RETURNS: void
#
# NOTES:		allows the socket to be reused
#
------------------------------------------------------------------------------*/
void Reuse(int socket){
  int y;
  if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int)) == -1){
    perror("setsockopt");
  }
}
