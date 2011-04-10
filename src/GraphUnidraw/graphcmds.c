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

#include <GraphUnidraw/graphclasses.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/graphdata.h>
#include <GraphUnidraw/grapheditor.h>
#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>

#include <TopoFace/topoedge.h>
#include <TopoFace/toponode.h>

#include <Unidraw/Commands/dirty.h>
#include <Unidraw/Components/text.h>
#include <Unidraw/catalog.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/editor.h>
#include <Unidraw/iterator.h>
#include <Unidraw/selection.h>
#include <Unidraw/statevars.h>
#include <Unidraw/ulist.h>
#include <Unidraw/unidraw.h>
#include <Unidraw/viewer.h>

#include <UniIdraw/idarrows.h>

#include <string.h>

/*****************************************************************************/

static boolean selected(Selection* s, NodeComp* comp) {
    Iterator i;
    for (s->First(i); !s->Done(i); s->Next(i)) {
        GraphicComp* gcomp = s->GetView(i)->GetGraphicComp();
	if (gcomp == comp)
	    return true;
    }
    return false;
}

static int node_index (Selection* s, NodeComp *comp) {
    Iterator i;
    int index = -1;

    for (s->First(i); !s->Done(i); s->Next(i)) {
	GraphicComp* tcomp = s->GetView(i)->GetGraphicComp();
	if (tcomp->IsA(NODE_COMP))
	    index++;
        if (tcomp == comp)
	    return index;
    }
    return -1;
}

static NodeComp* node (Clipboard* cb, int index) {
    if (index == -1)
	return nil;

    Iterator i;
    int count = -1;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
	GraphicComp* gcomp = cb->GetComp(i);
	if (gcomp->IsA(NODE_COMP)) {
	    count++;
	    if (count == index)
	        return (NodeComp*)gcomp;
        }
    }
    return nil;
}

static void index_clipboard(Selection* s, Clipboard* cb) {
    Iterator i, j;
    cb->First(j);
    for (s->First(i); !s->Done(i); s->Next(i)) {
        GraphicComp* gcomp = s->GetView(i)->GetGraphicComp();
        if (gcomp->IsA(EDGE_COMP)) {
            GraphicComp* cbgcomp = cb->GetComp(j);
	    EdgeComp* comp = (EdgeComp*)gcomp;
            TopoEdge* topoedge = comp->Edge();
            const TopoNode* node;
            int start = -1;
            int end = -1;
            if ((node = topoedge->start_node()) && selected(s, (NodeComp*)node->value()))
	        start = node_index(s, (NodeComp*)node->value());
            if ((node = topoedge->end_node()) && selected(s, (NodeComp*)node->value()))
	        end = node_index(s, (NodeComp*)node->value());

	    EdgeComp* cbcomp = (EdgeComp*)cbgcomp;
	    cbcomp->SetStartNode(start);
	    cbcomp->SetEndNode(end);
        }
        cb->Next(j);
    }
}

/*****************************************************************************/

EdgeConnectCmd::EdgeConnectCmd(Editor* ed, EdgeComp* ec, 
    NodeComp* nc1, NodeComp* nc2)
    : Command(ed) {
    edge = ec;
    node1 = nc1;
    node2 = nc2;
}

EdgeConnectCmd::~EdgeConnectCmd() {}

ClassId EdgeConnectCmd::GetClassId() { return EDGECONNECT_CMD; }

boolean EdgeConnectCmd::IsA(ClassId id) {
    return id == EDGECONNECT_CMD || Command::IsA(id);
}

boolean EdgeConnectCmd::Reversible() { return true; }

void EdgeConnectCmd::Execute() {
    if (edge)
	edge->Interpret(this);
}

void EdgeConnectCmd::Unexecute() {
    if (edge)
	edge->Uninterpret(this);
}

EdgeUpdateCmd::EdgeUpdateCmd(Editor* ed, EdgeComp* ec)
: Command(ed)
{
    edge = ec;
}

EdgeUpdateCmd::~EdgeUpdateCmd() {}

ClassId EdgeUpdateCmd::GetClassId() { return EDGEUPDATE_CMD; }

boolean EdgeUpdateCmd::IsA(ClassId id) {
    return id == EDGEUPDATE_CMD || Command::IsA(id);
}

boolean EdgeUpdateCmd::Reversible() { return false; }

void EdgeUpdateCmd::Execute() {
    if (edge)
	edge->Interpret(this);
}

NodeTextCmd::NodeTextCmd(Editor* ed, NodeComp* nc, TextGraphic* tg)
: Command(ed)
{
    node = nc;
    text = nil;
    size = 0;
    tgraphic = tg;
}

NodeTextCmd::~NodeTextCmd() {
    delete text;
}

ClassId NodeTextCmd::GetClassId() { return NODETEXT_CMD; }

boolean NodeTextCmd::IsA(ClassId id) {
    return id == NODETEXT_CMD || Command::IsA(id);
}

boolean NodeTextCmd::Reversible() { return false; }

void NodeTextCmd::Execute() {
  if (node) {
    node->Interpret(this);
    DirtyCmd cmd(_editor);
    cmd.Execute();
  }
}

