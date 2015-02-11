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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

#include "neardal_traces_prv.h"

#define NB_COLUMN		16

int (*neardal_output_cb)(FILE *fp, const char *fmt, va_list ap) = vfprintf;

void neardal_trace(const char *func, FILE *fp, char *fmt, ...)
{
int dummy;

	va_list ap;
	char *f = fmt;
	if (func)
		dummy = asprintf(&f, "%s(): %s", func, fmt);
	va_start(ap, fmt);
	neardal_output_cb(fp, f, ap);
	va_end(ap);
	if (f != fmt)
		free(f);
}

static void neardal_prv_dump_data_as_binary_format(char *bufToReadP,
						    int remainingSize,
						    GString *bufDestP,
						    int nbColumn)
{
	int offset = 0;

	while (offset < nbColumn && offset < remainingSize) {
		g_string_append_printf(bufDestP, "%02hhX ", bufToReadP[offset]);
		offset++;
	}
	/* Adding space to align ascii format */
	if (offset < nbColumn) {
		/* 3 space because each byte in binary format as 2 digit and
		1 space */
		remainingSize = (nbColumn - offset) * 3;
		offset = 0;
		while ((offset++) < remainingSize)
			g_string_append_c(bufDestP, ' ');
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
 	if (offset < nbColumn) {
		remainingSize = nbColumn - offset;
		offset = 0;
		while ((offset++) < remainingSize)
			g_string_append_c(bufDestP, '.');
	}
}


void neardal_trace_prv_dump_mem(char *bufToReadP, int size)
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
