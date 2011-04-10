XCOMM!/bin/sh
XCOMM 
XCOMM generate a Makefile from an Imakefile outside of the source tree
XCOMM 

usage="usage: $0 [-a]"
do_all=

case "$#" in
	0)	;;
	1)	case "$1" in
			-a)	do_all="yes"
				;;
			*)	echo "$usage" 1>&2; exit 1
				;;
		esac
		;;
	*)	echo "$usage" 1>&2; exit 1
		;;
esac

if [ -f Makefile ]; then 
	rm -f Makefile.bak
	mv Makefile Makefile.bak
fi

#ifdef USE_INSTALLED
case "$do_all" in
	yes)	set -x
		imake CONFIGDIRSPEC -DUseInstalled &&
		make Makefile MAKEMAKESPEC UseInstalled=1 &&
		make Makefiles MAKEMAKESPEC UseInstalled=1 &&
		make depend UseInstalled=1
		;;
	*)	set -x
		imake CONFIGDIRSPEC -DUseInstalled
		make Makefile MAKEMAKESPEC UseInstalled=1
		;;
esac
#else
case "$do_all" in
	yes)	set -x
		imake CONFIGDIRSPEC &&
		make Makefile MAKEMAKESPEC &&
		make Makefiles MAKEMAKESPEC &&
		make depend
		;;
	*)	set -x
		imake CONFIGDIRSPEC
		make Makefile MAKEMAKESPEC
		;;
esac
#endif