ClassId GraphDeleteCmd::GetClassId () { return GRAPHDELETE_CMD; }

boolean GraphDeleteCmd::IsA (ClassId id) {
    return GRAPHDELETE_CMD == id || DeleteCmd::IsA(id);
}

GraphDeleteCmd::GraphDeleteCmd (ControlInfo* c, Clipboard* cb) : DeleteCmd(c,cb) {
    connections = new UList;
}

GraphDeleteCmd::GraphDeleteCmd (Editor* ed, Clipboard* cb) : DeleteCmd(ed, cb) {
    connections = new UList;
}

GraphDeleteCmd::~GraphDeleteCmd () { 
    if (connections) {
	UList* elt = connections->First();
	while (elt != connections) {
	    GraphData* data = (GraphData*)(*elt)();
	    delete data;
	    elt = elt->Next();
	}
	delete connections;
    }
}

Command* GraphDeleteCmd::Copy () {
    Command* copy = new GraphDeleteCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}


/*****************************************************************************/

ClassId GraphNewViewCmd::GetClassId () { return OVNEWVIEW_CMD; }

boolean GraphNewViewCmd::IsA (ClassId id) {
    return GRAPHNEWVIEW_CMD == id || OvNewViewCmd::IsA(id);
}

GraphNewViewCmd::GraphNewViewCmd (ControlInfo* c) : OvNewViewCmd(c) { }
GraphNewViewCmd::GraphNewViewCmd (Editor* ed) : OvNewViewCmd(ed) { }

Command* GraphNewViewCmd::Copy () {
    Command* copy = new GraphNewViewCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void GraphNewViewCmd::Execute () {
    Editor* ed = GetEditor();
    Editor* newEd = new GraphEditor((OverlayComp*)GetGraphicComp());

    *newEd->GetState("ModifStatusVar") = *ed->GetState("ModifStatusVar");

    unidraw->Open(newEd);
}

/*****************************************************************************/

ClassId GraphCutCmd::GetClassId () { return GRAPHCUT_CMD; }

boolean GraphCutCmd::IsA (ClassId id) { 
    return GRAPHCUT_CMD == id || CutCmd::IsA(id); 
}

GraphCutCmd::GraphCutCmd (ControlInfo* c, Clipboard* cb) : CutCmd(c, cb) {
    _executed = false;
}

GraphCutCmd::GraphCutCmd (Editor* ed, Clipboard* cb) : CutCmd(ed, cb) {
    _executed = false;
}

GraphCutCmd::~GraphCutCmd () {
}

Command* GraphCutCmd::Copy () {
    Command* copy = new GraphCutCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void GraphCutCmd::Execute () {
    Editor* editor = GetEditor();
    Selection* s = editor->GetSelection();
    Clipboard* cb = new Clipboard();
    GraphicView* views = editor->GetViewer()->GetGraphicView();
    s->Sort(views);
    cb->CopyInit(s);
    index_clipboard(s, cb);

    editor->GetComponent()->Interpret(this);

    Iterator i, j, k;
    Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();
    Clipboard* cmdcb = GetClipboard();
    globalcb->First(j);
    cmdcb->First(k);
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
        GraphicComp* gcomp = cb->GetComp(i);
        if (gcomp->IsA(EDGE_COMP)) {
	    GraphicComp* globalgcomp = globalcb->GetComp(j);
	    GraphicComp* cmdgcomp = cmdcb->GetComp(k);
	    EdgeComp* comp = (EdgeComp*)gcomp;
	    EdgeComp* globalcomp = (EdgeComp*)globalgcomp;
	    EdgeComp* cmdcomp = (EdgeComp*)cmdgcomp;
	    globalcomp->SetStartNode(comp->GetStartNode());
	    globalcomp->SetEndNode(comp->GetEndNode());
	    cmdcomp->SetStartNode(comp->GetStartNode());
	    cmdcomp->SetEndNode(comp->GetEndNode());
        }
        globalcb->Next(j);
        cmdcb->Next(k);
    }
    cb->DeleteComps();
    delete cb;

    _executed = true;
}

void GraphCutCmd::Unexecute () {
    GetEditor()->GetComponent()->Uninterpret(this);
    Clipboard* cb = GetClipboard();
    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
        GraphicComp* gcomp = cb->GetComp(i);
        if (gcomp->IsA(EDGE_COMP)) {
	    EdgeComp* comp = (EdgeComp*)gcomp;
            NodeComp* start = node(cb, comp->GetStartNode());
   	    NodeComp* end = node(cb, comp->GetEndNode());
            EdgeConnectCmd* cmd = new EdgeConnectCmd(GetEditor(), comp, start, end);
	    cmd->Execute();
	    delete cmd;
        }
    }
    _executed = false;
}

/*****************************************************************************/

ClassId GraphCopyCmd::GetClassId () { return GRAPHCOPY_CMD; }

boolean GraphCopyCmd::IsA (ClassId id) { 
    return GRAPHCOPY_CMD == id || CopyCmd::IsA(id);
}

