## To Do: Document all this stuff.

## Miscellaneous Notes

## Cannot put the main Makefile.am in src/Makefile.am because targets
## like drawtool clash with pre-existing directories of the same name.
## Maybe there is a way to push these generated executables in a
## subdirectory?

## Various source files linked into the same target library sometimes
## require different options, in some cases in such a way that these
## options cannot be combined into a single set of options that works
## for all the files.  This is addressed by breaking up such libraries
## into a few "convenience libtool libraries", each made of files
## compiled with identical options, and then combined to form the
## final library.

## Pending Issues:

## Do
##   TIFF/tiff.h TIFF/tiffio.h TIFF/format.h
## really belong in the installed include files, even when using an
## external tiff library?

## Snatch the source code for refman.PS from the ivtools-doc CVS repo.

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES =

lib_LTLIBRARIES = libComTerp.la libIV-common.la libTime.la		\
libIVGlyph.la libDrawServ.la libUnidraw-common.la libComUnidraw.la	\
libComGlyph.la libIV.la libUnidraw.la libAttrGlyph.la libComUtil.la	\
libTopoFace.la libUniIdraw.la libGraphUnidraw.la libFrameUnidraw.la	\
libOverlayUnidraw.la libAttribute.la

if HAVE_LIBACE
lib_LTLIBRARIES += libAceDispatch.la
endif

LIBVER = -version-info $(LIB_IV_VER)

libComTerp_la_LDFLAGS		= $(LIBVER)
libIV_common_la_LDFLAGS		= $(LIBVER)
libTime_la_LDFLAGS		= $(LIBVER) -lIV $(LIBS)
libIVGlyph_la_LDFLAGS		= $(LIBVER) -lIV $(LIBS)
libDrawServ_la_LDFLAGS		= $(LIBVER) -lFrameUnidraw -lGraphUnidraw -lUnidraw -lIV $(LIBS)
libUnidraw_common_la_LDFLAGS	= $(LIBVER) -lIV
libComUnidraw_la_LDFLAGS	= $(LIBVER) -lUnidraw -lIV $(LIBS)
libComGlyph_la_LDFLAGS		= $(LIBVER) -lIV $(LIBS)
libIV_la_LDFLAGS		= $(LIBVER)
libUnidraw_la_LDFLAGS		= $(LIBVER) -lIV
libAttrGlyph_la_LDFLAGS		= $(LIBVER) -lIV $(LIBS)
libComUtil_la_LDFLAGS		= $(LIBVER)
libTopoFace_la_LDFLAGS		= $(LIBVER) -lIV
libUniIdraw_la_LDFLAGS		= $(LIBVER) -lUnidraw -lIV $(LIBS)
libGraphUnidraw_la_LDFLAGS	= $(LIBVER) -lComUnidraw -lUnidraw -lIV $(LIBS)
libFrameUnidraw_la_LDFLAGS	= $(LIBVER) -lComUnidraw -lUnidraw -lIV $(LIBS)
libOverlayUnidraw_la_LDFLAGS	= $(LIBVER) -lUnidraw -lIV $(LIBS)
libAceDispatch_la_LDFLAGS	= $(LIBVER) -lIV -lACE
libAttribute_la_LDFLAGS		= $(LIBVER) -lIV

IVSTD_INCLUDES = -Isrc -Isrc/include -Isrc/include/ivstd
IV2_6_INCLUDES = -Isrc -Isrc/include/IV-2_6 -Isrc/include -Isrc/include/ivstd

### libComTerp

libComTerp_la_SOURCES =

libComTerp_la_LIBADD = libComTerpPART1.la libComTerpPART2.la libComTerpPART3.la

noinst_LTLIBRARIES += $(libComTerp_la_LIBADD)

libComTerpPART1_la_SOURCES = src/ComTerp/comhandler.cc	\
src/ComTerp/comterpserv.cc src/ComTerp/ctrlfunc.cc

libComTerpPART1_la_CPPFLAGS = \
 $(ACE_INLINE_CPPFLAGS) \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

libComTerpPART2_la_SOURCES = src/ComTerp/assignfunc.cc			\
src/ComTerp/bitfunc.cc src/ComTerp/boolfunc.cc				\
src/ComTerp/bquotefunc.cc src/ComTerp/charfunc.cc			\
src/ComTerp/comfunc.cc src/ComTerp/comvalue.cc src/ComTerp/condfunc.cc	\
src/ComTerp/debugfunc.cc src/ComTerp/dotfunc.cc				\
src/ComTerp/helpfunc.cc src/ComTerp/iofunc.cc src/ComTerp/listfunc.cc	\
src/ComTerp/mathfunc.cc src/ComTerp/numfunc.cc src/ComTerp/parser.cc	\
src/ComTerp/postfunc.cc src/ComTerp/randfunc.cc src/ComTerp/scanner.cc	\
src/ComTerp/statfunc.cc src/ComTerp/strmfunc.cc				\
src/ComTerp/symbolfunc.cc src/ComTerp/typefunc.cc			\
src/ComTerp/xformfunc.cc

libComTerpPART2_la_CPPFLAGS = \
 $(ACE_CPPFLAGS) -Dcplusplus_2_1 $(IVSTD_INCLUDES)

libComTerpPART3_la_SOURCES = src/ComTerp/comterp.cc

libComTerpPART3_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include \
 $(ACE_CPPFLAGS) -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libIV_common

libIV_common_la_SOURCES = src/OS/listimpl.cc src/OS/math.cc		\
src/OS/memory.cc src/InterViews/regexp.cc src/InterViews/resource.cc	\
src/OS/string.cc src/IV-2_6/textbuffer.cc src/IV-2_6/tform2_6.cc	\
src/InterViews/transformer.cc

libIV_common_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libTime

libTime_la_SOURCES = src/Time/Date.cc src/Time/obstime.cc	\
src/Time/Time.cc src/Time/timeglyph.cc

