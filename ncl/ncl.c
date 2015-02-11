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

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>

#ifdef HAVE_LIBEDIT
#include <editline/readline.h>
#else
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "ncl.h"
#include "ncl_cmd.h"

#define NCL_PROMPT		"NCL> "
#define NB_COLUMN		16

NCLContext	gNclCtx;

static void ncl_trace_prv_dump_data_as_binary_format(char *bufToReadP,
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

static void ncl_trace_prv_dump_data_as_ascii_format(char *bufToReadP,
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


void ncl_trace_dump_mem(char *bufToReadP, int size)
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
		ncl_trace_prv_dump_data_as_binary_format(&bufToReadP[offset],
							len, bufTrace,
							NB_COLUMN);
		ncl_trace_prv_dump_data_as_ascii_format(&bufToReadP[offset],
						       len, bufTrace,
						       NB_COLUMN);
		NCL_CMD_PRINT("%s\n", bufTrace->str);
		len		-= NB_COLUMN;
		offset	+= NB_COLUMN;
		g_string_truncate(bufTrace, 0);
	}
	g_string_free(bufTrace, TRUE);
}


char *ncl_error_get_text(NCLError ec)
{
	switch (ec) {
	case NCLERR_NOERROR:
		return "Success";

	case NCLERR_GLOBAL_ERROR:
		return "General error";

	case NCLERR_PARSING_PARAMETERS:
		return "Invalid parameter";

	case NCLERR_MEM_ALLOC:
		return "Memory allocation error";

	case NCLERR_INIT:
		return "Error while initializing command line interpretor";

	case NCLERR_LIB_ERROR:
		return "Error from Linked Library";

	default:
		return "UNKNOWN ERROR !!!";
	}
}

static ncl_cmd_func ncl_prv_find_func(char *cmd)
{
	int			index;
	NCLCmdInterpretor	*it; /* commands interpretor array */
	int			nbClCmd;

	it = ncl_cmd_get_list(&nbClCmd);
	if (it == NULL || cmd == NULL)
		return NULL;

	for (index = 0; index < nbClCmd; index++)
		if (!strncmp(it[index].cmdName, cmd, strlen(it[index].cmdName)))
			return it[index].func;

	return NULL;
}

NCLError ncl_exec(char *cmd)
{
	NCLError ret = NCLERR_PARSING_PARAMETERS;
	GError *gerr = NULL;
	ncl_cmd_func func = NULL;
	char **argv = NULL;
	int argc;

	if (!g_shell_parse_argv(g_strstrip(cmd), &argc, &argv, &gerr))
		goto exit;

	if (!(func = ncl_prv_find_func(argv[0]))) {
		NCL_CMD_PRINTERR("'%s': Not NCL command, trying shell\n", cmd);
		g_spawn_command_line_async(cmd, &gerr);
		goto exit;
	}

	if ((ret = func(argc, argv)) != NCLERR_NOERROR)
		NCL_CMD_PRINTERR("'%s': %s\n", cmd, ncl_error_get_text(ret));
exit:
	if (gerr) {
		if (gerr->code == G_SHELL_ERROR_EMPTY_STRING)
			ret = NCLERR_NOERROR;
		if (ret != NCLERR_NOERROR)
			NCL_CMD_PRINTERR("%s\n", gerr->message);
		g_error_free(gerr);
	}
	g_strfreev(argv);
	return ret;
}

static void ncl_parse_line(char *line)
{
	add_history(line);
	ncl_exec(line);
	free(line);
}

static gboolean ncl_prv_kbinput_cb(GIOChannel *source, GIOCondition condition,
				   gpointer data)
{
	rl_callback_read_char();
#ifdef HAVE_LIBEDIT
	/* Editline bug workaround: handler install with the original prompt
	   corrects EL_UNBUFFERED state without side-effects. */
	rl_callback_handler_install(NCL_PROMPT, ncl_parse_line);
#endif
	return TRUE;
}

void ncl_prompt(void)
{
	NCL_CMD_PRINT(NCL_PROMPT);
}

NCLContext *ncl_get_ctx(void)
{
	return &gNclCtx;
}


