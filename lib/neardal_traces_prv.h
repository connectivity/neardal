/*
 *     NEARDAL (Neard Abstraction Library)
 *
 *     Copyright 2012-2014 Intel Corporation. All rights reserved.
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

#ifndef NEARDAL_TRACES_PRV_H
#define NEARDAL_TRACES_PRV_H

/* a debug output macro */
#ifdef NEARDAL_TRACES
	#define NEARDAL_TRACE(...)	neardal_trace(NULL, stdout, __VA_ARGS__)
	#define NEARDAL_TRACEDUMP(...)	neardal_trace_dump_mem(__VA_ARGS__)

	/* Macro including function name before traces */
	#define NEARDAL_TRACEF(...)	neardal_trace(__func__, stdout, \
							__VA_ARGS__)
	#define NEARDAL_TRACEIN()	neardal_trace(__func__, stdout, \
							"Processing...\n")
#else
	#define NEARDAL_TRACE(...)
	#define NEARDAL_TRACEDUMP(...)
	#define NEARDAL_TRACEF(...)
	#define NEARDAL_TRACEIN(...)
#endif /* NEARDAL_DEBUG */
/* always defined */
#define NEARDAL_TRACE_LOG(...)	neardal_trace(__func__, stdout,	__VA_ARGS__)
#define NEARDAL_TRACE_ERR(...)	neardal_trace(__func__, stderr, \
						"Error: " __VA_ARGS__)

void neardal_trace_dump_mem(char *dataP, int size);

#endif	/* NEARDAL_TRACES_PRV_H */
