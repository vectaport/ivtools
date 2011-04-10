/*
 * Copyright (c) 1998 Vectaport Inc.
 * Copyright (c) 1997 Vectaport Inc. and R.B. Kissh & Associates
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

#define NDEBUG

/*
 * Implementation of ip commands
 */

#include <OS/math.h>
#include <OS/string.h>

#include <OverlayUnidraw/ovdialog.h> // putting this first works for Alpha/egcs
#include <OverlayUnidraw/ovipcmds.h>

#include <OverlayUnidraw/grayraster.h>
#include <OverlayUnidraw/ovcatalog.h>
#include <OverlayUnidraw/ovclasses.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovcomps.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovpage.h>
#include <OverlayUnidraw/ovselection.h>
#include <OverlayUnidraw/ovunidraw.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/ovimport.h>
#include <OverlayUnidraw/ovraster.h>
#include <OverlayUnidraw/rastercmds.h>

#include <ComTerp/comvalue.h>

#include <Attribute/aliterator.h>
#include <Attribute/attrlist.h>

#include <Unidraw/Components/grview.h>
#include <Unidraw/clipboard.h>
#include <Unidraw/grid.h>
#include <Unidraw/iterator.h>
#include <Unidraw/statevars.h>
#include <Unidraw/viewer.h>
#include <Unidraw/globals.h>

#include <IVGlyph/gdialogs.h>
#include <IVGlyph/stredit.h>

#include <InterViews/cursor.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <InterViews/window.h>
#include <IV-X11/xwindow.h>

#include <IV-2_6/InterViews/perspective.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <strstream>


RasterTerp::RasterTerp(Editor* ed) : _editor(ed) {
  add_command(ScaleGrayFunc::Tag(), new ScaleGrayFunc(this));
  add_command(PseudocolorFunc::Tag(), new PseudocolorFunc(this));
  add_command(LogScaleFunc::Tag(), new LogScaleFunc(this));
  add_command(GrayRampFunc::Tag(), new GrayRampFunc(this));
}


RasterTerp::~RasterTerp() {
}


int RasterTerp::execute(
    RasterOvComp* comp, const CopyString& exp
) {
#ifndef NDEBUG
    cerr << "RasterTerp::execute: " << exp.string() << endl;
#endif

    RasterFunc::SetComp(comp);

    // #### ignored for now
    const ComValue& result = run(exp.string());
    return 0;
}


Editor* RasterTerp::editor() {
    return _editor;
}

// -----------------------------------------------------------------------

Clipboard RasterFunc::_comps;


RasterFunc::RasterFunc(
    ComTerp* c
)
    : ComFunc(c)
{
}


RasterFunc::~RasterFunc() {
}


/* static */ void RasterFunc::SetComp(RasterOvComp* comp) {
    _comps.Clear();
    _comps.Append(comp);
}

// -----------------------------------------------------------------------


ScaleGrayFunc::ScaleGrayFunc(
    RasterTerp* comterp
) 
    : RasterFunc(comterp), _rterp(comterp)
{
}


/* static */ const char* ScaleGrayFunc::Tag() {
    return "scale";
}


/* static */ const char* ScaleGrayFunc::CommandString(
    ColorIntensity mingray, ColorIntensity maxgray
) {
    sprintf(sbuf,"%s(%.3f %.3f)", Tag(), mingray, maxgray);
    return sbuf;
}


void ScaleGrayFunc::execute() {

    ComValue mingrayv(stack_arg(0));
    ComValue maxgrayv(stack_arg(1));
    reset_stack();

    if (mingrayv.is_num() && maxgrayv.is_num()) {

        float mingray = mingrayv.double_val();
        float maxgray = maxgrayv.double_val();

#ifndef NDEBUG
cerr << "mingray: " << mingray << ", maxgray: " << maxgray << endl;
#endif

        ScaleGrayCmd* cmd = new ScaleGrayCmd(_rterp->editor(), mingray, maxgray);
        cmd->SetClipboard(_comps.Copy());
        cmd->Execute();

        _comps.Clear();
        cmd->GetResult(_comps);

        if (cmd->Reversible()) {
            cmd->Log();
        }
        else {
            delete cmd;
        }
    }
    else {
        push_stack(ComValue::nullval());
    }
} 


// -----------------------------------------------------------------------

PseudocolorFunc::PseudocolorFunc(
    RasterTerp* comterp
) 
    : RasterFunc(comterp), _rterp(comterp)
{
}


/* static */ const char* PseudocolorFunc::Tag() {
    return "pseudocolor";
}