libTime_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libIVGlyph

libIVGlyph_la_SOURCES = src/IVGlyph/bdfltform.cc			\
src/IVGlyph/bdtable.cc src/IVGlyph/bdvalue.cc src/IVGlyph/boolform.cc	\
src/IVGlyph/charfield.cc src/IVGlyph/dragedit.cc			\
src/IVGlyph/enumform.cc src/IVGlyph/exportchooser.cc			\
src/IVGlyph/fieldedit.cc src/IVGlyph/figure.cc src/IVGlyph/gdialogs.cc	\
src/IVGlyph/globals.cc src/IVGlyph/idraw.cc				\
src/IVGlyph/importchooser.cc src/IVGlyph/namestate.cc			\
src/IVGlyph/observables.cc src/IVGlyph/odialogs.cc			\
src/IVGlyph/ofilechooser.cc src/IVGlyph/printchooser.cc			\
src/IVGlyph/saveaschooser.cc src/IVGlyph/scrollable.cc			\
src/IVGlyph/strchooser.cc src/IVGlyph/stredit.cc			\
src/IVGlyph/strlist.cc src/IVGlyph/textbuff.cc src/IVGlyph/textedit.cc	\
src/IVGlyph/texteditor.cc src/IVGlyph/textform.cc			\
src/IVGlyph/textview.cc src/IVGlyph/textwindow.cc			\
src/IVGlyph/toolbutton.cc src/IVGlyph/valuator.cc

libIVGlyph_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libDrawServ

libDrawServ_la_SOURCES =

libDrawServ_la_LIBADD = libDrawServPART1.la libDrawServPART2.la libDrawServPART3.la

noinst_LTLIBRARIES += $(libDrawServ_la_LIBADD)

libDrawServPART1_la_SOURCES = src/DrawServ/ackback-handler.cc	\
src/DrawServ/drawfunc.cc src/DrawServ/drawlink.cc		\
src/DrawServ/drawserv-handler.cc

libDrawServPART1_la_CPPFLAGS = \
 $(ACE_INLINE_CPPFLAGS) -Div2_6_compatible \
 -Dcplusplus_2_1 $(IV2_6_INCLUDES)

libDrawServPART2_la_SOURCES = src/DrawServ/drawcatalog.cc	\
src/DrawServ/drawcmds.cc src/DrawServ/drawcomps.cc		\
src/DrawServ/drawcreator.cc src/DrawServ/draweditor.cc		\
src/DrawServ/drawlinkcomp.cc src/DrawServ/drawlinklist.cc	\
src/DrawServ/drawserv.cc src/DrawServ/drawviews.cc		\
src/DrawServ/grid.cc src/DrawServ/gridlist.cc			\
src/DrawServ/linkselection.cc src/DrawServ/sid.cc

libDrawServPART2_la_CPPFLAGS = \
 -Div2_6_compatible $(ACE_CPPFLAGS) \
 -Dcplusplus_2_1 $(IV2_6_INCLUDES)

libDrawServPART3_la_SOURCES = src/DrawServ/drawimport.cc	\
src/DrawServ/drawkit.cc src/DrawServ/rcdialog.cc

libDrawServPART3_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include -Div2_6_compatible $(ACE_CPPFLAGS) \
 -Dcplusplus_2_1 $(IV2_6_INCLUDES)

### libUnidraw

libUnidraw_common_la_SOURCES = src/Unidraw/component.cc			\
src/Unidraw/compview.cc src/Unidraw/geomobjs.cc				\
src/Unidraw/iterator.cc src/Unidraw/uarray.cc src/Unidraw/uhash.cc	\
src/Unidraw/ulist.cc src/Unidraw/umap.cc

libUnidraw_common_la_CPPFLAGS = \
 -DUnidrawCommon \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libComUnidraw

libComUnidraw_la_SOURCES =
libComUnidraw_la_LIBADD = libComUnidrawPART1.la libComUnidrawPART2.la

noinst_LTLIBRARIES += $(libComUnidraw_la_LIBADD)

libComUnidrawPART1_la_SOURCES = src/ComUnidraw/comeditor.cc		\
src/ComUnidraw/comterp-iohandler.cc src/ComUnidraw/dialogfunc.cc	\
src/ComUnidraw/grdotfunc.cc src/ComUnidraw/grfunc.cc			\
src/ComUnidraw/grlistfunc.cc src/ComUnidraw/groupfunc.cc		\
src/ComUnidraw/grstatfunc.cc src/ComUnidraw/highlightfunc.cc		\
src/ComUnidraw/nfunc.cc src/ComUnidraw/pixelfunc.cc			\
src/ComUnidraw/plotfunc.cc src/ComUnidraw/unifunc.cc

libComUnidrawPART1_la_CPPFLAGS = \
 -Dcplusplus_2_1 -Div2_6_compatible $(ACE_CPPFLAGS) \
 $(IV2_6_INCLUDES)

libComUnidrawPART2_la_SOURCES = src/ComUnidraw/comterp-acehandler.cc

libComUnidrawPART2_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include \
 -Dcplusplus_2_1 -Div2_6_compatible $(ACE_INLINE_CPPFLAGS) \
 $(IV2_6_INCLUDES)

### libComGlyph

libComGlyph_la_SOURCES = src/ComGlyph/attrdialog.cc	\
src/ComGlyph/comtextedit.cc src/ComGlyph/comtextview.cc	\
src/ComGlyph/terpdialog.cc

libComGlyph_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libIV

libIV_la_SOURCES =

libIV_la_LIBADD = libIVPART1.la libIVPART2.la libIVPART3.la libIVPART4.la libIVPART5.la

noinst_LTLIBRARIES += $(libIV_la_LIBADD)

libIVPART1_la_SOURCES = src/InterViews/session.cc

