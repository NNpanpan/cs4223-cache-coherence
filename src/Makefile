CC = g++
CFLAGS = -Wall -std=c++11

LINKERFLAG = -lm

SRCS := $(wildcard *.cpp)
BINS := $(patsubst %.cpp, %.o, ${SRCS})

all : Bus.o Device.o Runner.o Core.o Cache.o MESIRunner.o DragonRunner.o MESIFRunner.o
${BINS} : %.o : %.cpp %.h
	${CC} -c $^ ${CFLAGS}

clean: 
	@echo "Cleaning object file"
	rm -rvf *.o *.h.gch 

