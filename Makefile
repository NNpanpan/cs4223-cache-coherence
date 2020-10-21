CC = g++
CFLAGS = -Wall -std=c++14

LINKERFLAG = -lm

main: main.cpp
	${CC} -o coherence main.cpp ${CFLAGS}

clean:
	rm -r coherence
