/*
 * Copyright (c) 1996-1999 Vectaport Inc., R.B. Kissh & Associates
 * Copyright (c) 1994-1995 Vectaport Inc.
 * Copyright (c) 1990, 1991 Stanford University
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

/*
 * OvImportCmd - a command for importing graphical objects
 */

#ifndef ov_import_h
#define ov_import_h

#include <Unidraw/Commands/command.h>
#include <stdio.h>
#include <sys/types.h>

class Bitmap;
class Component;
class Editor;
class FileHelper;
class GraphicComp;
class ImportChooser;
class OverlayRaster;
class PortableImageHelper;
class Raster;
class RasterOvComp;

//: command for importing arbitrary graphical files.
// command for importing arbitrary graphical files: a wide variety of raster files, 
// idraw "PostScript" and regular "PostScript", compressed or not compressed, 
// by URL or pathname.  Useful static methods for constructing components, as well
// as full-on command for hooking into a file menu.
class OvImportCmd : public Command {
public:
    enum FileType { UnknownFile, RasterFile, PostScriptFile, IvtoolsFile, CompressedFile };
    // types of files to be imported.

    OvImportCmd(Editor* = nil, ImportChooser* = nil);
    OvImportCmd(ControlInfo* = nil, ImportChooser* = nil);
    virtual ~OvImportCmd();
    void Init(ImportChooser*);

    void instream(istream*);
    // set istream to import file from.
    void pathname(const char*, boolean popen=false);
    // set pathname to import file from; if 'popen' treat the path as a command. 
    const char* pathname() { return path_; }
    // return pointer to pathname.
    void preserve_selection(boolean);
    // set flag to import without changing current selection in editor.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void Execute();
    // check for non-empty selection, pop-up dialog box to specify input path
    // and other import parameters, then import and paste result.
    virtual boolean Reversible();
    // returns false.
    virtual GraphicComp* PostDialog();
    // method that pops-up the dialog box and constructs the imported component.

    virtual GraphicComp* Import(const char*);
    // import from pathname.
    virtual GraphicComp* Import(istream&);
    // import from istream.
    virtual GraphicComp* Import(istream&, boolean& empty);
    // import from istream, returning flag to indicate if anything happened.

    static boolean changeComp(RasterOvComp* oldComp, RasterOvComp* newComp);
    static void detach(RasterOvComp*);

    static GraphicComp* DoImport(
        istream& instrm, boolean& empty, FileHelper& helper, Editor* ed, 
        boolean is_strm, const char* pathname, int& pnmfd
    );

    static GraphicComp* TIFF_Image(const char*);
    // generate RasterOvComp from TIFF file.
    static OverlayRaster* TIFF_Raster(const char*);
    // generate raster from TIFF file.

    static GraphicComp* PGM_Image(const char*);
    // generate RasterOvComp from PGM file.
    static OverlayRaster* PGM_Raster(
	const char*, boolean delayed = false, OverlayRaster* = nil,
	IntCoord xbeg=-1, IntCoord xend=-1, IntCoord ybeg=-1, IntCoord yend=-1
    );
    // generate raster from PGM file, with option to delay reading pixel data
    // until needed for display, and ability to specify a subimage.  This method
    // supports pre-tiled rasters as well, indicated by a "# tile <width> <height>"
    // comment in the PGM header.
    static GraphicComp* PGM_Image(istream&, boolean ascii=false);
    // generate RasterOvComp from PGM istream.
    static OverlayRaster* PGM_Raster(istream&, boolean ascii=false);
    // generate raster from PGM istream.

