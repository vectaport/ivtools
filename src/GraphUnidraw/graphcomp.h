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

#ifndef graphcomp_h
#define graphcomp_h

#include <OverlayUnidraw/ovviews.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/scriptview.h>

#include <Unidraw/Components/psview.h>

class EdgeComp;
class NodeComp;
class UList;

class GraphComp : public OverlaysComp {
public:
    GraphComp(const char* pathname = nil, OverlayComp* parent = nil);
    GraphComp(Graphic*, OverlayComp* parent = nil);
    GraphComp(istream&, const char* pathname = nil, OverlayComp* parent = nil);
    virtual ~GraphComp();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    Component* Copy();
 
    void AppendEdge(EdgeComp*);
    UList* GraphEdges();

    virtual void GrowIndexedGS(Graphic*);
    virtual Graphic* GetIndexedGS(int);

    int GetNumEdge() { return _num_edge; }
    int GetNumNode() { return _num_node; }

    virtual void SetPathName(const char*);
    virtual const char* GetPathName();
    virtual const char* GetBaseDir();
    virtual const char* GetFile();

protected:
    UList* _graphedges;
    int _num_edge;
    int _num_node;

    char* _pathname;
    char* _basedir;
    char* _file;

    Picture* _gslist;

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _graph_params;
};

inline UList* GraphComp::GraphEdges() { return _graphedges; }

class GraphView : public OverlaysView {
public:
    GraphView(GraphComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class GraphScript : public OverlaysScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    GraphScript(GraphComp* = nil);

    virtual void SetGSList(Clipboard*);
    virtual Clipboard* GetGSList();

    static int ReadChildren(istream&, void*, void*, void*, void*);
    virtual boolean Definition(ostream&);

protected:
    Clipboard* _gslist;
};

/*****************************************************************************/

class GraphIdrawComp : public OverlayIdrawComp {
public:
    GraphIdrawComp(const char* pathname = nil, OverlayComp* parent = nil);
    GraphIdrawComp(istream&, const char* pathname = nil, OverlayComp* parent = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    virtual void Interpret(Command*);
    void Ungroup(OverlayComp*, Clipboard*, Command*);

    Component* Copy();
 
    int GetNumEdge() { return _num_edge; }
    int GetNumNode() { return _num_node; }

protected:
    int _num_edge;
    int _num_node;

protected:
    ParamList* GetParamList();
    void GrowParamList(ParamList*);
    static ParamList* _graph_idraw_params;
};

class GraphIdrawView : public OverlayIdrawView {
public:
    GraphIdrawView(GraphIdrawComp* = nil);

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class GraphIdrawScript : public OverlayIdrawScript {
public:
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
    GraphIdrawScript(GraphIdrawComp* = nil);

    static int ReadChildren(istream&, void*, void*, void*, void*);
    virtual boolean Emit(ostream&);
};

#endif




