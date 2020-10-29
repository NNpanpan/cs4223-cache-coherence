CC = g++
CFLAGS = -Wall -std=c++14

LINKERFLAG = -lm

SRCS := $(wildcard *.cpp)
BINS := $(patsubst %.cpp, %.o, ${SRCS})

${BINS} : %.o : %.cpp %.h
	${CC} -c $^ ${CFLAGS}

all: $(BINS)

clean: 
	@echo "Cleaning object file"
	rm -rvf *.o *.h.gch 

