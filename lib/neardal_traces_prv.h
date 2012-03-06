/*
 *     NEARDAL (Neard Abstraction Library)
 *
 *     Copyright 2012 Intel Corporation. All rights reserved.
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU Lesser General Public License version 2
 *     as published by the Free Software Foundation.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software Foundation,
 *     Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __NEARDAL_TRACES_PRV_H
#define __NEARDAL_TRACES_PRV_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* a debug output macro */
#ifdef NEARDAL_TRACES
	#define NEARDAL_TRACE(msg, ...)	neardal_trace(stdout, \
						msg, ##__VA_ARGS__)
	#define NEARDAL_TRACEDUMP(adr, size)	neardal_trace_dump_mem(adr, \
						size)

	/* Macro including function name before traces */
	#define NEARDAL_TRACEF(msg, ...)	neardal_trace(stdout, \
				"%s() : " msg, __func__, ## __VA_ARGS__)
	#define NEARDAL_TRACEIN()		neardal_trace(stdout, \
	"%s() : Processing...\n", __func__)
#else

	#define NEARDAL_TRACE(msg, ...)	(void)0
	#define NEARDAL_TRACEDUMP(adr, size)	(void)0

	#define NEARDAL_TRACEF(msg, ...)	(void)0
	#define NEARDAL_TRACEIN()		(void)0
#endif /* NEARDAL_DEBUG */
/* always defined */
#define NEARDAL_TRACE_LOG(msg, ...)		neardal_trace(stdout, \
				"%s() : " msg, __func__, ##__VA_ARGS__)
#define NEARDAL_TRACE_ERR(msg, ...)		neardal_trace(stderr, \
				"%s(ERR) : " msg, __func__, ##__VA_ARGS__)

void neardal_trace(FILE *stream, char *format, ...);
void neardal_trace_dump_mem(char *dataP, int size);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif	/* __NEARDAL_TRACES_PRV_H */
