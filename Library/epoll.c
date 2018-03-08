#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "epoll.h"
#include "main.h"

int createEpollFd(){
  int fd;
  if ((efd = epoll_create1(0)) == -1) {
      perror("epoll_create1");
  }
  return efd;
}
}

void addEpollSocket(const int epollfd, const int sock, struct epoll_event *ev) {
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, ev) == -1) {
        perror("epoll_ctl");
    }
}

int waitForEpollEvent(const int epollfd, struct epoll_event *events) {
    int ev;
    if ((ev = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1)) == -1) {
        if (errno == EINTR) {
            //Interrupted by signal, ignore it
            return 0;
        }
        perror("epoll_wait");
    }
    return ev;
}
