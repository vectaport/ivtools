# ivtools configure.ac
# Run 'autoreconf -i' to generate ./configure from this file.

AC_INIT([ivtools], [2.1.1], [ivtools-user@lists.sourceforge.net])
AC_CONFIG_AUX_DIR([src/scripts])

# Determine canonical host triple (uses config.guess / config.sub)
AC_CANONICAL_HOST

# ---------------------------------------------------------------------------
# Platform detection
# Derive both CPU (for config/site.def.$CPU and config/config.mk)
# and platform_part (for selecting config/$platform_part-gcc.mk template)
# ---------------------------------------------------------------------------
case "$host" in
    *-*-darwin*)        CPU=DARWIN;  platform_part=darwin    ;;
    *-*-freebsd2.1*)    CPU=FREEBSD; platform_part=freebsd2.1 ;;
    *-*-freebsd*)       CPU=FREEBSD; platform_part=freebsd   ;;
    *-*-hpux*10.20*)    CPU=HP800;   platform_part=hpux10.20 ;;
    *-*-hpux*)          CPU=HP800;   platform_part=hpux       ;;
    *-*-netbsd*)        CPU=NETBSD;  platform_part=netbsd    ;;
    *-*-linux*)         CPU=LINUX;   platform_part=linux      ;;
    m88k-*-sysv4*)                   platform_part=m88k-sysv4 ;;
    m88k-*-sysv3*)                   platform_part=m88k-sysv3 ;;
    *-*-osf*3.0*)       CPU=ALPHA;   platform_part=alpha3.0  ;;
    *-*-osf*3.2*)       CPU=ALPHA;   platform_part=alpha3.2  ;;
    *-*-osf*4.0*)       CPU=ALPHA;   platform_part=alpha4    ;;
    *sgi*-irix*5*)      CPU=SGI;     platform_part=irix5     ;;
    *sgi*-irix*6*)      CPU=SGI;     platform_part=irix6     ;;
    *-*-sco3.2v4*)                   platform_part=sco3.2.4  ;;
    *-*-solaris*)       CPU=SUN4;    platform_part=solaris   ;;
    *-*-sunos4*)        CPU=SUN4;    platform_part=sunos4    ;;
    *-*-cygwin*)        CPU=CYGWIN;  platform_part=default   ;;
    *)                  CPU=LINUX;   platform_part=default   ;;
esac

AC_MSG_RESULT([platform: CPU=$CPU  platform_part=$platform_part])

# ---------------------------------------------------------------------------
# Optional dependencies: ACE
# ---------------------------------------------------------------------------
AC_ARG_WITH([ace],
    [  --with-ace=<path>         Path to ACE include files],
    [ACE=$withval
     if test -f "$ACE/ace/ACE.h"; then
         AC_MSG_RESULT([Found ace/ACE.h in $ACE])
     else
         AC_MSG_ERROR([Cannot find ace/ACE.h in $ACE (--with-ace=$ACE)])
     fi])

AC_ARG_WITH([ace-libs],
    [  --with-ace-libs=<path>    Path to ACE libraries],
    [ACE_LIBS=$withval])

ACE_ENABLED=0
if test x"$ACE" = x; then
    AC_MSG_NOTICE([No ACE path specified. Use --with-ace=<path> to enable ACE support.])
else
    AC_MSG_RESULT([Building with ACE support: ACE=$ACE])
    if test -f "$ACE_LIBS/libACE.a"    -o -f "$ACE_LIBS/libACE.so" \
         -o -f "$ACE_LIBS/libACE.dll.a" -o -f "$ACE_LIBS/libACE.dylib"; then
        AC_MSG_RESULT([Found libACE.* in $ACE_LIBS])
        ACE_ENABLED=1
    elif test -f "$ACE/ace/libACE.a"    -o -f "$ACE/ace/libACE.so" \
           -o -f "$ACE/ace/libACE.dll.a" -o -f "$ACE/ace/libACE.dylib"; then
        AC_MSG_RESULT([Found libACE.* in $ACE/ace])
        ACE_ENABLED=1
        ACE_LIBS=$ACE/ace
    else
        AC_MSG_ERROR([Cannot find libACE.* in $ACE_LIBS or $ACE/ace. Use --with-ace-libs=<path>.])
    fi
