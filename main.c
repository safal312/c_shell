#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#define MAX 80
#define PORT 5600
#define SA struct sockaddr

// estimated time for programs we don't know the burst time for
#define DEFAULT_WAIT_TIME 100

//compile using -lpthread
//run multiple clients in several terminal windows
void* ThreadRun(void *);
void* scheduler_thread(void*);

#include "utils/parser.h"           // for parse_input
#include "shell_commands/commands.h"        // for executing commands
#include "utils/waitlist.h"			// for waitlist
#include "globals.h"

typedef struct {
	int socket;
	pthread_t* thread;
} thread_args;

sem_t continue_semaphore, add_node_sm;
NodeList waiting_list = {.head = NULL}; // Head of the global waiting list

int main(int argc, char const *argv[]) 
{ 
	sem_init(&continue_semaphore, 0, 0);
	sem_init(&add_node_sm, 0, 1);

	int server_fd, new_socket; 
	struct sockaddr_in address; 

	int addrlen = sizeof(address); 

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
			perror("socket failed"); 
			exit(EXIT_FAILURE); 
	} 

	if(setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&(int){1},sizeof(int))<0)
    {
        perror("setsockopt error");
        exit(EXIT_FAILURE);
    }

	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 

	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 

	//keep listeninig for incoming connections
	printf("Server is listening\n");

	// Start the scheduler thread
    pthread_t scheduler_th;
	pthread_attr_t sc_attr;
	pthread_attr_init(&sc_attr);
	pthread_attr_setdetachstate(&sc_attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&scheduler_th, &sc_attr, scheduler_thread, NULL);

	while(1)
	{
		if (listen(server_fd, 3) < 0) 		////Listen for incoming connections
		{ 
				perror("listen"); 
				exit(EXIT_FAILURE); 
		}

		//accept the incoming connection
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
										(socklen_t*)&addrlen))<0) 
		{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
		} 

		/*Declaring it as a detached thread, so there is no need to wait for the thread to terminate
		No other threads are waiting on it
		The server will keep running after the termination of this thread*/
		pthread_t th;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

		// make struct and add arguments to send to ThreadRun method
		thread_args args;
		args.socket = new_socket;
		args.thread = &th;

		pthread_create(&th,&attr,ThreadRun,&args);
	}
    close(server_fd);
    return 0;
}

//Thread handler function 
void* ThreadRun(void * args){
	thread_args* actual_args = (thread_args*)args;
    int client_socket = actual_args->socket;
	pthread_t* current_thread = actual_args->thread;

	printf("[%d]<<< client connected\n", client_socket);
	// infinite loop for the terminal
	for (;;) {
        char input[MAX_INPUT];          // array to store input from the terminal
		bzero(input, sizeof(input));

		// read the message from client and copy it in buffer
		if (recv(client_socket, &input, sizeof(input), 0) == -1) {
            perror("Error recieving from client");
            break;
        }
		printf("[%d]>>> %s\n", client_socket, input);
	
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
            close(client_socket);
            printf("[%d]--- client disconnected\n", client_socket);
            break;
        }

		int rem_time = DEFAULT_WAIT_TIME;
		// check if the given command is a program
		if (strncmp(commands[0], "./", 2) == 0) {
			// check if the user has given a time limit
			char* command_copy = strdup(commands[0]);
			strtok(command_copy, " ");
			char* time = strtok(NULL, " ");
			
			int time_temp = atoi(time);
			if (time != NULL && time_temp != 0) {
				// get the remaining time from the command
				rem_time = time_temp;
			}
		} else {
			rem_time = -1;
		}

		// before execution add to the scheduler
		// critical section
		sem_wait(&add_node_sm);
		
		ThreadNode* curr_node = addNode(&waiting_list, *current_thread, client_socket, rem_time, 2, 8);
		printf("(%d)--- ", client_socket);
		printf(BLUE_TEXT "created " RESET_TEXT);
		printf("(%d)\n", rem_time);
		
		sem_post(&(continue_semaphore));
		sem_post(&add_node_sm);

        // execute the commands
        execute(commands, commands_count, client_socket, curr_node);
	}
	close(client_socket);
    pthread_exit(NULL);
}