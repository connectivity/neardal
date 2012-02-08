/*
 *     NEARDAL Tester command line interpreter
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


#ifndef __NCL_H__
#define __NCL_H__

#include <glib.h>

/* Buffer size for the command line interpretor */
#define	CL_BUF_SIZE			1024

/* Test Application Errors list (Only negative values are errors) */
typedef enum {
	NCLERR_NOERROR_HELP_DISP	= 1,
	NCLERR_NOERROR			= 0,
	NCLERR_GLOBAL_ERROR		= -1,
	NCLERR_PARSING_PARAMETERS	= -2,
	NCLERR_INIT			= -3,
	NCLERR_MEM_ALLOC		= -4,
	NCLERR_LIB_ERROR		= -5
} NCLError;

/* Test Application Context */
typedef struct {
	GMainLoop	*main_loop;
	GIOChannel	*channel;	/* for stdin descriptor */
	guint		tag;		/* the ID of the source */
	NCLError	errOnExit;	/* Error returned on exit */
} NCLContext;

NCLContext *ncl_get_ctx(void);

/* Name of the command interpretor to display commands list */
#define LISTCMD_NAME	"help"

/* Display prompt */
void ncl_prompt(void);

#endif /* __NCL_H__ */