/* static */ const char* PseudocolorFunc::CommandString(
    ColorIntensity mingray, ColorIntensity maxgray
) {
    sprintf(sbuf,"%s(%.3f %.3f)", Tag(), mingray, maxgray);
    return sbuf;
}


void PseudocolorFunc::execute() {

    ComValue mingrayv(stack_arg(0));
    ComValue maxgrayv(stack_arg(1));
    reset_stack();

    if (mingrayv.is_num() && maxgrayv.is_num()) {

        float mingray = mingrayv.double_val();
        float maxgray = maxgrayv.double_val();

#ifndef NDEBUG
cerr << "mingray: " << mingray << ", maxgray: " << maxgray << endl;
#endif

        PseudocolorCmd* cmd = new PseudocolorCmd(
            _rterp->editor(), mingray, maxgray
        );
        cmd->SetClipboard(_comps.Copy());
        cmd->Execute();

        _comps.Clear();
        cmd->GetResult(_comps);

        if (cmd->Reversible()) {
            cmd->Log();
        }
        else {
            delete cmd;
        }
    }
    else {
        push_stack(ComValue::nullval());
    }
} 


// -----------------------------------------------------------------------

LogScaleFunc::LogScaleFunc(
    RasterTerp* comterp
) 
    : RasterFunc(comterp), _rterp(comterp)
{
}


/* static */ const char* LogScaleFunc::Tag() {
    return "logscale";
}


/* static */ const char* LogScaleFunc::CommandString(
    ColorIntensity mingray, ColorIntensity maxgray)
{
    sprintf(sbuf,"%s(%.3f %.3f)", Tag(), mingray, maxgray);
    return sbuf;
}


void LogScaleFunc::execute() {

    ComValue mingrayv(stack_arg(0));
    ComValue maxgrayv(stack_arg(1));
    reset_stack();

    if (mingrayv.is_num() && maxgrayv.is_num())
      {
        float mingray = mingrayv.double_val();
        float maxgray = maxgrayv.double_val();

#ifndef NDEBUG
cerr << "mingray: " << mingray << ", maxgray: " << maxgray << endl;
#endif

        LogScaleCmd* cmd = new LogScaleCmd(_rterp->editor(), mingray, maxgray);
        cmd->SetClipboard(_comps.Copy());
        cmd->Execute();

        _comps.Clear();
        cmd->GetResult(_comps);

        if (cmd->Reversible()) {
            cmd->Log();
        }
        else {
            delete cmd;
        }
    }
    else {
        push_stack(ComValue::nullval());
    }
} 


// -----------------------------------------------------------------------


const char* GrayRampFunc::rpos[] = 
    { "LB", "LT", "TL", "TR", "RT", "RB", "BR", "BL", nil };


GrayRampFunc::GrayRampFunc(
    RasterTerp* comterp
) 
    : RasterFunc(comterp), _rterp(comterp)
{
}


/* static */ const char* GrayRampFunc::Tag() {
    return "grayramp";
}


/* static */ const char* GrayRampFunc::CommandString(
    RampAlignment align
) {
    sprintf(sbuf,"%s(\\\"%s\\\")", Tag(), rpos[align]);
    return sbuf;
}


void GrayRampFunc::execute() {

    ComValue alignv(stack_arg(0));

    reset_stack();

    if (
        alignv.type() == ComValue::StringType
    ) {
        const char* align = alignv.string_ptr();

#ifndef NDEBUG
cerr << "align:" << align << "\n";
#endif

        int i = 0;
        while (rpos[i] != nil && strcmp(align, rpos[i])) {
          i++;
        }

        if (rpos[i] != nil) {

            GrayRampCmd* cmd = new GrayRampCmd(
                _rterp->editor(), RampAlignment(i)
            );

            cmd->SetClipboard(_comps.Copy());
            cmd->Execute();

            _comps.Clear();
            cmd->GetResult(_comps);

            if (cmd->Reversible()) {
                cmd->Log();
            }
            else {
                delete cmd;
            }
        }
        else {
            push_stack(ComValue::nullval());
	}
    }
    else {
        push_stack(ComValue::nullval());
    }
}


// -----------------------------------------------------------------------


ClassId ImageCmd::GetClassId () { return IMAGE_CMD; }

boolean ImageCmd::IsA (ClassId id) {
    return IMAGE_CMD == id || Command::IsA(id);
}


ImageCmd::ImageCmd (ControlInfo* c) 
    : Command(c) {
}


ImageCmd::ImageCmd () 
    : Command((Editor*)nil) 
{ 
}


