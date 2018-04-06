/*------------------------------------------------------------------------------
# SOURCE FILE: 		proxy.cpp
#
# PROGRAM:  		COMP8005 - Assignment 3
#
# FUNCTIONS:
#          void NewData(int fd, bool flush);
#          void NewConnection(int socket, const int epollfd);
#          int GetConfig();
#          void ForwardTraffic();
#          bool newConnectionFound(int fd, int epollfd);
#          void close_fd (int signo);
#          int echo(int recvfd, int sendfd, int pipe[2]);
#          void FlushOut(int fd, int pipe[2]);
#          int GetPort(int socket);
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# NOTES:		This program forwards all the packets that are being listened to, to
#           the destination ip/port
#
------------------------------------------------------------------------------*/
#include "server.h"
#include "client.h"
#include "library.h"
#include "epoll.h"
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <cstring>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <limits.h>
#include "proxy.h"

/*------------------------------------------------------------------------------
# FUNCTION:   Main
#
# DATE:			  Apr 6, 2018
#
# INTERFACE:  Main()
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int argc, char *argv[]
#
# RETURNS:    int
#
# NOTES:		  This program forwards all the packets that are being listened to, to
#             the destination ip/port
#
------------------------------------------------------------------------------*/
int listenarray[MAXEVENTS];
int numofconnections;
int clientcount;

int main(int argc, char *argv[]){
    int epollfd, listensock;
    struct epoll_event event;
    struct epoll_event *events;
    clientcount = 0;
    struct sigaction act;

	// set up the signal handler to close the server socket when CTRL-c is received
    act.sa_handler = close_fd;
    act.sa_flags = 0;
    if ((sigemptyset (&act.sa_mask) == -1 || sigaction (SIGINT, &act, NULL) == -1))
    {
        perror ("Failed to set SIGINT handler");
        exit (EXIT_FAILURE);
    }

    epollfd = createEpollFd();
    ClientList =(client_info *)calloc(BUFLEN, sizeof(client_info));
    numofconnections = GetConfig();

    for (int i = 0; i < numofconnections; i++){
        //listening for packets coming in at port 7005
        Server *server = new Server(server_list[i].port);
        cout << "Listening socket " << server->GetSocket() << endl;
        listensock = server->GetSocket();
        SetNonBlocking(listensock);

        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = listensock;
        addEpollSocket(epollfd, listensock, &event);
        listenarray[i] = listensock;
    }

    events =(epoll_event *) calloc(MAXEVENTS, sizeof(event));

    while(1){
        int fds, i;
        fds = waitForEpollEvent(epollfd, events);

        for (i = 0; i < fds; i++){
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)){
                close(events[i].data.fd);
                continue;
            } else if((events[i].events & EPOLLIN)){
                if(newConnectionFound(events[i].data.fd, epollfd)){
                    printf("New connection \n");
                } else {
                    printf("New data \n");
                    NewData(events[i].data.fd, false);
                }
            }  else {
                NewData(events[i].data.fd, true);
            }
        }
    }
    close(epollfd);
    return 0;
}

/*------------------------------------------------------------------------------
# FUNCTION:   newConnectionFound
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int fd - file descriptor
#             int epollfd - Epoll fil descriptor
#
# RETURNS:    true if new connection is found
#
# NOTES:		  Checks if there is a new connection and accepts new connection
#
------------------------------------------------------------------------------*/
bool newConnectionFound(int fd, int epollfd) {
    for (int i = 0; i < numofconnections; i++) {
        if(fd == listenarray[i]){
            printf("Accepting Client connection \n");
            NewConnection(fd, epollfd);

            return true;
        }
    }
    return false;
}

