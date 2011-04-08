dnl ICE_CXX_INCLUDE_DIR
dnl -------------------
dnl
dnl Set output variable CXX_INCLUDE_DIR to the name of a directory
dnl where the C++ compiler looks for C++ include files.
dnl
AC_DEFUN(ICE_CXX_INCLUDE_DIR,
[
AC_MSG_CHECKING(for directory to install c++ include files)
AC_CACHE_VAL(ice_cv_cxx_include_dir,
[
cat > conftest.cc << EOF
#include <iostream.h>
EOF
changequote(,)dnl
$CXXCPP $DEFS conftest.cc > conftest.ii 2>&5
if test $? != 0; then
AC_MSG_ERROR(${CXXCPP} could not find iostream.h)
else
ice_file=`grep '^# 1' conftest.ii | grep iostream.h | \
        head -1 | sed 's/^.*"\(.*\)".*$/\1/'`
ice_cv_cxx_include_dir=`echo $ice_file | sed 's%iostream.h%%'`
fi
if test "$ice_cv_cxx_include_dir" = ""; then
ice_cv_cxx_include_dir="$prefix/include"
for pfx in "$prefix" "$exec_prefix"; do
for dir in "$pfx/lib/g++-include" "$pfx/include/CC" \
    "$pfx/include" /usr/include /usr/local/include/g++-v3 /usr/local/include/g++-3 /usr/local/include/g++-2 ; do
if test -d "$dir"; then
ice_cv_cxx_include_dir="$dir"
break
fi
done
done
fi
changequote([,])dnl
rm -f conftest.cc conftest.ii
])
CXX_INCLUDE_DIR=$ice_cv_cxx_include_dir
AC_MSG_RESULT(${CXX_INCLUDE_DIR})
AC_SUBST(CXX_INCLUDE_DIR)
])dnl

AC_DEFUN(AC_TYPE_SOCKLEN_T,
[AC_CACHE_CHECK([for socklen_t], ac_cv_type_socklen_t,
[
  AC_TRY_COMPILE(
  [#include <sys/types.h>
#include <sys/socket.h>],
  [socklen_t len = 42; return len;],
  ac_cv_type_socklen_t=YES,
  ac_cv_type_socklen_t=NO)
])
  if test $ac_cv_type_socklen_t != yes; then
    AC_DEFINE(socklen_t, int)
  fi
])

