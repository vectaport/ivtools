#!/bin/bash
# 
# cntsrclines [dir]
#
# $1 directory in which to recursively count lines of C and C++ source
#
case "$#" in
    0)    ;;
    *)    cd $1
          ;;
esac
linecount1=`tmpnam`
linecount2=`tmpnam`
find . -name \*.c -exec wc -l {} \; >$linecount1
find . -name \*.h -exec wc -l {} \; >>$linecount1
find . -name \*.ci -exec wc -l {} \; >>$linecount1
find . -name \*.cpp -exec wc -l {} \; >>$linecount1
find . -name \*.C -exec wc -l {} \; >>$linecount1
echo "0L +" >$linecount2
sed -e "s/.\/.*/+/" $linecount1 >>$linecount2
echo 0 >>$linecount2
cat $linecount2 | comterp
rm $linecount1 $linecount2


