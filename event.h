/*
 * The MIT License (MIT)
 *
 * Copyright © 2014 faith
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * refer: weston/simple-shm
 *   http://cgit.freedesktop.org/wayland/weston/
 */
#ifndef __EVENT_H__
#define __EVENT_H__

/*======================================
	Header include
======================================*/

#include <pthread.h>


/*======================================
	Constants
======================================*/

enum event_type {
	TOUCH_EV_DOWN = 0x0,
	TOUCH_EV_UP,
	TOUCH_EV_MOTION,
	TOUCH_EV_FRAME,
	TOUCH_EV_CANCEL,

	POINTER_EV_ENTER = 0x10,
	POINTER_EV_LEAVE,
	POINTER_EV_MOTION,
	POINTER_EV_BUTTON,
	POINTER_EV_AXIS,

	KEYBOARD_EV_KEYMAP = 0x20,
	KEYBOARD_EV_ENTER,
	KEYBOARD_EV_LEAVE,
	KEYBOARD_EV_KEY,
	KEYBOARD_EV_MODIFIERS,
};


/*======================================
	Structures
======================================*/

struct event {
	unsigned int ev_type;
	void *data;
	unsigned int data_len;
};

/*======================================
	Prototypes
======================================*/

extern int start_event_thread(pthread_t *tid);
extern int wait_for_event(struct event *cond, struct event *result, int timeout);

#endif /* __EVENT_H__ */

