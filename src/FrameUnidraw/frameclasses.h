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

#ifndef frameclasses_h
#define frameclasses_h

#include <OverlayUnidraw/ovclasses.h>

#define FRAME_OVERLAYS_COMP 8001
#define FRAME_COMP          8002
#define FRAMES_COMP         8003
#define FRAME_IDRAW_COMP    8004
#define FRAME_FILE_COMP     8005

#define CREATEFRAME_CMD     8055
#define DELETEFRAME_CMD     8056
#define MOVEFRAME_CMD       8057
#define FRAME_GROUP_CMD     8058
#define FRAME_UNGROUP_CMD   8059
#define FRAME_FRONT_CMD     8060
#define FRAME_BACK_CMD      8061
#define FRAME_COPY_CMD      8062
#define CREATEMOVEFRAME_CMD 8063
#define FRGRIDSPACING_CMD   8064
#define FRAMEBEGIN_CMD      8065
#define FRAMEEND_CMD        8066
#define FRAMEOPEN_CMD       8067
#define FRAMENEWVIEW_CMD    8068
#define COPYMOVEFRAME_CMD   8069
#define SHOWOTHERFRAME_CMD  8070
#define AUTONEWFRAME_CMD    8071
#define FRAMEIMPORT_CMD     8072
#define FRAMEIMPORTPASTE_CMD 8073

#define FRAME_OVERLAYS_VIEW   Combine(FRAME_OVERLAYS_COMP, COMPONENT_VIEW)
#define FRAME_VIEW            Combine(FRAME_COMP, COMPONENT_VIEW)
#define FRAMES_VIEW           Combine(FRAMES_COMP, COMPONENT_VIEW)
#define FRAME_IDRAW_VIEW      Combine(FRAME_IDRAW_COMP, COMPONENT_VIEW)
#define FRAME_FILE_VIEW       Combine(FRAME_FILE_COMP, COMPONENT_VIEW)

#define FRAME_OVERLAYS_PS     Combine(FRAME_OVERLAYS_COMP, POSTSCRIPT_VIEW)
#define FRAME_PS              Combine(FRAME_COMP, POSTSCRIPT_VIEW)
#define FRAMES_PS             Combine(FRAMES_COMP, POSTSCRIPT_VIEW)
#define FRAME_IDRAW_PS        Combine(FRAME_IDRAW_COMP, POSTSCRIPT_VIEW)
#define FRAME_FILE_PS         Combine(FRAME_FILE_COMP, POSTSCRIPT_VIEW)

#define FRAME_OVERLAYS_SCRIPT Combine(FRAME_OVERLAYS_COMP, SCRIPT_VIEW)
#define FRAME_SCRIPT          Combine(FRAME_COMP, SCRIPT_VIEW)
#define FRAMES_SCRIPT         Combine(FRAMES_COMP, SCRIPT_VIEW)
#define FRAME_IDRAW_SCRIPT    Combine(FRAME_IDRAW_COMP, SCRIPT_VIEW)
#define FRAME_FILE_SCRIPT     Combine(FRAME_FILE_COMP, SCRIPT_VIEW)

#endif



