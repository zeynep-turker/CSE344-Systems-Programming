
#include <getopt.h>  
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#define BUF_SIZE 5000000
int array_size(char* array);
void convert(char* array,char* complex,int size);
int findDigit(int number,char* chars);
void cc(char c1,char c2,char* complex,int hold[]);
int main(int argc, char *argv[])  
{ 
    int opt; 
    char* filename1;
	char* filename2;
	char* time;
    
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
                time = optarg;
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

	mode_t mode = S_IRUSR | S_IRGRP | S_IROTH;
	
	
	int child = fork();
	if(child == 0)
		filename1 = filename1;
	else 
		filename1 = "inputPathA2";
	
	
	int input = open(filename1,O_RDONLY,mode);
	if(input == -1){
		fprintf(stderr,"File Open Error!\n");
		return -1;
	}
	
   
    char buffer[BUF_SIZE];
 	char lines[100][100];
 	
    int a = read (input, &buffer, BUF_SIZE);
    if( a == -1){
    	fprintf(stderr,"File Read Error!\n");
    	return -1;
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
		
		if(temp == 33){
			lines[m][j] = '\n';
			j=0;
			m++;
			temp = 1;
	
		}
	}
	if(m == 0){
		fprintf(stderr,"Error! Size is bigger or smaller than 32.\n");
		return -1;
	}

	char complex[m][300];
	
	for(int n=0;n<m;++n){
		if(lines[n] != "\n"){
		 	convert(lines[n],complex[n],array_size(lines[n]));
	 	}
		
	}
	for(int i=0;i<m;++i){
		if(lines[i] != "\n\n"){
			sleep(0.1);
			int control = write (output,complex[i],array_size(complex[i])+1);
			if(control == -1)
				fprintf(stderr,"File Write Error!\n");
		}
	}
		
	int closee;
	closee = close(output);
	if(closee == -1)
		fprintf(stderr,"File Close Error!\n");
	closee = close(input);
	if(closee == -1)
		fprintf(stderr,"File Close Error!\n");
    return 0; 
}

void cc(char c1,char c2,char* complex,int hold[]){
	int i1 = c1;
	int i2 = c2;
	char t1[3];
	char t2[3];
	int k;
	int d1 = findDigit(i1,t1);
	int d2 = findDigit(i2,t2);
	for(k=0;k<d1;++k){
		if(t1[k] != '\0'){
			complex[hold[0]] = t1[k];
			(hold[0])++;
		}
	}
	complex[hold[0]] = ' ';
	(hold[0])++;
	complex[hold[0]] = '+';
	(hold[0])++;
	complex[hold[0]] = 'i';
	(hold[0])++;
	for(k=0;k<d2;++k){
		if(t2[k] != '\0'){
			complex[hold[0]] = t2[k];
			(hold[0])++;
		}
	}
	complex[hold[0]] = ',';
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

void convert(char* array,char* complex,int size){
	int hold[1];
	hold[0] = 0;
   	int j=0;
	int i=0;
	char i2;
	while(i<size){
		char i1 = array[i];
		i++;
		if(i == size){
			i2 = '0';
			cc(i1,i2,complex,hold);	
			break;
		}
		else
			i2 = array[i];
		i++;
		cc(i1,i2,complex,hold);	
		if(i==size)
			break;
			
	}
	complex[hold[0]] = '\n';

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

