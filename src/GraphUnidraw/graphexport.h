/*
 * Copyright (c) 2012 Wave Semiconductor Inc.
 * Copyright (c) 1994,1999 Vectaport Inc.
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
 * GraphExportCmd - a command for exporting graphical objects
 */

#ifndef graph_export_h
#define graph_export_h

#include <OverlayUnidraw/ovexport.h>

//: command to export graph selection in idraw "PostScript" or drawtool serialized format or GraphViz dot format.
class GraphExportCmd : public OvExportCmd {
public:
    GraphExportCmd(ControlInfo*, ExportChooser* = nil);
    GraphExportCmd(Editor* = nil, ExportChooser* = nil);
    virtual ~GraphExportCmd();

    virtual void Execute();
    // present ExportChooser to specify pathname, export flags, and export mode.
    virtual boolean Export(const char* path);
    // export by constructing appropriate ExternView for idraw or drawtool format,
    // and using it to transcribe the contents of the selected components to 'path'.

    virtual Command* Copy();
    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);


    virtual boolean dot_format();
    // true when format is "dot".

};

#endif
