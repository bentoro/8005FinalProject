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
    struct sockaddr_in servaddr;
} server_info;

typedef struct {
    int ClientSendPort;
    int ProxyRecvPort;
    int ProxySendPort;
    int sock;
}client_info;

client_info *ClientList;
server_info *server_list;

void NewData(int fd);
void NewConnection(int socket, const int epollfd);
int GetConfig();
void ForwardTraffic();
int NewServerSock(char* ip, int port);

int numofconnections;
int clientcount;
int sendport;
Connection *Connections;

int main(int argc, char *argv[]){
    int epollfd, listensock;
    struct epoll_event event;
    struct epoll_event *events;
    clientcount = 0;
    sendport = 8000;
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
    }

    events =(epoll_event *) calloc(MAXEVENTS, sizeof(event));

    for(int j = 0; j < numofconnections; j++){

        //make proxy connection to actual servers
        cout << "Setting proxy connection sockets" << endl;
        int proxy_sock = server_list[j].sock;
        SetNonBlocking(proxy_sock);
        event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
        event.data.fd = proxy_sock;
        addEpollSocket(epollfd, server_list[j].sock, &event);
        Connect(server_list[j].sock, server_list[j].servaddr);
    }

    while(1){
        int fds, i;
        fds = waitForEpollEvent(epollfd, events);

        for (i = 0; i < fds; i++){
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)){
                close(events[i].data.fd);
                continue;
            } else if((events[i].events & EPOLLIN)){
                if(events[i].data.fd == listensock){
                    printf("Accepting connection");
                    //accept connection
                    NewConnection(events[i].data.fd, epollfd);
                } else {
                    printf("New data \n");
                    NewData(events[i].data.fd);
                }

            }
        }
    }
    close(epollfd);
    return 0;
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
        int proxy_sock;

        fscanf(fp, "%s %u", ip, &port);
        cout << ip << endl;
        cout << port << endl;

        server_list[count].port = port;
        proxy_sock = Socket(AF_INET, SOCK_STREAM, 0);
        ClientConfig(&server_list[count].servaddr, ip, port);
        server_list[count].sock = proxy_sock;

        count++;
    }

    return lines;
}

int NewServerSock(char* ip, int port) {
    int sock;
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
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if(getsockname(socket, (struct sockaddr *)&sin, &len) == -1){
            perror("getsockename");
        }
        ClientList[clientcount].ProxySendPort = sendport;
        ClientList[clientcount].ProxyRecvPort = ntohs(sin.sin_port);
        ClientList[clientcount].sock = newfd;
        SetNonBlocking(newfd);
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = newfd;
        addEpollSocket(epollfd, newfd, &event);
        printf("Adding a new client \n");
        clientcount++;
        sendport++;
    }
}

void NewData(int fd){
    FILE *fp;
    char buffer[BUFLEN];
    int bytesread, write;
    struct sockaddr_in sin;
    socklen_t addrlen;
    int byteswrote;

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

    if(getsockname(fd, (struct sockaddr *)&sin, &addrlen) == -1){
        perror("getsockename");
    }

    int port = ntohs(sin.sin_port);
    printf("Received data from port: %d\n", port);

    for(int i = 0; i < numofconnections; i++) {
        if(server_list[i].port == port) {
            byteswrote = send(server_list[i].sock, buffer, sizeof(buffer), 0);
            printf("Sent to server: %d\n", byteswrote);
            break;
        }
    }

    for(int i = 0; i < clientcount; i++) {
        if(ClientList[i].ProxyRecvPort == port) {
            byteswrote = send(ClientList[i].sock, buffer, sizeof(buffer), 0);
            printf("Sent to client: %d\n", byteswrote);
            break;
        }
    }

    fclose(fp);
}