fi

# ---------------------------------------------------------------------------
# Optional dependencies: clippoly
# ---------------------------------------------------------------------------
AC_ARG_WITH([clippoly],
    [  --with-clippoly=<path>    Path to clippoly include files],
    [CLIPPOLY=$withval
     if test -f "$CLIPPOLY/poly.h"; then
         AC_MSG_RESULT([Found poly.h in $CLIPPOLY])
     else
         AC_MSG_ERROR([Cannot find poly.h in $CLIPPOLY (--with-clippoly=$CLIPPOLY)])
     fi])

AC_ARG_WITH([clippoly-libs],
    [  --with-clippoly-libs=<path>  Path to clippoly libraries],
    [CLIPPOLY_LIBS=$withval])

CLIPPOLY_ENABLED=0
if test x"$CLIPPOLY" = x; then
    AC_MSG_NOTICE([No clippoly path specified. Use --with-clippoly=<path> to enable.])
else
    if test -f "$CLIPPOLY_LIBS/libclippoly.a"  -o -f "$CLIPPOLY_LIBS/libclippoly.so" \
         -o -f "$CLIPPOLY_LIBS/libclippoly.dylib"; then
        AC_MSG_RESULT([Found libclippoly.* in $CLIPPOLY_LIBS])
        CLIPPOLY_ENABLED=1
    elif test -f "$CLIPPOLY/libclippoly.a"  -o -f "$CLIPPOLY/libclippoly.so" \
           -o -f "$CLIPPOLY/libclippoly.dylib"; then
        AC_MSG_RESULT([Found libclippoly.* in $CLIPPOLY])
        CLIPPOLY_ENABLED=1
        CLIPPOLY_LIBS=$CLIPPOLY
    else
        AC_MSG_ERROR([Cannot find libclippoly.* in $CLIPPOLY or $CLIPPOLY_LIBS. Use --with-clippoly-libs=<path>.])
    fi
fi

# ---------------------------------------------------------------------------
# Optional dependencies: TIFF
# ---------------------------------------------------------------------------
TIFF_ENABLED=0
TIFF=""
TIFFLIBS=""
TIFFINCS=""

AC_ARG_WITH([tiff],
    [  --with-tiff=<path>        Path to installed TIFF files],
    [TIFF=$withval
     if test x"$TIFFLIBS" = x; then
         if test -d "$TIFF/lib"; then
             TIFFLIBS=$TIFF/lib
         else
             TIFFLIBS=$TIFF/Build/lib
         fi
     fi])

AC_ARG_WITH([tiff-libs],
    [  --with-tiff-libs=<path>   Path to compiled TIFF libraries],
    [TIFFLIBS=$withval])

if test x"$TIFF" = x; then
    AC_MSG_NOTICE([No TIFF path specified. Use --with-tiff=<path> to enable.])
else
    if test ! -f "$TIFF/include/tiff.h"; then
        AC_MSG_WARN([Cannot find $TIFF/include/tiff.h — TIFF may not work])
    else
        TIFFINCS="$TIFF/include"
    fi
    AC_MSG_RESULT([Building with TIFF support: includes=$TIFFINCS libs=$TIFFLIBS])
    TIFF_ENABLED=1
fi

# ---------------------------------------------------------------------------
# Optional dependencies: Qt (future migration target — detected but not used)
# ---------------------------------------------------------------------------
QT_ENABLED=0
QT=""
QTLIBS=""
QTINCS=""

AC_ARG_WITH([qt],
    [  --with-qt=<path>          Path to Qt directory],
    [QT=$withval
     if test x"$QTLIBS" = x; then
         if test -d "$QT/lib"; then
             QTLIBS=$QT/lib
         else
             QTLIBS=$QT/Build/lib
         fi
     fi])

AC_ARG_WITH([qt-libs],
    [  --with-qt-libs=<path>     Path to compiled Qt libraries],
    [QTLIBS=$withval])

if test x"$QT" = x; then
    AC_MSG_NOTICE([No Qt path specified. Use --with-qt=<path> to enable.])
