#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <math.h>
#include <signal.h>
#include<sys/wait.h>

#define BUFFSIZE 256

//pipes
int fd_G[2];
int fd_S[2];
int fd_L[2];

void P(char *config_data[][2]);

void L();

void error(const char *msg);

int main(int argc, char *argv[]){
    char *config_data[4][2];
    pid_t pid_G,pid_L,pid_S;
    int return_G, return_L, return_S;
    int fd, result;
    int i = 0;
    char buf[BUFFSIZE];
    char g_param[32], s_param[32];
    char *g_arg[3],*s_arg[3];
    //open config file
    fd = open("config_file", O_RDONLY);
    result = read(fd, buf, BUFFSIZE);

    if (result == -1) {
        error("ERROR: Read from config_file failed");
    }

    //remove the last \n character
    buf[result-2] = 0;

    //split config file into indnividual data
    //config_data[0][0] = IP of machine         	|config_data[0][1] = port number
    //config_data[1][0] = Reference frequency   	|config_data[1][1] = NULL
	//config_data[2][0] = Wait Time (microseconds)	|config_data[2][1] = NULL

    // Extract the first token
    // loop through the string to extract all other tokens
    char *config_string = strtok(buf, "\n");
    char *split_data;
    while(config_string != NULL) {
        config_data[i][0] = config_string;
        i++;
        config_string = strtok(NULL, "\n");
    }
    for (i = 0;i < 2; i++) {
        split_data = strtok(config_data[i][0], ":");
        config_data[i][0] = split_data;
        split_data = strtok(NULL, " ");
        config_data[i][1] = split_data;
    }

    printf("This program connects to other machines based off information\n");
    printf("in the readme. It will recieve tokens from the previous machine,\n");
    printf("calculate a new token and send it to the next machine.\n");
    printf("To control this process you must send signals to a process called S.\n");
    printf("This can be done by using the killall command.\n");
    printf("There are three modes for this process:\n");
    printf("1. Start recieving/sending tokens.\n");
    printf("    - Use SIGINT signal.\n");
    printf("2. Stop recieving/sending tokens.\n");
    printf("    - Use SIGUSR1 signal.\n");
    printf("3. Print log to terminal.\n");
    printf("    - Use SIGUSR2 signal.\n");

    //fork all processes
    if (pipe(fd_G) == -1){
        error("ERROR: Pipe to G Failed");
    }
    pid_G = fork();
    if (pid_G < 0){
        error("ERROR: Fork for G Faied");
        return 1;
    }else if (pid_G > 0){
        if (pipe(fd_L) == -1){
            error("ERROR: Pipe to L Failed");
        }
        //parent - P
        pid_L = fork();
        if (pid_L < 0) {
            error("ERROR: Fork for L Failed");
        } else if (pid_L > 1) {
            if (pipe(fd_S) == -1){
                error("ERROR: Pipe to S Failed");
            }
            //parent - P
            pid_S = fork();
            if (pid_S < 0) {
                error("ERROR: Fork for S Failed");
            } else if (pid_S > 1) {
                //parent - P
                P(config_data);
            } else {
                //child - S
                close(fd_S[0]);//close reading pipe
                sprintf(s_param, "%d", fd_S[1]);
                //char *s_cmd = "./S";  //executable name
                //strcpy(s_arg[0],"./S");
                s_arg[0] = (char *)"./S";
                s_arg[1] = s_param;
                s_arg[2] = NULL;
                execvp(s_arg[0], s_arg);
            }
        } else {
            //child -L
            L();
        }
    }else{
        //child - G
        //receive tokens
        //from P via socket and
        //dispatch them back to P via pipe
        //close(fd_G[0]);//close reading pipe

        char g_param2[32];
        //sprintf(g_param2, "%d", fd_G[0]);
        sprintf(g_param, "%d", fd_G[1]);
        g_arg[0] = (char *)"./G";
        g_arg[1] = g_param;
        g_arg[2] = NULL;
        execvp(g_arg[0], g_arg);
    }

    waitpid(pid_G, &return_G, 0);
    waitpid(pid_S, &return_S, 0);
    waitpid(pid_L, &return_L, 0);
}

