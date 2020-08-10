#include <getopt.h>  
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>
#include<sys/wait.h> 
#include <stddef.h>
#include <fcntl.h>
#define BUF_SIZE 5000000
int array_size(char* array);
void convert(char* array,char* coord,int *x,int *y,int size);
int findDigit(int number,char* chars);
void cc(char c1,char c2,char* coord,int *x,int *y,int hold[],int i[]);
void LSM(char *coord,int *x,int *y,int hold[]);
void reverse(char* str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char* res, int afterpoint);
float mypow(int base,int exp);
int size(char* str);
void mySigintHandler(int signal);
void mySigstopHandler(int signal);
void split(char array[],int size,float result []);
void errors(char array[],float values[],float *Mae,float *Mse, float *Rmse,int index);
int readLine(int inputT,char line[500],int den[1]);
float standardDeviation(float arr[],int size,float mean);
sig_atomic_t term=0;
sig_atomic_t finishP1=0;
//Signal Handler for SIGUSR1 and SIGUSR2
void sigusr(int signal)
{
	switch(signal){
		case SIGUSR1 :
			finishP1=1;
			break;
		case SIGUSR2 :
	  		term=1;
			break;
		default:
			printf("DEFAULT\n");
	}
		
}
//Signal Handler for SIGTERM
void sigterm(int signal){
	printf("Sinyal SIGTERM is caught %d.\n",signal);

}
//Signal Handler for SIGSTOP
void mySigstopHandler(int signal){
	printf("Sinyal SIGSTOP is caught %d.\n", signal); 
}
//Signal Handler for SIGINT
void mySigintHandler(int signal){
 	printf("Sinyal SIGINT is caught %d.\n", signal); 
}
int main(int argc, char *argv[])  
{ 
    int opt; 
    char* filename1;
	char* filename2;
	
    while((opt = getopt(argc, argv,"i:o:")) != -1)  
    {  
        switch(opt)  
        {  
			case 'i':  
                filename1 = optarg;  
                break;  
            case 'o':  
                filename2 = optarg;  
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
  
	//mask
	struct sigaction sint;
	memset(&sint,0,sizeof(sint));
	sint.sa_handler = &mySigintHandler;
	sigaction(SIGINT,&sint,NULL);
	struct sigaction sstop;
	memset(&sstop,0,sizeof(sstop));
	sstop.sa_handler = &mySigstopHandler;
	sigaction(SIGSTOP,&sstop,NULL);
	struct sigaction action1;
	memset(&action1, 0, sizeof(action1));
	action1.sa_handler = &sigusr;
	sigaction(SIGUSR2, &action1, NULL);
	struct sigaction action2;
	memset(&action2, 0, sizeof(action2));
	action2.sa_handler = &sigusr;
	sigaction(SIGUSR1, &action2, NULL);
	struct sigaction sterm;
	memset(&sterm, 0, sizeof(sterm));
	sterm.sa_handler = &sigterm;
	sigaction(SIGTERM, &sterm, NULL);
	sigset_t intmask,mymask;;
	//mkstemp
	int tempFile;
	char tempFileName[] = "/tmp/tempXXXXXX";
	tempFile = mkstemp(tempFileName);
	printf("file : %s\n",tempFileName);
	if(tempFile == -1){
		fprintf(stderr,"mkstemp : %s\n",strerror(errno));     
        exit(EXIT_FAILURE);
	}
	int Sint=0, Sstop=0;
	//fork()
	pid_t child = fork();
	
	if(child != 0){	
		mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
		struct flock inputlock;
		int input = open(filename1,O_RDONLY,mode);
		if(input == -1){
			fprintf(stderr,"File Open Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}//locking
		memset(&inputlock,0,sizeof(inputlock));
		inputlock.l_type = F_RDLCK;
		fcntl(input,F_SETLKW,&inputlock);
		mode_t modeTemp = S_IWUSR | S_IWGRP | S_IWOTH;
		struct flock templock1;
		int inputTemp = open(tempFileName,O_WRONLY,modeTemp);
		if(inputTemp == -1){
			fprintf(stderr,"Temp File Open Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}//locking
		memset(&templock1,0,sizeof(templock1));
		templock1.l_type = F_RDLCK;
		fcntl(inputTemp,F_SETLKW,&templock1);
		char buffer[BUF_SIZE];
	 	char lines[100][100];
	 	
		int a = read (input, &buffer, BUF_SIZE);
		if( a == -1){
			fprintf(stderr,"File Read Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
		int m=0;
		int j=0;
		int temp=1;
	
		for(int i=0; i<a; ++i){
			if(buffer[i] != '\n'){
				lines[m][j] = buffer[i];
				j++;
				temp++;
			}
		
			if(temp == 21){
				lines[m][j] = '\n';
				j=0;
				m++;
				temp = 1;
	
			}
		}
		if(m == 0){
			fprintf(stderr,"Error! File size is bigger or smaller than 20. : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
		char coord[m][300];
		int x[m][10];
		int y[m][10];
	
		sigset_t which_sig;
		if((sigemptyset(&intmask) == -1) || (sigaddset(&intmask,SIGINT) == -1) || (sigaddset(&intmask,SIGSTOP))){
			fprintf(stderr,"Failed to initialize the signal mask : %s\n",strerror(errno));   
    		exit(EXIT_FAILURE);
		}
		//block
		if(sigprocmask(SIG_BLOCK,&intmask,NULL) == -1){
			fprintf(stderr,"ERROR SIG_BLOCK : %s\n",strerror(errno));   
    		exit(EXIT_FAILURE);
		}
		sigprocmask (SIG_SETMASK, &intmask, NULL);
	
		for(int n=0;n<m;++n){
			if(lines[n] != "\n"){
		 		convert(lines[n],coord[n],x[n],y[n],array_size(lines[n]));
	 		}
		}
		sigpending (&which_sig);
		if (sigismember (&which_sig, SIGINT)) {
		  	Sint=1;
		}
		else if (sigismember (&which_sig, SIGTSTP)) {
		 	Sstop=1;
		}
		//unblock
		if(sigprocmask(SIG_UNBLOCK,&intmask,NULL) == -1){
			fprintf(stderr,"ERROR SIG_UNBLOCK : %s\n",strerror(errno));   
    		exit(EXIT_FAILURE);
		}
		//write temp file
		for(int i=0;i<m;++i){
			int control = write (inputTemp,coord[i],array_size(coord[i])+1);
			if(control == -1){
				fprintf(stderr,"File Write Error! : %s\n",strerror(errno));   
        		exit(EXIT_FAILURE);
			}
			if(kill(0,SIGUSR2) != 0){
				fprintf(stderr,"Kill Error! : %s\n",strerror(errno));     
 	       		exit(EXIT_FAILURE);
			}
		}
		//unlocking
		inputlock.l_type = F_UNLCK;
		fcntl(input,F_SETLKW,&inputlock);
		//unlocking
		templock1.l_type = F_UNLCK;
		fcntl(inputTemp,F_SETLKW,&templock1);
		//Close Input and Temp File
		int closeFile;
		closeFile = close(input);
		if(closeFile == -1){
			fprintf(stderr,"File Close Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
		closeFile = close(inputTemp);
		if(closeFile == -1){
			fprintf(stderr,"Temp File Close Error! : %s\n",strerror(errno));     
       		exit(EXIT_FAILURE);
		}
		if(kill(0,SIGUSR1) != 0){
			fprintf(stderr,"Kill Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}

		//print result number
		printf("P1 read %d bytes\n",20*m);
		printf("P1 estimated %d line equations\n",m);
		if(Sint == 1)
			printf("SIGINT is sent in the critical section in P1.\n");
		else if(Sstop == 1)
			printf("SIGSTOP is sent in the critical section in P1.\n");
		else
			printf("No signal sent in the critical section in P1\n");

		wait(NULL);
		if(kill(0,SIGTERM) != 0){
			fprintf(stderr,"Kill Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
	}
	else{
		struct flock templock2;
		mode_t modeT = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP;
		int inputT = open(tempFileName,O_RDWR,modeT);
		if(inputT == -1){
			fprintf(stderr,"Temp File Open Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}//locking
		memset(&templock2,0,sizeof(templock2));
		templock2.l_type = F_WRLCK;
		fcntl(inputT,F_SETLKW,&templock2);
		struct flock outlock;
		int output = open(filename2,O_WRONLY | O_CREAT | O_APPEND,0666);
		if(output == -1){
			fprintf(stderr,"Output File Open Error! : %s\n",strerror(errno));     
		    exit(EXIT_FAILURE);
		}//locking
		memset(&outlock,0,sizeof(outlock));
		outlock.l_type = F_WRLCK;
		fcntl(output,F_SETLKW,&outlock);
		int size=0;
		char line[500];
		float mae=0.0,mse=0.0,rmse=0.0;
		int lineSize=0;
		int errSize = 5000000;
		float *Mae = (float*)malloc(errSize * sizeof(float));
		float *Mse = (float*)malloc(errSize * sizeof(float));
		float *Rmse = (float*)malloc(errSize * sizeof(float));
		sigset_t newmask,mymask;
		sigemptyset(&newmask);
		sigaddset(&newmask,SIGUSR2);

		int den[1];//to understand that the file is empty.
		while(1){
			readLine(inputT,line,den);
			if(den[0] == 5){//if the file is empty
				if(finishP1 != 1){//if P1 is not terminated,P2 waits signal from P2.
					while(term == 0){
						sigsuspend(&newmask);
					}
					term=0;
					readLine(inputT,line,den);//Again read a file in temp file
				}
				else{//if P1 is terminated,break.
					break;
				}
			}
			den[0]=0;
			lseek(inputT,0,SEEK_SET);//Go the top in temp file.
			int k;
			size = size + array_size(line)+1;
			float values[22];//holds coordinates,a and b for the each line.
		
			//mask for SIGINT
			if((sigemptyset(&mymask) == -1) || (sigaddset(&mymask,SIGINT) == -1) || (sigaddset(&mymask,SIGINT) == -1)){
				fprintf(stderr,"Failed to initialize the signal mask! : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
			}
			//block
			if(sigprocmask(SIG_BLOCK,&mymask,NULL) == -1){
				fprintf(stderr,"ERROR SIG_BLOCK : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
			}
			//sleep(1);
			split(line,array_size(line),values);
			errors(line,values,Mae,Mse,Rmse,lineSize);
			//unblock
			if(sigprocmask(SIG_UNBLOCK,&mymask,NULL) == -1){
				fprintf(stderr,"ERROR SIG_UNBLOCK! : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
			}
			//Write Output Line
			int wrt = write (output,line,array_size(line)+1);
			if(wrt == -1){
				fprintf(stderr,"File Write Error! : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
			}
			char temp[size];//Holds as many null characters as the lines you have read so far 
			for(k=0; k<size; ++k)//(including null at the end of the line)
				temp[k] = '\0';	
			int del = write(inputT,temp,size);
			if(del == -1){
				fprintf(stderr,"File Delete Error! : %s\n",strerror(errno));     
        		exit(EXIT_FAILURE);
			}
			for(int i=0; i<500; ++i)
				line[i] = '\0';
			lineSize++;
			//Realloc For Mae,Mse and Rmse
			if(lineSize == errSize){
				Mae = (float*)realloc(Mae,errSize*2);
				Mse = (float*)realloc(Mse,errSize*2);
				Rmse = (float*)realloc(Rmse,errSize*2);
			}
		}//unlocking
		outlock.l_type = F_UNLCK;
		fcntl(output,F_SETLKW,&outlock);
		//unlocking
		templock2.l_type = F_UNLCK;
		fcntl(inputT,F_SETLKW,&templock2);
		//Calculate Mean for Mae,Mse,Rmse
	 	for(int l=0; l<lineSize; ++l){
			//printf("%f %f %f\n",Mae[l]
	 		mae = mae + Mae[l];
	 		mse = mse + Mse[l];
	 		rmse = rmse + Rmse[l]; 
 		}
 		float meanMae = mae/(float)lineSize;
 		float meanMse = mse/(float)lineSize;
 		float meanRmse = rmse/(float)lineSize;
 		
 		printf("Mean for MAE : %.3f\n",meanMae);
 		printf("Mean for MSE : %.3f\n",meanMse);
 		printf("Mean for RMSE : %.3f\n",meanRmse);
 		printf("Standard derivation for MAE : %.3f\n",standardDeviation(Mae,lineSize,meanMae));
 		printf("Standard derivation for MSE : %.3f\n",standardDeviation(Mse,lineSize,meanMse));
 		printf("Standard derivation for RMSE : %.3f\n",standardDeviation(Rmse,lineSize,meanRmse));
 	
		//Close Output and Temp File
		int closeFile;
		closeFile = close(inputT);
		if(closeFile == -1){
			fprintf(stderr,"File Close Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
		closeFile = close(output);
		if(closeFile == -1){
			fprintf(stderr,"File Close Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
		//Remove temp and input file
		if(unlink(tempFileName) != 0){
			fprintf(stderr,"File Remove Error! : %s\n",strerror(errno));     
    		exit(EXIT_FAILURE);
		}
		if(unlink(filename1) != 0){
			fprintf(stderr,"File Remove Error! : %s\n",strerror(errno));     
    		exit(EXIT_FAILURE);
		}
		if(raise(SIGTERM) != 0){
			fprintf(stderr,"Raise Error! : %s\n",strerror(errno));     
    		exit(EXIT_FAILURE);
		}
	
	}
	
    return 0; 
}
float standardDeviation(float arr[],int size,float mean){
	float result=0.0;
	
	for(int i=0; i<size; ++i){
		result += pow((arr[i]-mean),2.0);
	}
	result = result / (float)(size-1);
	result = sqrt(result);

	return result;
}
int readLine(int inputT,char line[500],int den[1]){
	char c= 'b';
	int mm=1;
	int i=0;
	while(c != '\n'){
		int aT = read (inputT, &c, 1);
		if( aT == -1){
			fprintf(stderr,"File Read Error! : %s\n",strerror(errno));     
        	exit(EXIT_FAILURE);
		}
		if(aT == 0){
			den[0] = 5;
			return 0;
		}
		line[i] = c;
		i++;
	}
	return 1;
}
void errors(char array[],float values[],float *Mae,float *Mse, float *Rmse,int ind){
	int x=0;
	int y=1;
	float mae,mse,rmse;
	float MAE=0.0,MSE=0.0,RMSE=0.0;
	char strA[20],tempA[20]; 
	char strB[20],tempB[20];
	char strC[20],tempC[20];
	int index = array_size(array);
	for(int i=0; i<10; ++i){
		float predicted = ((float)values[20]*(float)values[x]) + (float)values[21];
		float orginal = (float)values[y] ;	
		float r = orginal - predicted;
		if(r < 0.0)
			mae = r * (-1.0);
		else 
			mae = r;
		MAE = MAE + mae;
		MSE = MSE + r * r;

		x = x + 2;
		y = y + 2;
	}
	
	MAE = MAE / 10.0;
	MSE = MSE / 10.0;
	RMSE = sqrt(MSE);
	
	array[index] = ',';
	index++;
	array[index] = ' ';
	index++;
	ftoa(MAE, tempA, 4);
	int sizeA = size(tempA);
	int tA = tempA[sizeA-1];
	if(tA > 52){
		MAE = MAE + 0.001;
	}
	ftoa(MAE, strA, 3);
	for(int i=0; strA[i] != '\0'; ++i){	
		array[index] = strA[i];
		index++;
	}
	array[index] = ',';
	index++;
	array[index] = ' ';
	index++;
	ftoa(MSE, tempB, 4);
	int sizeB = size(tempB);
	int tB = tempB[sizeB-1];
	if(tB > 52){
		MSE = MSE + 0.001;
	}
	ftoa(MSE, strB, 3);
	for(int i=0; strB[i] != '\0'; ++i){	
		array[index] = strB[i];
		index++;
	}
	array[index] = ',';
	index++;
	array[index] = ' ';
	index++;
	ftoa(RMSE, tempC, 4);
	int sizeC = size(tempC);
	int tC = tempC[sizeC-1];
	if(tC > 52){
		RMSE = RMSE + 0.001;
	}
	ftoa(RMSE, strC, 3);
	for(int i=0; strC[i] != '\0'; ++i){	
		array[index] = strC[i];
		index++;
	}
	array[index] = '\n';
	index++;
	array[index] = '\0';
	Mae[ind] = MAE;
	Mse[ind] = MSE;
	Rmse[ind]= RMSE;



}
void split(char array[],int size,float result []){
	int i=0;
	int r=0;
	while(i<size){
		if(array[i] == '\n')
			break;
		char temp[20];
		temp[0] = '*';
		int j=0;
		while(array[i] != '(' && array[i] != ')' &&array[i] != ',' && array[i] != '+' && array[i] != ' ' && array[i] != 'x'){
			if(array[i] != ',' && array[i] != '+' && array[i] != ' ' && array[i] != 'x' && array[i] != '(' && array[i] != ')'){
				temp[j] = array[i];
				j++;
				i++;
			}
			if(array[i] == '\n')
				break;
		}
		if(temp[0] != '*'){
			temp[j] = '\0';
			result[r] = strtof(temp,NULL);
			r++;
		}
		++i;
	}
}
void LSM(char *coord,int *x,int *y,int hold[]){
	
	float a,b;
	float xsum=0,x2sum=0,ysum=0,xysum=0;
    for (int i=0;i<10;i++)
    {
        xsum=xsum+x[i];                        
        ysum=ysum+y[i];                        
        x2sum=x2sum+(x[i]*x[i]);                
        xysum=xysum+x[i]*y[i];                    
    }
    a=(10*xysum-xsum*ysum)/(10*x2sum-xsum*xsum);           
    b=(x2sum*ysum-xsum*xysum)/(x2sum*10-xsum*xsum);
    
	char strA[20],tempA[20]; 
	char strB[20],tempB[20];
	
	if(a < 0.0){
		coord[hold[0]] = '-';
		(hold[0])++;
		a = a * (-1.0);
	}
	ftoa(a, tempA, 4);
	int sizeA = size(tempA);
	int tA = tempA[sizeA-1];
	if(tA > 52){
		a = a+ 0.001;
	}
	ftoa(a, strA, 3);
	for(int i=0; strA[i] != '\0'; ++i){	
		
		coord[hold[0]] = strA[i];
		(hold[0])++;
	}
	coord[hold[0]] = 'x';
	(hold[0])++;
	if(b < 0.0){
		coord[hold[0]] = '-';
		(hold[0])++;
		b=b*(-1.0);
	}
	else{
		coord[hold[0]] = '+';
		(hold[0])++;
	}
	ftoa(b, tempB, 4);
	int sizeB = size(tempB);
	int tB = tempB[sizeB-1];
	if(tB > 52){
		b = b+ 0.001;
	}
	ftoa(b, strB, 3);
	for(int i=0; strB[i] != '\0'; ++i){
		coord[hold[0]] = strB[i];
		(hold[0])++;
	}
	
}

void cc(char c1,char c2,char* coord,int *x,int *y,int hold[],int i[]){
	int i1 = c1;
	int i2 = c2;
	char t1[3];
	char t2[3];
	int k;
	int d1 = findDigit(i1,t1);
	int d2 = findDigit(i2,t2);
	x[i[0]] = i1;
	y[i[0]] = i2;
	(i[0])++;
	coord[hold[0]] = '(';
	(hold[0])++;
	for(k=0;k<d1;++k){
		if(t1[k] != '\0'){
			coord[hold[0]] = t1[k];
			(hold[0])++;
		}
	}
	
	coord[hold[0]] = ',';
	(hold[0])++;
	coord[hold[0]] = ' ';
	(hold[0])++;
	for(k=0;k<d2;++k){
		if(t2[k] != '\0'){
			coord[hold[0]] = t2[k];
			(hold[0])++;
		}
	}
	coord[hold[0]] = ')';
	(hold[0])++;
	coord[hold[0]] = ',';
	(hold[0])++;
	coord[hold[0]] = ' ';
	(hold[0])++;
		

}
int findDigit(int number,char chars[]){
	int digit = 0;
	char temp;
	char c[1];
	while(number > 0){
		chars[digit] = 48 + (number % 10);
		digit++;
		number = number/10;
	}
	if(digit == 3){
		temp = chars[2];
		chars[2] = chars[0];
		chars[0] = temp;
	}
	else if(digit == 2){
		temp = chars[1];
		chars[1] = chars[0];
		chars[0] = temp;
	}
	return digit;



}
void convert(char* array,char* coord,int *x,int *y,int size){
	int hold[1],temp[1];
	hold[0] = 0;
	temp[0] = 0;
   	int j=0;
	int i=0;
	char i2;
	while(i<size){
		char i1 = array[i];
		i++;
		if(i == size){
			i2 = '0';
			cc(i1,i2,coord,x,y,hold,temp);	
			break;
		}
		else
			i2 = array[i];
		i++;
		cc(i1,i2,coord,x,y,hold,temp);	
		if(i==size)
			break;
			
	}
	LSM(coord,x,y,hold);
	coord[hold[0]] = '\n';
	(hold[0])++;
	coord[hold[0]] = '\0';

}//Find line size
int array_size(char* array){
	int count=0;
	for(int i=0;i<BUF_SIZE;++i){
		if(array[i] == '\n')
			return count;
		count++;
	}
	
	return count;

}
void reverse(char* str, int len) 
{ 
    int i = 0;
	int j = len - 1;
	int temp; 
    while (i < j) { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; 
        j--; 
    } 
}//convert a integer to a string
int intToStr(int x, char str[], int d) 
{
    int i = 0; 
    while (x) { 
        str[i++] = (x % 10) + '0'; 
        x = x / 10; 
    }
    while (i < d) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 
float mypow(int base,int exp){
	float result= 1.0;
	while (exp != 0) {
        result *= base;
        --exp;
    }
}
//Converts a float to a string. 
void ftoa(float n, char* res, int afterpoint) 
{ 
    // Extract integer part 
    int main = (int)n; 
  
    // Extract floating part 
    float decimal = n - (float)main; 
  
    // convert integer part to string 
    int i = intToStr(main, res, 0); 
  	if(main == 0)
	{
		res[0] = '0';
		i=1;
	}
    // check for display option after point 
    if (afterpoint != 0) { 
        res[i] = '.';
        decimal = decimal * mypow(10, afterpoint); 
        intToStr((int)decimal, res + i + 1, afterpoint); 
    }
}//finf array size
int size(char* str){
	int i=0;
	while(str[i] != '\0')
		i++;
	return i;
}
