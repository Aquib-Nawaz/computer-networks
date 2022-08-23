// Extrenal Source: https://www.geeksforgeeks.org/udp-server-client-implementation-c/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include<time.h>

#define MAXLINE 100000 

int main(int argc, char *argv[] ) { 
	int sockfd_reciever; 
	char buffer[MAXLINE]; 
	int Sender_port, Reciever_port;
	double Drop_prob;
	struct sockaddr_in servaddr,cliaddr; 
	FILE *filePointer;
	if ((filePointer = fopen("receiver.txt", "w")) == NULL){printf("File not opening\n");}
	// files to write the output on terminal screen
	// parsing Arguments from the command line
	if( argc == 4)
	{
		Sender_port = atoi(argv[2]);
		Reciever_port = atoi(argv[1]);
		Drop_prob = atof(argv[3]);
	}
	else
	{
		printf("Invalid Number of Arguments.\n");
		return 0;
	}
	
	// Creating socket file descriptor for reciever
	if ( (sockfd_reciever = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {  perror("socket creation failed"); exit(EXIT_FAILURE); } 
	
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
	
	// Bind the socket with the reciever address 
	if ( bind(sockfd_reciever, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0 ) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
		
	int n, len, pck_num=1; 
	
	srand(time(0));
	
	while(1)
	{
		char messageToWrite[MAXLINE],msg[MAXLINE];
		// waiting for message from sender
		n = recvfrom(sockfd_reciever, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len); 
		buffer[n] = '\0'; 
		// Hardcoded string "end" being sent after the halt of sender
		if(n==3) 
			break;
		printf("|Message Recieved ~ %s|", buffer+7);
		snprintf(msg, MAXLINE, "|Message Recieved ~ %s|", buffer+7);
		fputs(msg, filePointer);
		int pack_num = atoi(buffer+7);
		char message[MAXLINE];
		//checking for expected sequence number
		if(pck_num!=pack_num)
		{
			// since not equal sending an invalid ack for the correct sequence number
			snprintf(message,MAXLINE,"Acknowledgment:%d",pck_num);
			sendto(sockfd_reciever, (const char *)message, strlen(message), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
			printf(" Seq Number Wrong  |\n");
			snprintf(messageToWrite, MAXLINE, " Seq Number Wrong  |\n");
			fputs(messageToWrite, filePointer);
		}
		else
		{
			//since valid generating a random probability
			double gen_prob = (double)rand() / (double) (RAND_MAX);
			if(gen_prob > Drop_prob)
			{	
				// condition 3 b satisfied so sending correct ack for the next pack
				snprintf(message,MAXLINE,"Acknowledgment:%d",pack_num+1);
				sendto(sockfd_reciever, (const char *)message, strlen(message), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr)); 
				printf("Acknowledgment: ~ %d|\n",pack_num+1);
				snprintf(messageToWrite, MAXLINE, "Acknowledgment: ~ %d|\n", pck_num);
				fputs(messageToWrite, filePointer);
				pck_num++;
			}
			else{
				// Dropping packet without any message being sent
				printf("Dropping Packet ~ %d|\n",pck_num);
				snprintf(messageToWrite, MAXLINE, "Dropping Packet ~ %d|\n", pck_num);
				fputs(messageToWrite, filePointer);
			}
		}
	}
	//finally closing the socket
	close(sockfd_reciever); 
	return 0; 
} 

