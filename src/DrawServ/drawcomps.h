/*
 * Copyright (c) 1994-1996,1999 Vectaport Inc.
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
 * Draw components
 */

#ifndef drawcomps_h
#define drawcomps_h

#include <FrameUnidraw/framecomps.h>
#include <FrameUnidraw/framescripts.h>

class EdgeComp;

//: top-level component for a drawserv document.
class DrawIdrawComp : public FrameIdrawComp {
public:
    DrawIdrawComp(const char* pathname = nil, OverlayComp* parent = nil);
    DrawIdrawComp(istream&, const char* pathname = nil, OverlayComp* parent = nil);
    virtual ~DrawIdrawComp();

    virtual Component* Copy();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    int GetNumEdge() { return _num_edge; }
    // number of edges in graph.
    int GetNumNode() { return _num_node; }
    // number of nodes in graph.

    void AppendEdge(EdgeComp*);
    // add edge component to the graph.

    UList* GraphEdges() { return _graphedges; }

protected:
    UList* _graphedges;
    int _num_edge;
    int _num_node;

    ParamList* GetParamList();
    void GrowParamList(ParamList* pl);
    static ParamList* _com_idraw_params;
};

//: serialized view of a DrawIdrawComp.
class DrawIdrawScript : public FrameIdrawScript {
public:
    DrawIdrawScript(DrawIdrawComp* = nil);
    virtual ~DrawIdrawScript();

    virtual boolean Emit(ostream&);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    static int ReadFrames(istream& in, void* addr1, void* addr2, void* addr3, void* addr4);

    virtual const char* script_name() { return "drawserv"; }
    // for overriding in derived classes
};


#endif



