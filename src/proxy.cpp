#include "server.h"
#include "client.h"
#include "library.h"
#include "epoll.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
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

struct Connection {
    char IP1;
    char IP2;
    char port1;
    char port2;
}

void NewData(int fd);
void NewConnection(int socket, const int epollfd);

int main(int argc, char *argv[]){
    int sock;
    struct epoll_event event;
    struct epoll_event *events;

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
