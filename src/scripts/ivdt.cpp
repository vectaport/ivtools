XCOMM!/bin/sh
XCOMM 
XCOMM ivdt  shell script for ivtools drawtool
XCOMM 

LDLIB_SPEC
PATH_SPEC
case "$#" in 
        0)       drawtool
                 ;;
        *)       drawtool "$*"
                 ;;
esac