libIVPART1_la_CPPFLAGS = \
 -DABSLIBALLDIR='"$(pkglibdir)"' \
 -DRELLIBALLDIR='"lib/$(PACKAGE)"' \
 -DX_LIBDIR='"$(libdir)"' \
 -Dcplusplus_2_1 -Isrc/IV $(IVSTD_INCLUDES)

libIVPART2_la_SOURCES = src/InterViews/psfont.cc

libIVPART2_la_CPPFLAGS = \
 -Dps_metrics_dir='"$(PSFontDir)"' \
 -Dcplusplus_2_1 -Isrc/IV $(IVSTD_INCLUDES)

libIVPART3_la_SOURCES = src/InterViews/dialogs.cc src/InterViews/kit.cc

libIVPART3_la_CPPFLAGS = \
 -Dsgi_motif_kit -Dmotif_kit -Ddefault_kit=SMFKit \
 -Dcplusplus_2_1 -Isrc/IV $(IVSTD_INCLUDES)

libIVPART4_la_SOURCES = src/Dispatch/dispatcher.cc			\
src/Dispatch/iohandler.cc src/IV-2_6/adjuster2_6.cc			\
src/IV-2_6/banner.cc src/IV-2_6/border2_6.cc src/IV-2_6/box2_6.cc	\
src/IV-2_6/button2_6.cc src/IV-2_6/compeditor.cc src/IV-2_6/control.cc	\
src/IV-2_6/deck2_6.cc src/IV-2_6/dialog2_6.cc				\
src/IV-2_6/filebrowser.cc src/IV-2_6/filechooser.cc			\
src/IV-2_6/frame.cc src/IV-2_6/glue2_6.cc src/IV-2_6/interactor.cc	\
src/IV-2_6/matcheditor.cc src/IV-2_6/menu2_6.cc src/IV-2_6/message.cc	\
src/IV-2_6/painter.cc src/IV-2_6/panner2_6.cc				\
src/IV-2_6/perspective.cc src/IV-2_6/rubband.cc src/IV-2_6/rubcurve.cc	\
src/IV-2_6/rubgroup.cc src/IV-2_6/rubline.cc src/IV-2_6/rubrect.cc	\
src/IV-2_6/rubverts.cc src/IV-2_6/scene.cc src/IV-2_6/scrollbar2_6.cc	\
src/IV-2_6/scroller2_6.cc src/IV-2_6/sensor.cc src/IV-2_6/shape.cc	\
src/IV-2_6/strbrowser.cc src/IV-2_6/strchooser.cc			\
src/IV-2_6/streditor.cc src/IV-2_6/subject.cc src/IV-2_6/textbuffer.cc	\
src/IV-2_6/textdisplay.cc src/IV-2_6/texteditor.cc			\
src/IV-2_6/tform2_6.cc src/IV-2_6/tray.cc src/IV-2_6/viewport.cc	\
src/IV-2_6/world.cc src/IV-2_6/xbitmap2_6.cc src/IV-2_6/xevent2_6.cc	\
src/IV-2_6/xinter.cc src/IV-2_6/xpainter.cc src/IV-2_6/xpattern.cc	\
src/IV-X11/xbitmap.cc src/IV-X11/xbrush.cc src/IV-X11/xcanvas.cc	\
src/IV-X11/xcolor.cc src/IV-X11/xcursor.cc src/IV-X11/xdrag.cc		\
src/IV-X11/xevent.cc src/IV-X11/xfont.cc src/IV-X11/xraster.cc		\
src/IV-X11/xreqerr.cc src/IV-X11/xselection.cc src/IV-X11/xwindow.cc	\
src/InterViews/action.cc src/InterViews/adjust.cc			\
src/InterViews/aggr.cc src/InterViews/align.cc				\
src/InterViews/alloctbl.cc src/InterViews/arrcomp.cc			\
src/InterViews/background.cc src/InterViews/bevel.cc			\
src/InterViews/border.cc src/InterViews/box.cc				\
src/InterViews/browser.cc src/InterViews/button.cc			\
src/InterViews/character.cc src/InterViews/composition.cc		\
src/InterViews/compositor.cc src/InterViews/debug.cc			\
src/InterViews/deck.cc src/InterViews/fbrowser.cc			\
src/InterViews/fchooser.cc src/InterViews/field.cc			\
src/InterViews/geometry.cc src/InterViews/glyph.cc			\
src/InterViews/group.cc src/InterViews/handler.cc			\
src/InterViews/hit.cc src/InterViews/image.cc src/InterViews/input.cc	\
src/InterViews/label.cc src/InterViews/layout.cc			\
src/InterViews/lrmarker.cc src/InterViews/menu.cc			\
src/InterViews/mf_dialogs.cc src/InterViews/mf_kit.cc			\
src/InterViews/mono_kit.cc src/InterViews/monoglyph.cc			\
src/InterViews/observe.cc src/InterViews/ol_dialogs.cc			\
src/InterViews/ol_kit.cc src/InterViews/page.cc				\
src/InterViews/patch.cc src/InterViews/place.cc				\
src/InterViews/polyglyph.cc src/InterViews/printer.cc			\
src/InterViews/regexp.cc src/InterViews/resource.cc			\
src/InterViews/rule.cc src/InterViews/scrbox.cc				\
src/InterViews/shadow.cc src/InterViews/simpcomp.cc			\
src/InterViews/slider.cc src/InterViews/smf_kit.cc			\
src/InterViews/stencil.cc src/InterViews/stepper.cc			\
src/InterViews/style.cc src/InterViews/superpose.cc			\
src/InterViews/target.cc src/InterViews/telltale.cc			\
src/InterViews/texcomp.cc src/InterViews/tformsetter.cc			\
src/InterViews/tile.cc src/InterViews/transformer.cc			\
src/InterViews/xymarker.cc src/OS/directory.cc src/OS/file.cc		\
src/OS/host.cc src/OS/listimpl.cc src/OS/math.cc src/OS/memory.cc	\
src/OS/string.cc src/OS/ustring.cc

