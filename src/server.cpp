#include "server.h"

using namespace std;

Server::Server(int listenport){

    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    connectfd = listenfd;
    ServerConfig(&servaddr, listenport);
    Reuse(listenfd);
    Bind(listenfd, &servaddr);
    Listen(listenfd,CONNECTIONS);
}
int Server::GetSocket(){
    return connectfd;
}