void ncl_finalize(void)
{
	NCL_CMD_PRINTIN();

	/* Freeing command line interpretor context */
	ncl_cmd_finalize();

	rl_callback_handler_remove();

	if (gNclCtx.main_loop)
		g_main_loop_unref(gNclCtx.main_loop);
}

static NCLError ncl_prv_init(void)
{
	/* Initialize Test App context */
	memset(&gNclCtx, 0, sizeof(gNclCtx));
	gNclCtx.main_loop = g_main_loop_new(NULL, FALSE);

	/* Initialize command line interpretor context */
	return ncl_cmd_init(NULL);
}

static void ncl_prv_parse_script_file(char *filename)
{
	char *line = NULL;
	size_t len = 0;
	FILE *file = fopen(filename, "r");

	if (file == NULL) {
		gNclCtx.errOnExit = NCLERR_GLOBAL_ERROR;
		return;
	}

	while (getline(&line, &len, file) != -1) {

		NCL_CMD_PRINT("$$$$$$$$$$$$$$$$$$$$$$$$$'\n");
		NCL_CMD_PRINT("Executing '%s'", line);
		NCL_CMD_PRINT("$$$$$$$$$$$$$$$$$$$$$$$$$'\n");

		if ((gNclCtx.errOnExit = ncl_exec(line)) != NCLERR_NOERROR)
			break;

		while (g_main_context_pending(NULL))
			g_main_context_iteration(NULL, FALSE);
	}
	g_free(line);
	fclose(file);
}

static int ncl_trace(FILE *fp, const char *fmt, va_list ap)
{
	int dummy;

	dummy = write(1, "\r           \r", 13);
	vfprintf(fp, fmt, ap);
	rl_forced_update_display();
	return 0;
}

int main(int argc, char *argv[])
{
	NCLError err;
	GOptionContext *context;
	GError *error = NULL;
	char **opt_command = NULL;
	char *opt_script = NULL;
	gboolean opt_keep_running = FALSE, show_help = FALSE;
	GOptionEntry options[] = {
		{ "exec", 'e', 0, G_OPTION_ARG_STRING_ARRAY, &opt_command,
		  "Execute command", "command" },
		{ "script", 's', 0, G_OPTION_ARG_STRING	, &opt_script,
		  "Execute script", "filename" },
		{ "keep", 'k', 0, G_OPTION_ARG_NONE, &opt_keep_running,
		  "Keep running after command/script execution" },
		{ NULL }
	};

	neardal_output_cb = ncl_trace;
	rl_callback_handler_install(NCL_PROMPT, ncl_parse_line);

	NCL_CMD_PRINT("Compiled at %s : %s\n\n", __DATE__, __TIME__);

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);
	if (g_option_context_parse(context, &argc, &argv, &error) == FALSE) {
		NCL_CMD_PRINTERR("%s\n", error->message);
		g_error_free(error);
		return NCLERR_INIT;
	}
	g_option_context_free(context);

	if ((err = ncl_prv_init()) != NCLERR_NOERROR)
		goto exit;

	if (!opt_script && !opt_command) {
		opt_keep_running = TRUE;
		show_help = TRUE;
	}

	if (opt_script)
		ncl_prv_parse_script_file(opt_script);

	while (opt_command) {
		gNclCtx.errOnExit = ncl_exec(*opt_command);
		while (g_main_context_pending(NULL))
			g_main_context_iteration(NULL, FALSE);
		g_free(*opt_command++);
		if (!*opt_command)
			opt_command = NULL;
	}

	if (opt_keep_running) {
		gNclCtx.channel = g_io_channel_unix_new(STDIN_FILENO);
		gNclCtx.tag = g_io_add_watch(gNclCtx.channel, G_IO_IN,
					(GIOFunc) ncl_prv_kbinput_cb, &gNclCtx);
		g_io_channel_unref(gNclCtx.channel);

		if (show_help)
			ncl_exec(LISTCMD_NAME);

		g_main_loop_run(gNclCtx.main_loop);
	}

	err = gNclCtx.errOnExit;
exit:
	ncl_finalize();

	if (err != NCLERR_NOERROR)
		NCL_CMD_PRINTERR("Exit with error %d\n", err);

	return err;
}