libIVPART4_la_CPPFLAGS = \
 -Dcplusplus_2_1 -Isrc/IV $(IVSTD_INCLUDES)

libIVPART5_la_SOURCES = src/InterViews/tiff.cc

libIVPART5_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(TIFF_LOC_CPPFLAG) -Isrc/IV $(IVSTD_INCLUDES)

### libUnidraw

libUnidraw_la_SOURCES = src/Unidraw/align.cc src/Unidraw/brushcmd.cc	\
src/Unidraw/catalog.cc src/Unidraw/catcmds.cc src/Unidraw/cglue.cc	\
src/Unidraw/clipboard.cc src/Unidraw/colorcmd.cc			\
src/Unidraw/command.cc src/Unidraw/component.cc				\
src/Unidraw/compview.cc src/Unidraw/connect.cc				\
src/Unidraw/connector.cc src/Unidraw/creator.cc src/Unidraw/csolver.cc	\
src/Unidraw/ctrlinfo.cc src/Unidraw/damage.cc src/Unidraw/data.cc	\
src/Unidraw/datas.cc src/Unidraw/dialogs.cc src/Unidraw/edit.cc		\
src/Unidraw/editor.cc src/Unidraw/editorinfo.cc src/Unidraw/ellipse.cc	\
src/Unidraw/ellipses.cc src/Unidraw/externview.cc src/Unidraw/font.cc	\
src/Unidraw/geomobjs.cc src/Unidraw/globals.cc src/Unidraw/graphic.cc	\
src/Unidraw/grblock.cc src/Unidraw/grcomp.cc src/Unidraw/grcomptool.cc	\
src/Unidraw/grid.cc src/Unidraw/grview.cc src/Unidraw/gvupdater.cc	\
src/Unidraw/import.cc src/Unidraw/iterator.cc src/Unidraw/keymap.cc	\
src/Unidraw/kybd.cc src/Unidraw/line.cc src/Unidraw/lines.cc		\
src/Unidraw/link.cc src/Unidraw/macro.cc src/Unidraw/magnify.cc		\
src/Unidraw/manip.cc src/Unidraw/manips.cc src/Unidraw/move.cc		\
src/Unidraw/nop.cc src/Unidraw/pad.cc src/Unidraw/patcmd.cc		\
src/Unidraw/path.cc src/Unidraw/picture.cc src/Unidraw/pin.cc		\
src/Unidraw/polygon.cc src/Unidraw/polygons.cc src/Unidraw/pspaint.cc	\
src/Unidraw/psview.cc src/Unidraw/rastercomp.cc				\
src/Unidraw/rasterrect.cc src/Unidraw/rect.cc src/Unidraw/reshape.cc	\
src/Unidraw/rotate.cc src/Unidraw/scale.cc src/Unidraw/select.cc	\
src/Unidraw/selection.cc src/Unidraw/slot.cc src/Unidraw/spline.cc	\
src/Unidraw/splines.cc src/Unidraw/statevar.cc				\
src/Unidraw/statevars.cc src/Unidraw/stateview.cc			\
src/Unidraw/stateviews.cc src/Unidraw/stencilcomp.cc			\
src/Unidraw/stretch.cc src/Unidraw/struct.cc src/Unidraw/text.cc	\
src/Unidraw/tool.cc src/Unidraw/transfn.cc src/Unidraw/transfns.cc	\
src/Unidraw/transforms.cc src/Unidraw/uarray.cc src/Unidraw/uctrl.cc	\
src/Unidraw/uctrls.cc src/Unidraw/uhash.cc src/Unidraw/ulabel.cc	\
src/Unidraw/ulist.cc src/Unidraw/umap.cc src/Unidraw/unidraw.cc		\
src/Unidraw/upage.cc src/Unidraw/ustencil.cc src/Unidraw/vertices.cc	\
src/Unidraw/verts.cc src/Unidraw/viewcmds.cc src/Unidraw/viewer.cc

libUnidraw_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libAttrGlyph

libAttrGlyph_la_SOURCES = src/AttrGlyph/attredit.cc

libAttrGlyph_la_CPPFLAGS = \
 -DLibStdCPlusPlus2=0 -DLibStdCPlusPlus3=1 \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libComUtil

libComUtil_la_SOURCES =

libComUtil_la_LIBADD = libComUtilPART1.la libComUtilPART2.la

noinst_LTLIBRARIES += $(libComUtil_la_LIBADD)

libComUtilPART1_la_SOURCES = src/ComUtil/atox.c src/ComUtil/comerr.c	\
src/ComUtil/dmm.c src/ComUtil/errfile.c src/ComUtil/funcptrs.c		\
src/ComUtil/_lexscan.c src/ComUtil/mblock.c src/ComUtil/optable.c	\
src/ComUtil/_parser.c src/ComUtil/_scanner.c src/ComUtil/symbols.c	\
src/ComUtil/txtutil.c src/ComUtil/types.c src/ComUtil/util.c		\
src/ComUtil/xdll.c

libComUtilPART1_la_CPPFLAGS = \
 -D_POSIX_C_SOURCE $(IVSTD_INCLUDES)

libComUtilPART2_la_SOURCES = src/ComUtil/errsys.c

# was hardwired:
#  -DABSLIBALLDIR=\"/usr/lib/ivtools/\"
#  -DRELLIBALLDIR=\"lib/ivtools/\"

libComUtilPART2_la_CPPFLAGS = \
 -DABSLIBALLDIR=\"$(pkglibdir)/\" \
 -DRELLIBALLDIR=\"lib/$(PACKAGE)/\" \
 -D_POSIX_C_SOURCE $(IVSTD_INCLUDES)

# NOTE: inconsistency in whether ABSLIBALLDIR has a trailing slash

