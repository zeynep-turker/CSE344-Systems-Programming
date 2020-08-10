#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <semaphore.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h> 
#include <sys/shm.h>
#include <sys/mman.h>
#include <pthread.h>
#include <time.h>
static char *ingredient; //malzemelerin teslim edildiği array
static int 	sinyal;//pusher ve cheflerin sonlanmasında etkilidir.
static int 	count;/*Son line okunduktan sonra son malzemler için gerekli olan chef işini bitirdikten sonra bütün cheflerin sonlanmasın için gereklidir.*/
static int 	countP;/*Son line okunduktan sonra son malzemeler için gerekli olan 2 pusher işini bitirdikten sonra bütün pusherların sonlaması için gereklidir.*/
sem_t milk;
sem_t flour;
sem_t walnuts;
sem_t sugar;
sem_t lock;
sem_t wholesaler; //wholesaler bu semaphore ile chef in tatlıyı yapmasını bekler.
sem_t chef1; //milk ve flour bekler
sem_t chef2; //milk ve walnuts bekler
sem_t chef3; //milk ve sugar bekler
sem_t chef4; //flour ve walnuts bekler
sem_t chef5; //flour ve sugar bekler
sem_t chef6; //walnuts ve sugar bekler
sem_t tempSem1;
sem_t tempSem2;
sem_t oneChef;

