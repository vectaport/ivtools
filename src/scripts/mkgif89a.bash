#!/bin/bash
# $1 scale
# $2 rotation
# $3 postscript file
echo "** rendering ppmraw images with Ghostscript **"
cat $3 | gs -g$4x$5 -sDEVICE=ppmraw -sOutputFile=$3-%02d.ppm -dNOPAUSE -
echo "** converting images from ppmraw to gif **"
for x in $3-??.ppm
do
echo writing `dirname $3`/`basename $x .ppm`.gif
pnmscale -xscale $1 -yscale $1 $x | pnmrotate $2 | ppmquant 256 | ppmtogif >`dirname $3`/`basename $x .ppm`.gif
done
echo "** merging images into multi-frame gif89a **"
bash -c "gifmerge -l0 -25 $3-??.gif >$3.gif"
echo "** loading multi-frame gif89a into xanim **"
xanim $3.gif &
echo "** removing intermediate ppmraw and gif images **"
rm $3-??.gif $3-??.ppm