### libTopoFace

libTopoFace_la_SOURCES = src/TopoFace/fgeomobjs.cc	\
src/TopoFace/topoedge.cc src/TopoFace/topoedgelist.cc	\
src/TopoFace/topoelt.cc src/TopoFace/topoface.cc	\
src/TopoFace/toponode.cc

libTopoFace_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libUniIdraw

libUniIdraw_la_SOURCES = src/UniIdraw/idarrow.cc			\
src/UniIdraw/idarrowhead.cc src/UniIdraw/idarrows.cc			\
src/UniIdraw/idcatalog.cc src/UniIdraw/idcmds.cc			\
src/UniIdraw/idcomp.cc src/UniIdraw/idcreator.cc			\
src/UniIdraw/iddialogs.cc src/UniIdraw/ided.cc src/UniIdraw/idkybd.cc	\
src/UniIdraw/idvars.cc

libUniIdraw_la_CPPFLAGS = \
 -Div2_6_compatible \
 -Dcplusplus_2_1 $(IV2_6_INCLUDES)

### libGraphUnidraw

libGraphUnidraw_la_SOURCES =
libGraphUnidraw_la_LIBADD = libGraphUnidrawPART1.la libGraphUnidrawPART2.la
noinst_LTLIBRARIES += $(libGraphUnidraw_la_LIBADD)

libGraphUnidrawPART1_la_SOURCES = src/GraphUnidraw/edgecomp.cc	\
src/GraphUnidraw/graphcatalog.cc src/GraphUnidraw/graphcmds.cc	\
src/GraphUnidraw/graphcomp.cc src/GraphUnidraw/graphcreator.cc	\
src/GraphUnidraw/graphdata.cc src/GraphUnidraw/grapheditor.cc	\
src/GraphUnidraw/graphtools.cc src/GraphUnidraw/nodecomp.cc

libGraphUnidrawPART1_la_CPPFLAGS = \
 -Div2_6_compatible -Dcplusplus_2_1 $(IV2_6_INCLUDES)

libGraphUnidrawPART2_la_SOURCES = src/GraphUnidraw/graphdialog.cc	\
src/GraphUnidraw/graphimport.cc src/GraphUnidraw/graphkit.cc

libGraphUnidrawPART2_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include \
 -Div2_6_compatible -Dcplusplus_2_1 $(IV2_6_INCLUDES)

# libFrameUnidraw

libFrameUnidraw_la_SOURCES =

libFrameUnidraw_la_LIBADD = libFrameUnidrawPART1.la libFrameUnidrawPART2.la

noinst_LTLIBRARIES += $(libFrameUnidraw_la_LIBADD)

libFrameUnidrawPART1_la_SOURCES = src/FrameUnidraw/framecatalog.cc	\
src/FrameUnidraw/framecmds.cc src/FrameUnidraw/framecomps.cc		\
src/FrameUnidraw/framecreator.cc src/FrameUnidraw/frameeditor.cc	\
src/FrameUnidraw/framefile.cc src/FrameUnidraw/framefunc.cc		\
src/FrameUnidraw/frameps.cc src/FrameUnidraw/framescripts.cc		\
src/FrameUnidraw/frameviewer.cc src/FrameUnidraw/frameviews.cc

libFrameUnidrawPART1_la_CPPFLAGS = \
 -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)

libFrameUnidrawPART2_la_SOURCES = src/FrameUnidraw/frameimport.cc	\
src/FrameUnidraw/framekit.cc src/FrameUnidraw/framestates.cc

libFrameUnidrawPART2_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include \
 -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)

### libOverlayUnidraw

# in directory but not included in build:
#  src/OverlayUnidraw/cliplinepoly.cc
#  src/OverlayUnidraw/clippoly.cc
#  src/OverlayUnidraw/ovclip.cc

libOverlayUnidraw_la_SOURCES =

libOverlayUnidraw_la_LIBADD = libOverlayUnidrawPART1.la libOverlayUnidrawPART2.la libOverlayUnidrawPART3.la

noinst_LTLIBRARIES += $(libOverlayUnidraw_la_LIBADD)

libOverlayUnidrawPART1_la_SOURCES = src/OverlayUnidraw/attrtool.cc	\
src/OverlayUnidraw/clipline.cc src/OverlayUnidraw/grayraster.cc		\
src/OverlayUnidraw/indexmixins.cc src/OverlayUnidraw/leafwalker.cc	\
src/OverlayUnidraw/ovadjuster.cc src/OverlayUnidraw/ovarrow.cc		\
src/OverlayUnidraw/ovcatalog.cc src/OverlayUnidraw/ovcomps.cc		\
src/OverlayUnidraw/ovcreator.cc src/OverlayUnidraw/ovdamage.cc		\
src/OverlayUnidraw/ovdialog.cc src/OverlayUnidraw/ovdoer.cc		\
src/OverlayUnidraw/oved.cc src/OverlayUnidraw/ovellipse.cc		\
src/OverlayUnidraw/ovfile.cc src/OverlayUnidraw/ovhull.cc		\
src/OverlayUnidraw/ovline.cc src/OverlayUnidraw/ovmanips.cc		\
src/OverlayUnidraw/ovpage.cc src/OverlayUnidraw/ovpainter.cc		\
src/OverlayUnidraw/ovpanner.cc src/OverlayUnidraw/ovpolygon.cc		\
src/OverlayUnidraw/ovprecise.cc src/OverlayUnidraw/ovpspict.cc		\
src/OverlayUnidraw/ovpsview.cc src/OverlayUnidraw/ovraster.cc		\
src/OverlayUnidraw/ovrect.cc src/OverlayUnidraw/ovrestimage.cc		\
src/OverlayUnidraw/ovselect.cc src/OverlayUnidraw/ovselection.cc	\
src/OverlayUnidraw/ovspline.cc src/OverlayUnidraw/ovstates.cc		\
src/OverlayUnidraw/ovstencil.cc src/OverlayUnidraw/ovtext.cc		\
src/OverlayUnidraw/ovunidraw.cc src/OverlayUnidraw/ovvars.cc		\
src/OverlayUnidraw/ovvertices.cc src/OverlayUnidraw/ovviewer.cc		\
src/OverlayUnidraw/ovviews.cc src/OverlayUnidraw/ptinpoly.cc		\
src/OverlayUnidraw/rastercmds.cc src/OverlayUnidraw/scriptview.cc	\
src/OverlayUnidraw/textfile.cc src/OverlayUnidraw/annotate.cc

