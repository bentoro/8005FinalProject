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


#define BUFLEN 1024
#define MAXEVENTS 64

typedef struct {
    struct sockaddr_in ip1;
    struct sockaddr_in ip2;
}Connection;

typedef struct {
    int port;
    int sock;
} server_info;

typedef struct {
    int ClientSendPort;
    int ProxyRecvPort;
    int ProxySendPort;
    int sock;
}client_info;

server_info *server_list;
client_info *ClientList;

void NewData(int fd);
void NewConnection(int socket, const int epollfd);
int GetConfig();
void ForwardTraffic();
int NewServerSock(char* ip, int port);

int numofconnections;
Connection *Connections;
int main(int argc, char *argv[]){
    int sock, epollfd;
    struct epoll_event event;
    struct epoll_event *events;

    epollfd = createEpollFd();
    numofconnections = GetConfig();


    for (int i = 0; i < numofconnections; i++){
        //listening for packets coming in at port 7005
        Server *server = new Server(Connections[i].ip1.sin_port);
        cout << "Listening socket " << server->GetSocket() << endl;
        sock = server->GetSocket();
        SetNonBlocking(sock);

        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = sock;
        addEpollSocket(epollfd, sock, &event);

        //make proxy connection to actual servers
        cout << "Setting proxy connection sockets" << endl;
        int proxy_sock = server_list[i].sock;
        SetNonBlocking(proxy_sock);

        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = sock;
        addEpollSocket(epollfd, sock, &event);
    }

    events =(epoll_event *) calloc(numofconnections, sizeof(event));

    while(1){
        int fds, i;
        fds = waitForEpollEvent(epollfd, events);

        for (i = 0; i < fds; i++){
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)){
                close(events[i].data.fd);
                free(events[i].data.ptr);
                continue;
            } else if((events[i].events & EPOLLIN)){
                if(events[i].data.fd == sock){
                    printf("Accepting connection");
                    //accept connection
                    NewConnection(events[i].data.fd, epollfd);
                } else {
                    printf("New data");
                    NewData(events[i].data.fd);
                }

            }
        }
    }
    close(epollfd);
    return 0;
}
void InitClientList(){

}


void ForwardTraffic(){

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
    server_list = (server_info*) calloc(numofconnections, sizeof(server_info));

    rewind(fp);
    int count = 0;
    while(count < lines){
        char ip[BUFLEN];
        int port;


        fscanf(fp, "%s %u", ip, &port);
        cout << ip << endl;
        cout << port << endl;

        server_list[count].port = port;
        server_list[count].sock = NewServerSock(ip, port);

        count++;
    }
    return lines;
}

int NewServerSock(char* ip, int port) {
    int sock;
    hostent* hp;
    struct sockaddr_in servaddr;

    sock = Socket(AF_INET, SOCK_STREAM, 0);
    ClientConfig(&servaddr, ip, port);

    return sock;
}


void NewConnection(int socket, const int epollfd){
    while(1){
        struct sockaddr_in addr;
        struct epoll_event event;
        socklen_t addrlen;
        int newfd;
        printf("Listening socket \n");
        //add all the new clients trying to connect
        if ((newfd = accept(socket, (struct sockaddr*)&addr, &addrlen)) == -1){
            if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
                //no more connections
                break;
            } else {
                perror("Accept error \n");
            }
        }
        SetNonBlocking(newfd);
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = newfd;
        //event.data.ptr = 
        addEpollSocket(epollfd, newfd, &event);
        printf("Adding a new client \n");
    }
}

void NewData(int fd){
    FILE *fp;
    char buffer[BUFLEN];
    int bytesread, write;

    if((fp = fopen("result","w")) == NULL){
        perror("File doesn't exist \n");
    }
    memset(buffer, '\0', BUFLEN);
    while((bytesread = read(fd, buffer,sizeof(buffer))) < 0){
        if((write = fwrite(buffer, 1, bytesread, fp)) < 0){
            perror("Write failed \n");
        }
    }
    cout << buffer << endl;
    fclose(fp);
}
