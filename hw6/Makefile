CC := gcc
CFLAGS := -Wall -g

all : aco

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

aco: main.o ant.o table.o util.o
	$(CC) $(CFLAGS) -o aco util.o main.o ant.o table.o -lm

clean:
	-rm *.o
	-rm aco

.PHONY: clean
