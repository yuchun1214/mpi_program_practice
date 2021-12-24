CXX := g++ 
CFLAGS := -g -Wall -fopenmp 

all : h5_problem1 h5_problem2 test

%.o : %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

h5_problem1: h5_problem1.o
	$(CXX) $(CFLAGS) -o sort h5_problem1.o

h5_problem2: key_word_finding.o queue.o
	$(CXX) $(CFLAGS) -o find key_word_finding.o queue.o

test : test.o queue.o
	$(CXX) $(CFLAGS) -o test test.o queue.o

clean:
	-rm *.o
	-rm sort
	-rm find

.PHONY: clean
