#CC = /usr/bin/gcc -V 2.95.2
CC = gcc

OPT = -O3
ifeq ($(shell uname -p),athlon)
# -mcpu=athlon-xp deprecated starting from 3.4.0
OPT += -march=athlon-xp  -msse
endif

CFLAGS += -I. $(OPT) -g

all: neuralnet.o hash.o

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.cc
	$(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -c $<

hash.o : cache.h hash.h 
neuralnet.o : neuralnet.h 
