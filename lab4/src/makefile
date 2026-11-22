CC=gcc
CFLAGS=-I.

all : parallel_min_max process_memory zombie psum

parallel_min_max : parallel_min_max.c utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max parallel_min_max.c utils.o find_min_max.o $(CFLAGS)

utils.o : utils.c utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : find_min_max.c find_min_max.h utils.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

zombie : zombie.c
	$(CC) -o zombie zombie.c $(CFLAGS)

process_memory : process_memory.c
	$(CC) -o process_memory process_memory.c $(CFLAGS)

sum.o : sum.c sum.h
	$(CC) -o sum.o -c sum.c $(CFLAGS)

libsum.a : sum.o
	ar rcs libsum.a sum.o

psum : parallel_sum.c libsum.a utils.o sum.h utils.h
	$(CC) -o psum parallel_sum.c utils.o -L. -lsum -pthread $(CFLAGS)

clean :
	rm -f utils.o find_min_max.o sum.o libsum.a parallel_min_max process_memory zombie psum
