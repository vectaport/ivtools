XCOMM!/bin/sh
XCOMM 
XCOMM ivfb  shell script for ivtools flipbook
XCOMM 

LDLIB_SPEC
PATH_SPEC
case "$#" in 
        0)       flipbook
                 ;;
        *)       flipbook "$*"
                 ;;
esac

