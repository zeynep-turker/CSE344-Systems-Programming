all: program
program: program.o
	gcc -Wall -Wextra -pedantic program.c -o program -pthread
program.o: program.c
	gcc -c program.c
clean: 
	rm -rf *.o program