ImageCmd::ImageCmd (Editor* ed, const CopyString& str) 
    : Command(ed), _cstr(str) 
{ 
}


ImageCmd::ImageCmd (ControlInfo* c, const CopyString& str) 
    : Command(c), _cstr(str) 
{
}


ImageCmd::~ImageCmd() {
}


Command* ImageCmd::Copy () {
    ImageCmd* copy = new ImageCmd(
        CopyControlInfo(), _cstr
    );
    InitCopy(copy);
    return copy;
}


const CopyString& ImageCmd::Cmd() const {
    return _cstr;
}


// -------------------------------------------------------------------------


ProcessingCmd::ProcessingCmd (ControlInfo* c) 
    : MacroCmd(c), _prepared(false), _comps(new Clipboard)
{ 
  _reversible = true;
}

ProcessingCmd::ProcessingCmd (Editor* ed)
    : MacroCmd(ed), _prepared(false), _comps(new Clipboard) 
{
  _reversible = true;
}

ProcessingCmd::~ProcessingCmd() {
    delete _comps;
}

void ProcessingCmd::Execute () {

    boolean do_something = false;

    if (!_prepared) {
        Selection* s = _editor ? _editor->GetSelection() : nil;
        Clipboard* cb = GetClipboard();


	/* grab the foremost raster if none is selected */
	if (!_ed_constructor && 
	    (!s || s->IsEmpty()) && (!cb || cb->IsEmpty())) {
	  OverlaysView* views = ((OverlayEditor*)GetEditor())->GetFrame();
	  Iterator i;
	  for (views->Last(i); !views->Done(i); views->Prev(i)) {
	    GraphicView* view = views->GetView(i);
	    if (view->IsA(OVRASTER_VIEW)) {
	      if (!cb)
		SetClipboard(cb = new Clipboard);
	      cb->Append(view->GetGraphicComp());
	      break;
	    }
	  }
	}
	

        if ( (s && !s->IsEmpty()) || (cb != nil && !cb->IsEmpty())) {
            Iterator i;

            if (cb == nil) {
                SetClipboard(cb = new Clipboard);
                cb->Init(s);
            }

            for (cb->First(i); !cb->Done(i); cb->Next(i)) {
                do_something = do_something || 
		  PrepareToExecute(cb->GetComp(i));
            }
        }

        _prepared = true;
    }

    if (do_something) {
      MacroCmd::Execute();
      unidraw->Update(); // for safety
    } else {
      _reversible = false;
      GAcknowledgeDialog::post(GetEditor()->GetWindow(),
			       "no gray-level raster found");
    }
}


boolean ProcessingCmd::PrepareToExecute(GraphicComp* comp) {

    if (comp->IsA(OVRASTER_COMP)) {
        RasterOvComp* rcomp = (RasterOvComp*)comp;
        OverlayRasterRect* rr = rcomp->GetOverlayRasterRect();
        OverlayRaster* orast = rr->GetOverlayRaster();

        if (orast) {

            // this is a kludge
            if (!orast->initialized()) {
                rr->load_image(rcomp->GetPathName());
            }

            CopyString scmd;
            OverlayRaster* newrast = Process(orast, scmd);

            if (newrast) {  // this rast understands the command 

                Command* cmd = new ReplaceRasterCmd(
                    GetEditor(), rcomp, newrast
                );
                Append(cmd);

                cmd = new ImageCmd(GetEditor(), scmd);
                cmd->SetClipboard(new Clipboard(rcomp));
                Append(cmd);

                _comps->Append(rcomp);
		return true;
    	    }
            else {
                // do nothing
                _comps->Append(comp);
		return false;
            }
        }
        else {
            // do nothing
            _comps->Append(comp);
	    return false;
        }
    }
    return false;
}


OverlayRaster* ProcessingCmd::Process(OverlayRaster* rast, CopyString& scmd) {
  return nil;
}


void ProcessingCmd::GetResult(Clipboard& clip) const {
    Iterator i;
    for (_comps->First(i); !_comps->Done(i); _comps->Next(i)) {
        clip.Append(_comps->GetComp(i));
    }
}

boolean ProcessingCmd::Reversible () {
  return _reversible;
}


// -------------------------------------------------------------------------


ClassId ScaleGrayCmd::GetClassId() { return SCALEGRAY_CMD; }

boolean ScaleGrayCmd::IsA(ClassId id) {
    return SCALEGRAY_CMD == id || ProcessingCmd::IsA(id);
}