int items[4] = {0,0,0,0};
//her şef kendisi için gerekli malzemeler gelene kadar bekler.
void *Chefs (void *args);
//wholesaler in getirmiş olduğu malzemelere bakıp ona göre belirli şefi uyandırır.
void *lookMilk(void *args);
void *lookFlour(void *args);
void *lookWalnuts(void *args);
void *lookSugar(void *args);
int main(int argc, char *argv[]){
	int opt;
	char* filePath;
	
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
	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
	int input = open(filePath,O_RDONLY,mode);
	if(input < 0){
		perror("Open File Error");
		exit(EXIT_FAILURE);
	}
	//Dosyada en az 10 line olacağı için dosyada 10 line olup olmadığı kontrol edilir.
	int oneChar = 'c';
	int lineNum = 0;
	int indexx = 0;
	while(read(input,&oneChar,1) != 0){
		if(oneChar != '\n'){
			indexx++;	
		}
		if(indexx == 2){
			lineNum++;
			indexx=0;
		}
		if(lineNum == 10)
			break;
	}
	//eğer dosya boşsa program sonlanır.
	if(lineNum == 0){
		printf("File is empty!\n");
		exit(EXIT_FAILURE);
	}
	//eğer 10 line yoksa program sonlanır.
	if(lineNum < 10){
		printf("File has not 10 line!\n");
		exit(EXIT_FAILURE);
	}
	//eğer 10 line var ise tekrar dosyanın en başına gidilir.
	lseek(input,0,SEEK_SET);
	//sem_init
	if(sem_init(&milk,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&flour,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&walnuts,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&sugar,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&lock,0,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&chef1,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&chef2,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&chef3,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&chef4,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&chef5,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&chef6,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&wholesaler,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&tempSem1,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&tempSem2,0,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(&oneChef,0,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	//lookMilk,lookFlour,lookWalnuts,lookSugar threadlerini oluşturdum.
	pthread_t pushers[4];
	countP = 2;
	int temp=1;
	//milk
	if(pthread_create(&pushers[0], NULL, lookMilk, &temp) != 0){
		perror("Pthread Create Error!");
		exit(EXIT_FAILURE);
	}
	//flour
	if(pthread_create(&pushers[1], NULL, lookFlour, &temp) != 0){
		perror("Pthread Create Error!");
		exit(EXIT_FAILURE);
	}
	//walnuts
	if(pthread_create(&pushers[2], NULL, lookWalnuts, &temp) != 0){
		perror("Pthread Create Error!");
		exit(EXIT_FAILURE);
	}
	//sugar
	if(pthread_create(&pushers[3], NULL, lookSugar, &temp) != 0){
		perror("Pthread Create Error!");
		exit(EXIT_FAILURE);
	}
	//6 tane chef threadi oluşturdum.
	pthread_t chefs[6];
	int Chef[6];
	sinyal=0;
	count = 1;
	for (int i = 0; i < 6; ++i) {
		Chef[i] = i+1;
		if(pthread_create(&chefs[i], NULL, Chefs, &Chef[i]) != 0){
		    perror("Pthread Create Error!");
			exit(EXIT_FAILURE);
		}
	}
	ingredient = (char*)malloc(sizeof(char)*3);
	char c = 'c';
	int index=0;
	read(input,&c,1);
	int b=1;
	while(1){
		if(b == 0)
			break;
		if(c != '\n'){
			ingredient[index] = c;
			index++;
			read(input,&c,1);	
		}
		else{
			b = read(input,&c,1);
			//eğer okunan line,dosyadaki son satırsa dosyanın biteceğini şef ve pusherlara bildirmek için sinyal değişkeni 5 yapılır.
			if(b == 0){
				sinyal = 5;
			}
			//dosyadan bir line okunduktan sonra okunan harflere göre içerikler gönderilir.
			if(index == 2){
				ingredient[index] = '\0';
				//M harfi okunduysa lookMilk e milk gönderilir.
				if(ingredient[0] == 'M' || ingredient[1] == 'M'){
					if(sem_post(&milk) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//F harfi okunduysa lookFlour a flour gönderilir.
				if(ingredient[0] == 'F' || ingredient[1] == 'F'){
					if(sem_post(&flour) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//W harfi okunduysa lookWalnuts a walnuts gönderilir.
				if(ingredient[0] == 'W' || ingredient[1] == 'W'){
					if(sem_post(&walnuts) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//S harfi okunduysa lookSugar a sugar gönderilir.
				if(ingredient[0] == 'S' || ingredient[1] == 'S'){
					if(sem_post(&sugar) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//whoselar şef tatlıyı yapana kadar bekler.
				if(sem_wait(&wholesaler) != 0){
					perror("Sem_wait Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son line okunduğunda while den çıkılır.
				if(b == 0)
					break;
			}
			index=0;			
		}
		
	}
	//dosyayla işim bittiği için kapadım.
	if(close(input) == -1){
		perror("File Close Error!");
		exit(EXIT_FAILURE);
	}
	sinyal = 5;
	//pusherın son malzemeleride okuması beklenir.
	if(sem_wait(&tempSem1) != 0){
		perror("Sem_wait Error!");
		exit(EXIT_FAILURE);
	}
	//bütün pusherların işi bittikten sonra semaphore da bekleyen pusherları da bitirmek için her pusher için postlama yaptım.
	if(sem_post(&milk) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&flour) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&walnuts) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&sugar) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	//chefin son malzemelerle tatlı yapması beklenir.
	if(sem_wait(&tempSem2) != 0){
		perror("Sem_wait Error!");
		exit(EXIT_FAILURE);
	}
	//bütün şeflerin işi bittikten sonra semaphore da bekleyen şefleri de bitirmek için her şef için postlama yaptım.
	if(sem_post(&chef1) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&chef2) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&chef3) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&chef4) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&chef5) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_post(&chef6) != 0){
		perror("Sem_post Error!");
		exit(EXIT_FAILURE);
	}
	//Join
	for(int j=0; j<4; ++j){
		if(pthread_join(pushers[j],NULL) != 0){
			perror("Pthread Join Error!");
			exit(EXIT_FAILURE);
		}
	}
	for(int j=0; j<6; ++j){
		if(pthread_join(chefs[j],NULL) != 0){
			perror("Pthread Join Error!");
			exit(EXIT_FAILURE);
		}
	}
	//sem destroy
	if(sem_destroy(&milk) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&flour) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&walnuts) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&sugar) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&chef1) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&chef2) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&chef3) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&chef4) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&chef5) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&chef6) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&lock) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&wholesaler) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&tempSem1) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&tempSem2) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(&oneChef) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	
	//free
	free(ingredient);

	return 0;
}
void *lookMilk(void *args){
	int actual_args = *(int*)args;
	if(actual_args == 1){
		while(1){
			//wholesaler den milk gelmesini bekler.
			if(sem_wait(&milk) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&lock) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sinyal == 5){
				countP--;
				//eğer dosyadaki son line da şefe teslim edildiyse lookMilk biter.
				if(countP != 0 && countP != 1){
					if(sem_post(&lock) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}
			if(countP == 2 || countP == 1 || countP == 0){ 
				//eğer flour da var ise chef1 içerikleri alır.
				if(items[1] == 1){
					items[1] = 0;
					if(sem_post(&chef1) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer walnuts da var ise chef2 içerikleri alır.
				else if(items[2] == 1){
					items[2] = 0;
					if(sem_post(&chef2) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer sugar da var ise chef3 içerikleri alır.
				else if(items[3] == 1){
					items[3] = 0;
					if(sem_post(&chef3) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer milkten başka içerik yoksa milk depolanır.
				else{
					items[0] = 1;
				}
			}
			if(countP == 0){
				if(sem_post(&tempSem1) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&lock) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}
	}
	return NULL;
}
void *lookFlour(void *args){
	int actual_args = *(int*)args;
	if(actual_args == 1){
		while(1){
			//wholesaler den flour gelmesini bekler.
			if(sem_wait(&flour) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&lock) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sinyal == 5){
				countP--;
				//eğer dosyadaki son line da şefe teslim edildiyse lookFlour biter.
				if(countP != 0 && countP != 1){
					if(sem_post(&lock) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}  
			if(countP == 2 || countP == 1 || countP == 0){  	
				//eğer milk de var ise chef1 içerikleri alır.
				if(items[0] == 1){
					items[0] = 0;
					if(sem_post(&chef1) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer walnuts da var ise chef4 içerikleri alır.
				else if(items[2] == 1){
					items[2] = 0;
					if(sem_post(&chef4) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer sugar da var ise chef5 içerikleri alır.
				else if(items[3] == 1){
					items[3] = 0;
					if(sem_post(&chef5) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer başka içerik yoksa flour depolanır.
				else{
					items[1] = 1;
				}
			}
			if(countP == 0){
				if(sem_post(&tempSem1) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&lock) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}
	}
	return NULL;
}
void *lookWalnuts(void *args){
	int actual_args = *(int*)args;
	if(actual_args == 1){
		while(1){
			//wholesaler den walnuts gelmesini bekler.
			if(sem_wait(&walnuts) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&lock) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sinyal == 5){
				countP--;
				//eğer dosyadaki son line da şefe teslim edildiyse lookWalnuts biter.
				if(countP != 0 && countP != 1){
					if(sem_post(&lock) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}  
			if(countP == 2 || countP == 1 || countP == 0){  	
				//eğer milk de var ise chef2 içerikleri alır.	
				if(items[0] == 1){
					items[0] = 0;
					if(sem_post(&chef2) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer flour da var ise chef4 içerikleri alır.
				else if(items[1] == 1){
					items[1] = 0;
					if(sem_post(&chef4) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer walnuts da var ise chef6 içerikleri alır.
				else if(items[3] == 1){
					items[3] = 0;
					if(sem_post(&chef6) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer başka içerik yoksa walnuts depolanır.
				else{
					items[2] = 1;
				}
			}
			if(countP == 0){
				if(sem_post(&tempSem1) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&lock) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}
	}
	return NULL;
}
void *lookSugar(void *args){
	int actual_args = *(int*)args;
	if(actual_args == 1){
		while(1){
			//wholesaler den sugar gelmesini bekler.
			if(sem_wait(&sugar) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&lock) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sinyal == 5){
				countP--;
				//eğer dosyadaki son line da şefe teslim edildiyse lookSugar biter.
				if(countP != 0 && countP != 1){
					if(sem_post(&lock) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}
			if(countP == 2 || countP == 1 || countP == 0){  
				//eğer milk de var ise chef3 içerikleri alır.	
				if(items[0] == 1){
					items[0] = 0;
					if(sem_post(&chef3) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer flour da var ise chef5 içerikleri alır.
				else if(items[1] == 1){
					items[1] = 0;
					if(sem_post(&chef5) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer walnuts da var ise chef6 içerikleri alır.
				else if(items[2] == 1){
					items[2] = 0;
					if(sem_post(&chef6) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
				}
				//eğer başka içerik yoksa sugar depolanır.
				else{
					items[3] = 1;
				}
			}
			if(countP == 0){
				if(sem_post(&tempSem1) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&lock) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}
	}
	return NULL;

}
void *Chefs (void *args) {
   int actual_args = *(int*)args;
    srand(time(0));
    if(actual_args == 1){//wait for M and F
    	while(1){
    		//whoseların koymuş olduğu içerikler milk-flour mu diye bakılır.
    		printf("chef%d is waiting for milk and flour\n",actual_args);
    		if(sem_wait(&chef1) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&oneChef) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
    		if(sinyal == 5){
    			count--;
				if(sem_post(&tempSem2) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son linedaki içeriklerden tatlı yapılmış mı diye bakılır eğer yapıldıysa biter.
				if(count != 0){
					if(sem_post(&oneChef) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}		
    		if(count == 1 || count == 0){
				printf("the wholesaler delivers milk and flour\n");
				printf("chef%d has taken the milk\n",actual_args);
				printf("chef%d has taken the flour\n",actual_args);
				printf("the wholesaler is waiting for the desert\n");
				printf("chef%d is preparing the dessert\n",actual_args);
				int random = (rand() % 5) + 1;
				sleep(random);
				printf("chef%d has delivered the dessert to the wholesaler\n",actual_args);
				printf("the wholesaler has obtained the dessert and left to sell it\n");
				if(sem_post(&wholesaler) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&oneChef) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		
		}
    }
    else if(actual_args == 2){//wait for M and W
    	while(1){
    		//whoseların koymuş olduğu içerikler milk-walnuts mu diye bakılır.
    		printf("chef%d is waiting for milk and walnuts\n",actual_args);
    		if(sem_wait(&chef2) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&oneChef) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
    		if(sinyal == 5){
    			count--;
				if(sem_post(&tempSem2) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son linedaki içeriklerden tatlı yapılmış mı diye bakılır eğer yapıldıysa biter.
				if(count != 0){
					if(sem_post(&oneChef) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}    	
			if(count == 1 || count == 0){
				printf("the wholesaler delivers milk and walnuts\n");
				printf("chef%d has taken the milk\n",actual_args);
				printf("chef%d has taken the walnuts\n",actual_args);
				printf("the wholesaler is waiting for the desert\n");
				printf("chef%d is preparing the dessert\n",actual_args);
				int random = (rand() % 5) + 1;
				sleep(random);
				printf("chef%d has delivered the dessert to the wholesaler\n",actual_args);
				printf("the wholesaler has obtained the dessert and left to sell it\n");
				if(sem_post(&wholesaler) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&oneChef) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}
    }
    else if(actual_args == 3){//wait for M and S
    	while(1){
    		//whoseların koymuş olduğu içerikler milk-sugar mu diye bakılır.
    		printf("chef%d is waiting for milk and sugar\n",actual_args);
    		if(sem_wait(&chef3) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&oneChef) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
    		if(sinyal == 5){
    			count--;
				if(sem_post(&tempSem2) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son linedaki içeriklerden tatlı yapılmış mı diye bakılır eğer yapıldıysa biter.
				if(count != 0){
					if(sem_post(&oneChef) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}    	
    		if(count == 1 || count == 0){
				printf("the wholesaler delivers milk and sugar\n");
				printf("chef%d has taken the milk\n",actual_args);
				printf("chef%d has taken the sugar\n",actual_args);
				printf("the wholesaler is waiting for the desert\n");
				printf("chef%d is preparing the dessert\n",actual_args);
				int random = (rand() % 5) + 1;
				sleep(random);
				printf("chef%d has delivered the dessert to the wholesaler\n",actual_args);
				printf("the wholesaler has obtained the dessert and left to sell it\n");
				if(sem_post(&wholesaler) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&oneChef) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}

    }
    else if(actual_args == 4){//wait for F and W
    	while(1){
    		//whoseların koymuş olduğu içerikler flour-walnuts mu diye bakılır.
    		printf("chef%d is waiting for flour and walnuts\n",actual_args);
    		if(sem_wait(&chef4) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&oneChef) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
    		if(sinyal == 5){
    			count--;
				if(sem_post(&tempSem2) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son linedaki içeriklerden tatlı yapılmış mı diye bakılır eğer yapıldıysa biter.
				if(count != 0){
					if(sem_post(&oneChef) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}    	
			if(count == 1 || count == 0){
				printf("the wholesaler delivers flour and walnuts\n");
				printf("chef%d has taken the flour\n",actual_args);
				printf("chef%d has taken the walnuts\n",actual_args);
				printf("the wholesaler is waiting for the desert\n");
				printf("chef%d is preparing the dessert\n",actual_args);
				int random = (rand() % 5) + 1;
				sleep(random);
				printf("chef%d has delivered the dessert to the wholesaler\n",actual_args);
				printf("the wholesaler has obtained the dessert and left to sell it\n");
				if(sem_post(&wholesaler) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&oneChef) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
		}
    }
    else if(actual_args == 5){//wait for F and S
    	while(1){
    		//whoseların koymuş olduğu içerikler flour-sugar mu diye bakılır.
    		printf("chef%d is waiting for flour and sugar\n",actual_args);
    		if(sem_wait(&chef5) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&oneChef) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
    		if(sinyal == 5){
    			count--;
				if(sem_post(&tempSem2) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son linedaki içeriklerden tatlı yapılmış mı diye bakılır eğer yapıldıysa biter.
				if(count != 0){
					if(sem_post(&oneChef) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}
			if(count == 1 || count == 0){
				printf("the wholesaler delivers flour and sugar\n");
				printf("chef%d has taken the flour\n",actual_args);
				printf("chef%d has taken the sugar\n",actual_args);
				printf("the wholesaler is waiting for the desert\n");
				printf("chef%d is preparing the dessert\n",actual_args);
				int random = (rand() % 5) + 1;
				sleep(random);
				printf("chef%d has delivered the dessert to the wholesaler\n",actual_args);
				printf("the wholesaler has obtained the dessert and left to sell it\n");
				if(sem_post(&wholesaler) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&oneChef) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
			
		}
    }
    else if(actual_args == 6){//wait for W and S
    	while(1){
    		//whoseların koymuş olduğu içerikler walnuts-sugar mu diye bakılır.
    		printf("chef%d is waiting for walnuts and sugar\n",actual_args);
    		if(sem_wait(&chef6) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
			if(sem_wait(&oneChef) != 0){
				perror("Sem_wait Error!");
				exit(EXIT_FAILURE);
			}
    		if(sinyal == 5){
    			count--;
				if(sem_post(&tempSem2) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
				//dosyadaki son linedaki içeriklerden tatlı yapılmış mı diye bakılır eğer yapıldıysa biter.
				if(count != 0){
					if(sem_post(&oneChef) != 0){
						perror("Sem_post Error!");
						exit(EXIT_FAILURE);
					}
					break;
				}
			}
			if(count == 1 || count == 0){
				printf("the wholesaler delivers walnuts and sugar\n");
				printf("chef%d has taken the walnuts\n",actual_args);
				printf("chef%d has taken the sugar\n",actual_args);
				printf("the wholesaler is waiting for the desert\n");
				printf("chef%d is preparing the dessert\n",actual_args);
				int random = (rand() % 5) + 1;
				sleep(random);
				printf("chef%d has delivered the dessert to the wholesaler\n",actual_args);
				printf("the wholesaler has obtained the dessert and left to sell it\n");
				if(sem_post(&wholesaler) != 0){
					perror("Sem_post Error!");
					exit(EXIT_FAILURE);
				}
			}
			if(sem_post(&oneChef) != 0){
				perror("Sem_post Error!");
				exit(EXIT_FAILURE);
			}
			
		}	
    }
    return NULL;
}










