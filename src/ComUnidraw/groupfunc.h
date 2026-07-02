/*
 * Copyright (c) 2000 IET Inc.
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

#if !defined(_groupfunc_h)
#define _groupfunc_h

#include <ComUnidraw/unifunc.h>

//: command to add graphic to existing group graphic
// newgrp=growgroup(groupview compview) -- add graphic to existing group graphic
class GrowGroupFunc : public UnidrawFunc {
public:
    GrowGroupFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "newgrp=%s(groupview compview) -- add graphic to existing group graphic"; }
};

//: command to remove graphic from existing group graphic
// newgrp=trimgroup(groupview compview) -- remove graphic from existing group graphic
class TrimGroupFunc : public UnidrawFunc {
public:
    TrimGroupFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() {
	return "newgrp=%s(groupview compview) -- remove graphic from existing group graphic"; }
};

//: command to group the current selection into a single group graphic
// newgrp=group() -- group the current selection into one group graphic
class GroupFunc : public UnidrawFunc {
public:
    GroupFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() {
	return "newgrp=%s() -- group the current selection into a single group graphic"; }
};

//: command to ungroup a group graphic back into its members
// ungroup([compview]) -- dissolve the selected group (or the given group) into its members
class UngroupFunc : public UnidrawFunc {
public:
    UngroupFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() {
	return "%s([compview]) -- ungroup the selected group (or given group) into its members"; }
};

//: command to move selection or graphic to the front
// front() -- move selection or graphic to the front
class FrontSelectionFunc : public UnidrawFunc {
public:
    FrontSelectionFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([compview]) -- move selection or graphic to the front"; }
};

//: command to move selection or graphic to the back
// back() -- move seledction or graphic to the back
class BackSelectionFunc : public UnidrawFunc {
public:
    BackSelectionFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s([compview]) -- move selection or graphic to the back"; }
};

#endif /* !defined(_groupfunc_h) */
