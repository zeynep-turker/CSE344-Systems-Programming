#include <getopt.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h> 
#include <math.h>
#include <time.h>
#define BUF_SIZE 5000000


int takeRandom(int lower, int upper);
int array_size(char* array);
void split(char array[],int size,int result[]);
void fft(int x[],char output[32][100]);
void combine(char input[32][100],char output[]);
int main(int argc, char *argv[]){
	int opt; 
    char* filename1;
	char* filename2;
	//char* time;
   
    while((opt = getopt(argc, argv,"i:o:t:")) != -1)  
    {  
        switch(opt)  
        {  
			case 'i':  
                filename1 = optarg;  
                break;  
            case 'o':  
                filename2 = optarg;  
                break;  
            case 't':  
                //time = optarg;
                break;  
			case '?':
				fprintf(stderr,"Wrong Command Line!\n");
				return 1;
			default:
				fprintf(stderr,"Wrong Command Line!\n");
				return 1;
				break;
        }  
    }
	 for(; optind < argc; optind++){  
	 	fprintf(stderr,"Wrong Command Line!\n");    
        return 1;
    }
	//write system call
	int output = open(filename2,O_WRONLY | O_CREAT | O_APPEND,0666);
	if(output == -1){
		fprintf(stderr,"File Open Error!\n");
		return -1;
	}
	
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
	
  
    char buffer[BUF_SIZE];
 	char lines[100][1000];
 	int input = open(filename1,O_RDWR,mode);
 	if(input == -1){
		fprintf(stderr,"File Open Error!\n");
		return -1;
	}
    int a = read (input, &buffer, BUF_SIZE);
    if( a == -1){
    	fprintf(stderr,"File Read Error!\n");
    	return -1;
	}

	int m=0;//file line number
	int j=0;
	
	for(int i=0; i<a; ++i){
		if(buffer[i] != '\n'){
			lines[m][j] = buffer[i];
			j++;
		}
		if(buffer[i] == '\n'){
			lines[m][j] = '\n';
			j=0;
			m++;
		}
	}
	//****
	char o[32][100];
	int abc[100];
	char r [100000];
	int h;
	//Take a random number
	int random;
	random = takeRandom(0,m-1);
	
	
	if(lines[random][0] == '\n'){
		int girdi =1;
		while(1){
			if(random == m-1 && lines[random][0] == '\n' && girdi == 1){
				random = 0;
				girdi = 0;
			}
			if(girdi == 0 && random == m-1){
				if(lines[random][0] != '\n')
					break;
				else{
					int rndm = takeRandom(1,50);
					printf("sleeeep\n");
					sleep(rndm/1000);
					break;//break yerine random = 0;
				}
			}
			if(lines[random][0] == '\n'){
				random++;
			}
			if(lines[random][0] != '\n'){
				break;
			}
			
		}
	}
	
	
	//Calculate FFT
	split(lines[random],array_size(lines[random]),abc);
	fft(abc,o);
	combine(o,r);
	//write output
	int control;
	control = write(output,r,array_size(r)+1);
	if(control == -1){
		fprintf(stderr,"File Write Error!\n");
	}
	//from readOnly to writeOnly
	int closee;
	closee = close(input);
	if(closee == -1){
		fprintf(stderr,"File Close Error!\n");
	}
	input = open(filename1,O_WRONLY | O_TRUNC,0666);
	if(input == -1){
		fprintf(stderr,"File Open Error!\n");
		return -1;
	}
	//Change inputPathA
	for(h=0; h<m;h++){
		if(h != random){
			control = write(input,lines[h],strlen(lines[h]));
			if(control == -1){
				fprintf(stderr,"File Write Error!\n");
			}
		}
		else{
			control = write(input,"\n",1);
			if(control == -1){
				fprintf(stderr,"File Write Error!\n");
			}
		}
	}
	
	
	
	return 0;
}
void combine(char input[32][100],char output[]){
	int k=0;
	for(int i=0; i<32; ++i){
		for(int j=0;j<strlen(input[i]); ++j){
			if(input[i][j] != '\n'){
				output[k] = input[i][j];
				k++;
			}	
		}
		if(i%2 == 0){
			output[k] = ' ';
			k++;
		}
		if(i%2 == 0 && input[i+1][0] != '-'){
			output[k] = '+';
			k++;
		}
		if(i%2 == 1){
			output[k] = 'i';
			k++;
			output[k] = ',';
			k++;
		}
		
	}
	output[k] = '\n';
}
void fft(int x[],char output[32][100]){
	
	double xre[16];
	double xim[16];
	int n =0;
	double PI = 6.8;
	for(int k=0; k<16; k++){
		xre[k] = 0;
		for(n=0; n<32;++n)
			xre[k] += x[n] * cos(n*k*PI/32);
		xim[k] = 0;
		for(n=0; n<32;++n)
			xim[k] -= x[n] * sin(n*k*PI/32);
		
	
	}
	int m=0;
	for(int p=0; p<32; p++){
		sprintf(output[p],"%.3f",xre[m]);
		p++;
		sprintf(output[p],"%.3f",xim[m]);
		m++;
	}
	 
	
}
void split(char array[],int size,int result []){
	int i=0;
	int r=0;
	while(i<size){
		char temp[3];
		int j=0;
		while(array[i] != ',' && array[i] != '+' && array[i] != ' ' && array[i] != 'i'){
			temp[j] = array[i];
			if(array[i] != ',' && array[i] != '+' && array[i] != ' ' && array[i] != 'i'){
				j++;
				i++;
			}
			if(array[i] == ',' || array[i] == '+' || array[i] == ' ' || array[i] == 'i'){
				result[r] = atoi(temp);
				r++;
			}
		}
		
		++i;
	}
}
int takeRandom(int lower, int upper){
	srand(time(0));
	int num = (rand() % (upper - lower + 1)) + lower;
	return num;
}
int array_size(char* array){
	int count=0;
	for(int i=0;i<BUF_SIZE;++i){
		if(array[i] == '\n')
			return count;
		count++;
	}
	
	return count;

}

