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

#include "utils/parser.h"           // for parse_input
#include "shell_commands/commands.h"        // for executing commands

int main() {
    
	int server_socket, client_socket;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(server_socket, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(server_socket, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	int addrlen = sizeof(cli);

	// Accept the data packet from client and verification
	client_socket = accept(server_socket, (SA*)&cli, (socklen_t *) &addrlen);
	if (client_socket < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");

    // char buff[MAX];
	// infinite loop for the terminal
	for (;;) {
        char input[MAX_INPUT];          // array to store input from the terminal
		bzero(input, MAX);

		// read the message from client and copy it in buffer
		recv(client_socket , &input , sizeof(input),0);

		if (input == NULL) continue;
		
        // execution loop
        char* commands[MAX_COMMANDS];   // array to store the commands after parsing input
        
        // // print buffer which contains the client contents
        printf("From client: %s\n", input);
        // printf("terminal> ");
        // fgets(input, sizeof(input), stdin);         // get input from the terminal
        
        // parse the input and store the commands in the commands array
        // this separates the commands by pipes
        int commands_count = parse_input(input, commands);

        // check if input is empty or made with only whitespace characters
        if (commands_count == 0) {
            continue;
        }

        // check if the user wants to exit the terminal
        if (strcmp(commands[0], "exit") == 0) {
            // close the socket
            close(client_socket);
            printf("Disconnecting Client\n...");
            break;
            // exit(0);
        }
        
        // execute the commands
        execute(commands, commands_count, client_socket);
	}
    close(server_socket);

    return 0;
}