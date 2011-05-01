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

#include <InterViews/enter-scope.h>
#include <Time/Time_.h>
#include <Time/Date.h>
#include <Time/obstime.h>
#include <Time/timeglyph.h>
#include <IVGlyph/bdvalue.h>
#include <IVGlyph/valuator.h>
#include <IV-look/kit.h>
#include <InterViews/action.h>
#include <InterViews/event.h>
#include <InterViews/hit.h>
#include <InterViews/layout.h>
#include <InterViews/style.h>
#include <OS/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

declareActionCallback(ObservableTime)
implementActionCallback(ObservableTime)
declareActionCallback(BoundedValue)
implementActionCallback(BoundedValue)
declareActionCallback(TimeGlyph)
implementActionCallback(TimeGlyph)

TimeGlyph::TimeGlyph(Style* s, ObservableTime* tm, boolean editable)
: MonoGlyph(nil)
{
    if (!tm)
	time_ = new ObservableTime();
    else
	time_ = tm;

    time_->attach(this);
    WidgetKit& kit_ = *WidgetKit::instance();
    const LayoutKit& layout_ = *LayoutKit::instance();

    Date date(time_->time()->date());
    StringList* wdaylist = new StringList(7);
    wdaylist->append(String("Mon"));
    wdaylist->append(String("Tue"));
    wdaylist->append(String("Wed"));
    wdaylist->append(String("Thu"));
    wdaylist->append(String("Fri"));
    wdaylist->append(String("Sat"));
    wdaylist->append(String("Sun"));
    wdayvalue = new StrListValue(wdaylist, date.weekDay()-1);
    wdayv = new DragValuator(wdayvalue, s);

    StringList* monthlist = new StringList(12);
    monthlist->append(String("Jan"));
    monthlist->append(String("Feb"));
    monthlist->append(String("Mar"));
    monthlist->append(String("Apr"));
    monthlist->append(String("May"));
    monthlist->append(String("Jun"));
    monthlist->append(String("Jul"));
    monthlist->append(String("Aug"));
    monthlist->append(String("Sep"));
    monthlist->append(String("Oct"));
    monthlist->append(String("Nov"));
    monthlist->append(String("Dec"));
    monthvalue = new StrListValue(monthlist, date.month()-1);
    monthv = new DragValuator(monthvalue, s);

    Action* upday = nil;
    Action* downday = nil;
    if (editable) {
	upday = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::plusday
	);
	downday = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::minusday
	);
    }
    mdayvalue = new BoundedValue(1, 31, 1, 1, date.dayOfMonth(), "%02.0f");
    mdayv = new DragValuator(mdayvalue, s, upday, downday);

    yearvalue = new BoundedValue(1901, 10000, 1, 1,
				 date.year(), "%4.0f");
    yearv = new DragValuator(yearvalue, s);

    Action* uphour = nil;
    Action* downhour = nil;
    if (editable) {
	uphour = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::plushour
	);
	downhour = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::minushour
	);
    }
    hourvalue = new BoundedValue(0, 23, 1, 1, time_->time()->hour(), "%02.0f");
    hourv = new DragValuator(hourvalue, s, uphour, downhour);

    Action* upmin = nil;
    Action* downmin = nil;
    if (editable) {
	upmin = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::plusmin
	);
	downmin = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::minusmin
	);
    }
    minutevalue = new BoundedValue(0, 59, 1, 1, time_->time()->minute(), "%02.0f");
    minutev = new DragValuator(minutevalue, s, upmin, downmin);

    Action* upsec = nil;
    Action* downsec = nil;
    if (editable) {
	upsec = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::plussec
	);
	downsec = new ActionCallback(ObservableTime)(
	    time_, &ObservableTime::minussec
	);
    }
    secondvalue = new BoundedValue(0, 59, 1, 1, time_->time()->second(), "%02.0f");
    secondv = new DragValuator(secondvalue, s, upsec, downsec);

    PolyGlyph* hb = layout_.hbox(13);
    Glyph* space = layout_.vcenter(kit_.label(" "));
    Glyph* colon = layout_.vcenter(kit_.label(":"));
    hb->append(layout_.vcenter(wdayv));
    hb->append(space);
    hb->append(layout_.vcenter(monthv));
    hb->append(space);
    hb->append(layout_.vcenter(mdayv));
    hb->append(space);
    hb->append(layout_.vcenter(yearv));
    hb->append(space);
    hb->append(layout_.vcenter(hourv));
    hb->append(colon);
    hb->append(layout_.vcenter(minutev));
    hb->append(colon);
    hb->append(layout_.vcenter(secondv));

    if (editable) {
	StringList* deltalist = new StringList(4);
	deltalist->append(String("seconds"));
	deltalist->append(String("minutes"));
	deltalist->append(String("hours"));
	deltalist->append(String("days"));
	deltavalue = new StrListValue(deltalist, 0);
	Action* dfwd = new ActionCallback(BoundedValue)(
	    deltavalue, &BoundedValue::scrollfwdX
	);
	Action* dbwd = new ActionCallback(BoundedValue)(
	    deltavalue, &BoundedValue::scrollbwdX
	);
	DragValuator* deltav = new DragValuator(deltavalue, s, dfwd, dbwd);

	timesvalue = new BoundedValue(0, 99, 1, 1, 0, "%2.0f");
	Action* tfwd = new ActionCallback(BoundedValue)(
	    timesvalue, &BoundedValue::scrollfwdX
	);
	Action* tbwd = new ActionCallback(BoundedValue)(
	    timesvalue, &BoundedValue::scrollbwdX
	);
	DragValuator* timesv = new DragValuator(timesvalue, s, tfwd, tbwd);

	Action* add = new ActionCallback(TimeGlyph)(
	    this, &TimeGlyph::addtimesdelta
	);
	Button* addbutton = kit_.push_button(kit_.label("+"), add);

	Action* sub = new ActionCallback(TimeGlyph)(
	    this, &TimeGlyph::subtimesdelta
	);
	Button* subbutton = kit_.push_button(kit_.label("-"), sub);

	hb->append(space);
	hb->append(space);
	hb->append(space);
	hb->append(layout_.vcenter(addbutton));
	hb->append(space);
	hb->append(layout_.vcenter(subbutton));
	hb->append(space);
	hb->append(layout_.vcenter(timesv));
	hb->append(space);
	hb->append(layout_.vcenter(deltav));
    }
    else {
	deltavalue = nil;
	timesvalue = nil;
    }
    
    body(kit_.outset_frame(layout_.margin(hb,5)));
}

