#include "../client.h"
#include "../library.h"

int main(int argc, char *argv[]){
    Client *client = new Client("127.0.0.1",7006);

    SetNonBlocking(client->GetSocket());

    while(1){
        char buffer[1024];

        printf("Enter message:");
        scanf("%s", buffer);
        send(client->GetSocket(), buffer, sizeof(buffer), 0);
    }
}
