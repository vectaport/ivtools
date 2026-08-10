# Qt Integration Design Notes

Status: **investigation, not started**. Captured so the reasoning doesn't have
to be reconstructed when the Qt work actually begins. Nothing here is
implemented; nothing here is committed to.

## The problem

X11/InterViews (the `Glyph` toolkit) and Qt will have to coexist in the same
process for the indefinite future — this isn't a big-bang port. The goal is
an abstraction that *hides* which toolkit is under the hood, confined to a
few judicious locations, rather than `#ifdef QT` sprinkled through
application code. A hard constraint on top of that: the whole point of
moving to Qt is to shed the X11 dependency, not relocate it — any design
that only works by reparenting X11 windows or otherwise assumes an X11
substrate defeats the purpose.

Long-term direction: Qt widgets become the norm for application chrome;
Glyph/X11 code is what should shrink to a diminishing set of legacy panels,
not the other way around.

This turns out to be one shared piece of rendering infrastructure, one case
that needs no bridge at all, and one still-open question about who owns the
event loop.

## 1. One shared rendering substrate: a Qt-backed `Canvas`

`Canvas` (`src/include/InterViews/canvas.h`) already sits behind a clean
bridge: the abstract class never leaks a platform type, and `CanvasRep` is
only forward-declared there, concretely defined today in `IV-X11`
(`src/include/IV-X11/xcanvas.h`). Both of the things that need to render —
Unidraw's `Painter`, drawing `Graphic`s onto a `Canvas`, and any `Glyph`,
via `Glyph::draw(Canvas*, const Allocation&) const`
(`src/include/InterViews/glyph.h`) — already funnel through this identical
contract.

That means a single new `CanvasRep` implementation, translating
`Canvas`'s calls (`fill_rect()`, `line()`, `character()`, `image()`, ...)
onto `QPainter` inside a plain `QWidget::paintEvent()`, serves **both**
Unidraw's drawing surface and any remaining Glyph-composed chrome
simultaneously. There is no separate "port the canvas" problem and "bridge
the chrome" problem — it's the same one piece of infrastructure, used
wherever legacy Glyph-composed content still exists: the Unidraw canvas
today, plus any not-yet-ported Glyph panels (e.g. something in the shape of
`scrollfield`'s grid), each embedded as one `QWidget` region inside an
otherwise Qt-native window. As individual legacy panels get rewritten in
native Qt over time, they simply stop needing this path — nothing about the
substrate itself changes.

### Rejected alternatives (recorded so they don't get rediscovered)

- **Wrap live Qt chrome widgets as `Glyph` subclasses**, rendering them by
  drawing off-screen into a `QImage` and blitting through `Canvas::image()`.
  Rejected: throws away Qt's own accelerated compositing, costs a
  full-frame software copy on every repaint, and breaks anything that needs
  a genuine native window — IME, accessibility, GL/video widgets,
  correctly-timed cursor blink.
- **Give those wrapped widgets a real native window and reparent it** as an
  X11 child of an InterViews-owned window (classic foreign-window
  embedding). Gets native painting and native input for free, but is
  X11-specific — Wayland has no equivalent primitive, and Qt has been
  deprecating APIs like `QX11EmbedWidget` for exactly that reason. Defeats
  the actual goal of the port.

Both of these assumed the top-level window stays InterViews/X11-owned and
Qt content is the guest smuggled in. Avoiding X11 entirely requires the
opposite assumption, which is what sections 1 and 2 are built on: **Qt owns
the top-level window**, via its own portable QPA backend (X11, Wayland,
Win32, Cocoa alike), and legacy Glyph content is the guest, carried by the
Qt-backed `Canvas` above.

## 2. New chrome needs no bridge at all

Once Qt owns the top-level window, newly written chrome is just built
directly with native Qt widgets and layouts (`QPushButton`, `QMenuBar`,
`QLayout`, ...). No `Glyph` wrapping, no factory abstraction, no X11
anywhere in that path — new chrome doesn't need to participate in the
legacy `Glyph` tree at all.

This supersedes an earlier idea in this investigation: a `QtKit : public
WidgetKit` (paired with a `-Dqt_kit` build flag, mirroring
`motif_kit`/`openlook_kit`/`sgi_motif_kit`/`bw_kit`) that would return Qt
widgets wrapped as `Glyph`s from `WidgetKit`'s factory methods. That would
only matter if new chrome still needed to compose into a `Glyph` tree — it
doesn't, once Qt owns the window. `WidgetKit`'s existing Motif/OpenLook/
SGI-Motif/Mono selection (`src/InterViews/kit.c`) stays exactly as it is
today, serving only whatever legacy Glyph-composed content still exists
behind the Qt-backed `Canvas` from section 1. It isn't something that needs,
or benefits from, a Qt sibling kit.

## 3. Event-loop splicing

InterViews' `Dispatcher` (`Dispatch/dispatcher.h`) and Qt's
`QApplication::exec()` can't both be "the" outer loop in one process. One
has to own the process and drive the other as a guest.

**There's already a precedent for exactly this shape in the codebase.**
`ComTerpIOHandler` (`src/ComUnidraw/comterp-iohandler.{h,c}`) is explicitly
documented as "class for splicing comterp into Unidraw event loop": it
registers comterp's input fd with
`Dispatcher::instance().link(fd, Dispatcher::ReadMask, this)`, and whenever
that fd is readable, `Dispatcher` calls `inputReady()`, which cranks the
comterp REPL for one input line before handing control back. `Dispatcher`
never stops being the one true outer loop; comterp is a guest driven by
fd-readiness. This is the template for splicing anything into anything else
in this codebase.

**Given sections 1 and 2, the direction is effectively settled, not just
favored:** Qt owns the top-level window, which means Qt owns native input
delivery for both newly written chrome and the embedded legacy-`Canvas`
region. For any of that input to arrive anywhere, `QApplication::exec()`
has to be the outer loop. `Dispatcher` becomes the guest, spliced in the
mirror image of the `ComTerpIOHandler` pattern: wrap `Dispatcher`'s fd set
in `QSocketNotifier`(s) and call into `Dispatcher::dispatch()` (or a
non-blocking/single-pass variant, if one exists or needs to be added) from
the notifier callback.

One more piece of work this reframing surfaces: input landing on the
embedded legacy-`Canvas` `QWidget` needs translating from `QEvent` into
InterViews' `Event`/`Handler` dispatch for just that one region — Qt
delivers it natively as far as the widget boundary, but the legacy
`Glyph`/Unidraw code inside that region still expects InterViews `Event`s.
Native Qt chrome elsewhere needs no such translation; it handles its own
input entirely natively.

### `update()` is already the model for what this section proposes

`update()` is the command that already does, at REPL-script granularity,
what this section proposes doing at the event-loop level: pause execution
so that whatever's arrived — or arrives within a bounded `usec` window —
gets handled, then resume the script. Two implementations exist today,
differing in how far they reach, and both already cover Unidraw, comterp,
and Glyph events:

- Plain comterp's `update()` (`UpdateFunc::execute`,
  `src/ComTerp/ctrlfunc.c:508`) is a bounded single-pass poll of the ACE
  reactor: `ComterpHandler::reactor_singleton()->handle_events(timeout)`.
