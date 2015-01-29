XCOMM!/bin/sh
XCOMM 
XCOMM ivcd  shell script for ivtools comdraw
XCOMM 

LDLIB_SPEC
PATH_SPEC
case "$#" in 
        0)       comdraw
                 ;;
        *)       comdraw "$*"
                 ;;
esac

