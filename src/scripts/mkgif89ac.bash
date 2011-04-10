#!/bin/bash
# $1 scale
# $2 rotation
# $3 width of clipping rectangle 
# $4 height of clipping rectangle 
# $5 hundreds of a second between frames
# $6 postscript file
echo "** rendering ppmraw images with Ghostscript **"
cat $6 | gs -g$3x$4 -sDEVICE=ppmraw -sOutputFile=$6-%02d.ppm -dNOPAUSE -
echo "** converting images from ppmraw to gif **"
for x in $6-??.ppm
do
echo writing `dirname $6`/`basename $x .ppm`.gif
pnmscale -xscale $1 -yscale $1 $x | pnmrotate $2 | ppmquant 256 | ppmtogif >`dirname $6`/`basename $x .ppm`.gif
done
echo "** merging images into multi-frame gif89a **"
bash -c "gifmerge -l0 -$5 $6-??.gif >$6.gif"
echo "** loading multi-frame gif89a into xanim **"
xanim $6.gif &
echo "** removing intermediate ppmraw and gif images **"
rm $6-??.gif $6-??.ppm