- comdraw's `update()` (`UpdateUnidrawFunc::execute`,
  `src/ComUnidraw/unifunc.c:111`) reaches further: it forces a repaint
  (`unidraw->Update(true)`), then temporarily reconfigures a timeout and
  calls `OverlayUnidraw::Run()` (`src/OverlayUnidraw/ovunidraw.c:130`),
  which is itself a `while (alive() && !session->done() && ...) {
  session->read(e, ...); ... }` loop — the same shape as the outer
  application loop, entered recursively from inside a comterp script that
  is already running inside that same outer loop (via the
  `ComTerpIOHandler` splice above).

**The Qt extension is the natural next step of this mechanism, not a new
one.** Once `Dispatcher` is a guest fed into Qt via `QSocketNotifier`s, a
single `QCoreApplication::processEvents(QEventLoop::AllEvents, timeout)`
call can replace both existing implementations at once: it covers Qt-native
chrome events directly, and Unidraw/comterp/Glyph events indirectly, since
`Dispatcher`'s own fds are just more `QSocketNotifier`s serviced on the way
through. `update()` goes from two mechanisms down to one.

### The reentrancy invariant this requires

Escape-and-resume only works if everything reachable from the call point
survives being reentered. That's not a risk to design around — it's the
actual content of building a foundational toolkit rather than a one-off
application, where getting this right is exactly the job. It has a concrete
precedent: comdraw's startup seed `update(1000000)` once triggered a
use-after-free where the reactor pump inside `update()` fired stdin's close
handler, and `ComterpHandler::destroy()` freed the live interpreter
(`if (comterp_->running()) delete_later(); else delete comterp_`) because
`running()` was false — even though that interpreter's own `execute()` call
was still on the stack above it. Fixed by adding a `running()` guard
bracket around the inline-eval `ComTerpServ::run` overloads so destruction
defers instead of freeing an object still executing. It was the second
occurrence of this same invariant being missed (the first was a reactor-
reentrancy UAF in the piped-REPL teardown path, "worked example 2" in
`config/SANITIZE.md`) — a standing invariant the framework has to hold
everywhere `update()` (or its equivalent) can be called from, not a bug
that, once patched, stays patched by construction.

Qt sharpens this rather than avoiding it: `QCoreApplication::processEvents()`
is explicitly designed to nest to unbounded depth (it's how modal dialogs
work — an `exec()` called from a slot that's itself inside another
`exec()`). So the depth an `update()`-triggered pump can recurse to is no
longer "one level of splice," it's unbounded. Every object reachable from
anything an `update()`-triggered pump can invoke — including the new
`Dispatcher`↔`QSocketNotifier` bridge objects themselves — needs to hold
the same "defer my own teardown while I'm still on the stack" invariant.
That should be verified at the point each new bridge class is designed, the
same way `running()` verifies it for `ComTerpServ` today, not discovered
reactively per crash the way the last two instances were.

**Open questions to resolve when this work starts:**
- Does `Dispatcher` have (or need) a non-blocking/single-pass entry point
  safe to call from inside a `QSocketNotifier` callback, given its normal
  mode is presumably a blocking `select`/`poll` loop? Needs to be checked
  against the actual `Dispatch/dispatcher.c` implementation.
- Which fd(s) does `Dispatcher` actually track that would need
  `QSocketNotifier` wrappers (X11 connection, comterp's own spliced-in fds,
  anything else registered via `link()`)?
- The `QEvent`→InterViews-`Event` translation for the embedded
  legacy-`Canvas` region: scope it against `Window::receive(const Event&)`
  and `Window::target(const Event&)` (`src/include/InterViews/window.h`)
  once this work starts.
- Audit every new bridge/handler class introduced by this splice (the
  `Dispatcher`↔`QSocketNotifier` adapter, any Qt-side analog of
  `ComterpHandler`) for the same reentrant-teardown invariant `running()`
  established for `ComTerpServ`, given nesting depth is now unbounded.

Not resolved here. Revisit when the Qt work actually begins.
