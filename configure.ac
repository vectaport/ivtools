#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Header
AC_PREREQ([2.67])

AC_INIT([InterViews Application Framework and Drawing Tools],[1.2.11],[johnston@vectaport.com],[ivtools],[http://www.ivtools.org])

dnl The foreign option allows the follow files to not exist: NEWS AUTHORS ChangeLog
AM_INIT_AUTOMAKE([subdir-objects foreign -Wall])
AC_CONFIG_SRCDIR([src/Unidraw/path.cc])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Process options
AC_MSG_CHECKING([with_ace])
AC_ARG_WITH([ace],
       [AS_HELP_STRING([--with-ace],[use the ACE library (default: check)])],,
       [with_ace=check])
AC_MSG_RESULT([${with_ace}])

AC_MSG_CHECKING([with_tiff])
AC_ARG_WITH([tiff],
       [AS_HELP_STRING([--with-tiff],[use the TIFF library (default: check)])],,
       [with_tiff=check])
AC_MSG_RESULT([${with_tiff}])

AC_MSG_CHECKING([with_clippoly])
AC_ARG_WITH([clippoly],
       [AS_HELP_STRING([--with-clippoly],[use the CLIPPOLY library (default: check)])],,
       [with_clippoly=check])
AC_MSG_RESULT([${with_clippoly}])

AC_MSG_CHECKING([PSFontDir])
AC_ARG_WITH([PSFontDir],
       [AS_HELP_STRING([--with-PSFontDir=DIR],[for font metrics (default: searches)])],,
       [with_PSFontDir=check])
AC_MSG_RESULT([${with_PSFontDir}])

# Checks for programs.
AC_LANG([C++])
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_AR
AC_PROG_INSTALL
ACLTX_PROG_PS2PDF([AC_MSG_WARN([Cannot find ps2pdf])])
AM_CONDITIONAL([PS2PDF], [test x$ps2pdf != xno])
LT_INIT

# Checks for libraries.
AC_PATH_XTRA
CFLAGS="${X_CFLAGS} ${CFLAGS}"
CXXFLAGS="${X_CFLAGS} ${CXXFLAGS}"
LDFLAGS="${X_LIBS} ${LDFLAGS}"

AC_CHECK_LIB([m], [pow], [MATH_LD=-lm], [AC_MSG_WARN([Cannot find math library])])
AC_CHECK_LIB([X11], [XOpenDisplay], [X11_LD=-lX11], [AC_MSG_ERROR([Cannot find X11 library])])
AC_CHECK_LIB([Xext], [XextFindDisplay], [XEXT_LD=-lXext], [AC_MSG_ERROR([Cannot find Xext library])])

AS_IF([test ${with_ace} != no],
 [AC_CHECK_LIB([ACE], [ace_cleanup_destroyer],
  [with_ace=yes
   ACE_LD=-lACE],
  [AS_IF([test ${with_ace} = check],
    [with_ace=no
     AC_MSG_WARN([Cannot find ACE library])],
    [AC_MSG_ERROR([Cannot find ACE library])])])])

AS_IF([test ${with_tiff} != no],
 [AC_CHECK_LIB([tiff], [TIFFOpen],
   [with_tiff=yes
    TIFF_LD=-ltiff],
   [AC_CHECK_LIB([TIFF], [TIFFOpen],
     [with_tiff=yes
      TIFF_LD=-lTIFF],
     [AS_IF([test ${with_tiff} = check],
       [with_tiff=no
        AC_MSG_WARN([Cannot find tiff or TIFF library])],
       [AC_MSG_ERROR([Cannot find tiff or TIFF library])])])])])

AS_IF([test ${with_clippoly} != no],
 [AC_CHECK_LIB([clippoly], [clippoly_version],
   [with_clippoly=yes
    CLIPPOLY_LD=-lclippoly],
   [AS_IF([test ${with_ace} = check],
     [with_clippoly=no
      AC_MSG_WARN([Cannot find clippoly library])],
     [AC_MSG_ERROR([Cannot find clippoly library])])])])

AM_CONDITIONAL([HAVE_LIBACE], [test ${with_ace} = yes])
AM_CONDITIONAL([HAVE_LIBTIFF], [test ${with_tiff} = yes])
AM_CONDITIONAL([HAVE_LIBCLIPPOLY], [test ${with_clippoly} = yes])

AC_SUBST(X11_LD)
AC_SUBST(XEXT_LD)
AC_SUBST(ACE_LD)
AC_SUBST(TIFF_LD)
AC_SUBST(CLIPPOLY_LD)

# Generated dynamic library version
dnl This is not the approved way of versioning dynamic libraries
dnl but we just want to generate version 1.2.8

dnl LIB_IV_VER=8:0:1		# generates 7.1.0
dnl LIB_IV_VER=2:8:2		# generates 0.2.8
dnl LIB_IV_VER=3:8:2		# generates 1.2.8

LIB_IV_VER=3:8:2
AC_SUBST(LIB_IV_VER)

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h inttypes.h malloc.h \
memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h \
sys/file.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h unistd.h])
AS_IF([test ${with_ace} = yes],
 [AC_CHECK_HEADERS([ace/Acceptor.h ace/ACE.h])])

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
AS_IF([test x"${with_PSFontDir}" = xcheck],
 [AC_MSG_NOTICE([scanning ad-hoc list of locations for PSFontDir])
  AC_CHECK_FILE([/usr/share/fonts/type1/gsfonts/a010015l.afm],
   [PSFontDir=/usr/share/fonts/type1/gsfonts],
   [AC_CHECK_FILE([/usr/share/ghostscript/fonts/bchr.afm],
     [PSFontDir=/usr/share/ghostscript/fonts],
     [AC_CHECK_FILE([/usr/lib/ghostscript/fonts/bchr.afm],
       [PSFontDir=/usr/lib/ghostscript/fonts],
       [AC_CHECK_FILE([/usr/lib/ps/bchr.afm],
	 [PSFontDir=/usr/lib/ps],
	 [AC_CHECK_FILE([/usr/lib/X11/fonts/Type1Adobe/afm/bchr.afm],
	   [PSFontDir=/usr/lib/X11/fonts/Type1Adobe/afm],
	   [AC_CHECK_FILE([/Library/Fonts/],
	     [PSFontDir=/Library/Fonts],
	     [AC_CHECK_FILE([/System/Library/Fonts/],
	       [PSFontDir=/System/Library/Fonts],
	       [AC_MSG_ERROR([cannot find PS Font Directory])])])])])])])])
  AC_MSG_NOTICE([Using PSFontDir ${PSFontDir}])],
 [PSFontDir="${with_PSFontDir}"])
AC_SUBST(PSFontDir)

# Random noise-control options.
AX_CFLAGS_WARN_ALL
AX_CXXFLAGS_WARN_ALL
AX_CHECK_COMPILE_FLAG([-Wno-narrowing], [CXXFLAGS="${CXXFLAGS} -Wno-narrowing"])
AX_CHECK_COMPILE_FLAG([-Wunused-variable], [CXXFLAGS="${CXXFLAGS} -Wunused-variable"])
AX_CHECK_COMPILE_FLAG([-Wchar-subscripts], [CXXFLAGS="${CXXFLAGS} -Wchar-subscripts"])

# Trailer
AC_CONFIG_FILES([Makefile src/Makefile src/include/Makefile])

AC_OUTPUT
