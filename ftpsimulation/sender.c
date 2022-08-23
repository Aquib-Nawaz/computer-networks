// Extrenal Source: https://www.geeksforgeeks.org/udp-server-client-implementation-c/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <stdbool.h> 

#define MAXLINE 100000 

int main(int argc, char *argv[] ) { 
	int sockfd_sender; 
	char buffer[MAXLINE]; 
	int Sender_port, Reciever_port, Retransmission_timer, Num_packets;
	struct sockaddr_in servaddr, cliaddr; 
	long int currRetransmitterTime; 
	FILE *filePointer;
	if ((filePointer = fopen("sender.txt", "w")) == NULL){printf("File not opening\n");}
	// files to write the output on terminal screen
	// parsing Arguments from the command line
	if( argc == 5)
	{
		Sender_port = atoi(argv[1]);
		Reciever_port = atoi(argv[2]);
		Retransmission_timer = atoi(argv[3]);
		Num_packets = atoi(argv[4]);
	}
	else
	{
		printf("Invalid Number of Arguments.\n");
		return 0;
	}
	
	// Creating socket file descriptor
	if ( (sockfd_sender = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {  perror("socket creation failed"); exit(EXIT_FAILURE); } 
	// SOCK_DGRAM is used for UDP if the third argument is zero
	
	// Setting timeout value for the socket created above
	struct timeval tv;
	tv.tv_sec = Retransmission_timer;
	tv.tv_usec = 0;
	currRetransmitterTime = 1000000*Retransmission_timer;
	if (setsockopt(sockfd_sender, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) { perror("Error setting timeout"); exit(EXIT_FAILURE);}
	
	memset(&servaddr, 0, sizeof(servaddr)); 
	memset(&cliaddr, 0, sizeof(cliaddr)); 
	
	// Filling sender information 
	servaddr.sin_family = AF_INET; // IPv4 
	servaddr.sin_addr.s_addr = INADDR_ANY; 
	servaddr.sin_port = htons(Sender_port); //Sender Port Information
	
	// Filling reciever information 
	cliaddr.sin_family = AF_INET; // IPv4 
	cliaddr.sin_addr.s_addr = INADDR_ANY; 
	cliaddr.sin_port = htons(Reciever_port); //Reciever Port Information
	
	// Bind the socket with the server address 
	if ( bind(sockfd_sender, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
		
	int len, n; 

	len = sizeof(cliaddr); 
	int rev_pack_num = 1;
	for(int pack_num = 1; pack_num <= Num_packets;)
	{
		// creating the Message to be sent in the form of a string
		char messageToWrite[MAXLINE],msg[MAXLINE];
		printf("|Packet Number ~ %d|",pack_num);
		snprintf(messageToWrite, MAXLINE, "|Packet Number ~ %d|",pack_num);
		fputs(messageToWrite, filePointer); 
		char message[MAXLINE];
		snprintf(message,MAXLINE,"Packet:%d",pack_num);
		printf("Message Sent ~ %s|",message);
		snprintf(messageToWrite, MAXLINE, "Message Sent ~ %s|",message);
		fputs(messageToWrite, filePointer);
		struct timeval tvB, tvA;
		bool rcvAgain = true;
		
		// sending packet to the reciever
		sendto(sockfd_sender,(const char *)message, strlen(message), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		
		while(rcvAgain){
			// waiting for recieving a message from the reciever
			gettimeofday (&tvB, NULL);
			n = recvfrom(sockfd_sender, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
			gettimeofday (&tvA, NULL);
			// to calculate the time interval in case of an invalid ack as stated by One of the TA
			buffer[n]='\0';
			rev_pack_num = atoi(buffer+15);
			if( n == -1 )
			{
				// since no message was recieved so retransmitting the packet
				printf("Timer Expired - Retransmitting Packet|");
				fputs("Timer Expired - Retransmitting Packet|", filePointer);
				tv.tv_sec = Retransmission_timer; tv.tv_usec = 0; currRetransmitterTime = 1000000*Retransmission_timer;
				if (setsockopt(sockfd_sender, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) { perror("Error setting timeout"); exit(EXIT_FAILURE);}	
				rcvAgain = false;
			}
			else if(rev_pack_num != pack_num+1){
				//checking for correct sequence number found false so means an invalid acknowledgement
				printf("Invalid Acknowledgment - Ignoring|");
				fputs("Invalid Acknowledgment - Ignoring|", filePointer);
				currRetransmitterTime -= 1000000*(tvA.tv_sec-tvB.tv_sec) + tvA.tv_usec - tvB.tv_usec;
				if (currRetransmitterTime <= 0){
					currRetransmitterTime = 1000000*Retransmission_timer;
					rcvAgain = false;
					printf("Timer Expired - Retransmitting Packet|");
					fputs("Timer Expired - Retransmitting Packet|", filePointer);
				}
				//dynamically changing the timeout value from Original to Original - Time elapsed so as to maintain the original time out
				tv.tv_sec = currRetransmitterTime/1000000;
				tv.tv_usec = currRetransmitterTime - 1000000*tv.tv_sec;
				if (setsockopt(sockfd_sender, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) { perror("Error setting timeout"); exit(EXIT_FAILURE);}	 
			}
			else{			
				// all conditions met so resetting the timeout to original value and updating the iterator for the outer "for"
				printf("Message Recieved ~ %s |",buffer);
				snprintf(msg, MAXLINE, "Message Recieved ~ %s |", buffer);
				fputs(msg, filePointer);
				pack_num = rev_pack_num;
				tv.tv_sec = Retransmission_timer; tv.tv_usec = 0; currRetransmitterTime = 1000000*Retransmission_timer;
				if (setsockopt(sockfd_sender, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) { perror("Error setting timeout"); exit(EXIT_FAILURE);}
				rcvAgain = false;
			}
		}
		printf("\n");
		fputs("\n",filePointer);	
	}
	char message[MAXLINE];
	snprintf(message,MAXLINE,"end");
	//sending an hardcoded message "end" to halt the reciever
	sendto(sockfd_sender,(const char *)message, strlen(message), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
	//finally closing the socket
	close(sockfd_sender);
	return 0; 
} 

