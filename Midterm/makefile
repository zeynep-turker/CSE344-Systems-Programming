all: program
program: program.o
	gcc program.o -lm -o program -lpthread
program.o: program.c
	 gcc -c program.c
clean: 
	rm -rf *.o program