/*------------------------------------------------------------------------------
# FUNCTION:   GetConfig
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  void
#
# RETURNS:    returns number of connections
#
# NOTES:		  Parses the data in the config file and stores it into a struct
#
------------------------------------------------------------------------------*/
int GetConfig(){
    //initialize after every line
    FILE *fp;
    fp = fopen("config","r");
    int c=0;
    int lines = 0;
    while(!feof(fp)){
        c=fgetc(fp);
        if(c == '\n'){
            lines++;
        }
    }
    cout << "lines in config: " << lines << endl;
    server_list = (server_info*) calloc(lines, sizeof(server_info));

    rewind(fp);
    int count = 0;
    while(count < lines){
        char ip[BUFLEN];
        int port;
        //int proxy_sock;

        fscanf(fp, "%s %u", ip, &port);
        cout << ip << endl;
        cout << port << endl;

        server_list[count].port = port;
        server_list[count].ip = ip;
        count++;
    }

    return lines;
}

/*------------------------------------------------------------------------------
# FUNCTION:   NewConnection
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int socket - file descriptor of connected socket
#             const int epollfd - Epoll fil descriptor
#
# RETURNS:    void
#
# NOTES:		  Accepts new conection and adds the connected socket into the client
#             list
#
------------------------------------------------------------------------------*/
void NewConnection(int socket, const int epollfd){
    while(1){
        struct sockaddr addr;
        struct epoll_event event;
        socklen_t addrlen;
        int newfd;

        //add all the new clients trying to connect
        if ((newfd = accept(socket, &addr, &addrlen)) == -1){
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                //no more connections
                break;
            } else {
                perror("Accept error \n");
            }
        }

        //Get the port it came in on
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if(getsockname(socket, (struct sockaddr *)&sin, &len) == -1){
            perror("getsockename");
        }

        //Creating the client_info
        pipe(ClientList[clientcount].pipefd);
        ClientList[clientcount].ProxyRecvPort = ntohs(sin.sin_port);
        ClientList[clientcount].client_sock = newfd;   //socket going to the client
        SetNonBlocking(newfd);
        event.events = EPOLLIN | EPOLLET | EPOLLOUT;
        event.data.fd = newfd;
        addEpollSocket(epollfd, newfd, &event);

        Client *proxy_connection;

        //Connect to epoll
        for(int i = 0; i < numofconnections; i++) {
            if(server_list[i].port == ntohs(sin.sin_port)) {
                proxy_connection = new Client(server_list[i].ip, ntohs(sin.sin_port));
                //ClientConfig(&connection_addr, server_list[i].ip, ntohs(sin.sin_port)); //connection to server
                break;
            }
        }

        ClientList[clientcount].server_sock = proxy_connection->GetSocket();
        ClientList[clientcount].ProxySendPort = GetPort(proxy_connection->GetSocket());

        SetNonBlocking(proxy_connection->GetSocket());
        event.events = EPOLLIN | EPOLLET | EPOLLOUT;
        event.data.fd = proxy_connection->GetSocket();
        addEpollSocket(epollfd, proxy_connection->GetSocket(), &event);
        printf("Adding a new client \n");
        clientcount++;
    }
}

/*------------------------------------------------------------------------------
# FUNCTION:   GetPort
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int socket - socket of connection
#
# RETURNS:    returns the port of the connection
#
# NOTES:		  Gets the port of the socket
#
------------------------------------------------------------------------------*/
int GetPort(int socket){
    struct sockaddr_in connectionsin;
    socklen_t connection_len = sizeof(connectionsin);
    if(getsockname(socket, (struct sockaddr *)&connectionsin, &connection_len) == -1){
        perror("getsockename");
    }
    return ntohs(connectionsin.sin_port);
}

