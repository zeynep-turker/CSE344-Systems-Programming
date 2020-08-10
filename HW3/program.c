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
#include <sys/wait.h> 
#include <signal.h>

#define Abs(x)  ((x)>0.0?  (x) : (-(x)) ) 
#define SIGN(u,v)  ((v)>=0.0 ? Abs(u) : -Abs(u) )
#define MAX(a,b) (((a)>(b))?(a):(b))

int twoPowN;//2^n
static int parent=0;
static volatile int numLiveChildren = 0;
//dosyayı karakter karakter okuyan ve bu karakterleri tutan fonksiyon
int readFile(int file,int **Arr,int twoPowN);
//singular value bulan internetten aldığım fonksiyonlar
int svd(float **a, int m, int n, float *w, float **v);
static double PYTHAG(double a, double b);
//sayıların virgülden sonrasının 3 basamağını alır.
double myRound(double var);
int svdRound(float **a, int m, int n, float *w, float **v);
static double PYTHAGRound(double a, double b);
//her çocuk bittiğinde SigChld sinyali yakalanır.
//Bu handler ın kitaptan aldım.
static void sigChildHandler(int sig){
	int status, savedErrno;
	pid_t childPid;
	savedErrno = errno;
	while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
		numLiveChildren--;
	}
	if (childPid == -1 && errno != ECHILD)
		printf("waitpid");

		sleep(5); 
		errno = savedErrno;
}
//SigInt sinyali geldiğinde yakalanır ve bütün processler sona erer.
void mySigintHandler(int signal){
	if(parent != 0){
		wait(NULL);
		wait(NULL);
		wait(NULL);
		wait(NULL);
	}
	if(signal == SIGINT){
	 	printf("Program receives SIGINT signal and terminates.\n");
 		exit(0);
	}
}
int main(int argc, char *argv[]){
    int opt; 
    char* filename1;
	char* filename2;
	char* n;
	
	//mask yaparken kullanacağım değişkenler
	sigset_t blockMask, emptyMask;
	//handlerini yaptığım sinyalleri tanımladım.
    struct sigaction action1;
	memset(&action1, 0, sizeof(action1));
	action1.sa_handler = &mySigintHandler;
	if(sigaction(SIGINT, &action1, NULL) == -1){
		fprintf(stderr, "%s%s\n","Sigaction: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	struct sigaction action2;
	memset(&action2, 0, sizeof(action2));
	action2.sa_handler = &sigChildHandler;
	if(sigaction(SIGCHLD, &action2, NULL) == -1){
		fprintf(stderr, "%s%s\n","Sigaction: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
    while((opt = getopt(argc, argv,"i:o:n:")) != -1)  
    {  
        switch(opt)
        {  
			case 'i':  
                filename1 = optarg;  
                break;  
            case 'o':  
                filename2 = optarg;  
                break;  
            case 'n':  
                n = optarg;
                break;
			case '?':
				fprintf(stderr,"Wrong Command Line! : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
				break;
			default:
				fprintf(stderr,"Wrong Command Line! : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
				break;
        }  
    }
	 for(; optind < argc; optind++){
	 	fprintf(stderr,"Wrong Command Line! : %s\n",strerror(errno));   
		exit(EXIT_FAILURE);
    }
    int exp = atoi(n);
    if(exp < 1){//n in 1 den küçük olma durumunda program sonlanır.
    	fprintf(stderr,"Wrong N VALUE! : %s\n",strerror(errno));   
		exit(EXIT_FAILURE);
    }
   	twoPowN = pow(2,exp);
   	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
   	int inputA = open(filename1,O_RDONLY,mode);
	if(inputA == -1){
		fprintf(stderr,"File Open Error! : %s\n",strerror(errno));     
    	exit(EXIT_FAILURE);
	}
	int inputB = open(filename2,O_RDONLY,mode);
	if(inputB == -1){
		fprintf(stderr,"File Open Error! : %s\n",strerror(errno));     
    	exit(EXIT_FAILURE);
	}
	//inputPathA dosyasındaki karakterler A matrix in içine yazılır.
	int **ArrayA;
	ArrayA = (int **) malloc(sizeof(int *) * twoPowN);
  	for (int k = 0; k < twoPowN; k++) {
  		ArrayA[k] = (int *) malloc(sizeof(int) * twoPowN);
  	}
  	//dosyada yeterli karakter yok ise program sonlanır.
	if((readFile(inputA,ArrayA,twoPowN)) != twoPowN*twoPowN){
		for(int i=0; i<twoPowN; ++i){
			free(ArrayA[i]);
		}
		free(ArrayA);
		fprintf(stderr,"File Size is not enough.Program Terminates : %s\n",strerror(errno));     
    	exit(EXIT_FAILURE);
	}
	//inputPathB dosyasındaki karakterler B matrix in içine yazılır.
	int **ArrayB;
	ArrayB = (int **) malloc(sizeof(int *) * twoPowN);
  	for (int k = 0; k < twoPowN; k++) {
  		ArrayB[k] = (int *) malloc(sizeof(int) * twoPowN);
  	}
	//dosyada yeterli karakter yok ise program sonlanır.
	if((readFile(inputB,ArrayB,twoPowN)) != twoPowN*twoPowN){
		for(int i=0; i<twoPowN; ++i){
			free(ArrayA[i]);
			free(ArrayB[i]);
		}
		free(ArrayA);
		free(ArrayB);
		fprintf(stderr,"File Size is not enough.Program Terminates : %s\n",strerror(errno));     
    	exit(EXIT_FAILURE);
	}
	//matrix çarpımı sonuncunda oluşacak C matrix i
	int **ArrayC;
	ArrayC = (int **) malloc(sizeof(int *) * twoPowN);
  	for (int k = 0; k < twoPowN; k++) {
  		ArrayC[k] = (int *) malloc(sizeof(int) * twoPowN);
  	}
	//pipe1 in içinde her çocuğun okuyup annenin yazacağı 4 tane pipe var.
 	int **pipe1;
 	pipe1 = (int **) malloc(sizeof(int *) * 4);
  	for (int k = 0; k < 4; k++) {
  		pipe1[k] = (int *) malloc(sizeof(int) * 2);
  	}
  	//pipe2 nin içinde her çocuğun yazıp annenin okuyacağı 4 tane pipe var.
 	int **pipe2;
 	pipe2 = (int **) malloc(sizeof(int *) * 4);
  	for (int k = 0; k < 4; k++) {
  		pipe2[k] = (int *) malloc(sizeof(int) * 2);
  	}
  	//pipelardaki hata kontrolünü yaptım.
	if(pipe(pipe1[0]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe1[1]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe1[2]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe1[3]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe2[0]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe2[1]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe2[2]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(pipe(pipe2[3]) < 0){
		fprintf(stderr, "%s%s\n","Pipe Error: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	//SigChld yakanlanması için mask oluşturdum.
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sigChildHandler;
	if(sigaction(SIGCHLD, &sa, NULL) == -1){
		fprintf(stderr, "%s%s\n","Sigaction: ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	if((sigemptyset(&blockMask) == -1) || (sigaddset(&blockMask,SIGCHLD) == -1)){
			fprintf(stderr,"Failed to initialize the signal mask : %s\n",strerror(errno));   
    		exit(EXIT_FAILURE);
	}
	sigprocmask (SIG_SETMASK, &blockMask, NULL);
 	for(int s=0; s<4; ++s){
 		int c;
	  	int nbytes;
	  	int size = twoPowN*twoPowN;
	  	int temp=0,i=0,j=0;
	  	//her çocuğun C nin bir çeyreğini hesaplarken gerekecek A ve B matrixinin elamanlarını tutan arrayler.
	  	int **ArrA;
	  	int **ArrB;
	  	ArrA = (int **) malloc(sizeof(int *) * (twoPowN/2));
	  	for (int k = 0; k < twoPowN/2; k++) {
	  		ArrA[k] = (int *) malloc(sizeof(int) * twoPowN);
	  	}
	  	ArrB = (int **) malloc(sizeof(int *) * (twoPowN/2));
	  	for (int k = 0; k < twoPowN/2; k++) {
	  		ArrB[k] = (int *) malloc(sizeof(int) * twoPowN);
	  	}
	  	numLiveChildren++;
   	 	switch(fork()){
   	 		case -1:
   	 			fprintf(stderr, "%s%s\n","Fork Error: ", strerror(errno));
   	 			exit(EXIT_FAILURE);
   	 		case 0:
 				//pipe1 deki pipelarda çocuk yazma yapmayacağı için her çocuk için o pipe larin 1. indexi kapatılır.
				if(close(pipe1[s][1]) <0){
					fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
					exit(EXIT_FAILURE);
				}
				//pipe2 deki pipelarda çocuk okuma yapmayacağı için her çocuk için o pipe larin 0. indexi kapatılır.
			  	if(close(pipe2[s][0]) < 0){
					fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
					exit(EXIT_FAILURE);
				}
				//pipe üzerinden anneden alınan karakter okunur ve arraylere atılır.
			   	while ((nbytes = read(pipe1[s][0], &c, sizeof(c))) > 0){
					if(nbytes == 0){
						break;
					}
					if(temp >(size/2)-1){
						ArrB[i][j] = c;
						j++;
					}
					else{
						ArrA[i][j] = c;
						j++;
					}
					if(temp == size-1)
			   		{
			   			break;
			   		}
					temp++;
					if(j == twoPowN)
						j=0;
					if((temp % twoPowN) == 0)
						i++;
					if(temp == size/2)
						i=0;   
				}
			   	int temp1=0,temp2=0;
			   	for(int k=0; k<twoPowN/2; ++k){
			   		for(int l=0; l<twoPowN/2; ++l){
			   			for(int m=0; m<twoPowN; ++m){
			   				temp1 = ArrA[k][m] * ArrB[l][m];
				    		temp2 += temp1;
			   			}
			   			//pipe2 de anneye C de hesapladığımız değer gönderilir.
			   			if(write(pipe2[s][1],&temp2,sizeof(temp2)) < 0){
			   				fprintf(stderr, "%s%s\n","Write Error: ", strerror(errno));
							exit(EXIT_FAILURE);
		   				}
			   			temp2 = 0;
		   			}
			   	}
			   	//okuma işlemi bittiği için pipe1 in 0. indexi kapatılır.
			   	if(close(pipe1[s][0]) ==-1){
			   		fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
					exit(EXIT_FAILURE);
		   		}
			   	if(close(pipe2[s][1]) ==-1){
			   		fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
					exit(EXIT_FAILURE);
		   		}
		   		//çocuk işlerini bitirdikten sonra sonlanır.
 				_exit(EXIT_SUCCESS);
   	 			break;
   	 		default:
   	 			break;
		}
		//kullanılan pointerlar free edilir.
		for(int i=0; i<twoPowN/2; ++i){
			free(ArrA[i]);
	   		free(ArrB[i]);
		}
   		free(ArrA);
   		free(ArrB);	
	}
	if (sigprocmask(SIG_UNBLOCK, &blockMask, NULL) == -1){
		fprintf(stderr, "%s%s\n","ERROR SIG_UNBLOCK : ", strerror(errno));
		exit(EXIT_FAILURE);
	}
	parent = getpid();
	//pipe1 deki pipelarda anne okuma yapmayacağı için o pipe larin 0. indexi kapatılır.
	//pipe2 deki pipelarda anne yazma yapmayacağı için o pipe larin 1. indexi kapatılır.
 	for(int h=0; h<4; ++h){	
 		if(close(pipe1[h][0]) < 0){
 			fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
			exit(EXIT_FAILURE);
		}
 		if(close(pipe2[h][1]) < 0){
 			fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	//anne her çocuğa özel, çocuğun C değerleri üretmesi için gerekli olan
	//A ve B matrix değerlerini pipe üzerinden gönderir.
	for(int i=0; i<twoPowN/2; ++i){
		for(int j=0; j<twoPowN; ++j){
			if(write(pipe1[0][1],&(ArrayA[i][j]),sizeof(ArrayA[i][j])) < 0){
				fprintf(stderr, "%s%s\n","Write Error: ", strerror(errno));
				exit(EXIT_FAILURE);
			}
			if(write(pipe1[1][1],&(ArrayA[i][j]),sizeof(ArrayA[i][j])) < 0){
				fprintf(stderr, "%s%s\n","Write Error: ", strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
	}
	for(int i=0; i<twoPowN/2; ++i){
		for(int j=0; j<twoPowN; ++j){
			write(pipe1[0][1],&(ArrayB[j][i]),sizeof(ArrayB[j][i]));
		}
	}
	for(int i=twoPowN/2; i<twoPowN; ++i){
		for(int j=0; j<twoPowN; ++j){
			write(pipe1[1][1],&(ArrayB[j][i]),sizeof(ArrayB[j][i]));
			write(pipe1[2][1],&(ArrayA[i][j]),sizeof(ArrayA[i][j]));
			write(pipe1[3][1],&(ArrayA[i][j]),sizeof(ArrayA[i][j]));
		}
	}
	for(int i=twoPowN/2; i<twoPowN; ++i){
		for(int j=0; j<twoPowN; ++j){
			write(pipe1[3][1],&(ArrayB[j][i]),sizeof(ArrayB[j][i]));

		}
	}
	for(int i=0; i<twoPowN/2; ++i){
		for(int j=0; j<twoPowN; ++j){
			write(pipe1[2][1],&(ArrayB[j][i]),sizeof(ArrayB[j][i]));
		}
	}
	//her çocuktan gelen C değerleri C matrixinin içine atılır.
  	int take;
  	int v;
  	int a=0,b=0;
   	while ((take = read(pipe2[0][0], &v, sizeof(v))) > 0){
   		if(take == 0)
   			break;
		if(b == twoPowN/2){
			b=0;
			a++;
		}
		ArrayC[a][b] = v;
		b++; 		
   	}
   	a=0;
   	b=twoPowN/2;
   	while ((take = read(pipe2[1][0], &v, sizeof(v))) > 0){
   		if(take == 0)
   			break;
		if(b == twoPowN){
			b=twoPowN/2;
			a++;
		}
		ArrayC[a][b] = v;
		b++;
   	
   	}
   	a = twoPowN/2;
   	b=0;
   	while ((take = read(pipe2[2][0], &v, sizeof(v))) > 0){
   		if(take == 0)
   			break;
		if(b == twoPowN/2){
			b=0;
			a++;
		}
		ArrayC[a][b] = v;
		b++;
   	
   	}
   	a = twoPowN/2;
   	b = twoPowN/2;
   	while ((take = read(pipe2[3][0], &v, sizeof(v))) > 0){
   		if(take == 0)
   			break;
		if(b == twoPowN){
			b=twoPowN/2;
			a++;
		}
		ArrayC[a][b] = v;
		b++;
   	
   	}
   	//annenin kullanmış olduğu pipe ları kapattım.
   	for(int h=0; h<4; ++h){
 		if(close(pipe1[h][1]) < 0){
 			fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
			exit(EXIT_FAILURE);
		}
 		if(close(pipe2[h][0]) < 0){
 			fprintf(stderr, "%s%s\n","Close Error: ", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
   	//C matrixinin float olan hali
   	float **dArrayC;
   	dArrayC = (float **) malloc(sizeof(float *) * twoPowN);
  	for (int k = 0; k < twoPowN; k++) {
  		dArrayC[k] = (float *) malloc(sizeof(float) * twoPowN);
  	}
   	for(int p=0; p<twoPowN; ++p){
   		for(int t=0; t<twoPowN; ++t){
   			dArrayC[p][t] = (float)(ArrayC[p][t]);
   		}
	}
	//singular değerleri tutan array
   	float *singular;
   	singular = (float *) malloc(sizeof(float) * twoPowN);
	float **s;
	s = (float **) malloc(sizeof(float *) * twoPowN);
  	for (int k = 0; k < twoPowN; k++) {
  		s[k] = (float *) malloc(sizeof(float) * twoPowN);
  	}
   	svd(dArrayC,twoPowN,twoPowN,singular,s);
   	for(int p=0; p<twoPowN; ++p)
   		printf("Singular Value %d ==> %.3f\n",p+1,singular[p]);
   		
 
   	while (numLiveChildren > 0) {
		if (sigsuspend(&emptyMask) == -1 && errno != EINTR){
			fprintf(stderr, "%s%s\n","Sigsuspend: ", strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	//pointerlar free edilir.
	for(int i=0; i<4; ++i){
		free(pipe1[i]);
		free(pipe2[i]);
	}
	free(pipe1);
	free(pipe2);
  	for(int i=0; i<twoPowN; ++i){
	   	free(ArrayA[i]);
   		free(ArrayB[i]);
	   	free(ArrayC[i]);
	   	free(dArrayC[i]);
	   	free(s[i]);
   	}
   	free(ArrayA);
   	free(ArrayB);
   	free(ArrayC);
   	free(dArrayC);
   	free(s);
   	free(singular);

	exit(EXIT_SUCCESS);

	return 0;
}
//dosyadan karakter okuyup onu da arraye atan fonksiyon
int readFile(int file,int **Arr,int twoPowN){
	char c;
    int size=0;
    int i=0;
	int j=0;
    while(read(file,&c,1) != 0){
    	if(i == twoPowN){
			return size;
		}
		Arr[i][j] = (unsigned char)c;
		size++;
		j++;
      
      	if(j == twoPowN){
			i++;
			j=0;
		}
		
    }
    return size;
}
//internetten alınmasına izin verilen fonksiyon 1
static double PYTHAG(double a, double b)
{
    double at = fabs(a), bt = fabs(b), ct, result;

    if (at > bt)       { ct = bt / at; result = at * sqrt(1.0 + ct * ct); }
    else if (bt > 0.0) { ct = at / bt; result = bt * sqrt(1.0 + ct * ct); }
    else result = 0.0;
    return(result);
}
//internetten alınmasına izin verilen fonksiyon 2
int svd(float **a, int m, int n, float *w, float **v)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1;
  
    if (m < n) 
    {
    	fprintf(stderr, "%s%s\n","#rows must be > #cols: ", strerror(errno));
		exit(EXIT_FAILURE);
    }
  
    rv1 = (double *)malloc((unsigned int) n*sizeof(double));

/* Householder reduction to bidiagonal form */
    for (i = 0; i < n; i++) 
    {
        /* left-hand reduction */
        l = i + 1;
        rv1[i] = scale * g;
        g = s = scale = 0.0;
        if (i < m) 
        {
            for (k = i; k < m; k++) 
                scale += fabs((double)a[k][i]);
            if (scale) 
            {
                for (k = i; k < m; k++) 
                {
                    a[k][i] = (float)((double)a[k][i]/scale);
                    s += ((double)a[k][i] * (double)a[k][i]);
                }
                f = (double)a[i][i];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][i] = (float)(f - g);
                if (i != n - 1) 
                {
                    for (j = l; j < n; j++) 
                    {
                        for (s = 0.0, k = i; k < m; k++) 
                            s += ((double)a[k][i] * (double)a[k][j]);
                        f = s / h;
                        for (k = i; k < m; k++) 
                            a[k][j] += (float)(f * (double)a[k][i]);
                    }
                }
                for (k = i; k < m; k++) 
                    a[k][i] = (float)((double)a[k][i]*scale);
            }
        }
        w[i] = (float)(scale * g);
    
        /* right-hand reduction */
        g = s = scale = 0.0;
        if (i < m && i != n - 1) 
        {
            for (k = l; k < n; k++) 
                scale += fabs((double)a[i][k]);
            if (scale) 
            {
                for (k = l; k < n; k++) 
                {
                    a[i][k] = (float)((double)a[i][k]/scale);
                    s += ((double)a[i][k] * (double)a[i][k]);
                }
                f = (double)a[i][l];
                g = -SIGN(sqrt(s), f);
                h = f * g - s;
                a[i][l] = (float)(f - g);
                for (k = l; k < n; k++) 
                    rv1[k] = (double)a[i][k] / h;
                if (i != m - 1) 
                {
                    for (j = l; j < m; j++) 
                    {
                        for (s = 0.0, k = l; k < n; k++) 
                            s += ((double)a[j][k] * (double)a[i][k]);
                        for (k = l; k < n; k++) 
                            a[j][k] += (float)(s * rv1[k]);
                    }
                }
                for (k = l; k < n; k++) 
                    a[i][k] = (float)((double)a[i][k]*scale);
            }
        }
        anorm = MAX(anorm, (fabs((double)w[i]) + fabs(rv1[i])));
    }
  
    /* accumulate the right-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        if (i < n - 1) 
        {
            if (g) 
            {
                for (j = l; j < n; j++)
                    v[j][i] = (float)(((double)a[i][j] / (double)a[i][l]) / g);
                    /* double division to avoid underflow */
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < n; k++) 
                        s += ((double)a[i][k] * (double)v[k][j]);
                    for (k = l; k < n; k++) 
                        v[k][j] += (float)(s * (double)v[k][i]);
                }
            }
            for (j = l; j < n; j++) 
                v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = rv1[i];
        l = i;
    }
  
    /* accumulate the left-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        l = i + 1;
        g = (double)w[i];
        if (i < n - 1) 
            for (j = l; j < n; j++) 
                a[i][j] = 0.0;
        if (g) 
        {
            g = 1.0 / g;
            if (i != n - 1) 
            {
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < m; k++) 
                        s += ((double)a[k][i] * (double)a[k][j]);
                    f = (s / (double)a[i][i]) * g;
                    for (k = i; k < m; k++) 
                        a[k][j] += (float)(f * (double)a[k][i]);
                }
            }
            for (j = i; j < m; j++) 
                a[j][i] = (float)((double)a[j][i]*g);
        }
        else 
        {
            for (j = i; j < m; j++) 
                a[j][i] = 0.0;
        }
        ++a[i][i];
    }

    /* diagonalize the bidiagonal form */
    for (k = n - 1; k >= 0; k--) 
    {                             /* loop over singular values */
        for (its = 0; its < 30; its++) 
        {                         /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--) 
            {                     /* test for splitting */
                nm = l - 1;
                if (fabs(rv1[l]) + anorm == anorm) 
                {
                    flag = 0;
                    break;
                }
                if (fabs((double)w[nm]) + anorm == anorm) 
                    break;
            }
            if (flag) 
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) 
                {
                    f = s * rv1[i];
                    if (fabs(f) + anorm != anorm) 
                    {
                        g = (double)w[i];
                        h = PYTHAG(f, g);
                        w[i] = (float)h; 
                        h = 1.0 / h;
                        c = g * h;
                        s = (- f * h);
                        for (j = 0; j < m; j++) 
                        {
                            y = (double)a[j][nm];
                            z = (double)a[j][i];
                            a[j][nm] = (float)(y * c + z * s);
                            a[j][i] = (float)(z * c - y * s);
                        }
                    }
                }
            }
            z = (double)w[k];
            if (l == k) 
            {                  /* convergence */
                if (z < 0.0) 
                {              /* make singular value nonnegative */
                    w[k] = (float)(-z);
                    for (j = 0; j < n; j++) 
                        v[j][k] = (-v[j][k]);
                }
                break;
            }
            if (its >= 30) {
                free((void*) rv1);
                fprintf(stderr, "%s%s\n","No convergence after 30,000! iterations: ", strerror(errno));
                exit(EXIT_FAILURE);
            }
    
            /* shift from bottom 2 x 2 minor */
            x = (double)w[l];
            nm = k - 1;
            y = (double)w[nm];
            g = rv1[nm];
            h = rv1[k];
            f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
            g = PYTHAG(f, 1.0);
            f = ((x - z) * (x + z) + h * ((y / (f + SIGN(g, f))) - h)) / x;
          
            /* next QR transformation */
            c = s = 1.0;
            for (j = l; j <= nm; j++) 
            {
                i = j + 1;
                g = rv1[i];
                y = (double)w[i];
                h = s * g;
                g = c * g;
                z = PYTHAG(f, h);
                rv1[j] = z;
                c = f / z;
                s = h / z;
                f = x * c + g * s;
                g = g * c - x * s;
                h = y * s;
                y = y * c;
                for (jj = 0; jj < n; jj++) 
                {
                    x = (double)v[jj][j];
                    z = (double)v[jj][i];
                    v[jj][j] = (float)(x * c + z * s);
                    v[jj][i] = (float)(z * c - x * s);
                }
                z = PYTHAG(f, h);
                w[j] = (float)z;
                if (z) 
                {
                    z = 1.0 / z;
                    c = f * z;
                    s = h * z;
                }
                f = (c * g) + (s * y);
                x = (c * y) - (s * g);
                for (jj = 0; jj < m; jj++) 
                {
                    y = (double)a[jj][j];
                    z = (double)a[jj][i];
                    a[jj][j] = (float)(y * c + z * s);
                    a[jj][i] = (float)(z * c - y * s);
                }
            }
            rv1[l] = 0.0;
            rv1[k] = f;
            w[k] = (float)x;
        }
    }
    free((void*) rv1);
    return(1);
}
double myRound(double var) 
{ 
    double value;
    if(var < 0.0)
		value = floor(var * 1000 - 0.5);
	else
		value = floor(var * 1000 + 0.5);
    double den = value/1000.0;
    return den;
}
//internetten alınmasına izin verilen fonksiyon 1
static double PYTHAGRound(double a, double b)
{
    double at = myRound(fabs(myRound(a))), bt = myRound(fabs(myRound(b))), ct, result;

    if (at > bt){
    	ct = myRound(myRound(bt) / myRound(at));
	 	result = myRound(myRound(at) * myRound(sqrt(1.0 + myRound(myRound(ct) * myRound(ct))))); 
 	}
    else if (bt > 0.0) {
 	ct = myRound(myRound(at) / myRound(bt));
 	result = myRound(myRound(bt) * myRound(sqrt(1.0 + myRound(myRound(ct) * myRound(ct)))));
 	}
    else result = 0.0;
    return(myRound(result));
}
//internetten alınmasına izin verilen fonksiyon 2
int svdRound(float **a, int m, int n, float *w, float **v)
{
    int flag, i, its, j, jj, k, l, nm;
    double c, f, h, s, x, y, z;
    double anorm = 0.0, g = 0.0, scale = 0.0;
    double *rv1;
  
    if (m < n) 
    {
    	fprintf(stderr, "%s%s\n","#rows must be > #cols: ", strerror(errno));
		exit(EXIT_FAILURE);
    }
  
    rv1 = (double *)malloc((unsigned int) n*sizeof(double));

/* Householder reduction to bidiagonal form */
    for (i = 0; i < n; i++) 
    {
        /* left-hand reduction */
        l = i + 1;
        rv1[i] = myRound(myRound(scale) * myRound(g));
        g = s = scale = 0.0;
        if (i < m) 
        {
            for (k = i; k < m; k++) 
                scale = myRound(scale) + myRound(fabs(myRound((double)a[k][i])));
            if (myRound(scale)) 
            {
                for (k = i; k < m; k++) 
                {
                    a[k][i] = myRound((float)(myRound((double)a[k][i]) /myRound(scale)));
                    s = myRound(s) +  myRound((myRound((double)a[k][i]) * myRound((double)a[k][i])));
                }
                f = (double)a[i][i];
                g = -SIGN(myRound(sqrt(myRound(s))), myRound(f));
                h = myRound(myRound(f) * myRound(g)) - myRound(s);
                a[i][i] = (float)(myRound(f) - myRound(g));
                if (i != n - 1) 
                {
                    for (j = l; j < n; j++) 
                    {
                        for (s = 0.0, k = i; k < m; k++) 
                            s = myRound(s) + myRound((myRound((double)a[k][i]) * myRound((double)a[k][j])));
                        f = myRound(myRound(s) / myRound(h));
                        for (k = i; k < m; k++) 
                            a[k][j] = myRound(a[k][j]) + myRound((float)(myRound(f) * myRound((double)a[k][i])));
                    }
                }
                for (k = i; k < m; k++) 
                    a[k][i] = myRound((float)(myRound(myRound((double)a[k][i])) * myRound(scale)));
            }
        }
        w[i] = myRound((float)(myRound(scale) * myRound(g)));
    
        /* right-hand reduction */
        g = s = scale = 0.0;
        if (i < m && i != n - 1) 
        {
            for (k = l; k < n; k++) 
                scale = myRound(scale) +  myRound(fabs(myRound(myRound((double)a[i][k]))));
            if (myRound(scale)) 
            {
                for (k = l; k < n; k++) 
                {
                    a[i][k] = myRound((float)(myRound(myRound((double)a[i][k]))/myRound(scale)));
                    s = myRound(s) + myRound((myRound((double)a[i][k]) * myRound((double)a[i][k])));
                }
                f = (double)a[i][l];
                g = -SIGN(myRound(sqrt(myRound(s))), myRound(f));
                h = myRound(myRound(f) * myRound(g) - myRound(s));
                a[i][l] = (float)(myRound(f) - myRound(g));
                for (k = l; k < n; k++) 
                    rv1[k] = myRound(myRound((double)a[i][k]) / myRound(h));
                if (i != m - 1) 
                {
                    for (j = l; j < m; j++) 
                    {
                        for (s = 0.0, k = l; k < n; k++) 
                            s = myRound(s) + myRound((myRound((double)a[j][k]) * myRound((double)a[i][k])));
                        for (k = l; k < n; k++) 
                            a[j][k] = myRound(a[j][k]) + myRound((float)(myRound(s) * myRound(rv1[k])));
                    }
                }
                for (k = l; k < n; k++) 
                    a[i][k] = myRound((float)(myRound((double)a[i][k])*myRound(scale)));
            }
        }
        anorm = MAX(myRound(anorm), myRound(myRound(fabs(myRound((double)w[i]))) + myRound(fabs(myRound(rv1[i])))));
    }
  
    /* accumulate the right-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        if (i < n - 1) 
        {
            if (g) 
            {
                for (j = l; j < n; j++)
                    v[j][i] = myRound((float)((myRound((double)a[i][j]) / myRound((double)a[i][l])) / myRound(g)));
                    /* double division to avoid underflow */
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < n; k++) 
                        s = myRound(s) + myRound((myRound((double)a[i][k]) * myRound((double)v[k][j])));
                    for (k = l; k < n; k++) 
                        v[k][j] = myRound(v[k][j]) + myRound((float)(myRound(s) * myRound((double)v[k][i])));
                }
            }
            for (j = l; j < n; j++) 
                v[i][j] = v[j][i] = 0.0;
        }
        v[i][i] = 1.0;
        g = myRound(rv1[i]);
        l = i;
    }
  
    /* accumulate the left-hand transformation */
    for (i = n - 1; i >= 0; i--) 
    {
        l = i + 1;
        g = myRound((double)w[i]);
        if (i < n - 1) 
            for (j = l; j < n; j++) 
                a[i][j] = 0.0;
        if (g) 
        {
            g = myRound(1.0 / myRound(g));
            if (i != n - 1) 
            {
                for (j = l; j < n; j++) 
                {
                    for (s = 0.0, k = l; k < m; k++) 
                        s = myRound(s) + myRound((myRound((double)a[k][i]) * myRound((double)a[k][j])));
                    f = myRound(myRound((myRound(s) / myRound((double)a[i][i]))) * myRound(g));
                    for (k = i; k < m; k++) 
                        a[k][j] = myRound(a[k][j]) + myRound((float)(myRound(f) * myRound((double)a[k][i])));
                }
            }
            for (j = i; j < m; j++) 
                a[j][i] = myRound((float)(myRound((double)a[j][i])*myRound(g)));
        }
        else 
        {
            for (j = i; j < m; j++) 
                a[j][i] = 0.0;
        }
        ++a[i][i];
    }

    /* diagonalize the bidiagonal form */
    for (k = n - 1; k >= 0; k--) 
    {                             /* loop over singular values */
        for (its = 0; its < 30; its++) 
        {                         /* loop over allowed iterations */
            flag = 1;
            for (l = k; l >= 0; l--) 
            {                     /* test for splitting */
                nm = l - 1;
                if (myRound(fabs(myRound(rv1[l]))) + myRound(anorm) == myRound(anorm)) 
                {
                    flag = 0;
                    break;
                }
                if (myRound(fabs(myRound((double)w[nm]))) + myRound(anorm) == myRound(anorm)) 
                    break;
            }
            if (flag) 
            {
                c = 0.0;
                s = 1.0;
                for (i = l; i <= k; i++) 
                {
                    f = myRound(myRound(s) * myRound(rv1[i]));
                    if (myRound(fabs(myRound(f))) + myRound(anorm) != myRound(anorm)) 
                    {
                        g = (double)myRound(w[i]);
                        h = PYTHAGRound(myRound(f), myRound(g));
                        w[i] = (float)myRound(h); 
                        h = myRound(1.0 / myRound(h));
                        c = myRound(myRound(g) * myRound(h));
                        s = myRound(myRound(- f) * myRound(h));
                        for (j = 0; j < m; j++) 
                        {
                            y = (double)a[j][nm];
                            z = (double)a[j][i];
                            a[j][nm] = (float)(myRound(myRound(y) * myRound(c)) + myRound(myRound(z) * myRound(s)));
                            a[j][i] = (float)(myRound(myRound(z) * myRound(c)) - myRound(myRound(y) * myRound(s)));
                        }
                    }
                }
            }
            z = myRound((double)w[k]);
            if (l == k) 
            {                  /* convergence */
                if (z < 0.0) 
                {              /* make singular value nonnegative */
                    w[k] = myRound((float)(-z));
                    for (j = 0; j < n; j++) 
                        v[j][k] = myRound(-v[j][k]);
                }
                break;
            }
            if (its >= 30) {
                free((void*) rv1);
                fprintf(stderr, "%s%s\n","No convergence after 30,000! iterations: ", strerror(errno));
                exit(EXIT_FAILURE);
            }
    
            /* shift from bottom 2 x 2 minor */
            x = myRound((double)w[l]);
            nm = k - 1;
            y = myRound((double)w[nm]);
            g = myRound(rv1[nm]);
	        h = myRound(rv1[k]);
            f = (myRound((myRound(y) - myRound(z)) * (myRound(y) + myRound(z))) + myRound(myRound((myRound(g) - myRound(h)) * (myRound(g) + myRound(h)))) / myRound(2.0 * myRound(h) * myRound(y)));
            g = PYTHAGRound(myRound(f), 1.0);
            f = myRound((myRound((myRound(x) - myRound(z)) * (myRound(x) + myRound(z))) + myRound(myRound(h) * (myRound((myRound(y) / (myRound(f) + SIGN(myRound(g), myRound(f)))))) - h)) / myRound(x));
            /* next QR transformation */
            c = s = 1.0;
            for (j = l; j <= nm; j++) 
            {
                i = j + 1;
                g = myRound(rv1[i]);
                y = myRound((double)w[i]);
                h = myRound(myRound(s) * myRound(g));
                g = myRound(myRound(c) * myRound(g));
                z = PYTHAGRound(myRound(f), myRound(h));
                rv1[j] = myRound(z);
                c = myRound(myRound(f) / myRound(z));
                s = myRound(myRound(h) / myRound(z));
                f = myRound(myRound(x) * myRound(c)) + myRound(myRound(g) * myRound(s));
                g = myRound(myRound(g) * myRound(c)) - myRound(myRound(x) * myRound(s));
                h = myRound(myRound(y) * myRound(s));
                y = myRound(myRound(y) * myRound(c));
                for (jj = 0; jj < n; jj++) 
                {
                    x = myRound((double)v[jj][j]);
                    z = myRound((double)v[jj][i]);
                    v[jj][j] = (float)(myRound(myRound(x) * myRound(c)) + myRound(myRound(z) * myRound(s)));
                    v[jj][i] = (float)(myRound(myRound(z) * myRound(c)) - myRound(myRound(x) * myRound(s)));
                }
                z = PYTHAGRound(myRound(f), myRound(h));
                w[j] = myRound((float)z);
                if (z) 
                {
                    z = myRound(1.0 / myRound(z));
                    c = myRound(myRound(f) * myRound(z));
                    s = myRound(myRound(h) * myRound(z));
                }
                f = myRound(myRound(c) * myRound(g)) + myRound(myRound(s) * myRound(y));
                x = myRound(myRound(c) * myRound(y)) - myRound(myRound(s) * myRound(g));
                for (jj = 0; jj < m; jj++) 
                {
                    y = myRound((double)a[jj][j]);
                    z = myRound((double)a[jj][i]);
                    a[jj][j] = (float)(myRound(myRound(y) * myRound(c)) + myRound(myRound(z) * myRound(s)));
                    a[jj][i] = (float)(myRound(myRound(z) * myRound(c)) - myRound(myRound(y) * myRound(s)));
                }
            }
            rv1[l] = 0.0;
            rv1[k] = myRound(f);
            w[k] = (float)myRound(x);
        }
    }
    free((void*) rv1);
    return(1);
}
