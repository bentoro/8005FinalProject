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
    struct sockaddr_in *ip1;
    struct sockaddr_in *ip2;
}Connection;

void NewData(int fd);
void NewConnection(int socket, const int epollfd);
void GetConfig();

int numofconnections;
Connection *Connections;
int main(int argc, char *argv[]){
    int sock;
    struct epoll_event event;
    struct epoll_event *events;

    GetConfig();
    /*
    int countconfig=0;
    for (int k=0; k < numofconnections; k++){
      COnnections[k] = Init_connection(k, countconfig);

      bzero((char*)Connections[k].ip1, sizeof(struct sockaddr_in));
      hostent* hp;
      if((hp = gethostbyname(config[countconfig].c_str())) == NULL){
        perror("gethostbyname");
      }
      bcopy(hp->h_addr, (char*) &Connections[k].ip1->sin_addr, hp->h_length);
      Connections[k].ip1->sin_family = AF_INET;
      Connections[k].ip1->sin_port = htons (atoi(config[countconfig+1].c_str()));


      countconfig=countconfig+4;
    }*/

    Server *server = new Server(7005);
    cout << "Listening socket" << server->GetSocket() << endl;
    sock = server->GetSocket();
    SetNonBlocking(sock);

    event.events = EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLET;
    event.data.fd = sock;
    int epollfd = createEpollFd();
    addEpollSocket(epollfd, sock, &event);

    events =(epoll_event *) calloc(MAXEVENTS, sizeof(event));

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
/*
void Init_connection(string ip1, string ip2, string port1, string port2){
  bzero((char*)Connections[k].ip1, sizeof(struct sockaddr_in));
  hostent* hp;
  if((hp = gethostbyname(config[countconfig].c_str())) == NULL){
    perror("gethostbyname");
  }
  bcopy(hp->h_addr, (char*) &Connections[k].ip1->sin_addr, hp->h_length);
  Connections[k].ip1->sin_family = AF_INET;
  Connections[k].ip1->sin_port = htons (atoi(config[countconfig+1].c_str()));
}*/
void GetConfig(){
  //initialize after every line
	/*ifstream file;
     	file.open("config");
     	//check if file exists
        if (!file){
        	perror("Files does not exist");
       	} else {
                while(!file.eof()){
                  string line;
                	getline(file, line);
                  cout << line << endl;
                  cout << line[0] << endl;
                  cout << line[1] << endl;
                  cout << line[2] << endl;
                  cout << line[3] << endl;
                  numofconnections++;
                }
        }
        numofconnections=numofconnections/2;
        numofconnections=numofconnections/2;*/
        FILE *fp;
        fp = fopen("config","r");
        int c=0;
        int lines;
        while(!feof(fp)){
            c=fgetc(fp);
            if(c == '\n'){
                lines++;
            }
        }
        cout << lines << endl;
        Connections = (Connection *)calloc(lines,sizeof(Connection));
        rewind(fp);
        int count;
        while(!feof(fp)){
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
                perror("gethostbyname");
            }
            bzero((char*)Connections[count].ip1, sizeof(struct sockaddr_in));
            bcopy(hp->h_addr, (char*) &Connections[count].ip1->sin_addr, hp->h_length);
            Connections[count].ip1->sin_family = AF_INET;
            Connections[count].ip1->sin_port = htons (port1);

            hostent* hp1;
            if((hp1 = gethostbyname(ip1)) == NULL){
                perror("gethostbyname");
            }
            bzero((char*)Connections[count].ip2, sizeof(struct sockaddr_in));
            Connections[count].ip2->sin_family = AF_INET;
            Connections[count].ip2->sin_port = htons (port2);

            count++;
        }
}

void NewConnection(int socket, const int epollfd){
  while(1){
    struct sockaddr_in addr;
    struct epoll_event event;
    socklen_t addrlen;
    int newfd;
    printf("Listening socket");
    //add all the new clients trying to connect
    if ((newfd = accept(socket, (struct sockaddr*)&addr, &addrlen)) == -1){
      if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
        //no more connections
        break;
      } else {
        perror("Accept error");
      }
    }
    SetNonBlocking(newfd);
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = newfd;
    addEpollSocket(epollfd, newfd, &event);
    printf("Adding a new client");
  }
}

void NewData(int fd){
  FILE *fp;
  char buffer[BUFLEN];
  int bytesread, write;

  if((fp = fopen("result","w")) == NULL){
    perror("File doesn't exist");
  }
  memset(buffer, '\0', BUFLEN);
  while((bytesread = read(fd, buffer,sizeof(buffer))) < 0){
    if((write = fwrite(buffer, 1, bytesread, fp)) < 0){
      perror("Write failed");
    }
  }
  cout << buffer << endl;
  fclose(fp);
}