    static GraphicComp* PPM_Image(const char*);
    // generate RasterOvComp from PPM file.
    static OverlayRaster* PPM_Raster(
	const char*, boolean delayed = false, OverlayRaster* = nil,
	IntCoord xbeg=-1, IntCoord xend=-1, IntCoord ybeg=-1, IntCoord yend=-1
    );
    // generate raster from PPM file, with option to delay reading pixel data
    // until needed for display, and ability to specify a subimage.  This method
    // supports pre-tiled rasters as well, indicated by a "# tile <width> <height>"
    // comment in the PPM header.
    static GraphicComp* PPM_Image(istream&, boolean ascii=false);
    // generate RasterOvComp from PPM istream.
    static OverlayRaster* PPM_Raster(istream& in, boolean ascii=false);
    // generate raster from PPM istream.

    static GraphicComp* PNM_Image(istream&, const char* creator = nil);
    // generate RasterOvComp from a PNM istream (PBM, PGM, or PPM).
    static GraphicComp* PNM_Image_Filter(istream&, boolean return_fd, int& fd,
					 const char* filter = nil);
    // generate RasterOvComp from a PNM istream (PBM, PGM, or PPM), using a 
    // specified filter to convert from another format to one of the PNM formats.
    // if 'return_fd' is true this method sets up and returns a file handle
    // to import a raw PPM image.

    static int Pipe_Filter(istream& in, const char* filter);
    // low-level mechanism to filter an istream using an arbitrary command line
    // filter.  Uses a double-pipe/double-fork mechanism, where a child process
    // is set up to read the istream and pipe it to a grandchild process,
    // which reads the other end of the pipe, runs the data through the filter,
    // and writes the result to a pipe whose other end is indicated by the 
    // return value of this method.  This double-pipe/double-fork architecture
    // avoids the deadlock possible in a double-pipe/single-fork architecture,
    // especially when decompressing the incoming istream.

    static boolean Tiling(int& width, int& height);
    // return on-the-fly tiling parameters from command line: -tile, -twidth w,
    // and -theight.  When enabled this causes large PGM or PNM images to be
    // read in as a grid of sub-image components, and any subsequent export or 
    // save to disk will reflect this.

    static GraphicComp* XBitmap_Image(const char*);
    // generate StencilOvComp from a X Bitmap file.
    static Bitmap* XBitmap_Bitmap(const char*);
    // generate bitmap from a X Bitmap file.
    static GraphicComp* PBM_Image(const char*);
    // generate StencilOvComp from a PBM file.
    static Bitmap* PBM_Bitmap(const char*);
    // generate bitmap from a PBM file.
    static GraphicComp* PBM_Image(istream&);
    // generate StencilOvComp from a PBM istream.
    static Bitmap* PBM_Bitmap(istream&);
    // generate bitmap from a PBM istream.

    static const char* ReadCreator(const char* pathname);
    // read creator from 'pathname', returning one of "COMPRESS", "GZIP",
    // "TIFF", "SUN", "PBM", "PGM", "PPM", "PBMA", "PGMA", "PPMA", "JPEG",
    // "BM", "ATK", "MP", "X11", "PCX", "IFF", "GIF", "RLE", "PNG", "idraw", 
    // or something else for arbitrary "PostScript".
    static const char* ReadCreator(istream& in, FileType& type);
    // read creator from istream, returning one of "COMPRESS", "GZIP",
    // "TIFF", "SUN", "PBM", "PGM", "PPM", "PBMA", "PGMA", "PPMA", "JPEG",
    // "BM", "ATK", "MP", "X11", "PCX", "IFF", "GIF", "RLE", "PNG", "idraw", 
    // or something else for arbitrary "PostScript", plus a FileType enum.
    // The bytes read to determine the creator are pushed back onto
    // the istream.

    static FILE* CheckCompression(
	FILE* file, const char *pathname, boolean& compressed);
    // check the compression status of a file specified by 'pathname'.

    static OverlayRaster* CreatePlaceImage();
    // create placeholder image when doing asynchronous incremental downloads.
	 
    boolean is_url();
    // return flag that indicates import is from a URL.

    static const char* Create_Tiled_File(
        const char* ppmfile, const char* tilefile, int twidth, int theight
    );
    // utility method for creating an internally tiled PGM or PPM file.

