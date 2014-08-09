dnl a macro to check for ability to create python extensions
dnl  AM_CHECK_PYTHON_HEADERS([ACTION-IF-POSSIBLE], [ACTION-IF-NOT-POSSIBLE])
dnl function also defines PYTHON_INCLUDES
AC_DEFUN([AM_CHECK_PYTHON_HEADERS],
[AC_REQUIRE([AM_PATH_PYTHON])
dnl deduce PYTHON_INCLUDES
if test -x "$PYTHON-config"; then
  PYTHON_CONFIG="$PYTHON-config"
else
  PYTHON_CONFIG="$PYTHON ./python-config"
fi
AC_MSG_CHECKING([for python headers using $PYTHON_CONFIG --includes])

PYTHON_LIBS=`$PYTHON_CONFIG --ldflags 2>/dev/null`
PYTHON_INCLUDES=`$PYTHON_CONFIG  --includes 2>/dev/null`
if test $? = 0; then
  AC_MSG_RESULT([$PYTHON_INCLUDES])
else
  AC_MSG_RESULT([failed, will try another way])
  py_prefix=`$PYTHON -c "import sys; print(sys.prefix)"`
  py_exec_prefix=`$PYTHON -c "import sys; print(sys.exec_prefix)"`
  PYTHON_INCLUDES="-I${py_prefix}/include/python${PYTHON_VERSION}"
  PYTHON_LIBS="-L${py_prefix}/libs/python${PYTHON_VERSION} -lpython${PYTHON_VERSION}"
  if test "$py_prefix" != "$py_exec_prefix"; then
    PYTHON_INCLUDES="$PYTHON_INCLUDES -I${py_exec_prefix}/include/python${PYTHON_VERSION}"
    PYTHON_LIBS="$PYTHON_LIBS -L${py_exec_prefix}/lib/python${PYTHON_VERSION}"
  fi
  AC_MSG_NOTICE([setting python headers to $PYTHON_INCLUDES])
  AC_MSG_NOTICE([setting python libs to $PYTHON_LIBS])
fi
dnl check if the headers exist:
AC_MSG_CHECKING([whether python headers are sufficient])
AC_SUBST(PYTHON_INCLUDES)
AC_SUBST(PYTHON_LIBS)
save_CPPFLAGS="$CPPFLAGS"
save_LDFLAGS="$LDFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
LDFLAGS="$LDFLAGS $PYTHON_LIBS"
AC_TRY_CPP([#include <Python.h>],dnl
[AC_MSG_RESULT(yes)
$1],dnl
[AC_MSG_RESULT(no)
$2])
CPPFLAGS="$save_CPPFLAGS"
LDFLAGS="$save_LDFLAGS"
])

