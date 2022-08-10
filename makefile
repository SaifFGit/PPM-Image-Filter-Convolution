CC = gcc

filter: filter.o
		$(CC) -o filter $?
clean:
		rm filter *.o 
		