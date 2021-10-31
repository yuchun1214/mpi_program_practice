CC := mpic++
CFLAGS := -g
RUNNER := mpirun

all: smooth

%.o : %.cpp
	$(CC) -c -o $@ $<

test : test.o
	$(CC) $(CFLAGS) -o test test.o

smooth : Smooth.o bmp.h
	$(CC) -g -o smooth Smooth.o

run :
	$(RUNNER) -n 16 ./smooth

clean:
	-rm *.o
	-rm test

.PHONY: clean