/*------------------------------------------------------------------------------
# FUNCTION:   NewData
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int fd - file descriptor
#             bool flush - flush tag
#
# RETURNS:    true if new connection is found
#
# NOTES:		  Reads new data and sends data to the designated server and clients
#
------------------------------------------------------------------------------*/
void NewData(int fd, bool flush){
    struct sockaddr_in sin;
    socklen_t addrlen;
    int serverport,clientport;

    if(getsockname(fd, (struct sockaddr *)&sin, &addrlen) == -1){
        perror("getsockename");
    }

    int port = ntohs(sin.sin_port);
    printf("Received data from port: %d\n", port);

    for(serverport = 0; serverport < numofconnections; serverport++) {
        if(ClientList[serverport].ProxyRecvPort == port) {
            if(flush) {
                FlushOut(fd, ClientList[serverport].pipefd);
            } else {
                echo(fd, ClientList[serverport].server_sock, ClientList[serverport].pipefd);
            }
            break;
        }
    }

    for(clientport = 0; clientport < clientcount; clientport++) {
        if(ClientList[clientport].ProxySendPort == port) {
            if(flush) {
                FlushOut(fd, ClientList[clientport].pipefd);
            } else {
                echo(fd, ClientList[clientport].client_sock, ClientList[clientport].pipefd);
            }
            break;
        }
    }
}

/*------------------------------------------------------------------------------
# FUNCTION:   echo
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int recvfd - received file descriptor
#             int sendfd - sending file descriptor
#             int pipe[2] - pipe to be used
#
# RETURNS:    int
#
# NOTES:		  sends the data to the server and sends the data returned from the server
#             to the client
#
------------------------------------------------------------------------------*/
int echo(int recvfd, int sendfd, int pipe[2]){
    while(1){
        int nr = splice(recvfd, 0, pipe[1], 0, USHRT_MAX, SPLICE_F_MOVE | SPLICE_F_MORE | SPLICE_F_NONBLOCK);
        if(nr == -1 && errno != EAGAIN){
            perror("splice");
         }
        if(nr <= 0){
            break;
        }
        printf("read: %d \n", nr);
        do{
            int ret = splice(pipe[0], 0, sendfd, 0, nr, SPLICE_F_MOVE | SPLICE_F_MORE | SPLICE_F_NONBLOCK);
            if(ret <= 0){
                if(ret == -1 && errno != EAGAIN){
                    perror("splice2");
                }
                break;
            }
            printf("wrote: %d\n", ret);
            nr -= ret;
        }while(nr);
    }
    return 0;
}

/*------------------------------------------------------------------------------
# FUNCTION:   FlushOut
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int sendfd - send file descriptor
#             int pipe[2] - pipe
#
# RETURNS:    void
#
# NOTES:		  Reads whatever data that wasn't fully read from the previous splice
#
------------------------------------------------------------------------------*/
void FlushOut(int sendfd, int pipe[2]) {
    while(1) {
        int nr = splice (pipe[0], 0, sendfd, 0, USHRT_MAX, SPLICE_F_MOVE | SPLICE_F_MORE | SPLICE_F_NONBLOCK);
        if(nr <= 0){
            if(nr == -1 && errno != EAGAIN){
                perror("splice2");
            }
            break;
        }
        printf("wrote: %d\n", nr);
    }
}

/*------------------------------------------------------------------------------
# FUNCTION:   close_fd
#
# DATE:			  Apr 6, 2018
#
# DESIGNER:		Benedict Lo & Aing Ragunathan
# PROGRAMMER:	Benedict Lo & Aing Ragunathan
#
# PARAMETER:  int signo - signal to close
#
# RETURNS:    void
#
# NOTES:		  Closes all the sockets that were open
#
------------------------------------------------------------------------------*/
void close_fd (int signo) {
  //close all open sockets
    for(int i = 0; i < numofconnections; i++){
        close(listenarray[i]);
        printf("Closing listener socket %d \n",numofconnections);
    }
    for(int i = 0; i < clientcount; i++){
        close(ClientList[clientcount].client_sock);
        close(ClientList[clientcount].server_sock);
        printf("Closing listener socket %d \n",ClientList[clientcount].client_sock);
    }

    exit (EXIT_SUCCESS);
}
