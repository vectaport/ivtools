/*
 * Copyright (c) 1994-1996 Vectaport Inc.
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

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphimport.h>
#include <GraphUnidraw/graphcreator.h>
#include <GraphUnidraw/nodecomp.h>

#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovpspict.h>
#include <OverlayUnidraw/ovpsview.h>

#include <Unidraw/catalog.h>

GraphCreator::GraphCreator () { }

/* for commands */
void* GraphCreator::Create (
    ClassId id, istream& in, ObjectMap* objmap, int objid
) {
    switch (id) {
    case EDGECONNECT_CMD:  CREATE(EdgeConnectCmd, in, objmap, objid);
    case EDGEUPDATE_CMD:   CREATE(EdgeUpdateCmd,  in, objmap, objid);
    case NODETEXT_CMD:     CREATE(NodeTextCmd,    in, objmap, objid);
    case GRAPHIMPORT_CMD:  CREATE(GraphImportCmd, in, objmap, objid);
    case GRAPHCUT_CMD:     CREATE(GraphCutCmd,    in, objmap, objid);
    case GRAPHCOPY_CMD:    CREATE(GraphCopyCmd,   in, objmap, objid);
    case GRAPHPASTE_CMD:   CREATE(GraphPasteCmd,  in, objmap, objid);
    case GRAPHDUP_CMD:     CREATE(GraphDupCmd,    in, objmap, objid);

    default:
	return OverlayCreator::Create(id, in, objmap, objid);
    }
}

/* for views */
void* GraphCreator::Create (ClassId id) {
  void* ptr = create(id);
  return ptr ? ptr : OverlayCreator::Create(id);
}

void* GraphCreator::create (ClassId id) {
    if (id == GRAPH_IDRAW_VIEW)   return new GraphIdrawView;
    if (id == GRAPH_VIEW)         return new GraphView;
    if (id == EDGE_VIEW)          return new EdgeView;
    if (id == NODE_VIEW)          return new NodeView;

    if (id == GRAPH_IDRAW_PS)     return new OverlayIdrawPS;
    if (id == GRAPH_PS)           return new PicturePS;
    if (id == EDGE_PS)            return new ArrowLinePS;
    if (id == NODE_PS)            return new PicturePS;

    if (id == GRAPH_IDRAW_SCRIPT) return new GraphIdrawScript;
    if (id == GRAPH_SCRIPT)       return new GraphScript;
    if (id == EDGE_SCRIPT)        return new EdgeScript;
    if (id == NODE_SCRIPT)        return new NodeScript;

    return nil;
}
