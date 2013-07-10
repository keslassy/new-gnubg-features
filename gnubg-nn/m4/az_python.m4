dnl
dnl @category InstalledPackages
dnl @author Robert White <kranki@mac.com>
dnl @author Dustin Mitchell <dustin@cs.uchicago.edu>
dnl @version 2005-01-14
dnl @license GPLWithACException

# AZ_PYTHON_DEFAULT( )
# -----------------
# Sets the default to not include Python support.

AC_DEFUN([AZ_PYTHON_DEFAULT],
[
    az_use_python=true
])

# AZ_PYTHON_CSPEC( )
# -----------------
# Set up the c compiler options to compile Python
# embedded programs/libraries in $PYTHON_CSPEC if
# $PYTHON has been defined.

AC_DEFUN([AZ_PYTHON_CSPEC],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -n "$PYTHON"
    then
        az_python_prefix=`${PYTHON} -c "import sys; print sys.prefix"`
        if test -z "$az_python_prefix"
        then
            AC_MSG_ERROR([Python Prefix is not known])
        fi
        az_python_execprefix=`${PYTHON} -c "import sys; print sys.exec_prefix"`
        az_python_version=`$PYTHON -c "import sys; print sys.version[[:3]]"`
        az_python_includespec="-I${az_python_prefix}/include/python${az_python_version}"
        if test x"$python_prefix" != x"$python_execprefix"; then
            az_python_execspec="-I${az_python_execprefix}/include/python${az_python_version}"
            az_python_includespec="${az_python_includespec} $az_python_execspec"
        fi
        az_python_ccshared=`${PYTHON} -c "import distutils.sysconfig; print distutils.sysconfig.get_config_var('CFLAGSFORSHARED')"`
        az_python_cspec="${az_python_ccshared} ${az_python_includespec}"
        AC_SUBST([PYTHON_CSPEC], [${az_python_cspec}])
        AC_MSG_NOTICE([PYTHON_CSPEC=${az_python_cspec}])
    fi
])

# AZ_PYTHON_LSPEC( )
# -----------------
# Set up the linker options to link Python embedded
# programs/libraries in $PYTHON_LSPEC if $PYTHON
# has been defined.

AC_DEFUN([AZ_PYTHON_LSPEC],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -n "$PYTHON"
    then
        AZ_PYTHON_RUN([
import sys
if (sys.platform == "darwin"):
    strLinkSpec = "-framework Python"
else:
    import distutils.sysconfig
    strUseFrameWork = "--enable-framework"
    dictConfig = distutils.sysconfig.get_config_vars( )
    strConfigArgs = dictConfig.get("CONFIG_ARGS")
    strLinkSpec =  dictConfig.get('LDFLAGS')
    if -1 ==  strConfigArgs.find(strUseFrameWork):
        strLibPL = dictConfig.get("LIBPL")
        if strLibPL and (strLibPL != ""):
            strLinkSpec += " -L%s" % (strLibPL)
        strSys = dictConfig.get("SYSLIBS")
        if strSys and (strSys != ""):
            strLinkSpec += " %s" % (strSys)
        strSHL = dictConfig.get("SHLIBS")
        if strSHL and (strSHL != ""):
            strLinkSpec += " %s" % (strSHL)
        # Construct the Python Library Name.
        strTmplte = " -lpython%d.%d"
        if (sys.platform == "win32") or (sys.platform == "os2emx"):
            strTmplte = " -lpython%d%d"
        strWrk = strTmplte % ( (sys.hexversion >> 24),
                            ((sys.hexversion >> 16) & 0xff))
        strLinkSpec += strWrk
    else:
        # This is not ideal since it changes the search path
        # for Frameworks which could have side-effects on
        # other included Frameworks.  However, it is necessary
        # where someone has installed more than one frameworked
        # Python.  Frameworks are really only used in MacOSX.
        strLibFW = dictConfig.get("PYTHONFRAMEWORKPREFIX")
        if strLibFW and (strLibFW != ""):
            strLinkSpec += " -F%s" % (strLibFW)
    strLinkSpec += " %s" % (dictConfig.get('LINKFORSHARED'))
print strLinkSpec
        ])
        AC_SUBST([PYTHON_LSPEC], [${az_python_output}])
        AC_MSG_NOTICE([PYTHON_LSPEC=${az_python_output}])
    fi
])


# AZ_PYTHON_PATH( )
# -----------------
# Look for Python and set the output variable 'PYTHON'
# to 'python' if found, empty otherwise.

AC_DEFUN([AZ_PYTHON_PATH],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    AC_PATH_PROG( PYTHON, python, [], $1 )
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable not found])
    fi
])

# AZ_PYTHON_PREFIX( )
# -------------------
# Use the values of $prefix and $exec_prefix for the corresponding
# values of PYTHON_PREFIX and PYTHON_EXEC_PREFIX.

AC_DEFUN([AZ_PYTHON_PREFIX],
[
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable Path is not known])
    fi
    ax_python_prefix=`${PYTHON} -c "import sys; print sys.prefix"`
    ax_python_execprefix=`${PYTHON} -c "import sys; print sys.exec_prefix"`
    AC_SUBST([PYTHON_PREFIX], ["${ax_python_prefix}"])
    AC_SUBST([PYTHON_EXECPREFIX], ["${ax_python_execprefix}"])
])



