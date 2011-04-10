XCOMM!/bin/sh
XCOMM 
XCOMM ivct  shell script for ivtools comterp
XCOMM 

LDLIB_SPEC
PATH_SPEC
case "$#" in 
        0)       comterp
                 ;;
        *)       comterp "$*"
                 ;;
esac

