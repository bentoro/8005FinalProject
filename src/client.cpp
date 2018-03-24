#include "client.h"

using namespace std;

Client::Client(const char* ip, int port) {
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);
    ClientConfig(&servaddr, ip, port);
    Connect(sockfd, servaddr);
}

int Client::GetSocket(){
    return sockfd;
}
