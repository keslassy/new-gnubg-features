CC = g++

FLAGS = -Wall -W
# remove those if you dont want debug or asserts
FLAGS += -DDEBUG -DDOICC 

include ../flags.make
FLAGS += $(EXTRAFLAGS)

OPT = -O3
ifeq ($(shell uname -p),athlon)
# deprecated since 3.4.0 -mcpu=athlon-xp
OPT += -march=athlon-xp  -msse
endif

GNUBG = $(wildcard ../gnubg)
BGOBJS = eval.o inputs.o positionid.o pub_eval.o mt19937int.o eggmoveg.o \
         osr.o br1.o br2.o bearoffgammon.o racebg.o bearoffdb.o

LIBOBGS = $(wildcard $(GNUBG)/lib/*.o)

OBJS =  bm.o danalyze.o analyze.o player.o equities.o bms.o mec.o dice_gen.o
BGO =   $(foreach o, $(BGOBJS), $(GNUBG)/$(o))

all: libanalyze.a

libanalyze.a : $(OBJS) $(BGO) $(LIBOBGS)
	ar rcv libanalyze.a $?

%.o: %.cc
	$(CC) -Wall -g -c $(FLAGS) $(OPT) -I$(GNUBG) $<

clean:
	rm *.a *.o

bgdefs.h: $(GNUBG)/eval.h $(GNUBG)/positionid.h
	touch bgdefs.h

analyze.o : minmax.h bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h misc.h ../gnubg/osr.h bms.h bm.h analyze.h defs.h equities.h dice_gen.h 
bm.o : minmax.h bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h equities.h defs.h bms.h bm.h 
bms.o : minmax.h equities.h defs.h bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h bms.h 
danalyze.o : bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h misc.h minmax.h equities.h defs.h danalyze.h analyze.h bm.h 
dice_gen.o : bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h minmax.h dice_gen.h 
equities.o : minmax.h bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h mec.h equities.h defs.h 
mec.o : mec.h 
player.o : minmax.h bgdefs.h ../gnubg/eval.h ../gnubg/positionid.h ../gnubg/mt19937int.h misc.h equities.h defs.h danalyze.h player.h analyze.h 
