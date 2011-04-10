XCOMM!/bin/sh
XCOMM 
XCOMM ivds  shell script for ivtools drawserv
XCOMM 

LDLIB_SPEC
PATH_SPEC
case "$#" in 
        0)       drawserv
                 ;;
        *)       drawserv "$*"
                 ;;
esac

