INCLUDES=-I$(srcdir)/../analyze \
         -I$(srcdir)/../gnubg \
	 @PYTHON_CSPEC@

sagnubg_SOURCES = GetOpt.h GetOpt.h analyze.h bms.h bm.h equities.h osr.h main.cc GetOpt.cc

sagnubg_LDADD =@PYTHON_LSPEC@ \
	       -L$(srcdir)/../analyze \
	       -L$(srcdir)/../gnubg \
	       -L$(srcdir)/../gnubg/lib \
	       $(srcdir)/../analyze/libanalyze.la \
	       $(srcdir)/../gnubg/libgnubg.la \
	       $(srcdir)/../gnubg/lib/libneuralnet.la
bin_PROGRAMS = sagnubg