GraphCopyCmd::GraphCopyCmd (ControlInfo* c, Clipboard* cb) : CopyCmd(c, cb) { }
GraphCopyCmd::GraphCopyCmd (Editor* ed, Clipboard* cb) : CopyCmd(ed, cb) { }

GraphCopyCmd::~GraphCopyCmd () {
}

Command* GraphCopyCmd::Copy () {
    Command* copy = new CopyCmd(CopyControlInfo());
    InitCopy(copy);
    return copy;
}

void GraphCopyCmd::Execute () {
    Editor* editor = GetEditor();
    Selection* s = editor->GetSelection();
    Clipboard* cb;

    if (!s->IsEmpty()) {
        cb = GetClipboard();
        cb = (cb == nil) ? unidraw->GetCatalog()->GetClipboard() : cb; 

        GraphicView* views = editor->GetViewer()->GetGraphicView();
        s->Sort(views);

        cb->DeleteComps();
        cb->CopyInit(s);
    }
 
    index_clipboard(s, cb);
}

/*****************************************************************************/

ClassId GraphPasteCmd::GetClassId () { return GRAPHPASTE_CMD; }

boolean GraphPasteCmd::IsA (ClassId id) { 
    return GRAPHPASTE_CMD==id || PasteCmd::IsA(id);
}

GraphPasteCmd::GraphPasteCmd (ControlInfo* c, Clipboard* cb) : PasteCmd(c, cb) {
    _executed = false;
}

GraphPasteCmd::GraphPasteCmd (Editor* ed, Clipboard* cb) : PasteCmd(ed, cb) {
    _executed = false;
}

GraphPasteCmd::~GraphPasteCmd () {
}

Command* GraphPasteCmd::Copy () {
    Command* copy = new GraphPasteCmd(CopyControlInfo(), DeepCopyClipboard());
    InitCopy(copy);
    return copy;
}

void GraphPasteCmd::Execute () {
    PasteCmd::Execute();
    Clipboard* cb = GetClipboard();
    Iterator i;
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
        GraphicComp* gcomp = cb->GetComp(i);
        if (gcomp->IsA(EDGE_COMP)) {
	    EdgeComp* comp = (EdgeComp*)gcomp;
            NodeComp* start = node(cb, comp->GetStartNode());
   	    NodeComp* end = node(cb, comp->GetEndNode());
            EdgeConnectCmd* cmd = new EdgeConnectCmd(GetEditor(), comp, start, end);
	    cmd->Execute();
	    delete cmd;
        }
    }
}

void GraphPasteCmd::Unexecute () {
    GetEditor()->GetComponent()->Uninterpret(this);
    _executed = false;
}

boolean GraphPasteCmd::Reversible () {
    Clipboard* cb = GetClipboard();
    Clipboard* globalcb = unidraw->GetCatalog()->GetClipboard();
   
    return (cb != nil && !cb->IsEmpty()) || !globalcb->IsEmpty();
}

/*****************************************************************************/

ClassId GraphDupCmd::GetClassId () { return GRAPHDUP_CMD; }

boolean GraphDupCmd::IsA (ClassId id) {
    return GRAPHDUP_CMD == id || DupCmd::IsA(id);
}

GraphDupCmd::GraphDupCmd (ControlInfo* c, Clipboard* cb) : DupCmd(c, cb) {
    _executed = false;
}

GraphDupCmd::GraphDupCmd (Editor* ed, Clipboard* cb) : DupCmd(ed, cb) {
    _executed = false;
}

GraphDupCmd::~GraphDupCmd () {
}

Command* GraphDupCmd::Copy () {
    Command* copy = new GraphDupCmd(CopyControlInfo(), GetClipboard());
    InitCopy(copy);
    return copy;
}

void GraphDupCmd::Execute () {
    Editor* editor = GetEditor();
    Selection* s = editor->GetSelection();
    Clipboard* cb = new Clipboard();
    GraphicView* views = editor->GetViewer()->GetGraphicView();
    s->Sort(views);
    cb->CopyInit(s);
    index_clipboard(s, cb);

    editor->GetComponent()->Interpret(this);

    Iterator i, k;
    Clipboard* cmdcb = GetClipboard();
    cmdcb->First(k);
    for (cb->First(i); !cb->Done(i); cb->Next(i)) {
        GraphicComp* gcomp = cb->GetComp(i);
        if (gcomp->IsA(EDGE_COMP)) {
	    GraphicComp* cmdgcomp = cmdcb->GetComp(k);
	    EdgeComp* comp = (EdgeComp*)gcomp;
	    EdgeComp* cmdcomp = (EdgeComp*)cmdgcomp;
            NodeComp* start = node(cmdcb, comp->GetStartNode());
   	    NodeComp* end = node(cmdcb, comp->GetEndNode());
            EdgeConnectCmd* cmd = new EdgeConnectCmd(editor, cmdcomp, start, end);
	    cmd->Execute();
	    delete cmd;
        }
        cmdcb->Next(k);
    }
    cb->DeleteComps();
    delete cb;

    _executed = true;
}

void GraphDupCmd::Unexecute () {
    GetEditor()->GetComponent()->Uninterpret(this);
    _executed = false;
}
