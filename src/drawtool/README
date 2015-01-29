NAME
     drawtool - idraw with extensions

SYNOPSIS
     drawtool -import n ['X-params'] [file]

DESCRIPTION

drawtool is an extended version of idraw (originally from the
InterViews 3.1).  Based on the ivtools OverlayUnidraw library, it adds
the following features:

	- double-buffered graphics drawing
	- 3d Motif-like menus and buttons
	- readable text save/restore
	- polygon intersection and clipping tools
	- graphical object attribute annotations

drawtool is derived from idraw, and implements the same set of
pulldown commands and pallette of toolbuttons.  Read the idraw man
page for details on that part of the drawtool user interace.  This
page lists only the extensions to idraw embodied in drawtool.

drawtool saves and restores documents in a readable text format.  This
allows for manual or programmatic creation of drawtool files.
idraw-format documents can still be imported with the "Import Graphic"
command, and exported with the Print command (both under the File
menu).

In addition to idraw format, the "Import Graphic" command supports the
importing of encapsulated drawtool files, and the following image file
formats: TIFF, X11 Bitmap, PBM, PGM, and PPM.  Compression via
compress (.Z) and gzip (.gz) is recognized and handled for the script
files themselves and the PBM, PGM, and PPM image formats.

There is also support for opening (via command line or dialog box) and
importing via the net (to the -import port) any arbitrary PostScript
file using the pstoedit filter, and any GIF, TIFF, or JPEG image,
using respectively the giftopnm, tifftopnm, or djpeg filters.

The "Export Clipboard" command writes the contents of the clipboard
out in idraw or drawtool format.

Commands have been added to the Edit menu for selecting by attributes
(on the graphic objects) and clipping one polygon against another.

The View menu has been expanded with zoom and pan commands, plus
commands to hide/show graphics and fix graphic sizes and locations.
In addition the viewer's panning and zooming can be chained to other
viewers, and the pointer device can be put into a continuous
"scribble" mode useful when drawing multilines or polygons.

The tools have been extended with a possible six new tools: Annotate,
Attribute, ClipRect, ClipPoly, ConvexHull, and GraphicLoc.  Annotate
brings up a text editor in which graphic-specific text can be entered
and/or edited.  Attribute brings up a dialog box to create/edit
property lists for the selected graphic.  ClipRect and ClipPoly clip
polygons, rectangles, lines, and multilines if drawtool has been built
with the clippoly library (otherwise just lines and
rectangles). ConvexHull is a convex hull drawing tool available if
qhull can be found in your path.  GraphicLoc displays the coordinates
of a mouse-click relative to the graphic that has been clicked on.
This is relative to the original screen coordinates of the graphic
when created (or the origin of the raster if an image).  Subsequent
rotations and transformations can give this a non-obvious value.

OPTIONS

"-import n" specifies the port number to run the import service on.  The
import service accepts connections over the net and reads drawtool
or idraw documents and pbmplus image formats (PBM/PGM/PPM), plus JPEG, GIF,
TIFF, and the non-raster portions of arbitrary PostScript if appropriate
filters are available (djpeg, giftopnm, tifftopnm, pstoedit).

"-color5" selects a colormap with 5 values per color or 125 entries (5
cubed).

"-nocolor6" overrides the selection of a colormap with 6 values per
color or 216 entries (6 cubed).

"-gray5" selects a colormap with 5 bits of gray-level intensity (32
gray-levels) arranged to maximize rubberband visibility.

"-gray6" selects a colormap with 6 bits of gray-level intensity (64
gray-levels)  arranged to maximize rubberband visibility.

"-gray7" selects a colormap with 7 bits of gray-level intensity (128
gray-levels) arranged to maximize rubberband visibility.

"-dithermap" causes imported JPEG rasters to be mapped to the standard
colormap (the default without -nocolor6) and dithered to avoid color
banding.

"-opaque_off" (or "-opoff") disables the opaque (visible)
interactive transformations of graphics.

"-pagecols" (or "-ncols") sets the horizontal page extent in pixels.

"-pagerows" (or "-nrows") sets the vertical page extent in pixels.

"-panner_align tl|tc|tr|cl|c|cr|cl|bl|br|l|r|t|b|hc|vc" (or "-pal
...") selects the alignment of the panner display.

"-panner_off" (or "-poff") disables the panner display.

"-slider_off" (or "-soff") disables the slider display.

"-zoomer_off" (or "-zoff") disables the zoomer display.

"-ptrloc" enables display of the current pointer location.

"-scribble_pointer (or "-scrpt") enables continuous line drawing.

"-toolbarloc r|l" (or "-tbl ...") selects the toolbar location.

Also see "-help" and the idraw man page for more options.

SEE ALSO  
	idraw

WEB PAGE
	 http://www.ivtools.org/ivtools/drawtool.html
