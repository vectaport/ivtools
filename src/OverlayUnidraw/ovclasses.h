/*
 * Copyright (c) 1994-1997 Vectaport Inc.
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
 * Unique overlay idraw class identifiers
 */

#ifndef ovclasses_h
#define ovclasses_h

#include <UniIdraw/idclasses.h>

#define OVABOUT_CMD           2001
#define OVNEWCOMP_CMD         2002
#define OVREVERT_CMD          2003
#define OVVIEWCOMP_CMD        2004
#define OVOPEN_CMD            2005
#define OVSAVECOMP_CMD        2006
#define OVSAVECOMPAS_CMD      2007
#define OV_IMPORT_CMD         2008
#define OV_EXPORT_CMD         2009
#define OVQUIT_CMD            2010
#define OV_DELETE_CMD         2011
#define OVSLCTALL_CMD         2012
#define OVGROUP_CMD           2013
#define OVNEWVIEW_CMD         2014
#define OVCLOSEEDITOR_CMD     2015
#define PAGE_CMD	      2016
#define PRECISEPAGE_CMD	      2017
#define ZOOM_CMD              2018
#define PRECISEZOOM_CMD       2019
#define PAN_CMD               2020
#define FIXEDPAN_CMD          2021
#define PRECISEPAN_CMD        2022
#define ANNOTATE_TOOL         2023
#define OVPRINT_CMD           2024
#define HIDE_VIEW_CMD         2025
#define UNHIDE_VIEWS_CMD      2026
#define CHAIN_VIEWERS_CMD     2027
#define UNCHAIN_VIEWERS_CMD   2028
#define CHAIN_VIEWER_CMD      2029
#define UNCHAIN_VIEWER_CMD    2030
#define CAMERA_MOTION_CMD     2031
#define FIX_VIEW_CMD          2032
#define UNFIX_VIEW_CMD        2033
#define ATTRIBUTE_TOOL        2034
#define CLIPPOLY_AMINB_CMD    2035
#define CLIPPOLY_BMINA_CMD    2036
#define CLIPPOLY_AANDB_CMD    2037
#define CLIPRECT_CMD          2038
#define CLIPRECT_TOOL         2039
#define SLCT_BY_ATTR_CMD      2040
#define CLIPPOLY_CMD          2041
#define CLIPPOLY_TOOL         2042
#define SCRIBBLE_POINTER_CMD  2043
#define TILEFILE_CMD  	      2044
#define DESENSITIZE_VIEW_CMD  2045
#define SENSITIZE_VIEWS_CMD   2046
#define SCALEGRAY_CMD  	      2047
#define IMAGE_CMD  	      2048
#define PSEUDOCOLOR_CMD	      2049
#define LOGSCALE_CMD	      2050
#define REPLACE_RASTER_CMD    2051
#define UNHIGHLIGHT_RASTER_CMD 2052
#define OVSELECT_TOOL         2053
#define GRAYRAMP_CMD	      2054
#define SETATTRBYEXPR_CMD     2054
#define GRLOC_TOOL            2055

#define OVERLAY_COMP          2150
#define OVERLAYS_COMP         2151
#define OVERLAY_IDRAW_COMP    2152

#define OVARROWLINE_COMP      2153
#define OVARROWMULTILINE_COMP 2154
#define OVARROWSPLINE_COMP    2155
#define OVCLOSEDSPLINE_COMP   2156
#define OVELLIPSE_COMP        2157
#define OVLINE_COMP	      2158
#define OVMULTILINE_COMP      2159
#define OVPOLYGON_COMP        2160
#define OVRASTER_COMP         2161
#define OVRECT_COMP	      2162
#define OVSPLINE_COMP         2163
#define OVSTENCIL_COMP        2164
#define OVTEXT_COMP	      2165
#define OVVERTICES_COMP       2166
#define TEXTFILE_COMP	      2167
#define OVFILE_COMP	      2168

#define OVERLAY_VIEW          2169
#define SCRIPT_VIEW           2170
#define OVERLAY_SCRIPT        SCRIPT_VIEW

#define PICTURE_PS            2171

#define OVERLAYS_VIEW	      Combine(OVERLAYS_COMP, COMPONENT_VIEW)
#define OVERLAY_IDRAW_VIEW    Combine(OVERLAY_IDRAW_COMP, COMPONENT_VIEW)