libOverlayUnidrawPART1_la_CPPFLAGS = \
 -Dcplusplus_2_1 -Div2_6_compatible $(ACE_CPPFLAGS) \
 $(IV2_6_INCLUDES)

libOverlayUnidrawPART2_la_SOURCES = src/OverlayUnidraw/grloctool.cc	\
src/OverlayUnidraw/ovabout.cc src/OverlayUnidraw/ovcamcmds.cc		\
src/OverlayUnidraw/ovchainview.cc src/OverlayUnidraw/ovcmds.cc		\
src/OverlayUnidraw/ovctrl.cc src/OverlayUnidraw/ovexport.cc		\
src/OverlayUnidraw/ovfixview.cc src/OverlayUnidraw/ovgdialog.cc		\
src/OverlayUnidraw/ovimport.cc src/OverlayUnidraw/ovipcmds.cc		\
src/OverlayUnidraw/ovkit.cc src/OverlayUnidraw/ovprint.cc		\
src/OverlayUnidraw/ovshowhide.cc src/OverlayUnidraw/setattrbyexpr.cc	\
src/OverlayUnidraw/slctbyattr.cc

libOverlayUnidrawPART2_la_CPPFLAGS = \
 -Div2_6_incompatible -Div2_6_compatible $(ACE_CPPFLAGS) \
 -Dcplusplus_2_1 -Isrc/OverlayUnidraw -Isrc/include $(IV2_6_INCLUDES)

libOverlayUnidrawPART3_la_SOURCES = src/OverlayUnidraw/aceimport.cc

libOverlayUnidrawPART3_la_CPPFLAGS = \
 -Div2_6_incompatible -Isrc/include $(ACE_INLINE_CPPFLAGS) -Div2_6_compatible \
 -Dcplusplus_2_1 $(IV2_6_INCLUDES)

### libAceDispatch

libAceDispatch_la_SOURCES = src/AceDispatch/ace_dispatcher.cc	\
src/AceDispatch/ace_iohandler.cc

libAceDispatch_la_CPPFLAGS = \
 $(ACE_CPPFLAGS) \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libAttribute

libAttribute_la_SOURCES = src/Attribute/alist.cc	\
src/Attribute/aliterator.cc src/Attribute/attribute.cc	\
src/Attribute/attrlist.cc src/Attribute/attrvalue.cc	\
src/Attribute/commodule.cc src/Attribute/lexscan.cc	\
src/Attribute/paramlist.cc

libAttribute_la_CPPFLAGS = \
 -Dcplusplus_2_1 $(IVSTD_INCLUDES)

### libTIFF

if HAVE_LIBTIFF
# use external tiff library
TIFF_LOC_CPPFLAG = -DEXTERN_TIFF
else
# build internal tiff library
lib_LTLIBRARIES += libTIFF.la
TIFF_LOC_CPPFLAG =
LIBS += -lTIFF
endif

libTIFF_la_SOURCES = src/TIFF/tif_aux.c src/TIFF/tif_ccittrle.c		\
src/TIFF/tif_close.c src/TIFF/tif_compat.c src/TIFF/tif_compress.c	\
src/TIFF/tif_dir.c src/TIFF/tif_dirinfo.c src/TIFF/tif_dirread.c	\
src/TIFF/tif_dirwrite.c src/TIFF/tif_dumpmode.c src/TIFF/tif_error.c	\
src/TIFF/tif_fax3.c src/TIFF/tif_fax4.c src/TIFF/tif_flush.c		\
src/TIFF/tif_getimage.c src/TIFF/tif_jpeg.c src/TIFF/tif_lzw.c		\
src/TIFF/tif_next.c src/TIFF/tif_open.c src/TIFF/tif_packbits.c		\
src/TIFF/tif_print.c src/TIFF/tif_read.c src/TIFF/tif_strip.c		\
src/TIFF/tif_swab.c src/TIFF/tif_thunder.c src/TIFF/tif_tile.c		\
src/TIFF/tif_version.c src/TIFF/tif_warning.c src/TIFF/tif_write.c

# Not in libTIFF:
#  src/TIFF/mkg3states.c src/TIFF/mkspans.c
#  src/TIFF/tif_machdep.c

libTIFF_la_CPPFLAGS = -Isrc/TIFF -Isrc/include/TIFF -Isrc/include

tif_fax3.$(OBJEXT) : g3states.h
.libs/libTIFF_la-tif_fax3.lo: g3states.h

g3states.h: mkg3states
	./mkg3states > $@

mkg3states: src/TIFF/mkg3states.c
	$(CC) -o $@ $<

### libACE Support

if HAVE_LIBACE
ACE_CPPFLAGS = -DHAVE_ACE
ACE_INLINE_CPPFLAGS = -D__ACE_INLINE__ $(ACE_CPPFLAGS)
else
ACE_CPPFLAGS =
ACE_INLINE_CPPFLAGS =
endif

### Support files

pkglib_DATA = src/ComUtil/comterp.err Idemo InterViews

Idemo: src/idemo/Idemo.defaults
	cp $< $@

InterViews: src/IV/InterViews.defaults
	cp $< $@

### binaries

bin_SCRIPTS = ivgetjpg ivtiftopnm pnmtopgm

