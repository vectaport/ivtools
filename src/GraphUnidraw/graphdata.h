/*
 * Copyright (c) 1995,1999 Vectaport Inc.
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

#ifndef graphdata_h
#define graphdata_h

#include <Unidraw/globals.h>

class EdgeComp;
class NodeComp;
class TopoEdge;
class TopoNode;

#define EDGE_DATA   1
#define NODE_DATA   2

//: base class for EdgeData and NodeData.
class GraphData {
public:
    GraphData() {}
    virtual boolean IsA(ClassId) = 0;
};

//: command data for storing edge information.
class EdgeData : public GraphData {
public:
    EdgeData(EdgeComp* ed, TopoNode* st, TopoNode* en);
    virtual boolean IsA(ClassId);
    EdgeComp* edge;
    TopoNode* start;
    TopoNode* end;
};

//: command data for storing node information.
class NodeData : public GraphData {
public:
    NodeData(NodeComp* nod, TopoEdge*, boolean);
    virtual boolean IsA(ClassId);
    NodeComp* node;
    TopoEdge* edge;
    boolean start;
};

#endif
