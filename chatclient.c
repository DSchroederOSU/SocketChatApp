#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

//This program heavily references Beej's guide and I will point out when/where

struct addrinfo* createAddress(char* address, char* port){             //This function ia used to create the address information
    struct addrinfo hints;
    struct addrinfo *res;
    int status;

    memset(&hints, 0, sizeof hints);        //Taken right from Beej's guide, empty the struct
    hints.ai_family = AF_INET;                  //Used to specify version
    hints.ai_socktype = SOCK_STREAM;    //TCP stream sockets

    if((status = getaddrinfo(address, port, &hints, &res)) != 0){   //Also from Beej's guide. Print error if status isn't 0
        fprintf(stderr,
                "Error. Please enter the correct port.\n",
                gai_strerror(status));
        exit(1);
    }

    return res;
}


int createSocket(struct addrinfo* res){           //The next step in Beej's guide after creating the address info. We want to feed that into the socket
    int sockfd;                                                 //It's either going to return a socket descriptor to be used later or it will return -1 and exit with a error

    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
        fprintf(stderr, "Error. Socket was not created.\n");
        exit(1);
    }
    return sockfd;
}


void connectSocket(int sockfd, struct addrinfo * res){     //What we will use for the conneciton itself. Very similar to the creation
    int status;
    if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
        fprintf(stderr, "Error in connection.\n");
        exit(1);
    }
}


void chat(int sockfd, char * username, char * servername){  //This is what we'll call to be able to actually chat
    int bytes = 0;      //We'll use these to keep track of errors
    int status;

    char inputChars[500];                   //We need some character buffers to hold the input and output. We also need to make sure they're clear
    memset(inputChars, 0 ,sizeof(inputChars));

    char outputChars[500];
    memset(outputChars, 0, sizeof(outputChars));


    fgets(inputChars, 500, stdin);       //Make sure to clear stdin

    while(1){
        printf("%s> ", username);   //Per assignment specs, this is the prompt that will be displayed for typing messages
        fgets(inputChars, 500, stdin);

        if (strcmp(inputChars, "\\quit\n") == 0){    //Also must be able to type \quit and the application closes
            break;
        }

        bytes = send(sockfd, inputChars, strlen(inputChars), 0);  //See the number of bytes to make sure everything was sent properly. Use send per Beej's guide

        if(bytes == -1){
            fprintf(stderr, "Error. Data not sent properly.\n");
            exit(1);
        }

        status = recv(sockfd, outputChars, 500, 0);      //Make sure there were no errors when receiving (Beej's guide as well)

        if (status == -1){
            fprintf(stderr, "Error. Data not received properly\n");
            exit(1);
        }

        else if (status == 0){                              //Server ended the program
            printf("Program ended by server.\n");
            break;
        }
        else{                                                       //Everything was okay. Now, print the message
            printf("%s> %s\n", servername, outputChars);
        }

        memset(inputChars, 0, sizeof(inputChars));      //Remember to clear out the char buffer for the next run
        memset(outputChars, 0, sizeof(outputChars));
    }

    close(sockfd);                                  //We can't forget to close the connection
    printf("Connection is now closed.\n");
}


void infoSave(int sockfd, char* userName, char* serveName){	        //A helper function that saves the descriptors of the user and server
    int sendingcode = send(sockfd, userName, strlen(userName), 0);
    int receivingcode = recv(sockfd, serveName, 10, 0);
}


int main(int argc, char *argv[]){                     //Finally, where we'll call all of our functions to enable chatting          
    char userName[10];       //Store the user name
    char serveName[10];    //Store the server name
    if(argc != 3){                                                      //If user does not enter three arguments, error out and give them the usage statement
        fprintf(stderr, "Usage: ./chatClient [server] [port]\n");
        exit(1);
    }

    printf("Enter a username that is 10 characters or less.");
    scanf("%s", userName);

    struct addrinfo* res = createAddress(argv[1], argv[2]);   //Create the new addrinfo struct and pass the proper arguments from the command line

    int sockfd = createSocket(res);        //Create a new socket and pass the proper struct

    connectSocket(sockfd, res);        //Connect that socket


    infoSave(sockfd, userName, serveName);  //Save info to use

    chat(sockfd, userName, serveName);      //Actually call the chat function using all the information

    freeaddrinfo(res);          //Beej's guide. Finished with struct
}
