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

#if !defined(_dialog_func_h)
#define _dialog_func_h

#include <ComUnidraw/unifunc.h>

//: command to popup an acknowledge dialog box
// acknowledgebox() -- popup an acknowledge dialog box
class AcknowledgeBoxFunc : public UnidrawFunc {
public:
    AcknowledgeBoxFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(message_str) -- popup an acknowledge dialog box"; }

};

//: command to popup a confirmation dialog box
// acknowledgebox() -- popup a confirmation dialog box, and return 1, 0, or -1 if cancelled.
class ConfirmBoxFunc : public UnidrawFunc {
public:
    ConfirmBoxFunc(ComTerp*,Editor*);
    virtual void execute();
    virtual const char* docstring() { 
	return "%s(message_str) -- popup a confirmation dialog box, and return 1, 0, or -1 if cancelled"; }

};

#endif /* !defined(_dialog_func_h) */