ivgetjpg: src/scripts/ivgetjpg.bash
	cp $< $@
	chmod a+x $@

ivtiftopnm: src/scripts/ivtiftopnm.bash
	cp $< $@
	chmod a+x $@

pnmtopgm: src/scripts/pnmtopgm.sh
	cp $< $@
	chmod a+x $@

bin_PROGRAMS = gclock graphdraw glyphterp stdcmapppm idemo comdraw	\
idraw ivtmpnam comterp drawserv dclock ivtext flipbook iclass comtest	\
drawtool

gclock_SOURCES = src/glyphs/gclock/main.cc

graphdraw_SOURCES = src/graphdraw/main.cc

glyphterp_SOURCES = src/glyphterp/main.cc

stdcmapppm_SOURCES = src/utils/stdcmapppm.c # C

idemo_SOURCES = src/idemo/main.cc

comdraw_SOURCES = src/comdraw/main.cc

idraw_SOURCES = src/idraw/main.cc

ivtmpnam_SOURCES = src/utils/ivtmpnam.c # C

comterp_SOURCES = src/comterp_/main.cc

drawserv_SOURCES = src/drawserv_/main.cc

dclock_SOURCES = src/dclock/clocktime.cc src/dclock/clocktime.h	\
src/dclock/data.cc src/dclock/data.h src/dclock/dclock.cc	\
src/dclock/dclock.h src/dclock/dface.cc src/dclock/dface.h	\
src/dclock/digit.cc src/dclock/digit.h src/dclock/segment.cc	\
src/dclock/segment.h

ivtext_SOURCES = src/ivtext/main.cc

flipbook_SOURCES = src/flipbook/main.cc

iclass_SOURCES = src/iclass/classbuffer.cc src/iclass/classbuffer.h	\
src/iclass/classeditor.cc src/iclass/classeditor.h			\
src/iclass/classinfo.cc src/iclass/classinfo.h src/iclass/dialogs.cc	\
src/iclass/dialogs.h src/iclass/direct.cc src/iclass/direct.h		\
src/iclass/globals.cc src/iclass/globals.h src/iclass/iclass.cc		\
src/iclass/iclass.h src/iclass/main.cc

comtest_SOURCES = src/comtest/main.cc

drawtool_SOURCES = src/drawtool/main.cc


AM_CPPFLAGS = -Isrc/include
AM_CXXFLAGS = -Wno-write-strings -Wno-deprecated


## Giving per-object flags ensures that all the "main.o" files have
## different names.  See info: (automake)Top > FAQ > Renamed Objects

gclock_CPPFLAGS		= -Dcplusplus_2_1 -Isrc/glyphs $(IVSTD_INCLUDES)
graphdraw_CPPFLAGS	= -Dcplusplus_2_1 -Div2_6_compatible $(ACE_CPPFLAGS) $(IV2_6_INCLUDES)
glyphterp_CPPFLAGS	= -Dcplusplus_2_1 $(ACE_CPPFLAGS) $(IVSTD_INCLUDES)
stdcmapppm_CPPFLAGS	=
idemo_CPPFLAGS		= -Dcplusplus_2_1 $(IVSTD_INCLUDES)
comdraw_CPPFLAGS	= $(ACE_INLINE_CPPFLAGS) -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)
idraw_CPPFLAGS		= -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)
ivtmpnam_CPPFLAGS	=
comterp_CPPFLAGS	= $(ACE_INLINE_CPPFLAGS) -Dcplusplus_2_1 $(IVSTD_INCLUDES)
drawserv_CPPFLAGS	= $(ACE_INLINE_CPPFLAGS) -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)
dclock_CPPFLAGS		= -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)
ivtext_CPPFLAGS		= -Dcplusplus_2_1 $(IVSTD_INCLUDES)
flipbook_CPPFLAGS	= -Dcplusplus_2_1 -Div2_6_compatible $(ACE_CPPFLAGS) $(IV2_6_INCLUDES)
iclass_CPPFLAGS		= -Dcplusplus_2_1 -Div2_6_compatible $(IV2_6_INCLUDES)
comtest_CPPFLAGS	= -Dcplusplus_2_1 $(ACE_CPPFLAGS) $(IVSTD_INCLUDES)
drawtool_CPPFLAGS	= $(ACE_INLINE_CPPFLAGS) -Div2_6_compatible -Dcplusplus_2_1 $(IV2_6_INCLUDES)

if HAVE_LIBACE
ACE_DISPATCH_LD = -lAceDispatch
else
ACE_DISPATCH_LD =
endif

AM_LDFLAGS = -lOverlayUnidraw $(ACE_DISPATCH_LD) -lComGlyph -lComTerp	\
-lAttrGlyph -lAttribute -lComUtil -lUniIdraw -lIVGlyph -lTopoFace	\
-lUnidraw -lIV $(LIBS)

stdcmapppm_LDFLAGS =
ivtmpnam_LDFLAGS =

graphdraw_LDFLAGS	= -lGraphUnidraw -lComUnidraw $(AM_LDFLAGS)
gclock_LDFLAGS		= -lTime $(AM_LDFLAGS)
comdraw_LDFLAGS		= -lComUnidraw $(AM_LDFLAGS)
drawserv_LDFLAGS	= -lDrawServ -lFrameUnidraw -lGraphUnidraw -lComUnidraw $(AM_LDFLAGS)
flipbook_LDFLAGS	= -lFrameUnidraw -lComUnidraw $(AM_LDFLAGS)

### directories to recurse into, for installation of include files

SUBDIRS = src src/include

### man pages to install

