#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFSIZE 8192

int fd[2];
static int alarm_fired = 0;

void error(const char *msg);

void sig_handler(int signo);

void print_log();

int main(int argc, char *argv[]){
    char buffer[BUFFSIZE];
    //recieves signals from terminal and will
    //send commands to P accordingly.
    if (signal(SIGINT, sig_handler) == SIG_ERR){
        error("ERROR: SIGINT signal handling failed");
    }
    if (signal(SIGUSR1, sig_handler) == SIG_ERR){
        error("ERROR: SIGUSR1 signal handling failed");
    }
    if (signal(SIGUSR2, sig_handler) == SIG_ERR){
        error("ERROR: SIGUSR2 signal handling failed");
    }
    printf("This is S\n");
    while(1){
        if (alarm_fired == 1){
            printf("P will start receiving/sending tokens.\n");
            strncpy(buffer, "1", BUFFSIZE);
            write(atoi(argv[1]), &buffer, BUFFSIZE);
            memset(buffer,0,BUFFSIZE);
            alarm_fired = 0;
        } else if (alarm_fired == 2){
            printf("P will stop receiving/sending tokens.\n");
            strncpy(buffer, "2", BUFFSIZE);
            write(atoi(argv[1]), &buffer, BUFFSIZE);
            memset(buffer,0,BUFFSIZE);
            alarm_fired = 0;
        } else if (alarm_fired == 3){
            printf("Print log\n");
            strncpy(buffer, "3", BUFFSIZE);
            write(atoi(argv[1]), &buffer, BUFFSIZE);
            memset(buffer,0,BUFFSIZE);
            print_log();
            alarm_fired = 0;
        }
        sleep(1);
    }
    return 0;
}

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void sig_handler(int signo){
    if (signo == SIGINT){
        //start sending/receiving tokens
        alarm_fired = 1;
    } else if (signo == SIGUSR1){
        //stop sending/recieving tokens
        alarm_fired = 2;
    } else if (signo == SIGUSR2){
        //print log
        alarm_fired = 3;
    }
}

//function to print contents of log file to terminal
void print_log(){
    FILE * log;
    char ch;
    log = fopen("posix.log", "r");
    while((ch = fgetc(log)) != EOF){
        printf("%c", ch);
    }
    fclose(log);
}