ScaleGrayCmd::ScaleGrayCmd(
    ControlInfo* c, ColorIntensity mingray, ColorIntensity maxgray
) 
    : ProcessingCmd(c), _mingray(mingray), _maxgray(maxgray)
{
  _ed_constructor = false;
}


ScaleGrayCmd::ScaleGrayCmd(
    ControlInfo* c
) 
    : ProcessingCmd(c), _mingray(0.0), _maxgray(-1.0)
{
  _ed_constructor = false;
}


ScaleGrayCmd::ScaleGrayCmd (
    Editor* ed, ColorIntensity mingray, ColorIntensity maxgray
)
    : ProcessingCmd(ed), _mingray(mingray), _maxgray(maxgray)
{ 
  _ed_constructor = true;
}


ScaleGrayCmd::ScaleGrayCmd (
    Editor* ed
)
    : ProcessingCmd(ed), _mingray(0.), _maxgray(-1.0)
{ 
  _ed_constructor = true;
}


ScaleGrayCmd::~ScaleGrayCmd() {
}


Command* ScaleGrayCmd::Copy () {
    ScaleGrayCmd* copy = new ScaleGrayCmd(
        CopyControlInfo(), _mingray, _maxgray
    );
    InitCopy(copy);
    copy->_ed_constructor = _ed_constructor;
    return copy;
}

OverlayRaster* ScaleGrayCmd::Process(OverlayRaster* rast, CopyString& scmd) {
  if (_maxgray<_mingray || !_ed_constructor) {
    char* newminmax = StrEditDialog::post
      (GetEditor()->GetWindow(), 
       "Enter min and max for linear scaling of gray values",
       "0.0 1.0");
    if (newminmax) {
      std::istrstream in(newminmax);
      float fmin, fmax;
      in >> fmin >> fmax;
      if (in.good()) {
	_mingray = fmin;
	_maxgray = fmax;
      }
      delete newminmax;
      GetEditor()->GetWindow()->cursor(hourglass);
      return rast->scale(_mingray, _maxgray, scmd);
    }
  } else {
    GetEditor()->GetWindow()->cursor(hourglass);
    return rast->scale(_mingray, _maxgray, scmd);
  }
  return nil;
}

// -------------------------------------------------------------------------


ClassId PseudocolorCmd::GetClassId() { return PSEUDOCOLOR_CMD; }

boolean PseudocolorCmd::IsA(ClassId id) {
    return PSEUDOCOLOR_CMD == id || ProcessingCmd::IsA(id);
}


PseudocolorCmd::PseudocolorCmd(
    ControlInfo* c, ColorIntensity mingray, ColorIntensity maxgray
) 
  : ProcessingCmd(c), _mingray(mingray), _maxgray(maxgray)
{
  _ed_constructor = false;
}


PseudocolorCmd::PseudocolorCmd (
    ControlInfo* c
)
  : ProcessingCmd(c), _mingray(0.), _maxgray(-1.0)

{ 
  _ed_constructor = false;
}


PseudocolorCmd::PseudocolorCmd (
    Editor* ed, ColorIntensity mingray, ColorIntensity maxgray
)
  : ProcessingCmd(ed), _mingray(mingray), _maxgray(maxgray)
{ 
  _ed_constructor = true;
}


PseudocolorCmd::PseudocolorCmd (
    Editor* ed
)
  : ProcessingCmd(ed), _mingray(0.), _maxgray(-1.0)
{ 
  _ed_constructor = true;
}


PseudocolorCmd::~PseudocolorCmd() {
}


Command* PseudocolorCmd::Copy () {
    PseudocolorCmd* copy = new PseudocolorCmd(
        CopyControlInfo(), _mingray, _maxgray
    );
    InitCopy(copy);
    copy->_ed_constructor = _ed_constructor;
    return copy;
}


OverlayRaster* PseudocolorCmd::Process(OverlayRaster* rast, CopyString& scmd) {
  if (_maxgray<_mingray || !_ed_constructor) {
    const char* message = rast->grayraster() 
      && AttributeValue::is_float(((GrayRaster*)rast)->value_type())
      ? "Enter actual min and max for pseudo coloring of gray values"
      : "Enter min and max for pseudo coloring of gray values";
    const char* range = rast->grayraster() 
      && AttributeValue::is_float(((GrayRaster*)rast)->value_type())
      ? "0.5 2.0"
      : "0.0 1.0";
    char* newminmax = StrEditDialog::post
      (GetEditor()->GetWindow(), message, range);
    if (newminmax) {
      std::istrstream in(newminmax);
      float fmin, fmax;
      in >> fmin >> fmax;
      if (in.good()) {
	_mingray = fmin;
	_maxgray = fmax;
      }
      delete newminmax;
      GetEditor()->GetWindow()->cursor(hourglass);
      return rast->pseudocolor(_mingray, _maxgray, scmd);
    }
  } else {
    GetEditor()->GetWindow()->cursor(hourglass);
    return rast->pseudocolor(_mingray, _maxgray, scmd);
  }
  return nil;
}


