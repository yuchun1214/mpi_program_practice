CC := mpicc
CXX := mpic++
RUNNER := mpirun
CFLAGS := -std=c99 -Wall

ta ?=
n ?=

all : h2_problem1 h2_problem2

h2_problem1 : h2_problem1.cpp bmp.h
	$(CXX) -O3 -o h2_problem1 h2_problem1.cpp

check_sorting_result : check_sorting_result.c
	$(CC) $(CFLAGS) -O3 -o check check_sorting_result.c

h2_problem2 : h2_problem2.c check_sorting_result
	$(CC) $(CFLAGS) -O3 -o h2_problem2 h2_problem2.c 

clean:
	-rm *.o
	-rm h2_problem1 
	-rm h2_problem2 
	-rm check

run : 
	$(RUNNER) -n $(n) ./$(ta)

.PHONY: clean
