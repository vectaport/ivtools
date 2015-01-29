/*
 * Copyright (c) 1995 Vectaport Inc.
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

#include <GraphUnidraw/graphdata.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>

#include <TopoFace/topoedge.h>
#include <TopoFace/toponode.h>

EdgeData::EdgeData(EdgeComp* ed, TopoNode* st, TopoNode* en) {
    edge = ed;
    start = st;
    end = en;
}

boolean EdgeData::IsA(ClassId id) { return id == EDGE_DATA; }

NodeData::NodeData(NodeComp* nod, TopoEdge* edg, boolean st) {
    node = nod;
    edge = edg;
    start = st;
}

boolean NodeData::IsA(ClassId id) { return id == NODE_DATA; }
