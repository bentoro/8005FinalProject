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

#define MAXEVENTS 64

typedef struct {
    int port;
    const char *ip;
    //int sock;
} server_info;

typedef struct {
    int ProxyRecvPort;
    int ProxySendPort;
    int pipefd[2];
    int client_sock;
    int server_sock;
}client_info;

client_info *ClientList;
server_info *server_list;

void NewData(int fd, bool flush);
void NewConnection(int socket, const int epollfd);
int GetConfig();
void ForwardTraffic();
bool newConnectionFound(int fd, int epollfd);
void close_fd (int signo);
int echo(int recvfd, int sendfd, int pipe[2]);
void FlushOut(int fd, int pipe[2]);
int GetPort(int socket);

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
/*
    for(int j = 0; j < numofconnections; j++){

        //make proxy connection to actual servers
        cout << "Setting proxy connection sockets" << endl;
        int proxy_sock = server_list[j].sock;
        pipe(server_list[j].pipefd);
        SetNonBlocking(proxy_sock);
        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = proxy_sock;
        addEpollSocket(epollfd, server_list[j].sock, &event);
        Connect(server_list[j].sock, server_list[j].servaddr);
    }
*/

    while(1){
        int fds, i;
        fds = waitForEpollEvent(epollfd, events);

        for (i = 0; i < fds; i++){
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)){
                close(events[i].data.fd);
                continue;
            } else if((events[i].events & EPOLLIN)){
                /*for (int k = 0; k < numofconnections; k++) {
                    if(events[i].data.fd == listenarray[k]){
                        printf("New connection \n");
                        NewConnection(events[i].data.fd, epollfd);
                        break;
                    }
                }*/
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

bool newConnectionFound(int fd, int epollfd) {
    /*for(int i = 0; i < numofconnections; i++) {
        if(ev->data.fd == server_list[i].sock){
            printf("Accepting Server connection");
            NewConnection(ev->data.fd, epollfd);

            return true;
        }
    }*/

    for (int i = 0; i < numofconnections; i++) {
        if(fd == listenarray[i]){
            printf("Accepting Client connection \n");
            NewConnection(fd, epollfd);

            return true;
        }
    }
    return false;
}

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
        //proxy_sock = Socket(AF_INET, SOCK_STREAM, 0);
        server_list[count].ip = ip;
        //ClientConfig(&server_list[count].servaddr, ip, port);
        //server_list[count].sock = proxy_sock;

        count++;
    }

    return lines;
}

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

        //Creating a new connection to the server at "sendport"
    //    int server_sock = Socket(AF_INET, SOCK_STREAM, 0);  //socket going to the server
     //   struct sockaddr_in connection_addr; //establishing the connection with the new socket
        Client *proxy_connection;

        //Connect to epoll
        for(int i = 0; i < numofconnections; i++) {
            if(server_list[i].port == ntohs(sin.sin_port)) {
                proxy_connection = new Client(server_list[i].ip, ntohs(sin.sin_port));
                //ClientConfig(&connection_addr, server_list[i].ip, ntohs(sin.sin_port)); //connection to server
                break;
            }
        }

//        Connect(server_sock, connection_addr);
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

int GetPort(int socket){
    struct sockaddr_in connectionsin;
    socklen_t connection_len = sizeof(connectionsin);
    if(getsockname(socket, (struct sockaddr *)&connectionsin, &connection_len) == -1){
        perror("getsockename");
    }
    return ntohs(connectionsin.sin_port);
}

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
                //printf("Sent to server: %d\n", byteswrote);
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
                //printf("Sent to server: %d\n", byteswrote);
                echo(fd, ClientList[clientport].client_sock, ClientList[clientport].pipefd);
            }
            break;
        }
    }
}

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


void close_fd (int signo)
{
    for(int i = 0; i < numofconnections; i++){
        close(listenarray[i]);
        printf("Closing listener socket %d \n",numofconnections);
    }
/*
    for(int i = 0; i < numofconnections; i++){
        close(server_list[i].sock);
        printf("Closing server socket %d \n",numofconnections);
    }
*/
    for(int i = 0; i < clientcount; i++){
        close(ClientList[clientcount].client_sock);
        close(ClientList[clientcount].server_sock);
        printf("Closing listener socket %d \n",ClientList[clientcount].client_sock);
    }

    exit (EXIT_SUCCESS);
}
