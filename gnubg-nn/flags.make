
# OS_BEAROFF_DB: Include code to load & use an extended race database
# (gnubg_os.db)
#
# CONTAINMENT_CODE: Experimntal code for containment class, a sub class of
# crashed
#
# HAVE_DLFCN_H: if you have a working dynamic loading code
#

EXTRAFLAGS = -DOS_BEAROFF_DB -DHAVE_DLFCN_H
