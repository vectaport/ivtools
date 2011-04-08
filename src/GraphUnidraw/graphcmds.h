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

#ifndef graphcmds_h
#define graphcmds_h

#include <Unidraw/Commands/command.h>
#include <Unidraw/Commands/edit.h>
#include <OverlayUnidraw/ovcmds.h>

class EdgeComp;
class NodeComp;
class TextGraphic;

class EdgeConnectCmd : public Command {
public:
    EdgeConnectCmd(Editor* =nil, EdgeComp* =nil, NodeComp* =nil, NodeComp* =nil);
    virtual ~EdgeConnectCmd();

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    EdgeComp* Edge() { return edge; }
    NodeComp* Node1() { return node1; }
    NodeComp* Node2() { return node2; }
protected:
    EdgeComp* edge;
    NodeComp* node1;
    NodeComp* node2;
};

class EdgeUpdateCmd : public Command {
public:
    EdgeUpdateCmd(Editor* =nil, EdgeComp* =nil);
    virtual ~EdgeUpdateCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    EdgeComp* Edge() { return edge; }
protected:
    EdgeComp* edge;
};

class NodeTextCmd : public Command {
public:
//    NodeTextCmd(Editor* =nil, NodeComp* =nil, const char* =nil, int =0);
    NodeTextCmd(Editor* =nil, NodeComp* =nil, TextGraphic* =nil);
    virtual ~NodeTextCmd();

    virtual void Execute();
    virtual boolean Reversible();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);

    NodeComp* Node() { return node; }
    const char* Text() { return text; }
    int Size() { return size; }
    TextGraphic* Graphic() { return tgraphic; }
protected:
    NodeComp* node;
    char* text;
    int size;
    TextGraphic* tgraphic;
};

class GraphDeleteCmd : public DeleteCmd {
public:
    GraphDeleteCmd(ControlInfo*, Clipboard* = nil);
    GraphDeleteCmd(Editor* = nil, Clipboard* = nil);
    virtual ~GraphDeleteCmd();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
public:
    UList* connections;
};

class GraphNewViewCmd : public OvNewViewCmd {
public:
    GraphNewViewCmd(ControlInfo*);
    GraphNewViewCmd(Editor* = nil);

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class GraphCutCmd : public CutCmd {
public:
    GraphCutCmd(ControlInfo*, Clipboard* = nil);
    GraphCutCmd(Editor* = nil, Clipboard* = nil);
    virtual ~GraphCutCmd();

    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class GraphCopyCmd : public CopyCmd {
public:
    GraphCopyCmd(ControlInfo*, Clipboard* = nil);
    GraphCopyCmd(Editor* = nil, Clipboard* = nil);
    virtual ~GraphCopyCmd();

    virtual void Execute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class GraphPasteCmd : public PasteCmd {
public:
    GraphPasteCmd(ControlInfo*, Clipboard* = nil);
    GraphPasteCmd(Editor* = nil, Clipboard* = nil);
    virtual ~GraphPasteCmd();

    virtual void Execute();
    virtual void Unexecute();
    virtual boolean Reversible();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

class GraphDupCmd : public DupCmd {
public:
    GraphDupCmd(ControlInfo*, Clipboard* = nil);
    GraphDupCmd(Editor* = nil, Clipboard* = nil);
    virtual ~GraphDupCmd();

    virtual void Execute();
    virtual void Unexecute();

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
};

#endif
