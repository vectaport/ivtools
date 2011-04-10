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

#ifndef graphclasses_h
#define graphclasses_h

#include <OverlayUnidraw/ovclasses.h>

#define GRAPH_IDRAW_COMP   9701
#define GRAPH_COMP         9702
#define EDGE_COMP          9703
#define NODE_COMP          9704

#define EDGECONNECT_CMD    9705
#define EDGEUPDATE_CMD     9706
#define NODETEXT_CMD       9707
#define GRAPHDELETE_CMD    9708
#define GRAPHNEWVIEW_CMD   9709
#define GRAPHIMPORT_CMD    9710
#define GRAPHCUT_CMD       9711
#define GRAPHCOPY_CMD      9712
#define GRAPHPASTE_CMD     9713
#define GRAPHDUP_CMD       9714
#define GRAPH_MOVE_TOOL    9715

#define GRAPH_IDRAW_VIEW   Combine(GRAPH_IDRAW_COMP, COMPONENT_VIEW)
#define GRAPH_VIEW         Combine(GRAPH_COMP,	     COMPONENT_VIEW)
#define EDGE_VIEW          Combine(EDGE_COMP, 	     COMPONENT_VIEW)
#define NODE_VIEW          Combine(NODE_COMP, 	     COMPONENT_VIEW)

#define GRAPH_IDRAW_PS     Combine(GRAPH_IDRAW_COMP, POSTSCRIPT_VIEW)
#define GRAPH_PS           Combine(GRAPH_COMP,	     POSTSCRIPT_VIEW)
#define EDGE_PS            Combine(EDGE_COMP, 	     POSTSCRIPT_VIEW)
#define NODE_PS            Combine(NODE_COMP, 	     POSTSCRIPT_VIEW)

#define GRAPH_IDRAW_SCRIPT Combine(GRAPH_IDRAW_COMP, SCRIPT_VIEW)
#define GRAPH_SCRIPT       Combine(GRAPH_COMP,       SCRIPT_VIEW)
#define EDGE_SCRIPT        Combine(EDGE_COMP,        SCRIPT_VIEW)
#define NODE_SCRIPT        Combine(NODE_COMP,        SCRIPT_VIEW)

#endif