    static FILE* Portable_Raster_Open(
        PortableImageHelper*&, const char* pathname, int ppm, int& ncols, 
        int& nrows, boolean& compressed, boolean& tiled, int& twidth, 
        int& theight
    );
   // utility method for tiled or untiled access of PGM and PPM disk files.

    static void center_import(Editor* ed, GraphicComp* comp);
    // center imported component in the viewer.

    Component* component() { return comp_; }
    // return pointer to imported component.   

     boolean is_popen() {return popen_;}
    // return true if 

protected:
    static OverlayRaster* PI_Raster_Read(
        PortableImageHelper*, FILE* file, int ncols, int nrows, 
        boolean compressed, boolean tiled, boolean delayed, 
        OverlayRaster* raster, IntCoord xbeg, IntCoord xend, IntCoord ybeg, 
        IntCoord yend
    );
   // utility method for tiled or untiled access of PGM and PPM disk files.

    static void PI_Normal_Read(
        PortableImageHelper*, FILE* file, OverlayRaster* raster, int ncols, 
        int nrows, int xbeg, int xend, int ybeg, int yend
    );
   // utility method for untiled access of PGM and PPM disk files.

    static void PI_Tiled_Read(
        PortableImageHelper*, FILE* file, OverlayRaster* raster, int ncols, 
        int nrows, int xbeg, int xend, int ybeg, int yend
    );
   // utility method for tiled access of PGM and PPM disk files.

    static GraphicComp* Portable_Image_Tiled(
        PortableImageHelper*, const char* pathname, int twidth, int theight, 
        int width, int height, boolean compressed, boolean tiled
    );
   // utility method for tiled access of PGM and PPM disk files.

    static GraphicComp* Create_Comp(
        PortableImageHelper* pih, FILE*, const char* pathname, int width, 
        int height, boolean compressed, boolean tiled, int twidth, 
        int theight
    );
   // utility method for tiled or untiled access of PGM and PPM disk files.

protected:
    FileHelper* helper_;
    ImportChooser* chooser_;
    istream* inptr_;
    char* path_;
    boolean popen_;
    boolean preserve_selection_;
    Component* comp_;
};



//: helper class for reading PGM or PPM images.
class PortableImageHelper {
public:
    PortableImageHelper(boolean is_ascii=false) 
      { _is_ascii = is_ascii; _maxval = 255;}
    virtual boolean ppm() = 0;
    virtual int bytes_per_pixel() = 0;
    virtual void read_write_pixel( FILE* in, FILE* out ) = 0;
    virtual const char* magic() = 0;
    virtual void read_poke( OverlayRaster*, FILE*, u_long x, u_long y ) = 0;
    virtual OverlayRaster* create_raster( u_long w, u_long h ) = 0;

    boolean is_ascii() { return _is_ascii; }
    void maxval(int maxv) { _maxval = maxv; }
    int maxval() { return _maxval; }
protected:
    boolean _is_ascii;
    int _maxval;
};


//: helper class for reading PGM images.
class PGM_Helper : public PortableImageHelper {
public:
    PGM_Helper(boolean is_ascii=false);
    virtual boolean ppm();
    virtual int bytes_per_pixel();
    virtual void read_write_pixel( FILE* in, FILE* out );
    virtual const char* magic();
    virtual void read_poke( OverlayRaster*, FILE*, u_long x, u_long y );
    virtual OverlayRaster* create_raster( u_long w, u_long h );
};

//: helper class for reading PPM images.
class PPM_Helper : public PortableImageHelper {
public:
    PPM_Helper(boolean is_ascii=false);
    virtual boolean ppm();
    virtual int bytes_per_pixel();
    virtual void read_write_pixel( FILE* in, FILE* out );
    virtual const char* magic();
    virtual void read_poke( OverlayRaster*, FILE*, u_long x, u_long y );
    virtual OverlayRaster* create_raster( u_long w, u_long h );
};


#endif
