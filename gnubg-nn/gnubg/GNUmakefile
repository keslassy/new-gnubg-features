CC = gcc
CXX = g++

OPT = -O3
ifeq ($(shell uname -p),athlon)
# deprecated since 3.4.0 -mcpu=athlon-xp
OPT += -march=athlon-xp  -msse
endif

CFLAGS += $(OPT) -Ilib -g -Wall -W

include ../flags.make
CFLAGS += $(EXTRAFLAGS) -DHAVE_BEAROFF2

files = pub_eval.o positionid.o osr.o eval.o inputs.o \
        br1.o br2.o eggmoveg.o mt19937int.o bearoffgammon.o racebg.o bearoffdb.o

all: $(files)

stdalf = eggmoveg.o  positionid.o
makebear: makebear.o $(stdalf)
	$(CXX) $(OPT) $(LDFLAGS) -o $@ $^

makegm: makegm.o $(stdalf)
	$(CXX) $(OPT) $(LDFLAGS) -o $@ $^


BEAROFFFILES = br1.cc br2.cc
ifdef WIN32
br1.cc: makebear.exe
     makebear $(BEAROFFFILES)
else
$(BEAROFFFILES) : makebear
	@echo Computing the Answer to Life, The Universe and Everything ...
	[ -s br1.cc ] || ./makebear $(BEAROFFFILES)
	chmod a-w $(BEAROFFFILES)

bearoffgammon.cc: makegm
	rm -f $@
	./makegm > $@
endif

clean:
	rm $(files)

%.o: %.c
	$(CC) $(CFLAGS)  -o $@ -c $<

%.o: %.cc
	$(CXX) $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) -o $@  -c $<

dep:
	makedep -Ilib *.c *.cc

bearoffdb.o : bearoffdb.h lib/hash.h positionid.h eval.h bearoffgammon.h 
eggmoveg.o : eval.h 
eval.o : config.h lib/cache.h lib/neuralnet.h eval.h positionid.h inputs.h mt19937int.h bearoffgammon.h racebg.h osr.h br.h bearoffdb.h lib/hash.h 
inputs.o : eval.h inputs.h 
positionid.o : positionid.h 
bearoffgammon.o : bearoffgammon.h positionid.h 
br1.o : br.h 
br2.o : br.h 
makebear.o : eval.h positionid.h 
makegm.o : eval.h positionid.h 
osr.o : br.h osr.h positionid.h eval.h mt19937int.h 
racebg.o : racebg.h positionid.h 
