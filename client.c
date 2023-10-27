#include <netdb.h>
#include <unistd.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 1024
#define MAX_OUT 4096
#define PORT 5600
#define SA struct sockaddr

void func(int sock)
{
	char buff[MAX];
	char output[MAX_OUT];

	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("terminal> ");
		n = 0;
		
		// copy  message in the buffer
		// while ((buff[n++] = getchar()) != '\n'){}

        fgets(buff, sizeof(buff), stdin);         // get input from the terminal
		buff[strlen(buff) - 1] = '\0';            // remove the newline character from the input

		printf("Sending: %s\n", buff);
		send(sock , buff , sizeof(buff),0);
		if ((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}

		bzero(output, sizeof(output));
		
		// print the stdout
		recv(sock , &output , sizeof(output),0);
		output[strlen(output)] = '\0';
		printf("%s", output);

		// bzero(output, sizeof(output));

		// // print the stderr
		// recv(sock , &output , sizeof(output),0);
		// output[strlen(output)] = '\0';
		// printf("%s", output);

	}
}

int main()
{
	int sock;
	struct sockaddr_in servaddr, cli;

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
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	// function for chat
	func(sock);

	// close the socket
	close(sock);
}