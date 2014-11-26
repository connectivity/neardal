/*
 *     NEARDAL Tester command line interpreter
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

#ifndef __NCL_CMD_H__
#define __NCL_CMD_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

#include "neardal.h"
#include "ncl.h"

/* Command Line Interpretor context... */
typedef struct {
	/* NEARDAL Callback already initialized? */
	gboolean	cb_initialized;

	/* command line interpretor context */
	GString		*clBuf;		/* Command line buffer */

} NCLCmdContext;

/* Array prototype of command line functions interpretor */
typedef	NCLError(*ncl_cmd_func)(int argc, char *argv[]);
typedef struct {
	char		*cmdName;	/* Command name */
	ncl_cmd_func	func;		/* Address of processing function */
	char		*helpStr;	/* Minimal help */
} NCLCmdInterpretor;

/* Initialize/Destroy command line interpretor context */
NCLError		ncl_cmd_init(char *execCmdLineStr);
void			ncl_cmd_finalize(void);

/* Return command line functions */
NCLCmdInterpretor	*ncl_cmd_get_list(int *nbCmd);
int			ncl_cmd_get_nbCmd(void);
NCLCmdContext		*ncl_cmd_get_ctx(void);

#define ncl_cmd_print(...) neardal_trace(__VA_ARGS__)

#define		NCL_CMD_PRINT(...) \
			ncl_cmd_print(NULL, stdout, __VA_ARGS__)

#define		NCL_CMD_PRINTF(...) \
			ncl_cmd_print(__func__, stdout, __VA_ARGS__);

#define		NCL_CMD_PRINTIN() \
			ncl_cmd_print(__func__, stdout, "Processing...\n")

#define		NCL_CMD_PRINTERR(...) \
	ncl_cmd_print(__func__, stderr, "Error: " __VA_ARGS__)

#define		NCL_CMD_DUMP(mem, size) \
			ncl_trace_dump_mem((char *) mem, size)

#endif /* __NCL_CMD_H__ */
