/*
 * Copyright (c) 2004 Scott E. Johnston
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

#if !defined(_drawfunc_h)
#define _drawfunc_h

#include <DrawServ/draweditor.h>
#include <ComUnidraw/unifunc.h>

//: command to connect to another drawserv
// drawlink([hoststr] :port portnum :state num :lid nu :rid num :close :dump) -- connect to remote drawserv
class DrawLinkFunc : public UnidrawFunc {
public:
    DrawLinkFunc(ComTerp*,DrawEditor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([hoststr] :port portnum :state num :lid num :rid num :close :dump) -- connect to remote drawserv"; }
};

//: command to reserve unique session id
// sid([sid osid :pid pid :user namestr :host hoststr :hostid hostid :remap] |  :all) -- command to manage session id's
class SessionIdFunc : public UnidrawFunc {
public:
    SessionIdFunc(ComTerp*,DrawEditor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([sid osid :pid pid :user namestr :host hoststr :hostid hostid :remap] | :all) -- command to manage session id's"; }
};

//: command to send message between remote selections
// grid(id selector :state selected :request newselector :grant oldselector) -- command to send message between remote selections
class GraphicIdFunc : public UnidrawFunc {
public:
    GraphicIdFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(id selector :state selected :request newselector :grant oldselector) -- command to send message between remote selections"; }
};

//: command to change session (or graphic id) to use local session id
// chgid(id) -- command to change session (or graphic id) to use local session id
class ChangeIdFunc : public UnidrawFunc {
public:
    ChangeIdFunc(ComTerp*,DrawEditor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(id) -- command to change session (or graphic id) to use local session id"; }
};

#endif /* !defined(_drawfunc_h) */

