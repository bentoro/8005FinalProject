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
    int ProxyRecvPort;
    int ProxySendPort;
    int sock;
}client_info;

client_info **ClientList;

void NewData(int fd);
void NewConnection(int socket, const int epollfd);
int GetConfig();
void ForwardTraffic();

int numofconnections;
int clientcount;
int sendport;
Connection *Connections;
int main(int argc, char *argv[]){
    int sock, epollfd;
    struct epoll_event event;
    struct epoll_event *events;
    clientcount = 0;
    sendport = 8000;
    epollfd = createEpollFd();
    ClientList =(client_info **)calloc(BUFLEN, sizeof(client_info));
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
        Connections = (Connection *)calloc(lines, sizeof(Connection));
        rewind(fp);
        int count = 0;
        while(count < lines){
            char ip1[BUFLEN];
            int port1;
            char ip2[BUFLEN];
            int port2;
            fscanf(fp, "%s %u %s %u", ip1, &port1, ip2, &port2);
            cout << ip1 << endl;
            cout << port1 << endl;
            cout << ip2 << endl;
            cout << port2 << endl;

            hostent* hp;
            if((hp = gethostbyname(ip1)) == NULL){
                perror("gethostbyname error");
            }
            Connections[count].ip1.sin_family = AF_INET;
            Connections[count].ip1.sin_port = htons (port1);
            bcopy(hp->h_addr, (char*) &Connections[count].ip1.sin_addr, hp->h_length);

            hostent* hp1;
            if((hp1 = gethostbyname(ip1)) == NULL){
                perror("gethostbyname");
            }

            bcopy(hp1->h_addr, (char*) &Connections[count].ip2.sin_addr, hp1->h_length);
            Connections[count].ip2.sin_family = AF_INET;
            Connections[count].ip2.sin_port = htons (port2);
            count++;
        }
    return lines;
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
    client_info *clientptr = ClientList[clientcount];
    clientptr->ProxySendPort = sendport;
    clientptr->ProxyRecvPort = ntohs(sin.sin_port);
    clientptr->sock = newfd;
    SetNonBlocking(newfd);
    event.events = EPOLLIN | EPOLLET;
    event.data.ptr = clientptr;
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
