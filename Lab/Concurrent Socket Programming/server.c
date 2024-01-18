#include <stdio.h>		/*for printf() and fprintf()*/
#include <sys/socket.h>		/*for socket(), bind() and connect()*/
#include <arpa/inet.h>		/*for sockaddr_in() and inet_ntoa()*/
#include <stdlib.h>		/*for atoi()*/
#include <string.h>		/*for memset()*/
#include <unistd.h>		/*for close()*/

#define MAXPENDING 5   /*the maximum number of connetion requests*/
#define RCVBUFFERSIZE 32

int main(int argc,char *argv[])
{

	int servSock;			/*the socket descriptor for the server socket*/
	int clntSock;			/*the socket descriptor for the client socket*/
	struct sockaddr_in echoServAddr;/*Local - Server address*/
	struct sockaddr_in echoClntAddr;/*Client Address*/
	int echoServPort;		/*Server Port*/
	int clntLen;			/*Length of Client address data structure*/
	char echoBuffer[RCVBUFFERSIZE];
	int recvMsgSize;

	if(argc!=2)
	{
		fprintf(stderr,"Usage : %s <Server port>\n",argv[0]);
		exit(1);
	}

	echoServPort=atoi(argv[1]);

	if((servSock=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP))<0)
	{
		perror("socket() failed");
		exit(1);
	}
	
	/*construct the local address for the server*/
	memset(&echoServAddr,0,sizeof(echoServAddr));
	echoServAddr.sin_family=AF_INET;
	echoServAddr.sin_port=htons(echoServPort);
	echoServAddr.sin_addr.s_addr=htonl(INADDR_ANY);

	/*bind the server socket*/
	if(bind(servSock,(struct sockaddr *)&echoServAddr,sizeof(echoServAddr))<0)
	{
		perror("bind() failed");
		exit(1);
	}
	
	/*make the server to listen for incoming connections*/
	if(listen(servSock,MAXPENDING)<0)
	{
		perror("listen() failed");
		exit(1);
	}
	
	for(;;)
	{
		clntLen=sizeof(echoClntAddr);
		if((clntSock=accept(servSock,(struct sockaddr *)&echoClntAddr,&clntLen))<0)
		{
			perror("accept() failed");
			exit(1);
		}
		printf("Handling client: %s\n",inet_ntoa(echoClntAddr.sin_addr));
		/*recv() and send()*/
		if((recvMsgSize=recv(clntSock,echoBuffer,RCVBUFFERSIZE,0))<0)
		{
			perror("recv() failed");
			exit(1);
		}
		while(recvMsgSize>0)   /*zero indicated the end of transmission*/
		{
			if(send(clntSock,echoBuffer,recvMsgSize,0)!=recvMsgSize)
			{
				perror("send() failed");
				exit(1);
			}
			if((recvMsgSize=recv(clntSock,echoBuffer,RCVBUFFERSIZE,0))<0)
			{
				perror("recv() failed");
				exit(1);
			}
		}
		close(clntSock);
	}
}

	
	