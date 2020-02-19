#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define BUFFSIZE 8192

//server
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char*argv[]){
    //acts as the server and will receive tokens
    //from the previous machine via socket and
    //dispatch them to P via pipe
    int g_sockfd, g_newsockfd, g_clilen, g_n;
    char g_buffer[BUFFSIZE];
    char g_token[BUFFSIZE];
    struct sockaddr_in g_serv_addr, g_cli_addr;
    struct timespec spec;
    char* pend;

    //create new socket
    g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sockfd < 0){
        error("ERROR opening socket");
    }

    //set values in buffer to 0
    bzero((char *) &g_serv_addr, sizeof(g_serv_addr));
    g_serv_addr.sin_family = AF_INET;
    g_serv_addr.sin_port = htons(PORT);
    g_serv_addr.sin_addr.s_addr = INADDR_ANY;

    //bind socket to address
    if (bind(g_sockfd, (struct sockaddr *) &g_serv_addr, sizeof(g_serv_addr)) < 0){
        error("ERROR on binding");
    }

    listen(g_sockfd,5);

    g_clilen = sizeof(g_cli_addr);
    g_newsockfd = accept(g_sockfd, (struct sockaddr *) &g_cli_addr, (socklen_t*)&g_clilen);
    if (g_newsockfd < 0){
        error("ERROR on accept");
    }

    bzero(g_buffer,256);
    //reading token from client through socket
    while (1){
        g_n = read(g_newsockfd, g_buffer, BUFFSIZE);
        if (g_n < 0){
            error("ERROR reading from socket");
        }
        //g_token = strtof(g_buffer, &pend);

        printf("This is what I receive: %s\n", g_buffer);
        //write(atoi(argv[2]), g_token,sizeof(message));  //write on the pipe to child2

        sleep(1);

    }

    return 0;
}
