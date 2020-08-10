#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#define MAX( a, b ) ( ( a > b) ? a : b )
struct client{
	char name[1000];
	double x;
	double y;
	char flower[1000];
};
struct florist{
	char name[1000];
	double x;
	double y;
	float speed;
	char **flowers;
	int flowerNum;
	struct client *orders;
	int orderNum;
	int total;
	int totalTime;
	int temp;
	int totalOrder;
};
struct sale{
	char name[1000];
	int number;
	int time;
};
int floristNum;
int clientNum;
int totalOrderNum;
struct florist *florists;
struct client *clients;
pthread_cond_t *cond;
pthread_mutex_t *lock;
pthread_cond_t sinyal;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
sigset_t mymask;
int flag1=0,flag2=0,flag3=0,flag4=0,flag5=0;
int info=0;
int finish=0;
void readFlorists(int file);
void readClients(int file);
void *floristThread(void *args);
double ChebyshevDistance(double x1,double y1,double x2,double y2);
int control(char **flowers,int size,char *flower);
void findFloristToOrder(struct client c,int a);
void findFloristNum(int input);
void findClientNum(int input);
void mySigintHandler(int signal){
 	printf("Signal SIGINT is caught %d.\n", signal);
 	printf("Deallocating all resources.\n");
 	printf("All threads shut down gracefully.\n");
 
 	if(flag1 == 1){
	 	for(int i=0; i<floristNum; ++i){
			free(florists[i].orders);
		}
	}
	if(flag2 == 1){
		for(int i=0; i<floristNum; ++i){
			for(int j=0; j<florists[i].flowerNum; ++j)
				free(florists[i].flowers[j]);
		}
		for(int i=0; i<floristNum; ++i){
			free(florists[i].flowers);
		}
		free(florists);
	}
	if(flag3 == 1)
		free(clients);
	if(flag4 == 1)
		free(cond);
	if(flag5 == 1)
		free(lock);
	
 	_exit(0);
}
int main(int argc, char *argv[]){
	int opt;
	char* filePath = NULL;
	while((opt = getopt(argc, argv,"i:")) != -1)  
	{
		switch(opt)
		{
        	case 'i':
		        filePath = optarg;
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
	//SIGINT
	struct sigaction sint;
	memset(&sint,0,sizeof(sint));
	sint.sa_handler = &mySigintHandler;
	sigaction(SIGINT,&sint,NULL);
	
	
	if((sigemptyset(&mymask) == -1) || (sigaddset(&mymask,SIGINT) == -1)){
		perror("Failed to initialize the signal mask!");     
		exit(EXIT_FAILURE);
	}
	printf("Florist application initializing from file: %s\n",filePath);
	floristNum=0;
	clientNum=0;
	totalOrderNum = 0;
	//open file
	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
	int input = open(filePath,O_RDONLY,mode);
	if(input < 0){
		perror("Open File Error");
		exit(EXIT_FAILURE);
	}
	//find florist Number
	findFloristNum(input);
	//find client Number
	findClientNum(input);
	//go to the beginning of the file
	lseek(input,0,SEEK_SET);
	//allocate
	florists = (struct florist*)malloc(sizeof(struct florist)*floristNum);
	flag2=1;
	clients = (struct client*)malloc(sizeof(struct client)*clientNum);
	flag3=1;
	//read Florists from file
	readFlorists(input);
	//allocate for cond and lock array
	cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t)*(floristNum));
	flag4=1;
	lock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t)*(floristNum+1));
	flag5=1;
	//condition variables init
	for(int i=0; i<floristNum; ++i){
		if(pthread_cond_init(&cond[i],NULL) != 0){
			perror("Pthread Condition Init Error!");
			exit(EXIT_FAILURE);
		}
	}
	if(pthread_cond_init(&sinyal,NULL) != 0){
		perror("Pthread Condition Destroy Error!");
		exit(EXIT_FAILURE);
	}
	//Mutex init
	for(int i=0; i<floristNum+1; ++i){
		lock[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	}
	//read Clients from file
	readClients(input);
	//close file
	if(close(input) == -1){
		perror("File Close Error!");
		exit(EXIT_FAILURE);
	}
	for(int i=0; i<floristNum; ++i){
		florists[i].orders = (struct client*)malloc(sizeof(struct client)*florists[i].temp);
	}
	flag1=1;
	if(sigprocmask(SIG_BLOCK,&mymask,NULL) == -1){
		perror("ERROR SIG_BLOCK");     
		exit(EXIT_FAILURE);
	}
	sigprocmask (SIG_SETMASK, &mymask, NULL);
	//create Florist Threads
	int temp[floristNum];
	for(int i=0; i<floristNum; ++i)
		temp[i] = i;
	printf("%d florists have been created\n",floristNum);
	pthread_t floristThreads[floristNum];
	for (int i = 0; i <floristNum; ++i) {
		if(pthread_create(&floristThreads[i], NULL, floristThread, &temp[i]) != 0){
		    perror("Pthread Create Error!");
			exit(EXIT_FAILURE);
		}
	}
	//send order to Florists
	printf("Processing requests\n");
	for(int i=0; i<clientNum; ++i){
		findFloristToOrder(clients[i],0);
	}
	//wait for orders to finish after sending all orders
	if(pthread_mutex_lock(&mutex2)){
		perror("Mutex Lock Error\n");
		exit(EXIT_FAILURE);
	}
	info=1;
	if(pthread_cond_wait(&sinyal, &mutex2)){
		perror("Pthread Cond Signal Error!");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_unlock(&mutex2)){
		perror("Mutex Unlock Error\n");
		exit(EXIT_FAILURE);
	}
	printf("All requests processed.\n");
	//send signal threads to finish
	for(int j=0; j<floristNum; ++j){
		for(int i=0; i<floristNum; ++i){
			if(pthread_cond_signal(&cond[i])){
				perror("Pthread Cond Signal Error!");
				exit(EXIT_FAILURE);
			}
		}
	}
	struct sale deneme[floristNum];
	//wait for the threads to finish and take return values
	struct sale *values[floristNum];
	for(int j=0; j<floristNum; ++j){
		if(pthread_join(floristThreads[j],(void**)&values[j]) != 0){
			perror("Pthread Join Error!");
			exit(EXIT_FAILURE);
		}
		strcpy(deneme[j].name,(*(struct sale*)values[j]).name);
		deneme[j].number = (*(struct sale*)values[j]).number;
		deneme[j].time = (*(struct sale*)values[j]).time;
		free(values[j]);
	}
	if(sigprocmask(SIG_UNBLOCK,&mymask,NULL) == -1){
		perror("ERROR SIG_UNBLOCK!");     
		exit(EXIT_FAILURE);
	}
	//print sale statistics with data from threads
	printf("Sale statistics for today:\n");
	printf("-------------------------------------------------\n");
	printf("Florist	 	# of sales	 Total time\n");
	printf("-------------------------------------------------\n");
	for(int i=0; i<floristNum; ++i){
		printf("%s		%d		%dms\n",deneme[i].name,deneme[i].number,deneme[i].time);
	}
	printf("-------------------------------------------------\n");
	//Condition variable destroy
	int s;
	for(int i=0; i<floristNum; ++i){
		s = pthread_cond_destroy(&cond[i]);
		if(s != 0){
			perror("Pthread Condition Destroy Error!");
			exit(EXIT_FAILURE);
		}
	}
	s = pthread_cond_destroy(&sinyal);
	if(s != 0){
		perror("Pthread Condition Destroy Error!");
		exit(EXIT_FAILURE);
	}
	//free
	for(int i=0; i<floristNum; ++i){
		free(florists[i].orders);
	}
	for(int i=0; i<floristNum; ++i){
		for(int j=0; j<florists[i].flowerNum; ++j)
			free(florists[i].flowers[j]);
	}
	for(int i=0; i<floristNum; ++i){
		free(florists[i].flowers);
	}
	free(florists);
	free(clients);
	free(cond);
	free(lock);
	
	return 0;
	
}
//read florists from file
void findFloristNum(int input){
	char oneChar ='c';
	while(1){
		while(oneChar != '\n'){
			read(input,&oneChar,1);	
		}
		floristNum++;
		read(input,&oneChar,1);
		if(oneChar == '\n')
			break;
	}
}
//read clients from file
void findClientNum(int input){
	char oneChar ='c';
	while(1){
		while(oneChar != '\n'){
			read(input,&oneChar,1);	
		}
		clientNum++;
		int a = read(input,&oneChar,1);
		if(oneChar == '\n' || a == 0)
			break;
	}
}
void *floristThread(void *args){
	srand(time(0));
	int i=0;
	int f = *(int*)args;
	int temp=0;
	struct sale *output = (struct sale*)malloc(sizeof(struct sale));
	while(temp == 0){ 
		if(pthread_mutex_lock(&lock[f])){
			perror("Mutex Lock Error\n");
			exit(EXIT_FAILURE);
		}
		//if there is not order,wait
		if((florists[f].orderNum == 0) && (florists[f].total < florists[f].temp)){
			if(pthread_cond_wait(&cond[f], &lock[f])){
				perror("Pthread Cond Signal Error!");
				exit(EXIT_FAILURE);
			}
		}
		//if there is order,prepare and delivery
		if(florists[f].orderNum != 0){
			int preparation = (rand() % 250) + 1;
			double delivery = ChebyshevDistance(florists[f].orders[i].x,florists[f].orders[i].y,florists[f].x,florists[f].y) / florists[f].speed;
			double time = (preparation+delivery);
			florists[f].totalTime += time;
			usleep(time*1000);
			printf("Florist %s has delivered a %s to %s in %dms\n",florists[f].name,florists[f].orders[i].flower,florists[f].orders[i].name,(int)time);
			i++;
			florists[f].orderNum = florists[f].orderNum - 1;
			florists[f].totalOrder = florists[f].totalOrder + 1;
			
			if(pthread_mutex_lock(&lock[floristNum])){
				perror("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
			totalOrderNum = totalOrderNum + 1;
			if(pthread_mutex_unlock(&lock[floristNum])){
				perror("Mutex Unlock Error\n");
				exit(EXIT_FAILURE);
			}
			
		}
		if(info == 1 && totalOrderNum == clientNum && finish == floristNum-1){
			if(pthread_cond_signal(&sinyal)){
				perror("Pthread Cond Signal Error!");
				exit(EXIT_FAILURE);
			}
			if(pthread_cond_wait(&cond[f], &lock[f])){
				perror("Pthread Cond Signal Error!");
				exit(EXIT_FAILURE);
			}
			finish++;
			temp=1;	
		}
		if(florists[f].totalOrder == florists[f].temp && finish < floristNum-1){
			if(pthread_mutex_lock(&mutex1)){
				perror("Mutex Lock Error\n");
				exit(EXIT_FAILURE);
			}
			finish++;
			if(pthread_mutex_unlock(&mutex1)){
				perror("Mutex Unlock Error\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_cond_wait(&cond[f], &lock[f])){
				perror("Pthread Cond Signal Error!");
				exit(EXIT_FAILURE);
			}
			break;
		}
		if(pthread_mutex_unlock(&lock[f])){
			perror("Mutex Unlock Error\n");
			exit(EXIT_FAILURE);
		}
	}
	strcpy(output->name,florists[f].name);
	output->number = florists[f].totalOrder;
	output->time = florists[f].totalTime;
	
	printf("%s closing shop.\n",florists[f].name);
	
	return (void*)output;
}
void findFloristToOrder(struct client order,int a){
	double min;
	int index;
	int c=1;
	for(int j=0; j<floristNum; ++j){
			if(control(florists[j].flowers,florists[j].flowerNum,order.flower) == 1){
				double tmp = ChebyshevDistance(order.x,order.y,florists[j].x,florists[j].y);
				if(c == 1){
					min = tmp;
					index = j;
					c = 0;
				}
				else{
					if(tmp < min){
						min = tmp;
						index = j;
					}
				}
			}
		}
		if(a == 0){
			c=1;
			if(pthread_mutex_lock(&lock[index])){
				perror("Mutex Lock Error!");
				exit(EXIT_FAILURE);
			}
			c=1;			
			strcpy(florists[index].orders[florists[index].total].name,order.name);
			florists[index].orders[florists[index].total].x = order.x;
			florists[index].orders[florists[index].total].y = order.y;
			strcpy(florists[index].orders[florists[index].total].flower,order.flower);
			florists[index].orderNum = florists[index].orderNum + 1;
			florists[index].total = florists[index].total + 1;
			if(pthread_mutex_unlock(&lock[index])){
				perror("Mutex Unlock Error\n");
				exit(EXIT_FAILURE);
			}
			if(pthread_cond_signal(&cond[index])){
				perror("Pthread Cond Signal Error!");
				exit(EXIT_FAILURE);
			}
		}
		if(a == 1)
			(florists[index].temp)++;
}
int control(char **flowers,int size,char *flower){
	for(int i=0; i<size; ++i){
		if(strcmp(flowers[i],flower) == 0)
			return 1;
	}
	return 0;
}
double ChebyshevDistance(double x1,double y1,double x2,double y2){
	double num1 = fabs(x1-x2);
	double num2 = fabs(y1-y2);
	return MAX(num1,num2);

}
void readFlorists(int input){
	int index=0;
	int oneChar = 'c';
	char isim[1000];
	char coordx[1000];
	char coordy[1000];
	char speedd[1000];
	int i=-1;
	read(input,&oneChar,1);
	while(index != floristNum){	
		florists[index].flowerNum = 0;
		i=-1;
		while(oneChar != ' '){
			i++;
			isim[i] = oneChar;
			read(input,&oneChar,1);
		}
		isim[++i] = '\0';
		strcpy(florists[index].name,isim);
		i=-1;
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		while(oneChar != ','){
			i++;
			coordx[i] = oneChar;
			read(input,&oneChar,1);
		}
		coordx[++i] = '\0';
		florists[index].x = atof(coordx);
		i=-1;
		read(input,&oneChar,1);
		while(oneChar != ';'){
			i++;
			coordy[i] = oneChar;
			read(input,&oneChar,1);
		}
		coordy[++i] = '\0';
		florists[index].y = atof(coordy);
		i=-1;
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		while(oneChar != ')'){
			i++;
			speedd[i] = oneChar;
			read(input,&oneChar,1);
		}
		speedd[++i] = '\0';
		florists[index].speed = atof(speedd);
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		while(oneChar != '\n'){
			if(oneChar == ' '){
				char tempFlowers[florists[index].flowerNum][100];
				for(int k=0; k<florists[index].flowerNum; ++k)
					strcpy(tempFlowers[k],florists[index].flowers[k]);
				int tmp = florists[index].flowerNum;
				if(florists[index].flowerNum != 0){
					for(int j=0; j<florists[index].flowerNum; ++j)
						free(florists[index].flowers[j]);
					free(florists[index].flowers);	
				}
				(florists[index].flowerNum)++;
				florists[index].flowers = (char**)malloc(sizeof(char*) * florists[index].flowerNum);
				for(int k=0; k<florists[index].flowerNum; ++k)
					florists[index].flowers[k] = (char*)malloc(sizeof(char) * 100);
				for(int k=0; k<tmp; ++k)
					strcpy(florists[index].flowers[k],tempFlowers[k]);
				
			}
			read(input,&oneChar,1);
			int j=-1;
			while(oneChar != ',' && oneChar != '\n'){
				j++;
				florists[index].flowers[(florists[index].flowerNum)-1][j] = oneChar;
				read(input,&oneChar,1);
			}
			florists[index].flowers[(florists[index].flowerNum)-1][++j] = '\0';
			if(oneChar == '\n'){
				break;
			}
			else
				read(input,&oneChar,1);
		}
		florists[index].orderNum = 0;
		florists[index].total = 0;
		florists[index].totalTime = 0;
		florists[index].temp = 0;
		florists[index].totalOrder = 0;
		read(input,&oneChar,1);
		if(oneChar == '\n')
			break;
		else{
			index++;
		}
			
	}
}
void readClients(int input){
	int index=0;
	int oneChar = 'c';
	int i=-1;
	read(input,&oneChar,1);
	while(index != clientNum){
		i=-1;
		char str1[1000];
		while(oneChar != ' '){
			i++;
			str1[i] = oneChar;
			read(input,&oneChar,1);
		}
		str1[++i] = '\0';
		strcpy(clients[index].name,str1);
		i=-1;
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		char str2[1000];
		while(oneChar != ','){
			i++;
			str2[i] = oneChar;
			read(input,&oneChar,1);
		}
		str2[++i] = '\0';
		clients[index].x = atof(str2);
		i=-1;
		read(input,&oneChar,1);
		char str3[1000];
		while(oneChar != ')'){
			i++;
			str3[i] = oneChar;
			read(input,&oneChar,1);
		}
		str3[++i] = '\0';
		clients[index].y = atof(str3);
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		read(input,&oneChar,1);
		char str4[1000];
		i=-1;
		while(oneChar != '\n'){
			i++;
			str4[i] = oneChar;
			read(input,&oneChar,1);
		}
		str4[++i] = '\0';
		strcpy(clients[index].flower,str4);
		int a = read(input,&oneChar,1);
		if(oneChar == '\n' || a==0)
			break;
		else{
			index++;
		}
	}
	for(int i=0; i<clientNum; ++i){
		findFloristToOrder(clients[i],1);
	}
}