// -------------------------------------------------------------------------


ClassId LogScaleCmd::GetClassId() { return LOGSCALE_CMD; }

boolean LogScaleCmd::IsA(ClassId id) {
    return LOGSCALE_CMD == id || ProcessingCmd::IsA(id);
}


LogScaleCmd::LogScaleCmd(
    ControlInfo* c, ColorIntensity mingray, ColorIntensity maxgray
) 
  : ProcessingCmd(c), _mingray(mingray), _maxgray(maxgray) 
{
  _ed_constructor = false;
}


LogScaleCmd::LogScaleCmd(
    ControlInfo* c
) 
  : ProcessingCmd(c), _mingray(0.0), _maxgray(-1.0)
{
  _ed_constructor = false;
}


LogScaleCmd::LogScaleCmd (
    Editor* ed, ColorIntensity mingray, ColorIntensity maxgray
)
  : ProcessingCmd(ed), _mingray(mingray), _maxgray(maxgray)
{ 
  _ed_constructor = true;
}

LogScaleCmd::LogScaleCmd (
    Editor* ed
)
  : ProcessingCmd(ed), _mingray(0.0), _maxgray(-1.0)
{ 
  _ed_constructor = true;
}


LogScaleCmd::~LogScaleCmd() {
}

Command* LogScaleCmd::Copy () {
    LogScaleCmd* copy = new LogScaleCmd(
        CopyControlInfo(), _mingray, _maxgray
    );
    InitCopy(copy);
    copy->_ed_constructor = _ed_constructor;
    return copy;
}


OverlayRaster* LogScaleCmd::Process(OverlayRaster* rast, CopyString& scmd) {
  if (_maxgray<_mingray || !_ed_constructor) {
    char* newminmax = StrEditDialog::post
      (GetEditor()->GetWindow(), 
       "Enter min and max for logarithmic scaling of gray values",
       "0.0 1.0");
    if (newminmax) {
      std::istrstream in(newminmax);
      float fmin, fmax;
      in >> fmin >> fmax;
      if (in.good()) {
	_mingray = fmin;
	_maxgray = fmax;
      }
      delete newminmax;
      GetEditor()->GetWindow()->cursor(hourglass);
      return rast->logscale(_mingray, _maxgray, scmd);
    }
  } else {
    GetEditor()->GetWindow()->cursor(hourglass);
    return rast->logscale(_mingray, _maxgray, scmd);
  }
  return nil;
}


// -------------------------------------------------------------------------


ClassId GrayRampCmd::GetClassId() { return GRAYRAMP_CMD; }

boolean GrayRampCmd::IsA(ClassId id) {
    return GRAYRAMP_CMD == id || ProcessingCmd::IsA(id);
}


GrayRampCmd::GrayRampCmd(
    ControlInfo* c, IntCoord x, IntCoord y
) 
    : ProcessingCmd(c), _x(x), _y(y), _use_align(false)
{
}

GrayRampCmd::GrayRampCmd(
    ControlInfo* c, RampAlignment align
) 
  : ProcessingCmd(c), _align(align), _use_align(true)
{
}


GrayRampCmd::GrayRampCmd (
    Editor* ed, IntCoord x, IntCoord y
)
  : ProcessingCmd(ed), _x(x), _y(y), _use_align(false)
{ 
}


GrayRampCmd::GrayRampCmd (
    Editor* ed, RampAlignment align
)
  : ProcessingCmd(ed), _align(align), _use_align(true)
{ 
}


GrayRampCmd::~GrayRampCmd() {
}


Command* GrayRampCmd::Copy () {
    GrayRampCmd* copy = nil;
    if (_use_align) {
        copy = new GrayRampCmd(CopyControlInfo(), _align);
    }
    else {
        copy = new GrayRampCmd(CopyControlInfo(), _x, _y);
    }
    InitCopy(copy);
    return copy;
}


OverlayRaster* GrayRampCmd::Process(OverlayRaster* rast, CopyString& scmd) {
    if (_use_align) {
        return rast->addgrayramp(scmd, _align);
    }
    else {
        return rast->addgrayramp(scmd, _x, _y);
    }
}
