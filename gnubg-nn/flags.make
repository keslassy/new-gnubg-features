
# OS_BEAROFF_DB: Include code to load & use an extended race database
# (gnubg_os.db)
#
# CONTAINMENT_CODE: Experimntal code for containment class, a sub class of
# crashed
#
# HAVE_DLFCN_H: if you have a working dynamic loading code
#
# LOADED_BO: define it if you want to use mainline gnubg_os0.bd and gnubg_ts0.bd
#            instead of compiled in version of those files.

EXTRAFLAGS =  -DOS_BEAROFF_DB -DLOADED_BO

ifeq ($(shell uname),Linux)
EXTRAFLAGS += -DHAVE_DLFCN_H
endif

