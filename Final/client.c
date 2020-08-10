#include <netdb.h> 
#include <getopt.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include<sys/time.h>

int main(int argc, char *argv[]) 
{
	int opt;
	char* port = NULL;
	char* ipAddr = NULL;
	char* source = NULL;
	char* destination = NULL;
	int Port;
	int src;
	int dest;
	
	while((opt = getopt(argc, argv,"a:p:s:d:")) != -1)  
	{  
		switch(opt)
		{
        	case 'a':
		        ipAddr = optarg;
		        break;
	        case 'p':
		        port = optarg;
		        break;
	        case 's':
		        source = optarg;
		        break;
	        case 'd':
		        destination = optarg;
		        break;
			case '?':
				fprintf(stderr,"Wrong Command Line! : %s\n",strerror(errno));  
				exit(EXIT_FAILURE);
				break;
			default:
				printf("Wrong Command Line!");       
				exit(EXIT_FAILURE);
				break;
		}
	}
	 for(; optind < argc; optind++){
	 	printf("Wrong Command Line!\n");      
		exit(EXIT_FAILURE);
	}
	Port = atoi(port);
	src = atoi(source);
	dest = atoi(destination);
	// socket create and verification 
	int sock = socket(AF_INET, SOCK_STREAM, 0); 
	if (sock == -1) {
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	struct sockaddr_in serveraddr;
	// assign IP, PORT 
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; 
	serveraddr.sin_addr.s_addr = inet_addr(ipAddr);
	serveraddr.sin_port = htons(Port);
	int id = getpid();
	//sockete bağlan
	if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	}
	time_t currtime;
	char *timeString;
	currtime = time(NULL);
	timeString = ctime(&currtime);
	timeString[strlen(timeString)-1] = '\0'; 
	printf("%s Client (%d) connecting to %s:%d\n",timeString,id,ipAddr,Port);
	int message[2]={src,dest};
	char *timeString2;
	currtime = time(NULL);
	timeString2 = ctime(&currtime);
	timeString2[strlen(timeString2)-1] = '\0';
	printf("%s Client (%d) connected and requesting a path from node %d to %d\n",timeString2,id,src,dest);
	if(write(sock,message,sizeof(message)) < 0){
		printf("Send Error\n");
	} 
	struct timeval start, end;
 	if(gettimeofday(&start, NULL)  != 0){
 		perror("Time Error!");
		exit(EXIT_FAILURE);
 	}
	int size;
	if(read(sock,&size,sizeof(size)) <0){
		perror("Receive error\n");
	}
	if(size > 0){
		int path[size];
		if(read(sock,path,sizeof(path)) <0){
			perror("Receive error\n");
		}
		char *timeString3;
		currtime = time(NULL);
		timeString3 = ctime(&currtime);
		timeString3[strlen(timeString3)-1] = '\0';
		printf("%s Server’s response to (%d): ",timeString3,id);
		for(int i=0; i<size; ++i){
			printf("%d",path[i]);
			if(i != size-1)
				printf("->");
		}
		if(gettimeofday(&end, NULL)  != 0){
	 		perror("Time Error!");
			exit(EXIT_FAILURE);
	 	}
		float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))*1.0 / 1000000.0;
		printf(", arrived in %.2f seconds.\n",time);
	}
	else{
		if(gettimeofday(&end, NULL)  != 0){
	 		perror("Time Error!");
			exit(EXIT_FAILURE);
	 	}
		char *timeString4;
		currtime = time(NULL);
		timeString4 = ctime(&currtime); 
		timeString4[strlen(timeString4)-1] = '\0';
		float time = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))*1.0 / 1000000.0;
		printf("%s Server’s response (%d): NO PATH, arrived in %.2f seconds, shutting down\n",timeString4,id,time);
	}
	
	// close the socket 
	close(sock);
	return 0;
} 