# AZ_PYTHON_RUN( PYTHON_PROGRAM )
# -----------------
# Run a Python Test Program saving its output
# in az_python_output and its condition code
# in az_python_cc.

AC_DEFUN([AZ_PYTHON_RUN],
[
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -z "$PYTHON"
    then
        AC_MSG_ERROR([Python Executable not found])
    else
        cat >conftest.py <<_ACEOF
$1
_ACEOF
        az_python_output=`$PYTHON conftest.py`
        az_python_cc=$?
        rm conftest.py
        if test -f "conftest.pyc"
        then
            rm conftest.pyc
        fi
    fi
])



# AZ_PYTHON_VERSION_CHECK( VERSION, [ACTION-IF-TRUE], [ACTION-IF-FALSE] )
# -----------------------------------------------------------------------------
# Run ACTION-IF-TRUE if the Python interpreter has version >= VERSION.
# Run ACTION-IF-FALSE otherwise.
# This test uses sys.hexversion instead of the string equivalant (first
# word of sys.version), in order to cope with versions such as 2.2c1.
# hexversion has been introduced in Python 1.5.2; it's probably not
# worth to support older versions (1.5.1 was released on October 31, 1998).

AC_DEFUN([AZ_PYTHON_VERSION_CHECK],
 [
    AC_ARG_VAR( [PYTHON], [Python Executable Path] )
    if test -n "$PYTHON"
    then
        AC_MSG_CHECKING([whether $PYTHON version >= $1])
        AZ_PYTHON_RUN([
import sys, string
# split strings by '.' and convert to numeric.  Append some zeros
# because we need at least 4 digits for the hex conversion.
minver = map(int, string.split('$1', '.')) + [[0, 0, 0]]
minverhex = 0
for i in xrange(0, 4): minverhex = (minverhex << 8) + minver[[i]]
if sys.hexversion >= minverhex:
    sys.exit( 0 )
else:
    sys.exit( 1 )
        ])
        if test $az_python_cc -eq 0
        then
            $2
        m4_ifvaln(
            [$3],
            [else $3]
        )
        fi
    fi
])



# AZ_PYTHON_VERSION_ENSURE( VERSION )
# -----------------
# Insure that the Python Interpreter Version
# is greater than or equal to the VERSION
# parameter.

AC_DEFUN([AZ_PYTHON_VERSION_ENSURE],
[
    AZ_PYTHON_VERSION_CHECK(
        [$1],
        [AC_MSG_RESULT(yes)],
        [AC_MSG_ERROR(too old)]
    )
])



# AZ_PYTHON_WITH( [path] )
# -----------------------------------------------------------------
# Handles the various --with-python commands.
# Input:
#   $1 is the optional search path for the python executable if needed
# Ouput:
#   USE_PYTHON (AM_CONDITIONAL) is true if python executable found
#   and --with-python was requested; otherwise false.
#   $PYTHON contains the full executable path to python if USE_PYTHON
#   is true.
#
# Example:
#   AZ_PYTHON_WITH( )
#   or
#   AZ_PYTHON_WITH("/usr/bin")

AC_DEFUN([AZ_PYTHON_WITH],
[
    AC_ARG_VAR([PYTHON],[Python Executable Path])

    # unless PYTHON was supplied to us (as a precious variable),
    # see if --with-python[=PythonExecutablePath], --with-python,
    # --without-python or --with-python=no was given.
    if test -z "$PYTHON"
    then
        AC_MSG_CHECKING(for --with-python)
        AC_ARG_WITH(
            python,
            AC_HELP_STRING([--with-python@<:@=PYTHON@:>@],
                [absolute path name of Python executable]
            ),
            [
                if test "$withval" = "yes"
                then
                    # "yes" was specified, but we don't have a path
                    AC_MSG_RESULT(yes)
                    AZ_PYTHON_PATH()
                    AZ_PYTHON_PREFIX( )
                    az_use_python=true
                    AM_CONDITIONAL(USE_PYTHON, test x"$az_use_python" = x"true")
                    AC_DEFINE(USE_PYTHON,1,[Define if you want to use Python inside gnubg])
                elif test "$withval" = "no"
                then
                    AC_MSG_RESULT(no)
                    az_use_python=false
                    AM_CONDITIONAL(USE_PYTHON, test x"$az_use_python" = x"true")
                else
                    # $withval must be the executable path then.
                    AC_SUBST([PYTHON], ["${withval}"])
                    AC_MSG_RESULT($withval)
                    az_use_python=true
                    AM_CONDITIONAL(USE_PYTHON, test x"$az_use_python" = x"true")
                    AC_DEFINE(USE_PYTHON,1,[Define if you want to use Python inside gnubg])
                    AZ_PYTHON_PREFIX( )
                fi
            ],
            [
                # --with-python was not specified.
                if test "x$az_use_python" = "xtrue"
                then
                    AC_MSG_RESULT(enabled by default)
                    AZ_PYTHON_PATH( )
                    AZ_PYTHON_PREFIX( )
                    AC_DEFINE(USE_PYTHON,1,[Define if you want to use Python inside gnubg])
                else
                     AC_MSG_RESULT(no)
                     az_use_python=false
                fi
                AM_CONDITIONAL(USE_PYTHON, test x"$az_use_python" = x"true")
            ]
        )
    fi

])