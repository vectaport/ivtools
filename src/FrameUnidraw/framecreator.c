/*
 * Copyright (c) 1994, 1995 Vectaport Inc.
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

#include <FrameUnidraw/frameclasses.h>
#include <FrameUnidraw/framecmds.h>
#include <FrameUnidraw/framecreator.h>
#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/framescripts.h>
#include <FrameUnidraw/framefile.h>
#include <FrameUnidraw/frameviews.h>
#include <FrameUnidraw/frameps.h>

#include <UniIdraw/idcomp.h>

#include <Unidraw/catalog.h>

/*****************************************************************************/

FrameCreator::FrameCreator () { }

/* for commands */
void* FrameCreator::Create (
    ClassId id, istream& in, ObjectMap* objmap, int objid
) {
    switch (id) {
    case CREATEFRAME_CMD:     CREATE(CreateFrameCmd,in,objmap,objid);
    case DELETEFRAME_CMD:     CREATE(DeleteFrameCmd,in,objmap,objid);
    case MOVEFRAME_CMD:       CREATE(MoveFrameCmd,in,objmap,objid);
    case FRAME_COPY_CMD:      CREATE(FrameCopyCmd,in,objmap,objid);
    case CREATEMOVEFRAME_CMD: CREATE(CreateMoveFrameCmd,in,objmap,objid);
    case FRAMEBEGIN_CMD:      CREATE(FrameBeginCmd,in,objmap,objid);
    case FRAMEEND_CMD:        CREATE(FrameEndCmd,in,objmap,objid);
        default:                   return OverlayCreator::Create(id, in,objmap,objid);
    }
}

/* for views */
void* FrameCreator::Create (ClassId id) {
  void* ptr = create(id);
  return ptr ? ptr : OverlayCreator::Create(id);
}


void* FrameCreator::create(ClassId id) {
    if (id == FRAME_OVERLAYS_VIEW)    return new FrameOverlaysView;
    if (id == FRAME_VIEW)             return new FrameView;
    if (id == FRAMES_VIEW)            return new FramesView;
    if (id == FRAME_IDRAW_VIEW)       return new FrameIdrawView;
    if (id == FRAME_FILE_VIEW)        return new FrameFileView;

    if (id == FRAME_OVERLAYS_PS)      return new OverlaysPS;
    if (id == FRAME_PS)               return new OverlaysPS;
    if (id == FRAMES_PS)              return new OverlaysPS;
    if (id == FRAME_IDRAW_PS)         return new FrameIdrawPS;
    if (id == FRAME_FILE_PS)          return new OverlaysPS;

    if (id == FRAME_OVERLAYS_SCRIPT)  return new FrameOverlaysScript;
    if (id == FRAME_SCRIPT)           return new FrameScript;
    if (id == FRAMES_SCRIPT)          return new FramesScript;
    if (id == FRAME_IDRAW_SCRIPT)     return new FrameIdrawScript;
    if (id == FRAME_FILE_SCRIPT)      return new FrameFileScript;

    return nil;
}
