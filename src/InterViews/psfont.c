/*
 * Copyright (c) 1987, 1988, 1989, 1990, 1991 Stanford University
 * Copyright (c) 1991 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Stanford and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Stanford and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 *
 * IN NO EVENT SHALL STANFORD OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

/*
 * PSFont31 - PostScript font metrics via FreeType2 and Fontconfig
 */

#include <InterViews/psfont.h>
#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string.h>
#include <stdlib.h>

class PSFontImpl {
private:
    friend class PSFont31;

    char* name_;
    const char* encoding_;
    Coord size_;
    Coord widths_[256];
};

static char* fc_find_font(const char* psname) {
    FcInit();
    FcPattern* pat = FcPatternCreate();
    FcPatternAddString(pat, FC_POSTSCRIPT_NAME, (FcChar8*)psname);
    FcConfigSubstitute(nullptr, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcResult result;
    FcPattern* match = FcFontMatch(nullptr, pat, &result);
    FcPatternDestroy(pat);
    if (!match)
        return nullptr;
    char* path = nullptr;
    FcChar8* file;
    if (FcPatternGetString(match, FC_FILE, 0, &file) == FcResultMatch)
        path = strdup((char*)file);
    FcPatternDestroy(match);
    return path;
}

static const char* ft_encoding_name(FT_Encoding enc) {
    switch (enc) {
    case FT_ENCODING_ADOBE_STANDARD: return "AdobeStandardEncoding";
    case FT_ENCODING_ADOBE_EXPERT:   return "AdobeExpertEncoding";
    case FT_ENCODING_ADOBE_CUSTOM:   return "AdobeCustomEncoding";
    case FT_ENCODING_UNICODE:        return "Unicode";
    default:                         return "Unknown";
    }
}

PSFont31::PSFont31(
    const char* psname, Coord size, const char* name, float scale
) : Font(name, scale) {
    PSFontImpl* p = new PSFontImpl;
    impl_ = p;
    p->name_ = nullptr;
    p->encoding_ = "Unknown";
    p->size_ = size;
    memset(p->widths_, 0, sizeof(p->widths_));

    char* filepath = fc_find_font(psname);
    if (!filepath)
        return;

    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        free(filepath);
        return;
    }

    FT_Face face;
    if (FT_New_Face(library, filepath, 0, &face) == 0) {
        const char* psn = FT_Get_Postscript_Name(face);
        p->name_ = strdup(psn ? psn : psname);
        if (face->charmap)
            p->encoding_ = ft_encoding_name(face->charmap->encoding);
        for (int c = 0; c < 256; c++) {
            FT_UInt gi = FT_Get_Char_Index(face, (FT_ULong)c);
            if (gi && FT_Load_Glyph(face, gi, FT_LOAD_NO_SCALE) == 0)
                p->widths_[c] = Coord(face->glyph->advance.x) / face->units_per_EM * size;
        }
        FT_Done_Face(face);
    }
    FT_Done_FreeType(library);
    free(filepath);
}

PSFont31::~PSFont31() {
    free(impl_->name_);
    delete impl_;
}

const char* PSFont31::name() const { return impl_->name_; }
const char* PSFont31::encoding() const { return impl_->encoding_; }
Coord PSFont31::size() const { return impl_->size_; }
Coord PSFont31::width(long c) const { return impl_->widths_[c & 0xff]; }
Coord PSFont31::width(const char* s, int n) const { return Font::width(s, n); }

boolean PSFont31::exists(const char* psname) {
    char* path = fc_find_font(psname);
    if (!path)
        return false;
    free(path);
    return true;
}
