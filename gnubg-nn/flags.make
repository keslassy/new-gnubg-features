
# Compilation flags:
#
# OS_BEAROFF_DB:
#  Include code to load & use an extended race database (gnubg_os.db)
#
# HAVE_DLFCN_H:
#  Define it if you have a working dynamic loading code (dlopen,dlsym,dlclose).
#
# LOADED_BO:
#  Define it if you want to use mainline gnubg_os0.bd/gnubg_ts0.bd instead of
#  compiled in version of those files.
#
# CONTAINMENT_CODE:
#  Experimntal code for containment class, a sub class of crashed.

EXTRAFLAGS =  -DOS_BEAROFF_DB -DLOADED_BO

ifeq ($(shell uname),Linux)
EXTRAFLAGS += -DHAVE_DLFCN_H
endif

