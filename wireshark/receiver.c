// external source:https://stackoverflow.com/questions/2014033/send-and-receive-a-file-in-socket-programming-in-linux-with-c-c-gcc-g
/*
Receive a file over a socket.

Saves it to output.tmp by default.

Interface:
	./executable [<output_file> [<port>] true] for verbose
	./executable [[<output_file> [<port>]] for no verbose
Defaults:

- output_file: output.tmp
- port: 12345
*/

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h> /* getprotobyname */
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

int main(int argc, char **argv) {
	char *file_path = "output.tmp";
	char buffer[BUFSIZ];
	char protoname[] = "tcp";
	int sender_sockfd;
	int enable = 1;
	int filefd;
	int i;
	int receiver_sockfd;
	socklen_t sender_len;
	ssize_t read_return;
	struct protoent *protoent;
	struct sockaddr_in sender_address, receiver_address;
	unsigned short receiver_port = 12345u;
	int n_s = atoi(argv[1]); // number of measurements to be taken
	if (argc > 2) {
		file_path = argv[2];
		if (argc > 3) {
			receiver_port = strtol(argv[3], NULL, 10); // reciever port number if given uses that else uses some random port number
		}
	}

	/* Create a socket and listen to it.. */
	protoent = getprotobyname(protoname);
	if (protoent == NULL) {
		perror("getprotobyname");
		exit(EXIT_FAILURE);
	}
	receiver_sockfd = socket(
		AF_INET,
		SOCK_STREAM,
		protoent->p_proto
	);                    // set TCP as the protocol to be used by the socket
	if (receiver_sockfd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	if (setsockopt(receiver_sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) { // enable reusage of address 
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(EXIT_FAILURE);
	}
	receiver_address.sin_family = AF_INET;
	receiver_address.sin_addr.s_addr = htonl(INADDR_ANY);
	receiver_address.sin_port = htons(receiver_port);
	if (bind(
			receiver_sockfd,
			(struct sockaddr*)&receiver_address,
			sizeof(receiver_address)                            // binding socket with the reciever address
		) == -1
	) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (listen(receiver_sockfd, 5) == -1) { // start listening as this has to write to the file named recv.txt
		perror("listen");
		exit(EXIT_FAILURE);
	}
	//fprintf(stderr, "listening on port %d\n", receiver_port);
	double throughputx[n_s];
	double avg_throughput = 0;
	if(argc > 4) printf("Time_taken,Throughput\n");
	for(int i=0;i<n_s;i++)
	{
		sender_len = sizeof(sender_address);
		//puts("waiting for sender");
		sender_sockfd = accept(                              //accept the connection which is incoming from the sender (20 times occuring)
			receiver_sockfd,
			(struct sockaddr*)&sender_address,
			&sender_len
		);
		filefd = open(file_path,                              // write to the file address as mentioned by the user else will be recv.txt
				O_WRONLY | O_CREAT | O_TRUNC, 
				S_IRUSR | S_IWUSR);
		if (filefd == -1) {
			perror("open");
			exit(EXIT_FAILURE);
		}
		struct timeval current_time,after_time; 
		gettimeofday(&current_time, NULL);  // get the start time of the transmission
		do {
			read_return = read(sender_sockfd, buffer, BUFSIZ);  // read the data coming from TCP and write to the filefd
			if (read_return == -1) {
				perror("read");
				exit(EXIT_FAILURE);
			}
			if (write(filefd, buffer, read_return) == -1) { //writing to the file
				perror("write");
				exit(EXIT_FAILURE);
			}
		} while (read_return > 0);
		gettimeofday(&after_time, NULL); // get the dnd time of transmission
		double time_diff = (after_time.tv_sec - current_time.tv_sec)*1000000 + after_time.tv_usec - current_time.tv_usec;
		time_diff = time_diff/1000000.0;
		double throughput = (5/time_diff); // calculate the throughput obatined using the file size as 5MB
		if(argc > 4) printf("%f,%f\n",time_diff,throughput);
		throughputx[i] = throughput;
		avg_throughput = avg_throughput + throughput;
		close(filefd);  // termintae the connectin and then wait for the new connection for the number of meausrements and then terminate
		close(sender_sockfd);
	}
	avg_throughput = avg_throughput / n_s; //calculate the average value of throughput
	double std_dev = 0;
	for(int i=0;i<n_s;i++)
	{
		std_dev = std_dev + (throughputx[i] - avg_throughput)*(throughputx[i] - avg_throughput);
	}
	std_dev = std_dev / n_s; // calculate std_deviation of the throughput values through the code
	std_dev = sqrt(std_dev);
	if( n_s != 1)
	printf("%f,%f\n",avg_throughput,std_dev); // print them both
	else
	printf("%f\n",avg_throughput);
	return EXIT_SUCCESS;
}
