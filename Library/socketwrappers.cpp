#include "socketwrappers.h"

using namespace std;

int Socket(int family, int type, int protocol){
    int sockfd;

    if((sockfd = (socket(family, type, protocol))) < 0){
        printf("socket error\n");
        return -1;
    }
    return sockfd;
}

int Bind(int socket, struct sockaddr_in *addr){
    int sockfd;

    if((sockfd = bind(socket,(struct sockaddr*)addr, sizeof(struct sockaddr_in))) < 0){
        perror("binding error");
        close(socket);
        return -1;
    }

    return sockfd;
}

bool Connect(int socket, struct sockaddr_in sockaddr){

    if(connect(socket, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1){
        printf("Client: can't connect to the server \n");
        return false;
    }

    printf("Client: Connected!\n");

    return true;
}

int Listen(int socket, int size){
    int sockfd;

    if ((sockfd = listen(socket,size)) < 0){
        printf("Listening error\n");
        close(socket);
        return -1;

    }

    return 0;
}

void SetNonBlocking(int socket){
    int sockfd = fcntl(socket, F_SETFL, O_NONBLOCK);
    if (sockfd < 0){
        perror("Error changing socket to non-blocking");
        exit(3);
    }
}

int Accept(int socket, struct sockaddr *addr, socklen_t *addrlen){
    int sockfd;

    printf("Waiting for connection\n");
    if((sockfd = accept(socket,addr, addrlen)) < 0){
        printf("Accept error");
        close(socket);
        return -1;
    }

    printf("Connection Accepted\n");
    return sockfd;
}

void ServerConfig(struct sockaddr_in *servaddr, int port){
    bzero((char*)servaddr, sizeof(struct sockaddr_in));
    servaddr->sin_family = AF_INET;
    servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr->sin_port = htons (port);
}

void ClientConfig(struct sockaddr_in *servaddr, const char* ip, int port){
    hostent* hp;

    if((hp = gethostbyname(ip)) == NULL){
        printf("Unknown server address %s \n", ip);
    }

    bzero((char*)servaddr, sizeof(struct sockaddr_in));
    servaddr->sin_family = AF_INET;
    servaddr->sin_port = htons (port);
    bcopy(hp->h_addr, (char*) &servaddr->sin_addr, hp->h_length);

}

void Reuse(int socket){
  int y;
  if(setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int)) == -1){
    perror("setsockopt");
  }
}
