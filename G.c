#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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

    char g_buffer_read[BUFFSIZE];
	char g_buffer_write[BUFFSIZE];
    int g_sockfd, g_newsockfd, g_clilen, g_n;
    struct sockaddr_in g_serv_addr, g_cli_addr;


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

	printf("G: We are listening\n");
    listen(g_sockfd,5);

    g_clilen = sizeof(g_cli_addr);
    g_newsockfd = accept(g_sockfd, (struct sockaddr *) &g_cli_addr, (socklen_t*)&g_clilen);
    if (g_newsockfd < 0){
        error("ERROR on accept");
    }
    bzero(g_buffer_read,BUFFSIZE);
    //reading token from client through socket

    //initialize and send token
    strncpy(g_buffer_write,"0", BUFFSIZE);
    g_n = write(atoi(argv[1]), &g_buffer_write, BUFFSIZE);
    if (g_n < 0){
        error("ERROR writing to pipe");
    }
    while (1){
        //read token from socket
        g_n = read(g_newsockfd, &g_buffer_read, BUFFSIZE);
        if (g_n < 0){
            error("ERROR reading from socket");
        }
        printf("G: We are reading: %s\n", g_buffer_read);
		
		strncpy(g_buffer_write,g_buffer_read, BUFFSIZE);
        //send token to P via pipe
        g_n = write(atoi(argv[1]), &g_buffer_write, BUFFSIZE);
        if (g_n < 0){
            error("ERROR writing to pipe");
        }
        printf("G: We write %s to P.\n", g_buffer_write);
        //sending token to P via pipe

    }

    return 0;
}
