/*
 * ACE-lite (issue #147).  src/ACE-lite/event_handler.c
 * (ACE-lite reimplements a subset of the ACE interface; ACE is (c) the DOC
 * group -- see src/ACE-lite/NOTICE.)
 *
 * ACE_Event_Handler default method bodies.  As in ACE, the demultiplexing
 * hooks default to -1 (remove me) so an un-overridden handler retires rather
 * than spins; ivtools' real handlers (ACE_IO_Handler, ACE_Svc_Handler
 * subclasses, the acceptor) override the ones they use.
 */

#include <ACE-lite/Event_Handler.h>

ACE_Event_Handler::~ACE_Event_Handler() {}

ACE_HANDLE ACE_Event_Handler::get_handle() const { return ACE_INVALID_HANDLE; }

void ACE_Event_Handler::set_handle(ACE_HANDLE) {}

int ACE_Event_Handler::handle_input(ACE_HANDLE) { return -1; }

int ACE_Event_Handler::handle_output(ACE_HANDLE) { return -1; }

int ACE_Event_Handler::handle_exception(ACE_HANDLE) { return -1; }

int ACE_Event_Handler::handle_timeout(const ACE_Time_Value&, const void*) { return -1; }

int ACE_Event_Handler::handle_close(ACE_HANDLE, ACE_Reactor_Mask) { return 0; }
