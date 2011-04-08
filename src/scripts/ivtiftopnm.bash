#!/bin/bash
#
# ivtiftopnm [file]
#
# bash script to wrap tifftopnm which can't handle stdin
#
# Parameters:
#     $1 optional tiff image filename
#
case "$#" in 
        0)      tempfile=`tmpnam`
		cat >$tempfile
		tifftopnm $tempfile
		rm $tempfile
                ;;
        *)      tifftopnm $1
                ;;
esac
