#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

int createEpollFd();
void addEpollSocket(const int epollfd, const int sock, struct epoll_event *ev);
int waitForEpollEvent(const int epollfd, struct epoll_event *events);
#endif
