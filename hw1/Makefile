CC := mpicc
CFLAGS := -std=c99
RUNNER := mpirun

N ?= 10

all: monte_carlo_method circuitSatisfiability

OBJS = circuitSatisfiability.o monte_carlo_method.o

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

monte_carlo_method: monte_carlo_method.o
	$(CC) $(CFLAGS) -o $@ $<

circuitSatisfiability: circuitSatisfiability.o
	$(CC) $(CFLAGS) -o $@ $<

run :
	$(RUNNER) -n $(N) ./${ta}	

clean:
	-rm circuitSatisfiability
	-rm monte_carlo_method
	-rm *.o

.PHONY: clean
