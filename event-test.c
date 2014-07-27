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
 */

/*======================================
	Header include
======================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <wayland-client.h>

#include "event.h"

/*======================================
	Structures
======================================*/

/*======================================
	Prototypes
======================================*/

/*======================================
	Variables
======================================*/

/*======================================
	Public functions
======================================*/

int
main(int argc, char **argv)
{
	pthread_t tid;
	struct event cond = {0};
	struct event result = {0};

	if (start_event_thread(&tid) < 0)
		return 1;

	printf("マウスをウィンドウ内に移動させてください\n");
	cond.ev_type = POINTER_EV_ENTER;
	if (!wait_for_event(&cond, &result, 10))
		printf("OK\n");
	else
		printf("NG\n");

	printf("マウスをウィンドウ内で動かしてください\n");
	cond.ev_type = POINTER_EV_MOTION;
	if (!wait_for_event(&cond, &result, 10))
		printf("OK\n");
	else
		printf("NG\n");

	printf("マウスをウィンドウ外に移動させてください\n");
	cond.ev_type = POINTER_EV_LEAVE;
	if (!wait_for_event(&cond, &result, 10))
		printf("OK\n");
	else
		printf("NG\n");

	pthread_cancel(tid);
	pthread_join(tid, NULL);

	return 0;
}


/*======================================
	Inner functions
======================================*/

