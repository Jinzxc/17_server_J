#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/stat.h> 
#include <sys/types.h> 

#define B_SIZE 100

int server_pipe, client_pipe;

void cipher(char * str, int shift);
void handshake_setup(char * buffer);

static void sighandler(int signo) {
    if (signo == SIGINT) {
        unlink("serpent");
        printf("\nThe server has been killed\n");
        exit(0);
    }
}

// Caesar cipher
void cipher(char * str, int shift) {
    int i;
    while(*str) {
        if(isupper(*str)) {
            // shift for uppercase letters
            *str = (((*str + shift) - 'A') % 26) + 'A';
        } else if(islower(*str)) {
            // shift for lowercase letters
            *str = (((*str + shift) - 'a') % 26) + 'a';
        }
        str++;
    }
}

// handshake for client&server
void handshake_setup(char * buffer) {
    char client_name[B_SIZE];

    mkfifo("serpent", 0644);
    printf("Waiting for a client...\n");

    server_pipe = open("serpent", O_RDONLY);
    read(server_pipe, client_name, B_SIZE);

    printf("Located client %s\n", client_name);
    client_pipe = open(client_name, O_WRONLY);

    printf("Giving handshake...\n");
    char message[] = "I can hear you.";
    write(client_pipe, message, sizeof(message));

    read(server_pipe, buffer, B_SIZE);

    printf("Handshake recieved!\nConnection with %s is now established\n", client_name);
    unlink("serpent");
}

int main() {
    char buffer[B_SIZE];
    signal(SIGINT, sighandler);

    while(1) {
        handshake_setup(buffer);

        while(1) {
            read(server_pipe, buffer, B_SIZE);
            if (!strcmp(buffer, "reset")) {
                break;
            }
            cipher(buffer, 13);
            write(client_pipe, buffer, B_SIZE); 
        }

        close(server_pipe);
        close(client_pipe);
        printf("The client has been disconnected.\n\n");
    }
    
    return 0;
}