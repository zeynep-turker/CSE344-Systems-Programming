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
#include <time.h>
static int *K;
static int *P;
static int *C;
static int *D;
static int *kitchenSize;
static int *emptyTable;
static int *counterSize;
static int *counterStudent;
static int *kitchenP;
static int *kitchenC;
static int *kitchenD;
sem_t *mutex1;
sem_t *mutex2;
sem_t *mutex3;
sem_t *mutex4;
sem_t *mutex5;
sem_t *mutex6;
sem_t *fillCount;
sem_t *emptyCount;
sem_t *fillCountCounter;
sem_t *emptyCountCounter;
sem_t *fillCountTable;
sem_t *emptyCountTable;
sem_t *fillCountkitchenP;
sem_t *emptyCountkitchenP;
sem_t *fillCountkitchenC;
sem_t *emptyCountkitchenC;
sem_t *fillCountkitchenD;
sem_t *emptyCountkitchenD;
//parent processlerin id leri
static int parent=0;
static int cook=0;
static int student=0;
void mySigintHandler(int signal){
	//eğer gelen parent process ise çocuklarını beklerler.
	if(parent != 0){
		while(wait(NULL) != -1);
	}
	if(cook != 0){
		while(wait(NULL) != -1);
	}
	if(student != 0){
		while(wait(NULL) != -1);
	}
	
	if(signal == SIGINT){
	 	printf("Program receives SIGINT signal and terminates.\n");
 		exit(0);
	}
}
int main(int argc, char *argv[]){
	int opt;
	char* n;
	char* m;
	char* t;
	char* s;
	char* l;
	char* filePath;
	
	while((opt = getopt(argc, argv,"N:M:T:S:L:F:")) != -1)  
	{  
		switch(opt)
		{  
			case 'N':  
		        n = optarg;  
		        break;  
		    case 'M':
		        m = optarg; 
		        break;  
		    case 'T':  
		        t = optarg;
		        break;
	        case 'S':  
		        s = optarg;
		        break;
	        case 'L':  
		        l = optarg;
		        break;
        	case 'F':
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
	struct sigaction action1;
	memset(&action1, 0, sizeof(action1));
	action1.sa_handler = &mySigintHandler;
	if(sigaction(SIGINT, &action1, NULL) == -1){
		perror("Sigaction Error!");
		exit(EXIT_FAILURE);
	}
	int N = atoi(n); 
	int M = atoi(m);
	int T = atoi(t);
	int S = atoi(s);
	int L = atoi(l);
	if(N<2 || M<N){
		printf("1. Constraint don't complied!\n");   
		exit(EXIT_FAILURE);
	}
	if(S < 3){
		printf("2. Constraint don't complied!\n");   
		exit(EXIT_FAILURE);
	}
	if(T < 1 || M<T){
		printf("3. Constraint don't complied!\n");   
		exit(EXIT_FAILURE);
	}
	if(L < 3){
		printf("4. Constraint don't complied!\n");   
		exit(EXIT_FAILURE);
	}
	K = mmap(NULL, (sizeof *K), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*K = 2*L*M + 1;
	P = mmap(NULL, (sizeof *P), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	C = mmap(NULL, (sizeof *C), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	D = mmap(NULL, (sizeof *D), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	kitchenSize = mmap(NULL, (sizeof *kitchenSize), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyTable = mmap(NULL, (sizeof *emptyTable), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	counterSize = mmap(NULL, (sizeof *counterSize), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	counterStudent = mmap(NULL, (sizeof *counterStudent), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	kitchenP = mmap(NULL, (sizeof *kitchenP), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	kitchenC = mmap(NULL, (sizeof *kitchenC), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	kitchenD = mmap(NULL, (sizeof *kitchenD), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	*kitchenSize = 0;
	*emptyTable = T;
	*counterSize = 0;
	*counterStudent = 0;
	*kitchenP = 0;
	*kitchenC = 0;
	*kitchenD = 0;
	*P = 0;
	*C = 0;
	*D = 0;
	//binary semaphore
	//kritik bölgeleri kilitleme için kullandım
 	mutex1 = mmap(NULL, (sizeof *mutex1), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 	mutex2 = mmap(NULL, (sizeof *mutex2), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 	mutex3 = mmap(NULL, (sizeof *mutex3), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	mutex4 = mmap(NULL, (sizeof *mutex4), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	mutex5 = mmap(NULL, (sizeof *mutex5), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	mutex6 = mmap(NULL, (sizeof *mutex6), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0); 
	//counting Semaphore
	//supplier ın mutfağın boşalmasını beklemesi gibi yerlerde kullandım
	fillCount = mmap(NULL, (sizeof *fillCount), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyCount = mmap(NULL, (sizeof *emptyCount), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fillCountCounter = mmap(NULL, (sizeof *fillCountCounter), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyCountCounter = mmap(NULL, (sizeof *emptyCountCounter), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fillCountTable = mmap(NULL, (sizeof *fillCountTable), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyCountTable = mmap(NULL, (sizeof *emptyCountTable), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fillCountkitchenP = mmap(NULL, (sizeof *fillCountkitchenP), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyCountkitchenP = mmap(NULL, (sizeof *emptyCountkitchenP), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 	fillCountkitchenC = mmap(NULL, (sizeof *fillCountkitchenC), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyCountkitchenC = mmap(NULL, (sizeof *emptyCountkitchenC), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	fillCountkitchenD = mmap(NULL, (sizeof *fillCountkitchenD), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	emptyCountkitchenD = mmap(NULL, (sizeof *emptyCountkitchenD), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(sem_init(mutex1,1,1) != 0){
		printf("sem_init\n");
		exit(0);
	}
	if(sem_init(mutex2,1,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(mutex3,1,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(mutex4,1,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(mutex5,1,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(mutex6,1,1) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(fillCount,1,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(emptyCount,1,*K) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(fillCountCounter,1,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(emptyCountCounter,1,S) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(fillCountTable,1,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(emptyCountTable,1,T) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(fillCountkitchenP,1,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(emptyCountkitchenP,1,L*M) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(fillCountkitchenC,1,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(emptyCountkitchenC,1,L*M) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(fillCountkitchenD,1,0) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_init(emptyCountkitchenD,1,L*M) != 0){
		perror("Sem_init Error!");
		exit(EXIT_FAILURE);
	}
	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
	int input = open(filePath,O_RDONLY,mode);
	if(input < 0){
		perror("Open File Error");
		exit(EXIT_FAILURE);
	}
	for(int i=0; i<3; ++i){
		switch(fork()){
			case -1:
				perror("Fork Error!");
   	 			exit(EXIT_FAILURE);
   	 			break;
 			case 0:
 				if(i == 0){//Supplier
 					char random;
 					while(read(input,&random,1) != 0){
 						if(random == 'P' || random == 'C' || random == 'D'){
	 						if(sem_wait(emptyCount) != 0){//mutfak boş ise bekle.
								perror("Sem_wait Error!");   
								exit(EXIT_FAILURE);
							}
	 						if(random == 'P'){//eğer dosyadan P okunduysa kitchen a soup bırakılır.
	 								if(sem_wait(emptyCountkitchenP) != 0){
										perror("Sem_wait Error!");   
										exit(EXIT_FAILURE);
									}
		 							if(sem_wait(mutex1) != 0){
										perror("Sem_wait Error!");   
										exit(EXIT_FAILURE);
									}
									printf("The supplier is going to the kitchen to deliver soup: kitchen items P:%d,C:%d,D:%d=%d\n",*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
									(*kitchenP)++;
									(*kitchenSize)++;
									printf("The supplier delivered soup – after delivery: kitchen items P:%d,C:%d,D:%d=%d\n",*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
									if(sem_post(mutex1) != 0){
										perror("Sem_post Error!");   
										exit(EXIT_FAILURE);
									}
									if(sem_post(fillCountkitchenP) != 0){
										perror("Sem_post Error!"); 
										exit(EXIT_FAILURE);
									}
	 						}
	 						else if(random == 'C'){//eğer dosyadan C okunduysa kitchen a main course bırakılır.
								if(sem_wait(emptyCountkitchenC) != 0){
									perror("Sem_wait Error!");   
									exit(EXIT_FAILURE);
								}
	 							if(sem_wait(mutex1) != 0){
									perror("Sem_wait Error!");   
									exit(EXIT_FAILURE);
								}
								printf("The supplier is going to the kitchen to deliver main course: kitchen items P:%d,C:%d,D:%d=%d\n",*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
								(*kitchenC)++;
								(*kitchenSize)++;
								printf("The supplier delivered main course – after delivery: kitchen items P:%d,C:%d,D:%d=%d\n",*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
								if(sem_post(mutex1) != 0){
									perror("Sem_post Error!"); 
									exit(EXIT_FAILURE);
								}
								if(sem_post(fillCountkitchenC) != 0){
									perror("Sem_post Error!");   
									exit(EXIT_FAILURE);
								}
	 						}
	 						else{
	 							if(random == 'D'){//eğer dosyadan D okunduysa kitchen a desert bırakılır.
	 								if(sem_wait(emptyCountkitchenD) != 0){
										perror("Sem_wait Error!");
										exit(EXIT_FAILURE);
									}
		 							if(sem_wait(mutex1) != 0){
										perror("Sem_wait Error!");   
										exit(EXIT_FAILURE);
									}
									printf("The supplier is going to the kitchen to deliver desert: kitchen items P:%d,C:%d,D:%d=%d\n",*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
									(*kitchenD)++;
									(*kitchenSize)++;
									printf("The supplier delivered desert – after delivery: kitchen items P:%d,C:%d,D:%d=%d\n",*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
									if(sem_post(mutex1) != 0){
										perror("Sem_post Error!"); 
										exit(EXIT_FAILURE);
									}
									if(sem_post(fillCountkitchenD) != 0){
										perror("Sem_post Error!");   
										exit(EXIT_FAILURE);
									}
		 						}
	 						}
	 						if(sem_post(fillCount) != 0){
								perror("Sem_post Error!");  
								exit(EXIT_FAILURE);
							}
						}
						
 					}
 					printf("Supplier finished supplying - GOODBYE!\n");
 					_exit(EXIT_SUCCESS);
 				}
 				if(i == 1){//Cook
 					int *placed;//countera bırakılan tabak sayısı tutulur.
 					placed = mmap(NULL, (sizeof *placed), PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
 					*placed = 0;
 					int f=0;
 					for(int s=0; s<N; s++){
 						switch(fork()){
 							case -1:
 								perror("Fork Error!");
   	 							exit(EXIT_FAILURE);
   	 							break;
							case 0:		
								while(*placed != 3*L*M){
									if(sem_wait(mutex5) != 0){
										perror("Sem_wait Error!");
										exit(EXIT_FAILURE);
									}
									if(*placed == 3*L*M){	
										if(sem_post(mutex5) != 0){
											perror("Sem_wait Error!");
											exit(EXIT_FAILURE);
										}
										if(sem_wait(mutex1) != 0){
											perror("Sem_wait Error!");  
											exit(EXIT_FAILURE);
										}
										printf("Cook %d finished serving - items at kitchen: %d – going home – GOODBYE!!!\n",s,*kitchenSize);
										if(sem_post(mutex1) != 0){
											perror("Sem_wait Error!");  
											exit(EXIT_FAILURE);
										}
										_exit(EXIT_SUCCESS);
									}
					 				if(sem_wait(emptyCountCounter) != 0){
										perror("Sem_wait Error!");
										exit(EXIT_FAILURE);
									}
 									if(*counterSize < S){
										if(sem_wait(fillCount) != 0){
											perror("Sem_wait Error!");
											exit(EXIT_FAILURE);
										}
					 					if(*P == 0){//counter da P yok ise counter a P götür.
					 						if(sem_wait(fillCountkitchenP) != 0){
												perror("Sem_wait Error!");   
												exit(EXIT_FAILURE);
											}
											if(sem_wait(mutex1) != 0){
												perror("Sem_wait Error!");  
												exit(EXIT_FAILURE);
											}
											printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d,C:%d,D:%d=%d\n",s,*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
											(*kitchenSize)--;
											(*kitchenP)--;
											if(sem_post(mutex1) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											if(sem_post(emptyCountkitchenP) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											if(sem_wait(mutex2) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
					 						printf("Cook %d is going to the counter to deliver soup – counter items P:%d,C:%d,D:%d=%d\n",s,*P,*C,*D,*counterSize);
				 							(*P)++;
				 							(*counterSize)++;
				 							(*placed)++;
				 							printf("Cook %d placed soup on the counter - counter items P:%d,C:%d,D:%d = %d\n",s,*P,*C,*D,*counterSize);
				 							if(sem_post(mutex2) != 0){
												perror("Sem_post Error!"); 
												exit(EXIT_FAILURE);
											}
					 					}
					 					else if(*C == 0){//counter da C yok ise counter a C götür.
					 						if(sem_wait(fillCountkitchenC) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
					 						if(sem_wait(mutex1) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
											printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d,C:%d,D:%d=%d\n",s,*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
											(*kitchenSize)--;
											(*kitchenC)--;
											if(sem_post(mutex1) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											if(sem_post(emptyCountkitchenC) != 0){
												perror("Sem_post Error!"); 
												exit(EXIT_FAILURE);
											}
											if(sem_wait(mutex2) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
					 						printf("Cook %d is going to the counter to deliver main course – counter items P:%d,C:%d,D:%d=%d\n",s,*P,*C,*D,*counterSize);
					 						(*C)++;
					 						(*counterSize)++;
					 						(*placed)++;
				 							printf("Cook %d placed main course on the counter - counter items P:%d,C:%d,D:%d = %d\n",s,*P,*C,*D,*counterSize);
				 							if(sem_post(mutex2) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}		 					
					 					}
					 					else if(*D == 0){//counter da D yok ise counter a D götür.
					 						if(sem_wait(fillCountkitchenD) != 0){
												perror("Sem_wait Error!");  
												exit(EXIT_FAILURE);
											}
					 						if(sem_wait(mutex1) != 0){
												perror("Sem_wait Error!");  
												exit(EXIT_FAILURE);
											}
											printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d,C:%d,D:%d=%d\n",s,*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
											(*kitchenSize)--;
											(*kitchenD)--;
											if(sem_post(mutex1) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											if(sem_post(emptyCountkitchenD) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											if(sem_wait(mutex2) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
					 						printf("Cook %d is going to the counter to deliver desert – counter items P:%d,C:%d,D:%d=%d\n",s,*P,*C,*D,*counterSize);
				 							(*D)++;	
				 							(*counterSize)++;
				 							(*placed)++;
				 							printf("Cook %d placed desert on the counter - counter items P:%d,C:%d,D:%d = %d\n",s,*P,*C,*D,*counterSize);
				 							if(sem_post(mutex2) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
					 					}
					 					else{//eğer P,C ve D nin hepsinden counter da varsa,içlerinden en küçüğü counter a götürülür.
					 						int nextPlate=0;
					 						if(*P <= *C && *P <= *D)
					 							nextPlate=0;
				 							else if(*C <= *P && *C <= *D)
				 								nextPlate=1;
			 								else if(*D <= *P && *D <= *C)
			 									nextPlate=2;
											if(nextPlate == 0){
												if(sem_wait(fillCountkitchenP) != 0){
													perror("Sem_wait Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_wait(mutex1) != 0){
													perror("Sem_wait Error!");
													exit(EXIT_FAILURE);
												}
												printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d,C:%d,D:%d=%d\n",s,*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
												(*kitchenSize)--;
												(*kitchenP)--;
												if(sem_post(mutex1) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_post(emptyCountkitchenP) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_wait(mutex2) != 0){
													perror("Sem_wait Error!");
													exit(EXIT_FAILURE);
												}
												printf("Cook %d is going to the counter to deliver soup – counter items P:%d,C:%d,D:%d=%d\n",s,*P,*C,*D,*counterSize);
				 								(*P)++;
				 								(*counterSize)++;
				 								(*placed)++;
				 								printf("Cook %d placed soup on the counter - counter items P:%d,C:%d,D:%d = %d\n",s,*P,*C,*D,*counterSize);
				 								if(sem_post(mutex2) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
											}
											else if(nextPlate == 1){
												if(sem_wait(fillCountkitchenC) != 0){
													perror("Sem_wait Error!"); 
													exit(EXIT_FAILURE);
												}
												if(sem_wait(mutex1) != 0){
													perror("Sem_wait Error!");  
													exit(EXIT_FAILURE);
												}
												printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d,C:%d,D:%d=%d\n",s,*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
												(*kitchenSize)--;
												(*kitchenC)--;
												if(sem_post(mutex1) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_post(emptyCountkitchenC) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_wait(mutex2) != 0){
													perror("Sem_wait Error!");
													exit(EXIT_FAILURE);
												}
												printf("Cook %d is going to the counter to deliver main course – counter items P:%d,C:%d,D:%d=%d\n",s,*P,*C,*D,*counterSize);
				 								(*C)++;
				 								(*counterSize)++;
				 								(*placed)++;
				 								printf("Cook %d placed main course on the counter - counter items P:%d,C:%d,D:%d = %d\n" ,s,*P,*C,*D,*counterSize);
				 								if(sem_post(mutex2) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
											}
											else if(nextPlate == 2){
												if(sem_wait(fillCountkitchenD) != 0){
													perror("Sem_wait Error!");   
													exit(EXIT_FAILURE);
												}
												if(sem_wait(mutex1) != 0){
													perror("Sem_wait Error!");
													exit(EXIT_FAILURE);
												}
												printf("Cook %d is going to the kitchen to wait for/get a plate - kitchen items P:%d,C:%d,D:%d=%d\n",s,*kitchenP,*kitchenC,*kitchenD,*kitchenSize);
												(*kitchenSize)--;
												(*kitchenD)--;
												if(sem_post(mutex1) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_post(emptyCountkitchenD) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
												if(sem_wait(mutex2) != 0){
													perror("Sem_wait Error!"); 
													exit(EXIT_FAILURE);
												}
												printf("Cook %d is going to the counter to desert – counter items P:%d,C:%d,D:%d=%d\n",s,*P,*C,*D,*counterSize);
				 								(*D)++;
				 								(*counterSize)++;
				 								(*placed)++;
				 								printf("Cook %d placed desert on the counter - counter items P:%d,C:%d,D:%d = %d\n",s,*P,*C,*D,*counterSize);
				 								if(sem_post(mutex2) != 0){
													perror("Sem_post Error!");
													exit(EXIT_FAILURE);
												}
											}
										}
										if(sem_post(emptyCount) != 0){
											perror("Sem_post Error!");
											exit(EXIT_FAILURE);
										}
									}
									if(sem_post(fillCountCounter) != 0){
										perror("Sem_post Error!");
										exit(EXIT_FAILURE);
									}
									if(sem_post(mutex5) != 0){
										perror("Sem_post Error!");
										exit(EXIT_FAILURE);
									}									
									if(*placed == 3*L*M){
										printf("Cook %d finished serving - items at kitchen: %d – going home – GOODBYE!!!\n",s,*kitchenSize);
										_exit(EXIT_SUCCESS);
									}
			 					}
			 					printf("Cook %d finished serving - items at kitchen: %d – going home – GOODBYE!!!\n",s,*kitchenSize);
			 					_exit(EXIT_SUCCESS);
		 					default:
		 						break;
	 					}
 					}
 					cook = getpid();
 					while(wait(NULL) != -1);
 					if(munmap(placed,sizeof *placed) != 0){
						perror("Removing Shared Variable Error!");
						exit(EXIT_FAILURE);
					}
 					exit(EXIT_SUCCESS);
 				}
 				if(i == 2){//Student
 					int *tables;//öğrencilerin oturacağı masalardan hangilerinin boş olduğunun bilinmesi için.
 					tables = mmap(0, T*sizeof(int), PROT_READ|PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS, -1, 0);
					if (!tables) {
						perror("Mmap Failed");
						exit(1);
					}
					memset((void *)tables, 0, T*sizeof(int));
 					for(int j=0; j<T; ++j){		
						tables[j] = 0;
					}
 					for(int s=0; s<M; ++s){
 						switch(fork()){
 							case -1:
 								perror("Fork Error!");
   	 							exit(EXIT_FAILURE);
   	 							break;
							case 0:
								while(1){
									int control=0;
									int t=0;
									for(int c=0; c<L;){//her öğrenci L kadar countera gidip yemek alır.
										if(sem_wait(mutex6) != 0){
											perror("Sem_wait Error!");
											exit(EXIT_FAILURE);
										}
										
										if(t==0){
											printf("Student %d is going to the counter (round %d) - # of students at counter: %d and counter items P:%d,C:%d,D:%d=%d\n",s,c+1,*counterStudent,*P,*C,*D,*counterSize);	
											(*counterStudent)++;
											t = 1;
										}
										if(sem_post(mutex6) != 0){
											perror("Sem_post Error!");
											exit(EXIT_FAILURE);
										}
										if(sem_wait(mutex3) != 0){
											perror("Sem_wait Error!");
											exit(EXIT_FAILURE);
										}
										if(sem_wait(fillCountCounter) != 0){
											perror("Sem_wait Error!");
											exit(EXIT_FAILURE);
										}//counterda P,C ve D den en az bir tane varsa öğrenci hepsinden birer tane alır.
										if(*counterSize > 2){
											if(sem_wait(mutex2) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}	
											(*P)--;
											(*counterSize)--;
											(*C)--;
											(*counterSize)--;
											(*D)--;
											(*counterSize)--;	
											control = 1;
											if(sem_wait(mutex6) != 0){
												perror("Sem_wait Error!");   
												exit(EXIT_FAILURE);
											}
											(*counterStudent)--;
											if(sem_post(mutex6) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											printf("Student %d got food and is going to get a table (round %d) - # of empty tables: %d\n",s,c+1,*emptyTable);
											if(sem_post(mutex2) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
										}
										
										if(sem_post(emptyCountCounter) != 0){
											perror("Sem_post Error!");
											exit(EXIT_FAILURE);
										}
										if(sem_post(mutex3) != 0){
											perror("Sem_post Error!");
											exit(EXIT_FAILURE);
										}
										if(control == 1){
											//eğer bütün masalar dolu ise bekler.
											if(sem_wait(emptyCountTable) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
											int x;
											for(x=0; x<T; ++x){//boş masa aranılır
												if(tables[x] == 0)
													break;	
											}
											tables[x] = 1;//bulduğu boş masa dolu yapılır
											if(sem_wait(mutex4) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
											(*emptyTable)--;
											if(sem_post(mutex4) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											printf("Student %d sat at table %d to eat (round %d) - empty tables:%d\n",s,x+1,c+1,*emptyTable);						
											if(sem_post(fillCountTable) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											if(sem_wait(fillCountTable) != 0){
												perror("Sem_wait Error!"); 
												exit(EXIT_FAILURE);
											}
											tables[x] = 0;//masadan kalkarken masa tekrardan boş yapılır
											if(sem_wait(mutex4) != 0){
												perror("Sem_wait Error!");
												exit(EXIT_FAILURE);
											}
											(*emptyTable)++;
											if(sem_post(mutex4) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											printf("Student %d left table %d to eat again (round %d) - empty tables:%d\n",s,x+1,c+1,*emptyTable);
											if(sem_post(emptyCountTable) != 0){
												perror("Sem_post Error!");
												exit(EXIT_FAILURE);
											}
											control = 0;
											t = 0;
											c++;
										}
									}
									break;
								 }	
								 printf("Student %d is done eating L=%d times - going home – GOODBYE!!!\n",s,L);
								_exit(EXIT_SUCCESS);			
							default:
								break;
						}
 					}
 					student = getpid();
 					while(wait(NULL) != -1);
 					if(munmap(tables,sizeof *tables) != 0){
						perror("Removing Shared Variable Error!");
						exit(EXIT_FAILURE);
					}
 					_exit(EXIT_SUCCESS);	
 				}
 			default:
 				break;
		}
	}
	parent = getpid();
	while(wait(NULL) != -1);
	
	//Remove shared variables and semaphores
	if(munmap(K,sizeof *K) != 0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(P,sizeof *P) != 0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(C,sizeof *C) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(D,sizeof *D) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(kitchenSize,sizeof *kitchenSize) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(emptyTable,sizeof *emptyTable) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(counterSize,sizeof *counterSize) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(counterStudent,sizeof *counterStudent) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(kitchenP,sizeof *kitchenP) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(kitchenC,sizeof *kitchenC) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(munmap(kitchenD,sizeof *kitchenD) !=0){
		perror("Removing Shared Variable Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(mutex1) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(mutex2) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(mutex3) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(mutex4) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(mutex5) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(mutex6) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(fillCount) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(emptyCount) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(fillCountCounter) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(emptyCountCounter) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(fillCountTable) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(emptyCountTable) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(fillCountkitchenP) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(emptyCountkitchenP) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(fillCountkitchenC) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(emptyCountkitchenC) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(fillCountkitchenD) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	if(sem_destroy(emptyCountkitchenD) !=0){
		perror("Removing Semaphore Error!");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
 	
	return 0;
	
}
