#include "server.h"

using namespace std;

Server::Server(int listenport){
    socklen_t adrlen;

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    ServerConfig(&servaddr, listenport);
    Reuse(listenfd);
    Bind(listenfd, &servaddr);
    Listen(listenfd,CONNECTIONS);

    //connectfd = Accept(listenfd, (struct sockaddr*)&adr, &adrlen);
}

int Server::GetSocket(){
    return connectfd;
}
