#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/stat.h> 
#include <sys/types.h> 

#define EXIT "quit!"
#define B_SIZE 100

int server_pipe, client_pipe, client_active;
char client_name[B_SIZE];

void readin(char * buffer, int len);
void reset(int pipe);
void handshake_setup(char * buffer);

static void sighandler(int signo) {
    if (signo == SIGINT) {
        reset(server_pipe);
        printf("\nThe client has been killed\n");
        exit(0);
    }
}

// read user input from stdin
void readin(char * buffer, int len) {
    fgets(buffer, len, stdin);
    if(strchr(buffer, '\n'))
        *strchr(buffer, '\n') = 0;
}

// sends "reset" to a pipe
void reset(int pipe) {
    write(pipe, "reset", 6);
}

// handshake for client&server
void handshake_setup(char * buffer) {
    printf("Connecting to server...\n");

    sprintf(client_name, "%d", getpid());
    server_pipe = open("serpent", O_WRONLY);

    if(server_pipe == -1) {
        printf("Server is not active.\n");
        exit(0);
    }

    mkfifo(client_name, 0644);

    write(server_pipe, client_name, B_SIZE);

    printf("Waiting for handshake...\n");
    client_pipe = open(client_name, O_RDONLY);
    read(client_pipe, buffer, B_SIZE);
    
    printf("Handshake recieved!\n");
    unlink(client_name);

    printf("Returning handshake...\n");
    char message[] = "I can hear, you too.";
    write(server_pipe, message, sizeof(message));
}

int main() {
    char buffer[B_SIZE];
    client_active = 1;
    
    signal(SIGINT, sighandler);
    handshake_setup(buffer);

    printf("\nWelcome to terrible_cipher. Type 'quit!' to exit.\n");

    while(client_active) {
        printf("Type to cipher: ");

        readin(buffer, B_SIZE);
        if(!strncmp(buffer, EXIT, B_SIZE) || !client_active) {
            printf("The client has been killed\n");
            reset(server_pipe);
            exit(0);
        }

        write(server_pipe, buffer, B_SIZE);
        read(client_pipe, buffer, B_SIZE);

        printf("Ciphered into: %s\n\n", buffer);
    }

    return 0;
}
