CC = g++
CFLAGS = -Wall -std=c++14

LINKERFLAG = -lm

main: main.cpp
	${CC} -o main main.cpp ${CFLAGS}

