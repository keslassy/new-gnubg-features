CXX = g++

GNUBG = ../gnubg
ANALYZE = ../analyze

OPT = -O3
ifeq ($(shell uname -p),athlon)
OPT += -march=athlon-xp -msse
endif

VERSION = 1.92

CXXFLAGS +=  $(OPT) -I$(ANALYZE) -I$(GNUBG)

include ../flags.make
CXXFLAGS += $(EXTRAFLAGS)

ELDFLAGS += -L$(ANALYZE)

OBJS = main.o GetOpt.o

%.o: %.cc
	$(CXX) -DDEBUG $(CXXFLAGS) -c -o $@  $<

all: sagnubg 

safe: libs all

libs:
	$(MAKE) -C $(GNUBG)
	$(MAKE) -C $(ANALYZE)

sagnubg: $(OBJS) $(ANALYZE)/libanalyze.a
	$(CXX)  -o $@ $(OBJS) $(OPT) $(ELDFLAGS) $(LDFLAGS) -lanalyze -ldl

