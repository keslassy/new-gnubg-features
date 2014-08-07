dnl a macro to check for ability to create python extensions
dnl  AM_CHECK_PYTHON_HEADERS([ACTION-IF-POSSIBLE], [ACTION-IF-NOT-POSSIBLE])
dnl function also defines PYTHON_INCLUDES
AC_DEFUN([AM_CHECK_PYTHON_HEADERS],
[AC_REQUIRE([AM_PATH_PYTHON])
AC_MSG_CHECKING(for headers required to compile python extensions)
dnl deduce PYTHON_INCLUDES
if test -x "$PYTHON"; then
if test -x "$PYTHON-config"; then
PYTHON_CONFIG="$PYTHON-config"
else
PYTHON_CONFIG="$PYTHON ./python-config"
fi
PYTHON_INCLUDES=`$PYTHON_CONFIG --includes 2>/dev/null`
PYTHON_LIBS=`$PYTHON_CONFIG --libs 2>/dev/null`
fi
AC_ARG_VAR(PYTHON_INCLUDES, [Location of the python header files])
AC_ARG_VAR(PYTHON_LIBS, [Libraries needed for python inclusion])
dnl check if the headers exist:
save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $PYTHON_INCLUDES"
AC_TRY_CPP([#include <Python.h>],dnl
[AC_MSG_RESULT(found)
$1],dnl
[AC_MSG_RESULT(not found)
PYTHON_INCLUDES=""
PYTHON_LIBS=""
$2])
CPPFLAGS="$save_CPPFLAGS"
])