void TimeGlyph::updatevalues() {
    Date date(time_->time()->date());
    wdayvalue->current_value(date.weekDay()-1);
    monthvalue->current_value(date.month()-1);
    mdayvalue->current_value(date.dayOfMonth());
    yearvalue->current_value(date.year());
    hourvalue->current_value(time_->time()->hour());
    minutevalue->current_value(time_->time()->minute());
    secondvalue->current_value(time_->time()->second());
}

void TimeGlyph::update(Observable* obs) {
    updatevalues();
}

void TimeGlyph::addtimesdelta() {
    int times = atoi(timesvalue->valuestring());
    if (times > 0) {
	const char* deltastring = deltavalue->valuestring();
	if (strcmp(deltastring,"seconds")==0)
	    time_->addsecond(times);
	else if (strcmp(deltastring,"minutes")==0)
	    time_->addminute(times);
	else if (strcmp(deltastring,"hours")==0)
	    time_->addhour(times);
	else if (strcmp(deltastring,"days")==0)
	    time_->addday(times);
    }
}

void TimeGlyph::subtimesdelta() {
    int times = atoi(timesvalue->valuestring());
    if (times > 0) {
	const char* deltastring = deltavalue->valuestring();
	if (strcmp(deltastring,"seconds")==0)
	    time_->addsecond(-times);
	else if (strcmp(deltastring,"minutes")==0)
	    time_->addminute(-times);
	else if (strcmp(deltastring,"hours")==0)
	    time_->addhour(-times);
	else if (strcmp(deltastring,"days")==0)
	    time_->addday(-times);
    }
}
