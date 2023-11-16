#include <netdb.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024
#define MAX_OUT 4096
#define PORT 5600
#define SA struct sockaddr

void func(int sock)
{
	char buff[MAX];				// buffer to store input from terminal
	char output[MAX_OUT];		// buffer to receive from server and print it

	for (;;) {
		bzero(buff, sizeof(buff));					//making buff array zero
		printf("\nterminal> ");
		
        fgets(buff, sizeof(buff), stdin);         // get input from the terminal
		buff[strlen(buff) - 1] = '\0';            // remove the newline character from the input

		// invalid if the ending character is special
		if(buff[strlen(buff) - 1] == '|' || buff[strlen(buff) - 1] == '>' || buff[strlen(buff) - 1] == '<') {
			printf("Invalid input\n");
			continue;
    	}

		// send command to the server from client
		// printf("Sending: %s\n", buff);
		if (send(sock, buff, sizeof(buff), 0) == -1) {
            perror("Error sending to server");
            break;
        }

		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
		bzero(output, sizeof(output));

		// receive output from server
		if (recv(sock, &output, sizeof(output), 0) == -1) {
            perror("Error recieving from server");
            break;
        }

		// print the stdout
		output[strlen(output)] = '\0';
		printf("%s", output);
	}
}

int main()
{
	int sock;
	struct sockaddr_in servaddr;

	// socket create and verification
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sock, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("Connection with the server failed...\n");
		exit(0);
	}
	else
		printf("Connected to the server..\n");

	// function for chat
	func(sock);

	// close the socket
	close(sock);
	return 0;
}