man_MANS = src/man/man1/drawtool.1 src/man/man1/iclass.1		\
src/man/man1/idraw.1 src/man/man1/comterp.1 src/man/man1/flipbook.1	\
src/man/man1/comdraw.1 src/man/man1/dclock.1 src/man/man1/graphdraw.1	\
src/man/man3/UArray.3 src/man/man3/RotateTool.3				\
src/man/man3/TextDisplay.3 src/man/man3/Banner.3			\
src/man/man3/TextComp.3 src/man/man3/ULabel.3 src/man/man3/Iterator.3	\
src/man/man3/KeyMap.3 src/man/man3/Dispatcher.3				\
src/man/man3/UHashTable.3 src/man/man3/GraphicView.3			\
src/man/man3/BasicDialog.3 src/man/man3/SlotComp.3			\
src/man/man3/ImportCmd.3 src/man/man3/UMap.3 src/man/man3/Scroller.3	\
src/man/man3/RpcService.3 src/man/man3/ScaleTool.3			\
src/man/man3/Message.3 src/man/man3/IOCallback.3			\
src/man/man3/PatternCmd.3 src/man/man3/Path.3				\
src/man/man3/ExternView.3 src/man/man3/ReqErr.3 src/man/man3/RpcHdr.3	\
src/man/man3/Tool.3 src/man/man3/lines.3 src/man/man3/Interactor.3	\
src/man/man3/FileChooser.3 src/man/man3/manips.3			\
src/man/man3/statevars.3 src/man/man3/EditorInfo.3			\
src/man/man3/CGlue.3 src/man/man3/align.3 src/man/man3/ellipses.3	\
src/man/man3/Editor.3 src/man/man3/UList.3 src/man/man3/Pattern.3	\
src/man/man3/iostreamb.3 src/man/man3/uctrls.3				\
src/man/man3/GraphicCompTool.3 src/man/man3/Command.3			\
src/man/man3/LinkComp.3 src/man/man3/splines.3				\
src/man/man3/SplineComp.3 src/man/man3/ComponentView.3			\
src/man/man3/MatchEditor.3 src/man/man3/pspaint.3			\
src/man/man3/rpcstream.3 src/man/man3/datas.3 src/man/man3/RpcReader.3	\
src/man/man3/Scene.3 src/man/man3/GraphicComp.3				\
src/man/man3/StretchTool.3 src/man/man3/Resource.3			\
src/man/man3/LineComp.3 src/man/man3/classes.3 src/man/man3/Tray.3	\
src/man/man3/Viewer.3 src/man/man3/Graphic.3				\
src/man/man3/MagnifyTool.3 src/man/man3/SelectTool.3			\
src/man/man3/CompletionEditor.3 src/man/man3/Painter.3			\
src/man/man3/TransferFunct.3 src/man/man3/Component.3			\
src/man/man3/Selection.3 src/man/man3/Frame.3				\
src/man/man3/Perspective.3 src/man/man3/viewcmds.3			\
src/man/man3/UnidrawIntro.3 src/man/man3/RpcRegistry.3			\
src/man/man3/Unidraw.3 src/man/man3/Page.3 src/man/man3/Viewport.3	\
src/man/man3/Vertices.3 src/man/man3/Regexp.3 src/man/man3/Clipboard.3	\
src/man/man3/Panner.3 src/man/man3/Picture.3 src/man/man3/transfns.3	\
src/man/man3/Damage.3 src/man/man3/StringEditor.3			\
src/man/man3/RpcWriter.3 src/man/man3/GraphicBlock.3			\
src/man/man3/GVUpdater.3 src/man/man3/RasterComp.3			\
src/man/man3/polygons.3 src/man/man3/stateviews.3			\
src/man/man3/Connector.3 src/man/man3/UControl.3			\
src/man/man3/Menu2_6.3 src/man/man3/World.3 src/man/man3/BrushCmd.3	\
src/man/man3/globals.3 src/man/man3/StrBrowser.3 src/man/man3/NOPCmd.3	\
src/man/man3/PolygonComp.3 src/man/man3/IOHandler.3			\
src/man/man3/Catalog.3 src/man/man3/RpcPeer.3				\
src/man/man3/EllipseComp.3 src/man/man3/UStencil.3			\
src/man/man3/StateVarView.3 src/man/man3/Dialog.3			\
src/man/man3/VerticesComp.3 src/man/man3/TextEditor.3			\
src/man/man3/kybd.3 src/man/man3/ConnectTool.3 src/man/man3/rpcbuf.3	\
src/man/man3/MacroCmd.3 src/man/man3/geomobjs.3 src/man/man3/struct.3	\
src/man/man3/edit.3 src/man/man3/RasterRect.3 src/man/man3/Control.3	\
src/man/man3/RectComp.3 src/man/man3/Border.3 src/man/man3/Shape.3	\
src/man/man3/PostScriptView.3 src/man/man3/FontCmd.3			\
src/man/man3/PinComp.3 src/man/man3/ColorCmd.3 src/man/man3/PadComp.3	\
src/man/man3/StrChooser.3 src/man/man3/Subject.3			\
src/man/man3/MoveTool.3 src/man/man3/ControlInfo.3			\
src/man/man3/catcmds.3 src/man/man3/Sensor.3 src/man/man3/Data.3	\
src/man/man3/Button2_6.3 src/man/man3/CSolver.3 src/man/man3/Creator.3	\
src/man/man3/ReshapeTool.3 src/man/man3/TextBuffer.3			\
src/man/man3/Grid.3 src/man/man3/StencilComp.3 src/man/man3/StateVar.3	\
src/man/man3/Box2_6.3 src/man/man3/Deck2_6.3 src/man/man3/Glue2_6.3	\
src/man/man3/Rubband.3 src/man/man3/Manipulator.3			\
src/man/man3/transforms.3 src/man/man3/FileBrowser.3			\
src/man/man3/Adjuster.3

### other documentation

if PS2PDF
doc_DATA = refman.pdf
refman.pdf: src/man/refman3.1/refman.PS
	$(ps2pdf) $< $@
else
doc_DATA = src/man/refman3.1/refman.PS
endif
