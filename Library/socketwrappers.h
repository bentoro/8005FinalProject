#ifndef SOCKETWRAPPERS_H
#define SOCKETWRAPPERS_H

#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFLEN 1024

int Socket(int family, int type, int protocol);
int Bind(int socket, struct sockaddr_in *addr);
bool Connect(int socket, struct sockaddr_in sockaddr);
int Listen(int socket, int size);
void SetNonBlocking(int socket);
int Accept(int socket, struct sockaddr *addr, socklen_t *addrlen);
void ServerConfig(struct sockaddr_in *servaddr, int port);
void ClientConfig(struct sockaddr_in *servaddr, const char* ip, int port);
void Reuse (int socket);
#endif
