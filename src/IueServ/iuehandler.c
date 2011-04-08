/*
 * Copyright (c) 1998 Vectaport Inc.
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

#include <IueServ/exportfunc.h>
#include <IueServ/ipfunc.h>
#include <IueServ/iuehandler.h>
#include <IueServ/iuefunc.h>
#include <IueServ/roifunc.h>
#include <ComTerp/comterpserv.h>

#include <iostream.h>

/*****************************************************************************/

// Default constructor.

IueHandler::IueHandler (void)
{
  comterp_->add_command("image", new IueImageFunc(comterp_));
  comterp_->add_command("getpix", new IueGetPixelFunc(comterp_));
  comterp_->add_command("ncols", new IueNcolsFunc(comterp_));
  comterp_->add_command("nrows", new IueNrowsFunc(comterp_));
  comterp_->add_command("export", new IueExportFunc(comterp_));
  comterp_->add_command("pixtype", new IuePixTypeFunc(comterp_));
  comterp_->add_command("gaussian", new IueGaussianFunc(comterp_));
  comterp_->add_command("threshold", new IueThresholdFunc(comterp_));
  comterp_->add_command("conncomp", new IueConnCompFunc(comterp_));
}

