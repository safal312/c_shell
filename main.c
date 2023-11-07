#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#define MAX 80
#define PORT 5600
#define SA struct sockaddr

#include <pthread.h>

//compile using -lpthread
//run multiple clients in several terminal windows
void* ThreadRun(void *);

#include "utils/parser.h"           // for parse_input
#include "shell_commands/commands.h"        // for executing commands

int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket; 
	struct sockaddr_in address; 

	int addrlen = sizeof(address); 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
			perror("socket failed"); 
			exit(EXIT_FAILURE); 
	} 

	int value  = 1;
	setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&value,sizeof(value)); //&(int){1},sizeof(int)

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	//keep listeninig for incoming connections
	while(1)
	{
			if (listen(server_fd, 3) < 0) 
			{ 
					perror("listen"); 
					exit(EXIT_FAILURE); 
			} 
			if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
											(socklen_t*)&addrlen))<0) 
			{ 
					perror("accept"); 
					exit(EXIT_FAILURE); 
			} 

			/*enhance performance and resource consumption by declaring
			it as a detached thread,
			so there is no need to wait for the thread to terminate
			No other threads are waiting on it
			The server will keep running after the termination of this thread*/
			pthread_t th;
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

			pthread_create(&th,&attr,ThreadRun,&new_socket);

			/*
			alternative way of declaration:
			pthread_t th;
			pthread_create(&th,NULL,ThreadRun,&new_socket);
			*/
	}
    close(server_fd);
    return 0;
}


void* ThreadRun(void * socket){
	int *sock=(int*)socket;
    int s=*sock;

	// infinite loop for the terminal
	for (;;) {
        char input[MAX_INPUT];          // array to store input from the terminal
		bzero(input, sizeof(input));

		// read the message from client and copy it in buffer
		recv(s , &input , sizeof(input),0);
		
        char* commands[MAX_COMMANDS];   // array to store the commands after parsing input
        
        // parse the input and store the commands in the commands array
        // this separates the commands by pipes
        int commands_count = parse_input(input, commands);

        // check if input is empty or made with only whitespace characters
        if (commands_count <= 0) {
            continue;
        }

        // check if the user wants to exit the terminal
        if (strcmp(commands[0], "exit") == 0) {
            // close the socket
            close(s);
            printf("Disconnecting Client\n...");
            break;
        }
        
        // execute the commands
        execute(commands, commands_count, s);
	}
	close(s);
    return NULL;
}