else
    if test -f "$QT/include/qt.h" -o -f "$QT/include/Qt/qglobal.h"; then
        QTINCS="$QT/include"
    else
        AC_MSG_WARN([Cannot find Qt headers in $QT/include])
    fi
    AC_MSG_RESULT([Building with Qt support: includes=$QTINCS libs=$QTLIBS])
    QT_ENABLED=1
fi

# ---------------------------------------------------------------------------
# Install options
# ---------------------------------------------------------------------------
AC_ARG_ENABLE([install-relative],
    [  --enable-install-relative   Install relative to source tree],
    [INSTALLRELATIVE=YES], [INSTALLRELATIVE=NO])

AC_ARG_ENABLE([use-rpath],
    [  --enable-use-rpath          Embed shared library paths in executables],
    [USERPATH=YES], [USERPATH=NO])

AC_ARG_ENABLE([install-subdir],
    [  --enable-install-subdir     Install into an ivtools subdirectory],
    [INSTALLSUBDIR=YES], [INSTALLSUBDIR=NO])

# ---------------------------------------------------------------------------
# Compiler and tool checks
# ---------------------------------------------------------------------------
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CXXCPP
AC_PROG_RANLIB

# ---------------------------------------------------------------------------
# X11 paths
# ---------------------------------------------------------------------------
AC_PATH_X

if test x"$x_libraries" = x; then
    AC_MSG_WARN([Cannot find X11 libraries. Use --x-libraries=<path> to specify.])
    AC_MSG_WARN([You may need to manually set XCONFIGDIR in config/config.mk after configure completes.])
    x_libraries="/usr/lib"
fi
if test x"$x_includes" = x; then
    AC_MSG_WARN([Cannot find X11 includes. Use --x-includes=<path> to specify.])
    x_includes="/usr/include"
fi

# ---------------------------------------------------------------------------
# libstdc++ ABI version checks (legacy — both expected to be 0 on modern toolchains)
# ---------------------------------------------------------------------------
AC_CHECK_LIB([stdc++], [peek__7istream],
    [LIBSTDCPLUSPLUS2=1], [LIBSTDCPLUSPLUS2=0])
AC_CHECK_LIB([stdc++], [_ZNSi4peekEv],
    [LIBSTDCPLUSPLUS3=1], [LIBSTDCPLUSPLUS3=0])

# ---------------------------------------------------------------------------
# socklen_t -- AC_TYPE_SOCKLEN_T removed in autoconf 2.70; check manually.
# Sets ac_cv_type_socklen_t to YES/NO for substitution into config.defs.in.
# ---------------------------------------------------------------------------
AC_MSG_CHECKING([for socklen_t])
AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([
#include <sys/types.h>
#include <sys/socket.h>
    ], [socklen_t len = 0; (void)len;])],
    [ac_cv_type_socklen_t=YES
     AC_MSG_RESULT([yes])],
    [ac_cv_type_socklen_t=NO
     AC_MSG_RESULT([no])])

# ---------------------------------------------------------------------------
# Verify the platform config fragment exists
# ---------------------------------------------------------------------------
if test "$GXX" = "yes"; then
    cc_part="gcc"
else
    cc_part=`echo "$CXX" | sed "s/ //g"`
fi

host_frag=${platform_part}-${cc_part}.mk
host_fragd=${platform_part}-${cc_part}.defs

if test -f "config/$host_frag"; then
    AC_MSG_RESULT([Found config/$host_frag])
else
    AC_MSG_ERROR([config/$host_frag not found — please create it for this platform.])
fi

# ---------------------------------------------------------------------------
# AC_SUBST declarations
# ---------------------------------------------------------------------------
AC_SUBST([CPU])
AC_SUBST([prefix])
AC_SUBST([CXX_INCLUDE_DIR])
AC_SUBST([LIBSTDCPLUSPLUS2])
AC_SUBST([LIBSTDCPLUSPLUS3])
AC_SUBST([x_includes])
AC_SUBST([x_libraries])
AC_SUBST([ACE])
AC_SUBST([ACE_LIBS])
AC_SUBST([ACE_ENABLED])
AC_SUBST([CLIPPOLY])
AC_SUBST([CLIPPOLY_LIBS])
AC_SUBST([CLIPPOLY_ENABLED])
AC_SUBST([TIFF])
AC_SUBST([TIFFLIBS])
AC_SUBST([TIFFINCS])
AC_SUBST([TIFF_ENABLED])
AC_SUBST([QT])
AC_SUBST([QTLIBS])
AC_SUBST([QTINCS])
AC_SUBST([QT_ENABLED])
AC_SUBST([INSTALLRELATIVE])
AC_SUBST([USERPATH])
AC_SUBST([INSTALLSUBDIR])
AC_SUBST([ac_cv_type_socklen_t])
PWD=`pwd`
AC_SUBST([PWD])

