/*------------------------------------------------------------------------------
# SOURCE FILE: 		epoll.cpp
#
# PROGRAM:  		COMP8005 - Assignment 3
#
# FUNCTIONS:  int createEpollFd - creates an epoll fd
#             void addEpollSocket - adds a socket to epoll
#             int waitForEpollEvent - wait for an epoll event
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# NOTES:		Epoll wrappers
#
------------------------------------------------------------------------------*/
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "epoll.h"

#define MAX_EPOLL_EVENTS 64

/*------------------------------------------------------------------------------
# FUNCTIONS: createEpollFd
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# RETURNS: file descriptor that was created
#
# NOTES:		Creates an epoll file descriptor
#
------------------------------------------------------------------------------*/
int createEpollFd(){
  int fd;
  if ((fd = epoll_create1(0)) == -1) {
      perror("epoll_create1");
  }
  return fd;
}

/*------------------------------------------------------------------------------
# FUNCTIONS: createEpollFd
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: const int epollfd - epoll file descriptor
#            const int sock - socket being added to epoll
#            struct epoll_event *ev - an epoll event
#
# RETURNS: VOID
#
# NOTES:		Add a socket for epoll to listen to
#
------------------------------------------------------------------------------*/
void addEpollSocket(const int epollfd, const int sock, struct epoll_event *ev) {
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, ev) == -1) {
        perror("epoll_ctl");
    }
}

/*------------------------------------------------------------------------------
# FUNCTIONS: waitForEpollEvent
#
# DATE:			Apr 6, 2018
#
# DESIGNER:		Benedict Lo
# PROGRAMMER:	Benedict Lo
#
# PARAMETER: const int epollfd - epoll file descriptor
#            const int sock - socket being added to epoll
#            struct epoll_event *ev - an epoll event
#
# RETURNS: file descriptor with event
#
# NOTES:		Add a socket for epoll to listen to
#
------------------------------------------------------------------------------*/
int waitForEpollEvent(const int epollfd, struct epoll_event *events) {
    int ev;
    if ((ev = epoll_wait(epollfd, events, MAX_EPOLL_EVENTS, -1)) == -1) {
        if (errno == EINTR) {
            return 0;
        }
        perror("epoll_wait");
    }
    return ev;
}
