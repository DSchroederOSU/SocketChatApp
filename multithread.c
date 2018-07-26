/*
** clientserve.c -- a stream socket server demo
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>

#define MYPORT 3491    // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold
#define MESSAGE_SIZE 500

char * getMessage();
char * promptHandle();
struct sockaddr_in initializeAddress();
int initSocket(struct sockaddr_in);
void *writeHelper(void *threadarg);
void *readHelper(void *threadarg);
int sockfd;
int clientfd;

struct reader_data{
    int  file_descriptor;
};

struct writer_data{
    int  file_descriptor;
    char handle[10];
};

int handle_length;

void sigchld_handler(int s)
{
    while(wait(NULL) > 0);
}
void beginChat(struct sockaddr_in client_address);

int main(void)
{
    /*
     * initialize our server's address information with:
     * PORT: MYPORT MACRO
     * address: INADDR_ANY (0.0.0.0)
     */
    struct sockaddr_in server_address;
    server_address = initializeAddress();    // my address information

    // we will get this information upon connection
    struct sockaddr_in client_address;

    // change the action taken by a process on receipt of a specific signal
    struct sigaction sa;

    /*---Open socket for streaming---*/
    //https://www.cs.utah.edu/~swalton/listings/sockets/programs/part1/chap1/simple-client.c
    //return file descriptor on success
    sockfd = initSocket(server_address);

    // used to print out information
    int len=20;
    char buffer[len];
    inet_ntop(AF_INET, &(server_address.sin_addr), buffer, len);
    printf("address:%s\n",buffer);


    // begin listening
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // allows the calling process to examine and/or specify the action to be associated with a specific signal
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // begin chat server
    beginChat(client_address);

    return 0;
}



void beginChat(struct sockaddr_in client_address){
    // this is the size of a (sockaddr_in struct)
    // it is needed for the socket "accept()" method
    socklen_t sin_size;
    sin_size = sizeof(struct sockaddr_in);
    char buf[MESSAGE_SIZE];

    // handle error while accepting new connection
    if ((clientfd = accept(sockfd, (struct sockaddr *)&client_address,
                         &sin_size)) == -1) {
        perror("accept");

    }
    // log connection success
    printf("SERVER: Got connection from %s\n\nBEGINNING CHAT\n--------------------\n\n",
           inet_ntoa(client_address.sin_addr));

    char * h = promptHandle();
    h = strcat(h, "> ");
    handle_length = strlen(h);


    pthread_t writer_thread;
    pthread_t reader_thread;

    // handle client connection in new child process
    if (!fork()) { // this is the child process
        close(sockfd); // child doesn't need the listener

        pthread_create(&writer_thread, NULL, writeHelper,
                            (void *) &h);
        pthread_create(&reader_thread, NULL, readHelper, NULL);

        pthread_join(writer_thread, NULL);
        pthread_join(reader_thread, NULL);
    }


}
void *writeHelper(void *threadarg)
{
    while(1)
    {
        char buf[MESSAGE_SIZE];
        memset(buf, '\0', MESSAGE_SIZE);
        char message[MESSAGE_SIZE];
        memset(message, '\0', MESSAGE_SIZE);


        char handle[10];
        strcpy(handle, threadarg);
        // message and add handle
        memset(message, '\0', MESSAGE_SIZE);
        memset(buf, '\0', MESSAGE_SIZE);
        strcpy(message, handle);
        printf("\n%s", handle);
        fgets(buf, MESSAGE_SIZE, stdin);
        strcat(message, buf);
        // handle error on send message
        if (send(clientfd, message, strlen(message), 0) < 0) {
            perror("send");
            pthread_exit(NULL);
            exit(0);
        }
    }
    pthread_exit(NULL);
}
void *readHelper(void *threadarg)
{
    char received[MESSAGE_SIZE];
    memset(received, '\0', MESSAGE_SIZE);
    while(1)
    {
        if (recv(clientfd, received, 255, 0) < 0)
        {
            perror("\nClient Error: Reading from Server");
            exit(0);
        } else{
            printf("\033[31m"); //change incoming color to red
            printf("\n%s\033[0m", received); // change text back to black
            fflush(stdout);
            memset(received, '\0', MESSAGE_SIZE);
        }
    }
    pthread_exit(NULL);
}
/*
 * taken from: https://stackoverflow.com/questions/314401/how-to-read-a-line-from-the-console-in-c
 * A simple stdin input reader
 * terminated with ENTER
 */
char * getMessage(){
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}

char * promptHandle(){
    printf("Enter a handle:\n");
    char * line = malloc(10), * linep = line;
    size_t lenmax = 10, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }

    *line = '\0';
    // strip new line from console ENTER
    linep[strcspn(linep, "\n")] = '\0';
    return linep;
}

struct sockaddr_in initializeAddress(){
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;         // host byte order, a code for the address family.
    server_address.sin_port = htons(MYPORT);     // short, network byte order
    server_address.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
    memset(&(server_address.sin_zero), '\0', 8); // zero the rest of the struct
    return server_address;
}

int initSocket(struct sockaddr_in server_address){
    int yes=1;
    int socket_fd;

    if ( (socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Socket");
        exit(errno);
    }

    // Specifies that the rules used in validating addresses supplied to bind()
    // should allow reuse of local addresses
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }

    // assigns the address specified by server_address to the socket referred to by the file descriptor sockfd
    if (bind(socket_fd, (struct sockaddr *)&server_address, sizeof(struct sockaddr))
        == -1) {
        perror("bind");
        exit(1);
    }

    return socket_fd;
}