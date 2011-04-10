/*
 * Copyright (c) 1997,1999 Vectaport Inc.
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
 * DrawCatalog - can read and write components in script or postscript
 */

#ifndef drawcatalog_h
#define drawcatalog_h

#include <FrameUnidraw/framecatalog.h>

class DrawIdrawComp;
class EdgeComp;
class NodeComp;

//: catalog for use with drawserv.
class DrawCatalog : public FrameCatalog{
public:
    DrawCatalog(const char*, Creator*);

    boolean Retrieve (const char*, Component*&);

    virtual OverlayComp* ReadComp(const char*, istream&, OverlayComp* =nil);

    void graph_init(DrawIdrawComp* comps, int num_edge, int num_node);
    void graph_finish();

protected:
    int* _startnode;
    int* _endnode;
    EdgeComp** _edges;
    NodeComp** _nodes;
    int _edge_cnt;
    int _node_cnt;
    int _num_edge;
    int _num_node;
    DrawIdrawComp* _comps;
};

#endif
