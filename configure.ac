#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Header
AC_PREREQ([2.67])

AC_INIT([InterViews Application Framework and Drawing Tools],[1.2.8],[johnston@vectaport.com],[ivtools],[http://www.ivtools.org])

dnl The foreign option allows the follow files to not exist: NEWS AUTHORS ChangeLog
AM_INIT_AUTOMAKE([subdir-objects foreign])

AC_CONFIG_SRCDIR([src/Unidraw/path.cc])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
ACLTX_PROG_PS2PDF(AC_MSG_WARN([Cannot find ps2pdf]))
AM_CONDITIONAL([PS2PDF], [test x$ps2pdf != xno])

# Checks for libraries.
AC_CHECK_LIB([m], [pow], [MATH_LD=-lm], AC_MSG_WARN([Cannot find math library]))
AC_CHECK_LIB([X11], [XOpenDisplay], [X11_LD=-lX11], AC_MSG_ERROR([Cannot find X11 library]))
AC_CHECK_LIB([Xext], [XextFindDisplay], [XEXT_LD=-lXext], AC_MSG_ERROR([Cannot find Xext library]))
AC_CHECK_LIB([ACE], [ace_cleanup_destroyer], [ACE_LD=-lACE], AC_MSG_WARN([Cannot find ACE library]))
AC_CHECK_LIB([tiff], [TIFFOpen], [TIFF_LD=-ltiff],
 AC_CHECK_LIB([TIFF], [TIFFOpen], [TIFF_LD=-lTIFF], AC_MSG_WARN([Cannot find tiff or TIFF library])))
AC_CHECK_LIB([clippoly], [clippoly_version], [CLIPPOLY_LD=-lclippoly], AC_MSG_WARN([Cannot find clippoly library]))
AM_CONDITIONAL([HAVE_LIBACE], [test x${ACE_LD} != x])
AM_CONDITIONAL([HAVE_LIBTIFF], [test x${TIFF_LD} != x])
AM_CONDITIONAL([HAVE_LIBCLIPPOLY], [test x${CLIPPOLY_LD} != x])

AC_SUBST(X11_LD)
AC_SUBST(XEXT_LD)
AC_SUBST(ACE_LD)
AC_SUBST(TIFF_LD)
AC_SUBST(CLIPPOLY_LD)

LT_INIT

# Generated dynamic library version
dnl This is not the approved way of versioning dynamic libraries
dnl but we just want to generate version 1.2.8

dnl LIB_IV_VER=8:0:1		# generates 7.1.0
dnl LIB_IV_VER=2:8:2		# generates 0.2.8
dnl LIB_IV_VER=3:8:2		# generates 1.2.8

LIB_IV_VER=3:8:2
AC_SUBST(LIB_IV_VER)

# Checks for header files.
AC_PATH_X
AC_PATH_XTRA

AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h malloc.h \
memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h \
sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h \
ace/Acceptor.h ace/ACE.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

AH_TEMPLATE([sig_atomic_t], [Define to int if <signal.h> does not define.])
AC_CHECK_TYPES([sig_atomic_t], [], [AC_DEFINE([sig_atomic_t], [int])], [
#include <signal.h>
])

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRTOD

AC_CHECK_FUNCS([bzero floor getcwd gethostbyname gethostname \
gettimeofday isascii memchr memmove memset munmap pow realpath \
regcomp select socket sqrt strcasecmp strchr strcspn strdup strerror \
strncasecmp strrchr strstr strtol uname hypot hypotf])

# Checks for locations
AC_CHECK_FILE([/usr/share/fonts/type1/gsfonts/a010015l.afm],
 [PSFontDir=/usr/share/fonts/type1/gsfonts],
 AC_CHECK_FILE([/usr/share/ghostscript/fonts/bchr.afm],
  [PSFontDir=/usr/share/ghostscript/fonts],
  AC_CHECK_FILE([/usr/lib/ghostscript/fonts/bchr.afm],
   [PSFontDir=/usr/lib/ghostscript/fonts],
   AC_CHECK_FILE([/usr/lib/ps/bchr.afm],
    [PSFontDir=/usr/lib/ps],
    AC_CHECK_FILE([/usr/lib/X11/fonts/Type1Adobe/afm/bchr.afm],
     [PSFontDir=/usr/lib/X11/fonts/Type1Adobe/afm],
     AC_MSG_ERROR([cannot find PS Font Directory]))))))
AC_SUBST(PSFontDir)

# Trailer
AC_CONFIG_FILES([Makefile src/Makefile src/include/Makefile])

AC_OUTPUT
