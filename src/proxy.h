#ifndef PROXY_H
#define PROXY_H

#define MAXEVENTS 64

typedef struct {
    int port;
    const char *ip;
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

#endif
