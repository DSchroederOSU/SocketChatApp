#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

// allow for string term character
// we are also putting the length as a 4 byte int
#define MESSAGE_SIZE 505

struct msgstruct {
    uint32_t length;
    char send_data[500];
};


//This function ia used to create the address information
struct addrinfo* getServerAddress(char* address, char* port){
    struct addrinfo myAddress;
    struct addrinfo *server;
    int status;

    // Beej's guide for creating addrinfo struct
    memset(&myAddress, 0, sizeof myAddress);
    myAddress.ai_family = AF_INET;
    myAddress.ai_socktype = SOCK_STREAM;
    myAddress.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(address, port, &myAddress, &server)) != 0){
        fprintf(stderr,
                "Error. Please enter the correct port. %s\n",
                gai_strerror(status));
        exit(1);
    }
    return server;
}

// Create a socket with the address fields provided in the command args
int createSocket(struct addrinfo* server){
    int sockfd;

    // create socket from address or error out
    if ((sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol)) == -1){
        fprintf(stderr, "Error. Socket was not created.\n");
        exit(1);
    }
    return sockfd;
}

// Error check the connection of the socket created
void connectSocket(int socket_fd, struct addrinfo * server){
    int status;
    if ((status = connect(socket_fd, server->ai_addr, server->ai_addrlen)) == -1){
        fprintf(stderr, "Error in connection.\n");
        exit(1);
    }
}

// this function acts as a handshake between the client and server
void establishTCP(int socket_fd, char* clientName, char* serveName){
    int numbytes;
    if ((numbytes =  send(socket_fd, clientName, strlen(clientName), 0)) == -1) {
        perror("send");
        exit(1);
    }
    if ((numbytes = recv(socket_fd, serveName, 10, 0)) == -1) {
        perror("recv");
        exit(1);
    }
}

void beginChat(int sockfd, char * username, char * servername) {  //This is what we'll call to be able to actually chat
    // status of message sent
    int bytes = 0;
    // status of message received
    int server_message;
    char message[MESSAGE_SIZE+4];
    memset(message, 0 ,sizeof(message));
    char input_string[MESSAGE_SIZE+1];
    memset(input_string, 0 ,sizeof(input_string));
    char received[MESSAGE_SIZE];
    memset(received, 0, sizeof(received));

    // flush out the console standard in
    fgets(message, MESSAGE_SIZE, stdin);

    while(1){
        // print out client handle and get message from keyboard input
        printf("%s> ", username);
        fgets(input_string, MESSAGE_SIZE, stdin);

        // check for termination statement
        if (strcmp(input_string, "\\quit\n") == 0){
            break;
        }

        int32_t conv = htonl((int) strlen(input_string));
        char *data = (char*)&conv;
        int to_send = sizeof(conv);

        // send the header first with the amount of bytes to read
        bytes = send(sockfd, data, to_send, 0);
        bytes = send(sockfd, input_string, strlen(input_string), 0);

        if(bytes == -1){
            perror("send");
            exit(1);
        }

        int server_message = recv(sockfd, received, MESSAGE_SIZE, 0);

        // handle all cases for send()
        if (server_message == -1){
            perror("recv");
            exit(1);
        }
        else if (server_message == 0){
            printf("Server ended the connection.\n");
            break;
        }
        else{
            // message was received, so print
            printf("%s> %s\n", servername, received);
        }

        // clear out the buffers
        memset(message, 0, sizeof(message));
        memset(received, 0, sizeof(received));
    }

    // close the socket
    close(sockfd);
    printf("Connection is now closed.\n");
}


int main(int argc, char *argv[]){
    char clientName[10];
    char serveName[10];
    // check command line args
    if(argc != 3){
        fprintf(stderr, "Usage: ./chatClient [server] [port]\n");
        exit(1);
    }

    // prompt for user handle
    printf("Enter a handle (0-10 Chars):\n");
    scanf("%s", clientName);

    // get command line arguments
    char* address = argv[1];
    char* port = argv[2];

    // call address creation function
    struct addrinfo* server = getServerAddress(address, port);

    // Create a socket with the address fields provided in the command args
    int socket_fd = createSocket(server);

    // Create a socket connection if creation was successful
    connectSocket(socket_fd, server);

    // make a "handshake"
    establishTCP(socket_fd, clientName, serveName);

    // begin the chatting loop function
    beginChat(socket_fd, clientName, serveName);

    freeaddrinfo(server);
}