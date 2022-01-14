CC := mpicc
CFLAGS := -Wall -g -std=c99
OMPFLAGS := -fopenmp 

all : aco

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $< $(OMPFLAGS)

aco: main.o ant.o table.o util.o common.o
	$(CC) $(CFLAGS) -o aco util.o main.o ant.o table.o common.o -lm $(OMPFLAGS)

clean:
	-rm *.o
	-rm aco

.PHONY: clean
