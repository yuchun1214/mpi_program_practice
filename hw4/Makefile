CXX := g++
CFLAGS := --std=c++11 -Wall -g -pthread

all : h4_problem1

%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

h4_problem1: Smooth.o bmp.h
	$(CXX) $(CFLAGS) -o main Smooth.o

clean:
	-rm *.o
	-rm main

.PHONY: clean