#define OVARROWLINE_VIEW      Combine(OVARROWLINE_COMP, COMPONENT_VIEW)
#define OVARROWMULTILINE_VIEW Combine(OVARROWMULTILINE_COMP, COMPONENT_VIEW)
#define OVARROWSPLINE_VIEW    Combine(OVARROWSPLINE_COMP, COMPONENT_VIEW)
#define OVCLOSEDSPLINE_VIEW   Combine(OVCLOSEDSPLINE_COMP, COMPONENT_VIEW)
#define OVELLIPSE_VIEW        Combine(OVELLIPSE_COMP, COMPONENT_VIEW)
#define OVLINE_VIEW           Combine(OVLINE_COMP, COMPONENT_VIEW)
#define OVMULTILINE_VIEW      Combine(OVMULTILINE_COMP, COMPONENT_VIEW)
#define OVPOLYGON_VIEW        Combine(OVPOLYGON_COMP, COMPONENT_VIEW)
#define OVRASTER_VIEW         Combine(OVRASTER_COMP, COMPONENT_VIEW)
#define OVRECT_VIEW           Combine(OVRECT_COMP, COMPONENT_VIEW)
#define OVSPLINE_VIEW         Combine(OVSPLINE_COMP, COMPONENT_VIEW)
#define OVSTENCIL_VIEW        Combine(OVSTENCIL_COMP, COMPONENT_VIEW)
#define OVTEXT_VIEW           Combine(OVTEXT_COMP, COMPONENT_VIEW)
#define OVVERTICES_VIEW       Combine(OVVERTICES_COMP, COMPONENT_VIEW)
#define TEXTFILE_VIEW         Combine(TEXTFILE_COMP, COMPONENT_VIEW)
#define OVFILE_VIEW           Combine(OVFILE_COMP, COMPONENT_VIEW)

#define OVERLAY_PS	      Combine(OVERLAY_COMP, POSTSCRIPT_VIEW)
#define OVERLAYS_PS	      Combine(OVERLAYS_COMP, POSTSCRIPT_VIEW)
#define OVERLAY_IDRAW_PS      Combine(OVERLAY_IDRAW_COMP, POSTSCRIPT_VIEW)

#define ARROWLINE_PS          Combine(OVARROWLINE_COMP, POSTSCRIPT_VIEW)
#define ARROWMULTILINE_PS     Combine(OVARROWMULTILINE_COMP, POSTSCRIPT_VIEW)
#define ARROWSPLINE_PS        Combine(OVARROWSPLINE_COMP, POSTSCRIPT_VIEW)
#define CLOSEDSPLINE_PS       Combine(OVCLOSEDSPLINE_COMP, POSTSCRIPT_VIEW)
#define ELLIPSE_PS            Combine(OVELLIPSE_COMP, POSTSCRIPT_VIEW)
#define LINE_PS               Combine(OVLINE_COMP, POSTSCRIPT_VIEW)
#define MULTILINE_PS          Combine(OVMULTILINE_COMP, POSTSCRIPT_VIEW)
#define POLYGON_PS            Combine(OVPOLYGON_COMP, POSTSCRIPT_VIEW)
#define RASTER_PS             Combine(OVRASTER_COMP, POSTSCRIPT_VIEW)
#define RECT_PS               Combine(OVRECT_COMP, POSTSCRIPT_VIEW)
#define SPLINE_PS             Combine(OVSPLINE_COMP, POSTSCRIPT_VIEW)
#define STENCIL_PS            Combine(OVSTENCIL_COMP, POSTSCRIPT_VIEW)
#define TEXT_PS               Combine(OVTEXT_COMP, POSTSCRIPT_VIEW)
#define VERTICES_PS           Combine(OVVERTICES_COMP, POSTSCRIPT_VIEW)
#define TEXTFILE_PS           Combine(TEXTFILE_COMP, POSTSCRIPT_VIEW)
#define OVFILE_PS             Combine(OVFILE_COMP, POSTSCRIPT_VIEW)

#define OVERLAYS_SCRIPT       Combine(OVERLAYS_COMP, SCRIPT_VIEW)
#define OVERLAY_IDRAW_SCRIPT  Combine(OVERLAY_IDRAW_COMP, SCRIPT_VIEW)

#define ARROWLINE_SCRIPT      Combine(OVARROWLINE_COMP, SCRIPT_VIEW)
#define ARROWMULTILINE_SCRIPT Combine(OVARROWMULTILINE_COMP, SCRIPT_VIEW)
#define ARROWSPLINE_SCRIPT    Combine(OVARROWSPLINE_COMP, SCRIPT_VIEW)
#define CLOSEDSPLINE_SCRIPT   Combine(OVCLOSEDSPLINE_COMP, SCRIPT_VIEW)
#define ELLIPSE_SCRIPT        Combine(OVELLIPSE_COMP, SCRIPT_VIEW)
#define LINE_SCRIPT           Combine(OVLINE_COMP, SCRIPT_VIEW)
#define MULTILINE_SCRIPT      Combine(OVMULTILINE_COMP, SCRIPT_VIEW)
#define POLYGON_SCRIPT        Combine(OVPOLYGON_COMP, SCRIPT_VIEW)
#define RASTER_SCRIPT         Combine(OVRASTER_COMP, SCRIPT_VIEW)
#define RECT_SCRIPT           Combine(OVRECT_COMP, SCRIPT_VIEW)
#define SPLINE_SCRIPT         Combine(OVSPLINE_COMP, SCRIPT_VIEW)
#define STENCIL_SCRIPT        Combine(OVSTENCIL_COMP, SCRIPT_VIEW)
#define TEXT_SCRIPT           Combine(OVTEXT_COMP, SCRIPT_VIEW)
#define VERTICES_SCRIPT       Combine(OVVERTICES_COMP, SCRIPT_VIEW)
#define TEXTFILE_SCRIPT       Combine(TEXTFILE_COMP, SCRIPT_VIEW)
#define OVFILE_SCRIPT         Combine(OVFILE_COMP, SCRIPT_VIEW)

#endif
