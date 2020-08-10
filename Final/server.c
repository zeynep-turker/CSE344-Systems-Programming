#include <stdio.h>
#include <getopt.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h> 
#include <linux/fs.h>
#include <linux/limits.h>
#define NR_OPEN 1024
#define BD_NO_CHDIR 01
#define BD_NO_CLOSE_FILES 02
#define BD_NO_REOPEN_STD_FDS 04
#define BD_NO_UMASK0 010
#define BD_MAX_CLOSE 8192

//Graph oluşturuken kullancağım data structurelar ve fonksiyonları
//node ve graph
struct node{
  int vertex;
  struct node* next;
};
struct node* createNode(int v);
struct Graph{
	int nodeNumber;
  	int *arr;
  	int size;
  	struct node** adjArray;
  	int* visited;
};
struct Graph* createAGraph(int vertices);
void addEdge(struct Graph* graph, int s, int d);
int control(int nodes[],int size,int num);
//***
//Bfs ile path bulurken kullanacağım queue data structure ı ve fonksiyonları
//queue
struct queue {
  int *nodes;
  int Size; 
  int capacity;
  int front;
  int rear;
};
struct queue* createQueue(int capacity);
void enqueue(struct queue* q, int);
int dequeue(struct queue* q);
int isEmpty(struct queue* q);
//***
//Nodelar arası path bulurken kullanacağım fonksiyonlar
//path bulur
int bfs(struct Graph* graph, int src,int dest,int *path);
//pathin cachede olup olmadığı kontrol edilir.
int isThere(int src,int dest);
//bir node un başka bir node ile komsu olup olmadıgı kontrol edilir.
int Adj(struct Graph* graph, int src,int dest);
//***
//gelen stringi log filea yazan fonksiyon
void writeLogFile(char *str);
//Threadlerim
//nodelar arası path bulup bunu clienta gönderen thread
void *Thread(void *args);
//thread poolunu gerektiği zaman büyüten thread
void *reSizeThreadPool(void *args);
//**
//değişkenler
//graph
struct Graph* graph;
//her threse ait node çiftini tutan array
int **values;
//her threadin çalışıp çalışmadıgını gösteren array
int *controler;
//cachedeki her pathin hangi nodelar arası olduğunu gösteren array
int **fromToNodes;
//pathleri tutan cache
int **cache;
//cache toplam sizeı
int cacheSize = 0;
//cache in capasitesi
int cacheCapacity = 1000;
//writer ve reader paradigmasını yaparken kullandığım değişkenler
int AR=0,AW=0,WR=0,WW=0;
//güncel thread sayısı
int threadNum;
//kaç tane threadin çalışır duruma geldiği gösteren thread
int look=0;
//client
int client;
//node sayısını tutar
int nodeIndex=0;
//Thread arrayi
pthread_t *Threads;
int *threads;
//max thread sayısı
int maxThreadNum;
//log file ve input file fd
int output;
int input;
//çalışan threadlere göre yoğunluk yüzdesi
float loading;
//clientın gönderdiği nodeları tutan array
int value[2];
//***
//Condition variables
pthread_cond_t okToRead,okToWrite,reSizeCond;
pthread_cond_t cond;
pthread_cond_t waitThread;
//***
//mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cont = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t availableMux = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;
//***
//semaphore
sem_t sem;
//***
void mySigintHandler(int signal){
	if(signal == 2){}
	time_t currtime;
	char *timeString1=NULL;
	currtime = time(NULL);
	timeString1 = ctime(&currtime);
	timeString1[strlen(timeString1)-1] = '\0';
	writeLogFile(timeString1);
	writeLogFile(" Termination signal received, waiting for ongoing threads to complete.\n");
	for(int i=0; i<graph->nodeNumber; ++i){
 		free(graph->adjArray[i]);
	}
	free(graph->visited);
	free(graph->arr);
	free(graph->adjArray);
	free(graph);
	free(threads);
	free(Threads);
	free(cache);
	free(fromToNodes);
	for(int i=0; i<maxThreadNum; ++i){
		free(values[i]);
	}
	free(values);
	free(controler);
	char *timeString2=NULL;
	currtime = time(NULL);
	timeString2 = ctime(&currtime);
	timeString2[strlen(timeString2)-1] = '\0';
	writeLogFile(timeString2);
	writeLogFile(" All threads have terminated, server shutting down.\n");
	int flags = 1;
	int fd,maxfd;
	if (chdir("/") == -1)
        exit(0);
	if (!(flags & BD_NO_UMASK0))
		umask(0);
	if (!(flags & BD_NO_CHDIR))
		chdir("/");
	if (!(flags & BD_NO_CLOSE_FILES)) { 
		maxfd = sysconf(_SC_OPEN_MAX);
		if (maxfd == -1) 
			maxfd = BD_MAX_CLOSE; 
		for (fd = 0; fd < maxfd; fd++)
			close(fd);
	}
	if (!(flags & BD_NO_REOPEN_STD_FDS)) {
		close(STDIN_FILENO);
		fd = open("/dev/null", O_RDWR);
		if (fd != STDIN_FILENO) 
			exit(0);
		if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
			exit(0);
		if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
		exit(0);
	}  
    close(input);
    close(output);
	exit(1);
}
int main(int argc, char *argv[]) 
{
	switch(fork()){
		case -1: return -1;
		case 0: break;
		default: exit(EXIT_SUCCESS);
	}
	
	if (setsid() == -1)
        exit(0);
    
    switch(fork()){
		case -1: return -1;
		case 0: break;
		default: exit(EXIT_SUCCESS);
	}
	int opt;
	char* filePath = NULL;
	char* port = NULL;
	char* outputFile = NULL;
	char* charS = NULL;
	char* charX = NULL;
	int Port;
	
	while((opt = getopt(argc, argv,"i:p:o:s:x:")) != -1)  
	{  
		switch(opt)
		{
        	case 'i':
		        filePath = optarg;
		        break;
	        case 'p':
		        port = optarg;
		        break;
	        case 'o':
		        outputFile = optarg;
		        break;
	        case 's':
		        charS = optarg;
		        break;
	        case 'x':
		        charX = optarg;
		        break;
			case '?':
				writeLogFile("Wrong Command Line!\n");  
				exit(EXIT_FAILURE);
				break;
			default:
				writeLogFile("Wrong Command Line!\n");       
				exit(EXIT_FAILURE);
				break;
		}  
	}
	 for(; optind < argc; optind++){
	 	writeLogFile("Wrong Command Line!\n");      
		exit(EXIT_FAILURE);
	}
	
	output = open(outputFile,O_WRONLY | O_CREAT,0666);
	if(output == -1){
		perror("Log File Open Error!\n");
		return -1;
	}
	struct sigaction action1;
	memset(&action1, 0, sizeof(action1));
	action1.sa_handler = &mySigintHandler;
	if(sigaction(SIGINT, &action1, NULL) == -1){
		perror("Sigaction Error!");
		exit(EXIT_FAILURE);
	}
	Port = atoi(port);
	threadNum = atoi(charS);
	if(threadNum < 2){
		writeLogFile("Thread Number is less than 2.\n");
		exit(EXIT_FAILURE);
	}
	maxThreadNum = atoi(charX);	
	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
	input = open(filePath,O_RDONLY,mode);
	if(input < 0){
		writeLogFile("Open File Error!\n");
		exit(EXIT_FAILURE);
	}
	time_t currtime;
	char *timeString1;
	currtime = time(NULL);
	timeString1 = ctime(&currtime);
	timeString1[strlen(timeString1)-1] = '\0';
	writeLogFile(timeString1);
	writeLogFile(" Executing with parameters:\n");
	char *timeString2;
	currtime = time(NULL);
	timeString2 = ctime(&currtime);
	timeString2[strlen(timeString2)-1] = '\0';
	writeLogFile(timeString2);
	writeLogFile(" -i ");
	writeLogFile(filePath);
	writeLogFile("\n");
	char *timeString3;
	currtime = time(NULL);
	timeString3 = ctime(&currtime);
	timeString3[strlen(timeString3)-1] = '\0';
	writeLogFile(timeString3);
	writeLogFile(" -p ");
	writeLogFile(port);
	writeLogFile("\n");
	char *timeString4;
	currtime = time(NULL);
	timeString4 = ctime(&currtime);
	timeString4[strlen(timeString4)-1] = '\0';
	writeLogFile(timeString4);
	writeLogFile(" -o ");
	writeLogFile(outputFile);
	writeLogFile("\n");
	char *timeString5;
	currtime = time(NULL);
	timeString5 = ctime(&currtime);
	timeString5[strlen(timeString5)-1] = '\0';
	writeLogFile(timeString5);
	writeLogFile(" -s ");	
	writeLogFile(charS);
	writeLogFile("\n");
	char *timeString6;
	currtime = time(NULL);
	timeString6 = ctime(&currtime);
	timeString6[strlen(timeString6)-1] = '\0';
	writeLogFile(timeString6);
	writeLogFile(" -x ");	
	writeLogFile(charX);
	writeLogFile("\n");
	char *timeString7;
	currtime = time(NULL);
	timeString7 = ctime(&currtime);
	timeString7[strlen(timeString7)-1] = '\0';
	writeLogFile(timeString7);
	writeLogFile(" Loading graph...\n");
		
	controler = (int*)malloc(sizeof(int)*maxThreadNum);
	values = (int**)malloc(sizeof(int*)*maxThreadNum);
	for(int f=0; f<maxThreadNum; ++f){
		values[f] = (int*)malloc(sizeof(int)*2);
	}
	//cond lock sem allocation
	if(pthread_cond_init(&cond,NULL) != 0){
		writeLogFile("Pthread Condition Init Error!\n");
		exit(EXIT_FAILURE);
	}
	//Condition and sem init
	if(pthread_cond_init(&okToRead,NULL) != 0){
			writeLogFile("Pthread Condition Init Error!\n");
			exit(EXIT_FAILURE);
	}
	if(pthread_cond_init(&okToWrite,NULL) != 0){
			writeLogFile("Pthread Condition Init Error!\n");
			exit(EXIT_FAILURE);
	}
	if(pthread_cond_init(&reSizeCond,NULL) != 0){
			writeLogFile("Pthread Condition Init Error!\n");
			exit(EXIT_FAILURE);
	}
	if(sem_init(&sem,0,0) != 0){
			writeLogFile("Sem_init Error!\n");
			exit(EXIT_FAILURE);
	}
	if(pthread_cond_init(&waitThread,NULL) != 0){
			writeLogFile("Pthread Condition Init Error!\n");
			exit(EXIT_FAILURE);
	}
	int oneChar = 'c';
	int lineNum = 0;
	char str[100];
	int temp=0;
	int i=0;
	int comment=0;
	while(read(input,&oneChar,1) != 0){
		if(oneChar == '#' && temp == 0){
			str[i] = oneChar;
			i++;
			temp=1;
		}
		else if(oneChar == '\n' && temp == 1){
			temp = 0;
			for(int j=0; j<i; ++j)
				str[j] = '\0';
			i=0;
			comment++;
		}
		else if(oneChar == '\n' && temp == 0){
			lineNum++;
			for(int j=0; j<i; ++j)
				str[j] = '\0';
			i=0;
		}
		else{
			str[i] = oneChar;
			i++;
		}
	}
	graph = createAGraph(lineNum*2);
	lseek(input,0,SEEK_SET);
	graph->arr = (int *)malloc(sizeof(int) * (lineNum*2));
	while(read(input,&oneChar,1) != 0){
		if(oneChar == '#' && temp == 0){
			str[i] = oneChar;
			i++;
			temp=1;
		}
		else if(oneChar == '\n' && temp == 1){
			temp = 0;
			for(int j=0; j<i; ++j)
				str[j] = '\0';
			i=0;
			comment++;
		}
		else if(oneChar == '\n' && temp == 0){
			str[i] = '\0';
			char from[100];
			int k=0;
			int m=0;
			while(str[k] != '\t'){
				from[m] = str[k];
				k++;
				m++;
			}
			k++;
			int l=0;
			char to[100];
			while(str[k] != '\0'){
				to[l] = str[k];
				k++;
				l++;
			}
			from[m] = '\0';
			to[l] = '\0';
			int first = atoi(from);
			if(control(graph->arr,nodeIndex,first) == 1){
				graph->arr[nodeIndex] = first;
				nodeIndex++;
			}
			int second = atoi(to);
			if(control(graph->arr,nodeIndex,second) == 1){
				graph->arr[nodeIndex] = second;
				nodeIndex++;
			}
			for(int j=0; j<m; ++j)
				from[j] = '\0';
			for(int j=0; j<l; ++j)
				to[j] = '\0';
			
			for(int j=0; j<i; ++j)
				str[j] = '\0';
			i=0;
		}
		else{
			str[i] = oneChar;
			i++;
		}
	}
	graph->nodeNumber = nodeIndex;
	graph->size = nodeIndex;
	lseek(input,0,SEEK_SET);
	struct timeval start, end;
 	if(gettimeofday(&start, NULL) != 0){
 		writeLogFile("Time Error!\n");
		exit(EXIT_FAILURE);
 	}
	while(read(input,&oneChar,1) != 0){
		if(oneChar == '#' && temp == 0){
			str[i] = oneChar;
			i++;
			temp=1;
		}
		else if(oneChar == '\n' && temp == 1){
			temp = 0;
			for(int j=0; j<i; ++j)
				str[j] = '\0';
			i=0;
		}
		else if(oneChar == '\n' && temp == 0){
			str[i] = '\0';
			char from[100];
			int k=0;
			int m=0;
			while(str[k] != '\t'){
				from[m] = str[k];
				k++;
				m++;
			}
			k++;
			int l=0;
			char to[100];
			while(str[k] != '\0'){
				to[l] = str[k];
				k++;
				l++;
			}
			from[m] = '\0';
			to[l] = '\0';
			int first = atoi(from);
			int second = atoi(to);
			addEdge(graph,first,second);
			for(int j=0; j<m; ++j)
				from[j] = '\0';
			for(int j=0; j<l; ++j)
				to[j] = '\0';
			
			for(int j=0; j<i; ++j)
				str[j] = '\0';
			i=0;
		}
		else{
			str[i] = oneChar;
			i++;
		}
	}
	if(gettimeofday(&end, NULL) != 0){
 		writeLogFile("Time Error!\n");
		exit(EXIT_FAILURE);
 	}
	float timee = ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))*1.0 / 1000000.0;
	char buffer[100];
	char *timeString8;
	currtime = time(NULL);
	timeString8 = ctime(&currtime);
	timeString8[strlen(timeString8)-1] = '\0';
	sprintf(buffer,"%s Graph loaded in %.2lf seconds with %d nodes and %d edges.\n",timeString8,timee,nodeIndex,lineNum);
	writeLogFile(buffer);
	
	//Allocate cache
	fromToNodes = (int**)malloc(sizeof(int*)*cacheCapacity);
	for(int d=0; d<cacheCapacity; ++d)
		fromToNodes[d] = (int*)malloc(sizeof(int)*2);
	cache = (int**)malloc(sizeof(int*)*cacheCapacity);
	
	
	//Thread Poll ReSize Thread
	pthread_t reSize;
	int resize = 0;
	if(pthread_create(&reSize, NULL, reSizeThreadPool, &resize) != 0){
	    writeLogFile("Pthread Create Error!\n");
		exit(EXIT_FAILURE);
	}
	
	char bufferr[100];
	char *timeString9;
	currtime = time(NULL);
	timeString9 = ctime(&currtime);
	timeString9[strlen(timeString9)-1] = '\0';
	sprintf(bufferr,"%s A pool of %d threads has been created\n",timeString9,threadNum);
	writeLogFile(bufferr);
	Threads = (pthread_t*)malloc(sizeof(pthread_t)*threadNum);
	threads = (int*)malloc(sizeof(int)*maxThreadNum);
	for (int i = 0; i < threadNum; ++i) {
		threads[i] = i;
		if(pthread_create(&Threads[i], NULL, Thread, &threads[i]) != 0){
		    writeLogFile("Pthread Create Error!\n");
			exit(EXIT_FAILURE);
		}
	}
	if(sem_wait(&sem) != 0){
		writeLogFile("Sem_post Error!\n");
		exit(EXIT_FAILURE);
	}
	//socket oluşturuldu
	int sock = socket(AF_INET, SOCK_STREAM, 0); 
	if (sock == -1) { 
		writeLogFile("Socket wasn't created.\n"); 
		exit(EXIT_FAILURE);
	} 
	struct sockaddr_in serveraddr,clientaddr; 
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET; 
	//serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(Port);

	if(bind(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) { 
		writeLogFile("Socket bind is failed.\n"); 
		exit(EXIT_FAILURE);
	}
	//socket hazır
	if ((listen(sock, 10)) != 0) { 
		writeLogFile("Listen is failed\n"); 
		exit(EXIT_FAILURE);
	} 
	socklen_t clientlen = sizeof(clientaddr);
	while(1){
		int buf[2];
		//client kabul et
		client = accept(sock, (struct sockaddr*)&clientaddr, &clientlen); 
		if (client < 0) { 
			writeLogFile("Server acccept is failed.\n"); 
			exit(EXIT_FAILURE);
		} 
		if(read(client,buf,sizeof(buf)) <0){
			writeLogFile("Receive error\n");
			exit(EXIT_FAILURE);
		}
		
		if(pthread_mutex_lock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		int f;
		int ava=0;
		for(f=0; f<threadNum; ++f){
			if(controler[f] != 0)
				ava++;
		}
		if(pthread_mutex_unlock(&mux)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
		if(ava == maxThreadNum){
			char *timeString10;
			currtime = time(NULL);
			timeString10 = ctime(&currtime);
			timeString10[strlen(timeString10)-1] = '\0';
			writeLogFile(timeString10);
			writeLogFile(" No thread is available! Waiting for one.\n");
			if(pthread_mutex_lock(&cont)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_cond_wait(&waitThread, &cont)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_mutex_unlock(&cont)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
		}
		loading = 100.0/(float)threadNum;
		loading = (float)ava*loading;
		if((loading >= 75.0) && (threadNum != maxThreadNum)){
			if(pthread_mutex_lock(&availableMux)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_cond_signal(&reSizeCond)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_cond_wait(&reSizeCond, &availableMux)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_mutex_unlock(&availableMux)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
		}
		if(pthread_mutex_lock(&lock)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}	
		value[0] = buf[0];
		value[1] = buf[1];
		if(pthread_cond_signal(&cond)){
			writeLogFile("Pthread Cond Signal Error!\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_unlock(&lock)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
	}
	
	for(int j=0; j<threadNum; ++j){
		if(pthread_join(Threads[j],NULL) != 0){
			writeLogFile("Pthread Join Error!\n");
			exit(EXIT_FAILURE);
		}
	}
 	
	 
	return 0;
}
void writeLogFile(char *str){
	int control = write (output,str,strlen(str));
	if(control == -1){
		fprintf(stderr,"File Write Error!\n");
		exit(EXIT_FAILURE);
	}
}
void *reSizeThreadPool(void *args){
	int actual = *(int*)args;
	if(actual == 0){}
	while(1){
		if(pthread_mutex_lock(&availableMux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
	
		if(pthread_cond_wait(&reSizeCond, &availableMux)){
			writeLogFile("Pthread Cond Signal Error!\n");
			exit(EXIT_FAILURE);
		}
		
		int num = (threadNum*25)/100;
		if(threadNum+num > maxThreadNum){
			num = maxThreadNum - threadNum;
	
		}
		pthread_t tempThreads[threadNum];
		for(int y=0; y<threadNum; ++y)
			tempThreads[y] = Threads[y];
		free(Threads);
		Threads = (pthread_t*)malloc(sizeof (pthread_t)*(threadNum+num));
		for(int y=0; y<threadNum; ++y)
			Threads[y] = tempThreads[y];
		for(int y=threadNum; y<threadNum+num; ++y){
			threads[y] = y;
			if(pthread_create(&Threads[y], NULL, Thread, &threads[y]) != 0){
				writeLogFile("Pthread Create Error!\n");
				exit(EXIT_FAILURE);
			}
		}
		threadNum = threadNum+num;
		char buf[100];
		time_t currtime;
		char *timeString1;
		currtime = time(NULL);
		timeString1 = ctime(&currtime);
		timeString1[strlen(timeString1)-1] = '\0';
		sprintf(buf,"%s System load %.1f %%, pool extended to %d threads\n",timeString1,loading,threadNum);
		writeLogFile(buf);
		if(pthread_cond_signal(&reSizeCond)){
			writeLogFile("Pthread Cond Signal Error!\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_unlock(&availableMux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
	}
	return NULL;
}
int isThere(int src,int dest){
	for(int i=0; i<cacheSize; ++i){
		if(fromToNodes[i][0] == src && fromToNodes[i][1] == dest)
			return i;
	}
	return -1;

}
void *Thread(void *args){
	int actual = *(int*)args;
	look++;
	if(look == threadNum){
		if(sem_post(&sem) != 0){
			writeLogFile("Sem_post Error!\n");
			exit(EXIT_FAILURE);
		}
	}
	time_t currtime;
	while(1){
		
		char buffer1[100];
		char *timeString1;
		currtime = time(NULL);
		timeString1 = ctime(&currtime);
		timeString1[strlen(timeString1)-1] = '\0';
		sprintf(buffer1,"%s Thread #%d: waiting for connection\n",timeString1,actual);
		writeLogFile(buffer1);
		if(pthread_mutex_lock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		controler[actual] = 0;
		if(pthread_mutex_unlock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_lock(&cont)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_cond_signal(&waitThread)){
			writeLogFile("Pthread Cond Signal Error!\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_unlock(&cont)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_lock(&lock)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_cond_wait(&cond, &lock)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
		}
		values[actual][0] = value[0];
		values[actual][1] = value[1];
		
		if(pthread_mutex_lock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		int ava = 0;
		for(int f=0; f<threadNum; ++f){
			if(controler[f] == 0)
				ava++;
		}
		if(pthread_mutex_unlock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		float load = 100/(float)threadNum;
		load = (float)((threadNum-ava)+1)*load;
		char buff[100];
		char *timeString2;
		currtime = time(NULL);
		timeString2 = ctime(&currtime);
		timeString2[strlen(timeString2)-1] = '\0';
		sprintf(buff,"%s A connection has been delegated to thread id #%d, system load %.1f%%\n",timeString2,actual,load);
		writeLogFile(buff);

		if(pthread_mutex_unlock(&lock)){
			writeLogFile("Mutex Unlock Error\n");
			exit(EXIT_FAILURE);
		}
		if(pthread_mutex_lock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		controler[actual] = 1;
		if(pthread_mutex_unlock(&mux)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		//READER
		if(pthread_mutex_lock(&mutex)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		int fd = client;
		while((AW+WW) > 0){
			WR++;
			if(pthread_cond_wait(&okToRead,&mutex)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
			}
			WR--;
		}
		AR++;
		if(pthread_mutex_unlock(&mutex)){
			writeLogFile("Mutex Unlock Error\n");
			exit(EXIT_FAILURE);
		}
		char buffer2[100];
		char *timeString3;
		currtime = time(NULL);
		timeString3 = ctime(&currtime);
		timeString3[strlen(timeString3)-1] = '\0';
		sprintf(buffer2,"%s Thread #%d: searching database for a path from node %d to node %d\n",timeString3,actual,values[actual][0],values[actual][1]);
		writeLogFile(buffer2);
		int index = isThere(values[actual][0],values[actual][1]);
		if(index > -1){
			int sizeFromCache = cache[index][0];
			int pathFromCache[cache[index][0]];
			int j=0;
			for(int i=1; i<sizeFromCache+1; i++){
				pathFromCache[j] = cache[index][i];
				j++;
			}
			char buffer3[100];
			char *timeString4;
			currtime = time(NULL);
			timeString4 = ctime(&currtime);
			timeString4[strlen(timeString4)-1] = '\0';
			sprintf(buffer3,"%s Thread #%d: path found in database: ",timeString4,actual);
			writeLogFile(buffer3);
			for(int q=0; q<sizeFromCache; ++q){
				char bb[10];
				sprintf(bb,"%d",pathFromCache[q]);
				writeLogFile(bb);
				if(q != sizeFromCache-1)
					writeLogFile("->");
			}
			writeLogFile("\n");
			if(write(fd,&sizeFromCache,sizeof(sizeFromCache)) < 0){
				writeLogFile("Send Error\n");
				exit(EXIT_FAILURE);
			}
			if(write(fd,pathFromCache,sizeof(pathFromCache)) < 0){
				writeLogFile("Send Error\n");
				exit(EXIT_FAILURE);
			}
		}
		if(pthread_mutex_lock(&mutex)){
			writeLogFile("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		AR--;
		if(AR == 0 && WW > 0){
			if(pthread_cond_signal(&okToWrite)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
			}
		}
		if(pthread_mutex_unlock(&mutex)){
			writeLogFile("Mutex Unlock Error\n");
			exit(EXIT_FAILURE);
		}
		//END READER
			
		
		//WRITER
		if(index < 0){
			if(pthread_mutex_lock(&mutex)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
			while ((AW + AR) > 0){
				WW++;
				if(pthread_cond_wait(&okToWrite,&mutex)){
					writeLogFile("Pthread Cond Signal Error!\n");
					exit(EXIT_FAILURE);
				}
				WW--;
			}
			AW++;
			if(pthread_mutex_unlock(&mutex)){
				writeLogFile("Mutex Unlock Error\n");
				exit(EXIT_FAILURE);
			}
			char buffer4[100];
			char *timeString5;
			currtime = time(NULL);
			timeString5 = ctime(&currtime);
			timeString5[strlen(timeString5)-1] = '\0';
			sprintf(buffer4,"%s Thread #%d: no path in database, calculating %d->%d\n",timeString5,actual,values[actual][0],values[actual][1]);
			writeLogFile(buffer4);
			int *path = (int*)malloc(sizeof(int)*nodeIndex);
			for(int i=0; i<graph->nodeNumber; ++i){
				graph->visited[i] = 0;
			}
			int si = bfs(graph,values[actual][0],values[actual][1],path);
			if(si > 1){
				char buffer5[100];
				char *timeString6;
				currtime = time(NULL);
				timeString6 = ctime(&currtime);
				timeString6[strlen(timeString6)-1] = '\0';
				sprintf(buffer5,"%s Thread #%d: path calculated: ",timeString6,actual);
				writeLogFile(buffer5);
				for(int i=0; i<si; ++i){
					char b[10];
					sprintf(b,"%d",path[i]);
					writeLogFile(b);
					if(i != si-1)
						writeLogFile("->");
				}
				writeLogFile("\n");
			}
			if(cacheSize == cacheCapacity){
				int oldcapacity = cacheCapacity;
				int c[cacheSize][2],**p;
				p = (int**)malloc(sizeof(int)*cacheSize);
				for(int i=0; i<cacheSize; ++i){
					p[i] = (int*)malloc(sizeof(int)*(cache[i][0]+1));				
				}
				for(int i=0; i<cacheSize; ++i){
					c[i][0] = fromToNodes[i][0];
					c[i][1] = fromToNodes[i][1];
					p[i][0] = cache[i][0];
					for(int j=1; j<cache[i][0]+1; ++j)
						p[i][j] = cache[i][j];
				
				}
				cacheCapacity = cacheCapacity * 10;
				fromToNodes = (int**)malloc(sizeof(int*)*cacheCapacity);
				for(int i=0; i<cacheCapacity; ++i){
					fromToNodes[i] = (int*)malloc(sizeof(int)*2);			
				}
				cache = (int**)malloc(sizeof(int*)*cacheCapacity);
				for(int i=0; i<oldcapacity; ++i){
					cache[i] = (int*)malloc(sizeof(int)*(p[i][0]+1));
				}
				for(int i=0; i<oldcapacity; ++i){
					fromToNodes[i][0] = c[i][0];
					fromToNodes[i][1] = c[i][1];
					cache[i][0] = p[i][0];
					for(int j=1; j<p[i][0]+1; ++j)
						cache[i][j] = p[i][j];
				}
			}
			if(si > 1){
				char buffer6[100];
				char *timeString7;
				currtime = time(NULL);
				timeString7 = ctime(&currtime);
				timeString7[strlen(timeString7)-1] = '\0';
				sprintf(buffer6,"%s Thread #%d: responding to client and adding path to database\n",timeString7,actual);
				writeLogFile(buffer6);
				fromToNodes[cacheSize][0] = values[actual][0];
				fromToNodes[cacheSize][1] = values[actual][1];
				cache[cacheSize] = (int*)malloc(sizeof(int)*(si+1));
				cache[cacheSize][0] = si;
				int j=0;
				for(int i=1; i<si+1; ++i){
					cache[cacheSize][i] = path[j];
					j++;
				}
				int pathSize = cache[cacheSize][0];
				int pathh[pathSize];
				int p=0;
				for(int i=1; i<cache[cacheSize][0]+1; i++){
					pathh[p] = cache[cacheSize][i];
					p++;
				}
				cacheSize++;
				if(write(fd,&pathSize,sizeof(pathSize)) < 0){
					writeLogFile("Send Error\n");
					exit(EXIT_FAILURE);
				}
				if(write(fd,pathh,sizeof(pathh)) < 0){
					writeLogFile("Send Error\n");
					exit(EXIT_FAILURE);
				}
				
			}
			else{
				char buffer7[100];
				char *timeString8;
				currtime = time(NULL);
				timeString8 = ctime(&currtime);
				timeString8[strlen(timeString8)-1] = '\0';
				sprintf(buffer7,"%s Thread #%d: path not possible from node %d to %d\n",timeString8,actual,values[actual][0],values[actual][1]);
				writeLogFile(buffer7);
				int zero = 0;
				if(write(fd,&zero,sizeof(zero)) < 0){
					writeLogFile("Send Error\n");
					exit(EXIT_FAILURE);
				}
			}
			if(pthread_mutex_lock(&mutex)){
				writeLogFile("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
			AW--;
			if (WW > 0){
				if(pthread_cond_signal(&okToWrite)){
				writeLogFile("Pthread Cond Signal Error!\n");
				exit(EXIT_FAILURE);
				}
			}
			else if (WR > 0){
				if(pthread_cond_broadcast(&okToRead)){
					writeLogFile("Pthread Cond Signal Error!\n");
					exit(EXIT_FAILURE);
				}
			}
			if(pthread_mutex_unlock(&mutex)){
				writeLogFile("Mutex Unlock Error\n");
				exit(EXIT_FAILURE);
			}
		}
		
		close(fd);
		//server her request yolladıgında bir thread o requesti alıp pathi bulur ve clienta yollar.
		//server bir diğer requesti aldığında threadlere yolladıgında bir önceki thread bitmiş olur çünkü
		//her thread çok hızlı çalışıyor.Bu yüzden aynı anda birden fazla thread çalışması için sleep kullanılabilir
		//ödevde yasak olduğu için yoruma alıyorum.
		//sleep(10);
	}
	return NULL;
}
int Adj(struct Graph* graph, int src,int dest){
	int i;
	for(i=0; i<graph->size; ++i){
    	if(graph->arr[i] == src)
    		break;
	}
    struct node* temp = graph->adjArray[i];
	while (temp) {
    	int adjVertex = temp->vertex;
		if(graph->arr[adjVertex] == dest)
			return 1;
      temp = temp->next;
    }
	return 0;
}
int  bfs(struct Graph* graph, int src,int dest,int *result) {
	int *path = (int*)malloc(sizeof(int)*nodeIndex);
	int size=0;
	int abc=0;
	struct queue* q = createQueue(nodeIndex);
	
	int k;
	for(k=0; k<graph->size; ++k){
		if(graph->arr[k] == src)
			break;
	}
	if(k == graph->size){
		writeLogFile("Graph has not the node\n");
		return 0;
	}
	
	graph->visited[src] = 1;
	enqueue(q, src);
	if(src < 0 || dest < 0){
		writeLogFile("Source or Destination node is less than zero.\n");
		return 0;
	}	
	
	int sameNodes = 0;
		
	if(src == dest)
		sameNodes=1;
	int count=0;
  	while (!isEmpty(q)) {
		int currentVertex = dequeue(q);
		int i;
		for(i=0; i<graph->size; ++i){
			if(graph->arr[i] == currentVertex)
				break;
		}
		if(i == graph->size){
			writeLogFile("Graph has not the node\n");
			return 0;
		}
		path[size] = currentVertex;
		size++;
		if((count==1)&&(sameNodes == 1) && (currentVertex == dest))
			sameNodes = 0;
		 count=1;		
		
		if((currentVertex == dest) && (sameNodes == 0)){
			int *tempResult = (int*)malloc(sizeof(int)*nodeIndex);
			int look;
			int s=0;
			for(int i=size-1; i>=0; --i){
				look = path[i];
				tempResult[s] = look;
				s++;
				int j;
				for(j=i-1; j>=0; --j){
					if(Adj(graph,path[j],look) == 1){
						i=j;
					}
				}
				if(i==0){
					tempResult[s] = path[i];
					s++;
				}
			
			}
			int z=0;
			for(int i=s-1; i>=0; i--){
				result[z] = tempResult[i];
				z++;
			}
			return s;
		}
		struct node* temp = graph->adjArray[i];
		while (temp){
			int adjVertex = temp->vertex;
		  	if (graph->visited[adjVertex] == 0) {
		  		if(abc==0){
		  			if(graph->arr[adjVertex] == dest)
		  				abc=1;
					graph->visited[adjVertex] = 1;
					enqueue(q, graph->arr[adjVertex]);
		    	}
		  	}
		  	temp = temp->next;
		}
		struct node* temp2 = (struct node*)malloc(sizeof(struct node));
		temp = temp2;
		free(temp);
		
	}
	size = 0;
	free(q->nodes);
	free(q);
	return size;
}
int control(int nodes[],int size,int num){
	for(int i=0; i<size; ++i){
		if(nodes[i] == num)
			return 0;
	}
	return 1;
}
//node oluştur
struct node* createNode(int v) {
  struct node* newNode = (struct node*) malloc(sizeof(struct node));
  newNode->vertex = v;
  newNode->next = NULL;
  return newNode;
}

//graph oluştur
struct Graph* createAGraph(int vertices){
  struct Graph* graph = malloc(sizeof(struct Graph));
  graph->nodeNumber = vertices;
  graph->adjArray = malloc(vertices * sizeof(struct node*));
  graph->visited = (int*)malloc(vertices * sizeof(int));

  int i;
  for (i = 0; i < vertices; i++) {
    graph->adjArray[i] = NULL;
    graph->visited[i] = 0;
  }

  return graph;
}
//edge ekle
void addEdge(struct Graph* graph, int src, int dest) {
	int i;
	int srcIndex;
	int destIndex;
	for(i = 0; i<graph->size; ++i){
		if(graph->arr[i] == src)
			srcIndex = i;
		if(graph->arr[i] == dest)
			destIndex = i;
	}
	struct node* newNode = createNode(destIndex);
	newNode->next = graph->adjArray[srcIndex];
	graph->adjArray[srcIndex] = newNode;
	
}
//queue oluştur
struct queue* createQueue(int capacity) {
  struct queue* q = (struct queue*)malloc(sizeof(struct queue));
  q->nodes = (int*)malloc(sizeof(int)*capacity);
  q->capacity = capacity;
  q->Size = 0;
  q->front = -1;
  q->rear = -1;
  return q;
}

//queuenun boş olup olmadığını kontrol et
int isEmpty(struct queue* q) {
  if (q->rear == -1)
    return 1;
  else
    return 0;
}

//queueye eleman ekle
void enqueue(struct queue* q, int value) {
   if (q->front == -1)
      q->front = 0;
    q->Size++;
    q->rear++;
    q->nodes[q->rear] = value;
}

//queueden eleman çıkar
int dequeue(struct queue* q) {
  int item;
  if (isEmpty(q)) {
    item = -1;
  } else {
    item = q->nodes[q->front];
    q->Size--;
    q->front++;
    if (q->front > q->rear) {
      q->front = q->rear = -1;
    }
  }
  return item;
}