# ---------------------------------------------------------------------------
# Generate config/config-$host_fragd from config/config.defs.in
# ---------------------------------------------------------------------------
AC_CONFIG_FILES([config/config-${host_fragd}:config/config.defs.in])
AC_OUTPUT

# ---------------------------------------------------------------------------
# Write config/config.mk (read by every imake-generated Makefile)
# ---------------------------------------------------------------------------
AC_MSG_NOTICE([Writing config/config.mk])
cat > config/config.mk << EOF
XCONFIGDIR = ${x_libraries}/X11/config
ABSTOP = ${PWD}
CPU = ${CPU}
LIBSTDCPLUSPLUS2 = ${LIBSTDCPLUSPLUS2}
LIBSTDCPLUSPLUS3 = ${LIBSTDCPLUSPLUS3}
EOF

# ---------------------------------------------------------------------------
# Bootstrap Makefile from Makefile.orig if not present
# (eliminates the manual 'cp Makefile.orig Makefile' step)
# ---------------------------------------------------------------------------
if test ! -f Makefile; then
    if test -f Makefile.orig; then
        AC_MSG_NOTICE([Copying Makefile.orig to Makefile])
        cp Makefile.orig Makefile
    else
        AC_MSG_WARN([Makefile.orig not found — you will need to provide a top-level Makefile])
    fi
fi

# ---------------------------------------------------------------------------
# Establish required symbolic links in src/
# All cd+modify sequences use subshells to prevent bare cd failures from
# executing destructive commands in the wrong directory.
# ---------------------------------------------------------------------------
AC_MSG_NOTICE([Establishing symbolic links in src/])

(cd src/include/IV-3_1 && rm -rf InterViews && ln -s ../InterViews) \
    || AC_MSG_ERROR([Failed to establish src/include/IV-3_1/InterViews symlink])

for f in listimpl.c math.c memory.c string.c; do
    if test ! -f src/IV-common/$f; then
        (cd src/IV-common && ln -s ../OS/$f) \
            || AC_MSG_ERROR([Failed to symlink $f in src/IV-common])
    fi
done
for f in regexp.c resource.c transformer.c; do
    if test ! -f src/IV-common/$f; then
        (cd src/IV-common && ln -s ../InterViews/$f) \
            || AC_MSG_ERROR([Failed to symlink $f in src/IV-common])
    fi
done
for f in textbuffer.c tform2_6.c; do
    if test ! -f src/IV-common/$f; then
        (cd src/IV-common && ln -s ../IV-2_6/$f) \
            || AC_MSG_ERROR([Failed to symlink $f in src/IV-common])
    fi
done

for f in component.c compview.c geomobjs.c globals.c iterator.c uarray.c uhash.c ulist.c umap.c; do
    base=`basename $f`
    if test ! -f src/Unidraw-common/$base; then
        (cd src/Unidraw-common && ln -s ../Unidraw/$base) \
            || AC_MSG_ERROR([Failed to symlink $base in src/Unidraw-common])
    fi
done

# ---------------------------------------------------------------------------
# Done
# ---------------------------------------------------------------------------
AC_MSG_NOTICE([])
AC_MSG_NOTICE([Configuration complete.])
AC_MSG_NOTICE([  CPU          = ${CPU}])
AC_MSG_NOTICE([  platform     = ${platform_part}])
AC_MSG_NOTICE([  X11 includes = ${x_includes}])
AC_MSG_NOTICE([  X11 libs     = ${x_libraries}])
AC_MSG_NOTICE([  ACE enabled  = ${ACE_ENABLED}])
AC_MSG_NOTICE([  prefix       = ${prefix}])
AC_MSG_NOTICE([])
AC_MSG_NOTICE([Now run: make])
AC_MSG_NOTICE([])
