// external source:https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
/*
Send a file over a socket.

Interface:

	./executable [<input_path> [<sender_hostname> [<port>]]]

Defaults:

- input_path: input.tmp
- sender_hostname: 127.0.0.1
- port: 12345
*/

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
	char protoname[] = "tcp";
	struct protoent *protoent;
	char *file_path = "input.tmp";
	char *sender_hostname = "127.0.0.1";
	char *sender_reply = NULL;
	char *user_input = NULL;
	char buffer[BUFSIZ];
	in_addr_t in_addr;
	in_addr_t sender_addr;
	int filefd;
	int sockfd;
	ssize_t i;
	ssize_t read_return;
	struct hostent *hostent;
	struct sockaddr_in sockaddr_in;
	unsigned short sender_port = 12345; // default port value

	if (argc > 1) {
		file_path = argv[1]; // file name to be given if required by the user
		if (argc > 2) {
			sender_hostname = argv[2]; // sender_hostname as given by the user if required
			if (argc > 3) {
				sender_port = strtol(argv[3], NULL, 10); //port number of sender user input argument if set 
			}
		}
	}

	filefd = open(file_path, O_RDONLY); // file opening
	if (filefd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	/* Get socket. */
	protoent = getprotobyname(protoname);
	if (protoent == NULL) {
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, protoent->p_proto); //set TCP protocol for the socket
	if (sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	/* Prepare sockaddr_in. */
	hostent = gethostbyname(sender_hostname); // get the sender host name default it is set to 127.0.0.1
	if (hostent == NULL) {
		fprintf(stderr, "error: gethostbyname(\"%s\")\n", sender_hostname);
		exit(EXIT_FAILURE);
	}
	in_addr = inet_addr(inet_ntoa(*(struct in_addr*)*(hostent->h_addr_list)));  // convert address from inet_addr to in_addr
	if (in_addr == (in_addr_t)-1) {
		fprintf(stderr, "error: inet_addr(\"%s\")\n", *(hostent->h_addr_list));
		exit(EXIT_FAILURE);
	}
	sockaddr_in.sin_addr.s_addr = in_addr;
	sockaddr_in.sin_family = AF_INET;
	sockaddr_in.sin_port = htons(sender_port); // define the socket
	/* Do the actual connection. */
	if (connect(sockfd, (struct sockaddr*)&sockaddr_in, sizeof(sockaddr_in)) == -1) {  // connect with the reciever so that we may send the file
		perror("connect");
		return EXIT_FAILURE;
	}
	while (1) // loop to send the file
	{
        read_return = read(filefd, buffer, BUFSIZ); // read the file 1500 lines at a time at most 
        if (read_return == 0)
            break;
        if (read_return == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        
        if (write(sockfd, buffer, read_return) == -1) { // write data read to the socket which internally sneds the data via TCP to reciever
            perror("write");
            exit(EXIT_FAILURE);
        }
    	}
	free(user_input); // freeing user_input
	free(sender_reply); // freeing sender reply
	close(filefd); // closing the socket
	exit(EXIT_SUCCESS);
}
