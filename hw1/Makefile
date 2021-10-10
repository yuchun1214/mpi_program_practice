CC := mpicc
CFLAGS := -std=c99
RUNNER := mpirun

N ?= 10
TARGET ?= circuitSatisfiability

all: $(TARGET)

OBJS = circuitSatisfiability.o

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ ${OBJS}

run :
	$(RUNNER) -n $(N) ./${TARGET}	

clean:
	-rm $(TARGET)
	-rm *.o

.PHONY: clean
