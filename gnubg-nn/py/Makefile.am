INCLUDES=-I$(srcdir)/../analyze \
         -I$(srcdir)/../gnubg \
	 @PYTHON_CSPEC@

pygnubg_SOURCES = analyze.h bgdefs.h bm.h defs.h equities.h misc.h player.h br.h eval.h inputs.h mt19937int.h osr.h positionid.h pygnubg.h pynets.h pytrainer.h raceinfo.h stdutil.h pygnubg.cc pynets.cc pytrainer.cc raceinfo.cc
pygnubg_LDADD =@PYTHON_LSPEC@ \
	       -L$(srcdir)/../analyze \
	       -L$(srcdir)/../gnubg \
	       -L$(srcdir)/../gnubg/lib \
	       $(srcdir)/../analyze/libanalyze.la \
	       $(srcdir)/../gnubg/libgnubg.la \
	       $(srcdir)/../gnubg/lib/libneuralnet.la
bin_PROGRAMS = pygnubg
#SYSLIBS= -ldl -lpthread -lutil
