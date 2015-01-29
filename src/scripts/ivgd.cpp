XCOMM!/bin/sh
XCOMM 
XCOMM ivgd  shell script for ivtools graphdraw
XCOMM 

LDLIB_SPEC
PATH_SPEC
case "$#" in 
        0)       graphdraw
                 ;;
        *)       graphdraw "$*"
                 ;;
esac

