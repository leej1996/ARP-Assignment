#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>

#define BUFFSIZE 8192
#define PORT 8080
int fd_G[2];
int fd_S[2];
int fd_L[2];

//client
void error(char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    //parent - P
    //Process P acts as the client and then
    //receives tokens, computes and sends an
    //updated token to the next machine
    int p_sockfd, p_portno, p_n;
    struct sockaddr_in p_serv_addr;
    struct timespec spec;

    char p_buffer[256];
    //p_portno = atoi(config_data[2][1]);
    p_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (p_sockfd < 0){
        error("ERROR opening socket");
    }

    bzero((char *) &p_serv_addr, sizeof(p_serv_addr));
    p_serv_addr.sin_family = AF_INET;
    //p_serv_addr.sin_addr.s_addr = config_data[2][0];
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &p_serv_addr.sin_addr)<=0) {
        error("Invalid address/Address not supported");
    }
    p_serv_addr.sin_port = htons(PORT);

    if (connect(p_sockfd, (struct sockaddr *) &p_serv_addr, sizeof(p_serv_addr)) < 0){
        error("ERROR connecting");
    }
    clock_gettime(CLOCK_REALTIME, &spec);

    printf("Please enter the message: ");
    bzero(p_buffer, 256);
    fgets(p_buffer,255,stdin);
    p_n = write(p_sockfd,p_buffer,strlen(p_buffer));
    if (p_n < 0){
        error("ERROR writing to socket");
    }
    bzero(p_buffer,256);
    p_n = read(p_sockfd,p_buffer,255);
    if (p_n < 0){
        error("ERROR reading from socket");
    }
    printf("%s\n",p_buffer);
    return 0;
}

