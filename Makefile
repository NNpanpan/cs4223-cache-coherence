CC = g++
CFLAGS = -Wall -std=c++14

LINKERFLAG = -lm

SRCS := $(wildcard src/*.cpp)
BINS := $(patsubst %.cpp, %.o, ${SRCS})

main: main.cpp
	@$(MAKE) all -C src
	${CC} -o coherence main.cpp ${BINS} ${CFLAGS}

clean:
	rm -r coherence
