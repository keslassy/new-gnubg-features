CXX = g++

GNUBG = ../gnubg/
ANALYZE = ../analyze

PYVERSION= 2.3
PYPATH=    /usr/local

WARNING_FLAGS= -Wall -W  -Wconversion 

CXXFLAGS +=  $(WARNING_FLAGS) -I$(ANALYZE) -I$(GNUBG) -g \
             -I$(PYPATH)/include/python$(PYVERSION)

include ../flags.make
CXXFLAGS += $(EXTRAFLAGS)

LIBPYTHON= -L$(PYPATH)/lib -lpython$(PYVERSION)
SYSLIBS= -ldl -lpthread -lutil

ALLLIBS=	$(LIBPYTHON) $(SYSLIBS)

LDFLAGS = -Xlinker -export-dynamic -g -L$(ANALYZE) $(ALLLIBS)

OBJS = pygnubg.o pynets.o pytrainer.o raceinfo.o

OPT =  -O3
ifeq ($(shell uname -p),athlon)
# deprecated since 3.4.0 -mcpu=athlon-xp
OPT += -march=athlon-xp -msse
endif

%.o: %.cc
	$(CXX) -DDEBUG $(CXXFLAGS) $(OPT) -c -o $@  $<


all: pygnubg

safe: libs all

clean:
	rm *.o *.pyc pygnubg

libs:
	$(MAKE) -C ../gnubg/lib
	$(MAKE) -C ../gnubg
	$(MAKE) -C ../analyze


pygnubg: $(OBJS) $(ANALYZE)/libanalyze.a
	$(CXX)  -o $@ \
        $(OBJS) $(LDFLAGS) -lanalyze

pygnubg.o : stdutil.h ../analyze/analyze.h ../analyze/defs.h ../analyze/bm.h ../analyze/player.h ../analyze/equities.h ../analyze/analyze.h ../analyze/equities.h ../analyze/misc.h ../analyze/bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h ../gnubg/br.h ../gnubg/osr.h pygnubg.h pynets.h pytrainer.h raceinfo.h 
pynets.o : pynets.h pygnubg.h ../gnubg/eval.h ../gnubg/inputs.h 
pytrainer.o : ../gnubg/positionid.h ../gnubg/eval.h pytrainer.h 
raceinfo.o : raceinfo.h 
