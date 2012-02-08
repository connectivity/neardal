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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <glib.h>

#include "neardal_traces_prv.h"

#define NB_COLUMN		16

/*
 * neardal_trace
 *
 * trace function.
 *
 */


void neardal_trace(FILE *stream, char *format, ...)
{
	va_list ap;
	gchar	*bufTrace;

	va_start(ap, format);

	bufTrace = g_strdup_vprintf(format, ap);
	if (bufTrace != NULL) {
		fprintf(stream, "%s", bufTrace);
		fflush(stream);
	}
	va_end(ap);
	g_free(bufTrace);
}

static void neardal_prv_dump_data_as_binary_format(char *bufToReadP,
						    int remainingSize,
						    GString *bufDestP,
						    int nbColumn)
{
	int offset = 0;

	while (offset < nbColumn && offset < remainingSize) {
		g_string_append_printf(bufDestP, "%02hX ",
				       ((unsigned char) bufToReadP[offset]));
		offset++;
	}
	/* Adding space to align ascii format */
	if (offset < nbColumn) {
		/* 3 space because each byte in binary format as 2 digit and
		1 space */
		g_string_append_len(bufDestP, "   ", nbColumn - offset);
	}
}

static void neardal_prv_dump_data_as_ascii_format(char *bufToReadP,
					      int remainingSize,
					      GString *bufDestP, int nbColumn)
{
	int		offset = 0;

	while (offset < nbColumn && offset < remainingSize) {
		if (g_ascii_isprint(((unsigned char) bufToReadP[offset])))
			g_string_append_c(bufDestP,
					  ((unsigned char) bufToReadP[offset]));
		else
			g_string_append_c(bufDestP, '.');
		offset++;
	}
	/* Adding space to finish ascii column */
	if (offset < nbColumn)
		g_string_append_len(bufDestP, " ", nbColumn - offset);
}


void neardal_trace_dump_mem(char *bufToReadP, int size)
{
	char	*memP = bufToReadP;
	int		len = size;
	int		offset = 0;
	GString *bufTrace;

	if (!memP || size <= 0)
		return;

	offset	= 0;

	bufTrace = g_string_new(NULL);
	while (len > 0) {
		g_string_append_printf(bufTrace, "%08lX : ",
				       (unsigned long) (&bufToReadP[offset]));
		neardal_prv_dump_data_as_binary_format(&bufToReadP[offset],
							len, bufTrace,
							NB_COLUMN);
		neardal_prv_dump_data_as_ascii_format(&bufToReadP[offset],
						       len, bufTrace,
						       NB_COLUMN);
		NEARDAL_TRACE("%s\n", bufTrace->str);
		len		-= NB_COLUMN;
		offset	+= NB_COLUMN;
		g_string_truncate(bufTrace, 0);
	}
	g_string_free(bufTrace, TRUE);
}