void P(char *config_data[][2]){
    //Process P acts as the client and then
    //receives tokens via pipe, computes and sends an
    //updated token to the next machine via socket
    int p_sockfd, p_portno, n;
    struct sockaddr_in p_serv_addr;
    struct timespec rec_time, send_time, start_time, end_time;
    float old_token, new_token, time_diff = 0.0f, initial_val = 0.0f;
    float RF = (float)atof(config_data[1][0]);
	float WT = (float)atof(config_data[2][0]);
    char g_buffer[BUFFSIZE], l_buffer[BUFFSIZE], p_buffer[BUFFSIZE], temp[BUFFSIZE], s_buffer[BUFFSIZE];
    fd_set set;
    int maxfd;
    bool start = true;
    bool unconnected = true;
    bool cont_while = true;

    //set timeout for select function
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    //text for logging purposes
    char from_G[11] = "<from G> <";
    char from_S[11] = "<from S> <";
    char p_sent[11] = "<P sent> <";

    close(fd_G[1]);//close writing part of pipe
    close(fd_L[0]);//close reading part of pipe
    close(fd_S[1]);//close writing part of pipe

    //set up the socket
    p_portno = atoi(config_data[0][1]);
    p_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (p_sockfd < 0){
        error("ERROR: opening socket");
    }

    bzero((char *) &p_serv_addr, sizeof(p_serv_addr));
    p_serv_addr.sin_family = AF_INET;
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, config_data[0][0], &p_serv_addr.sin_addr)<=0) {
        error("ERROR: Invalid address/Address not supported");
    }
    p_serv_addr.sin_port = htons(p_portno);

    clock_gettime(CLOCK_REALTIME, &start_time);

    printf("This is the IP I am trying to connect to: %s\n", config_data[0][0]);

    do {
        if (connect(p_sockfd, (struct sockaddr *) &p_serv_addr, sizeof(p_serv_addr)) < 0){
        } else {
            unconnected = false;
            printf("I have connected!\n");
        }
        clock_gettime(CLOCK_REALTIME, &end_time);
        time_diff = (float)(((end_time.tv_sec - start_time.tv_sec) * 1000000000 + (end_time.tv_nsec - start_time.tv_nsec))/1000000000.0);
        if (time_diff > 10.0 || !unconnected){
            cont_while = false;
        }
    } while (cont_while);
    if (unconnected) {
        error("ERROR: could not connect to G\n");
    }

    sprintf(p_buffer,"%f",initial_val);
    //send new token
    n = write(p_sockfd,&p_buffer,BUFFSIZE);
    printf("I am writing %s\n", p_buffer);
    memset(p_buffer,0,BUFFSIZE);
    while (1){
        FD_ZERO(&set);
        FD_SET(fd_G[0], &set);
        FD_SET(fd_S[0], &set);
        maxfd = fd_G[0] > fd_S[0] ? fd_G[0] : fd_S[0];
        select(maxfd+1, &set, NULL, NULL, &tv); //2 seconds before timeout
        if (FD_ISSET(fd_S[0], &set)) {
            // We can read from S
            printf("We are in S\n");
            read(fd_S[0], &s_buffer, BUFFSIZE);
            //send message to log process
            memset(temp,0,BUFFSIZE);
            strcat(temp, from_S);
            if (atoi(s_buffer) == 1){
                start = true;
                //send message to log process
                strcat(temp, "Start recieving/sending tokens");
            } else if (atoi(s_buffer) == 2){
                start = false;
                //send message to log process
                strcat(temp, "Stop recieving/sending tokens");
            } else{
                strcat(temp, "Printing log to terminal");
            }
            strncpy(l_buffer, temp, BUFFSIZE);
            write(fd_L[1], &l_buffer, BUFFSIZE);

            //empty buffers
            memset(l_buffer,0,BUFFSIZE);
            memset(s_buffer,0,BUFFSIZE);
        } else if (FD_ISSET(fd_G[0], &set) && start) {
            // We can read from G
            printf("We are in G\n");
            //get token from Gn process
            read(fd_G[0], g_buffer, BUFFSIZE);
            //time that we recieved token
            clock_gettime(CLOCK_REALTIME, &rec_time);
			
			printf("P reads %s from G\n", g_buffer);

            //send message to log process
            memset(temp,0,strlen(temp));
            strcat(temp, from_G);
            strcat(temp, g_buffer);
            strncpy(l_buffer, temp, BUFFSIZE);
            write(fd_L[1], &l_buffer, BUFFSIZE);
			
			//apply wait time
			usleep(WT);

            //empty buffer
            memset(l_buffer,0,BUFFSIZE);

            //calculate new token
            old_token = atof(g_buffer);
            clock_gettime(CLOCK_REALTIME, &send_time);
            time_diff = (float)(WT/1000000 + ((send_time.tv_sec - rec_time.tv_sec) * 1000000000 + (send_time.tv_nsec - rec_time.tv_nsec))/1000000000.0);
            new_token = old_token + time_diff*(1 - pow(old_token,2)/2)*2*M_PI*RF;
            printf("time diff: %f\n", time_diff);
            sprintf(p_buffer,"%.10f",new_token);

            //send new token
            printf("This is what we are sending: %s\n", p_buffer);
            n = write(p_sockfd, &p_buffer, BUFFSIZE);
            if (n < 0) error("ERROR writing to socket");

            //send new token to log
            memset(temp,0,strlen(temp));
            strcat(temp, p_sent);
            strcat(temp, p_buffer);
            strncpy(l_buffer, temp, BUFFSIZE);
            write(fd_L[1], &l_buffer, BUFFSIZE);

            //empty buffers
            memset(g_buffer,0,BUFFSIZE);
            memset(p_buffer,0,BUFFSIZE);
            memset(l_buffer,0,BUFFSIZE);
        }
        sleep(1);
    }
}

void L(){
    //prints a line to the log file whenever P
    //receives a value from S|G or sends a token
    close(fd_L[1]);
    FILE * log;
    char buffer[BUFFSIZE];
    time_t rawtime;
    struct tm *ptm;

    //clear log file
    log = fopen("posix.log", "w");
    fclose(log);

    while (1) {
        log = fopen("posix.log", "a+");
        read(fd_L[0], buffer, BUFFSIZE);
        if (log == NULL){
            error("ERROR: log was not created properly");
        }
        rawtime = time(NULL);
        if (rawtime == -1) {
            error("ERROR: time() function failed");
        }
        ptm = localtime(&rawtime);
        if (ptm == NULL) {
            error("ERROR: localtime() function failed");
        }
        fprintf(log,"<%02d:%02d:%02d> %s>\n",
                ptm->tm_hour, ptm->tm_min, ptm->tm_sec, buffer);
        fclose(log);
    }
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}
