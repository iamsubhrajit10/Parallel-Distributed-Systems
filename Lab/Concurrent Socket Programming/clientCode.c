#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4040

int main(int argc, char *argv[]){

	int clientSocket, ret;
	struct sockaddr_in serverAddr;
	char buffer[1024];
	int clientNo=atoi(argv[1]);

	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(clientSocket < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Client%d Socket is created.\n",clientNo);

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	if(ret < 0){
		printf("[-]Error in connection.\n");
		exit(1);
	}
	printf("[+]Connected to Server.\n");
	sleep(10);
	int i=1;
	while(1){
		printf("Client%d: \t",clientNo);
		
		strcpy(buffer,"Hello ");

		send(clientSocket, buffer, strlen(buffer), 0);
		
		if(i==3){
			close(clientSocket);
			printf("[-]Disconnected from server.\n");
			exit(1);
		}
		if(recv(clientSocket, buffer, 1024, 0) < 0){
			printf("[-]Error in receiving data.\n");
		}else{
			printf("Server: \t%s\n", buffer);
		}
		i++;
	}

	return 0;
}