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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <signal.h>

#include <neardal.h>
#include "neardal_traces_prv.h"

#include "ncl.h"
#include "ncl_cmd.h"

#define PROMPT_PREFIX		"NCL> "
#define NB_MAX_PARAMETERS	20	/* Max number of parameters in a
					command */
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

static NCLError ncl_prv_split_cmdLine(gchar  *cmdLine, int *iArgc,
				      char *iArgv[])
{
	NCLError	err	= NCLERR_NOERROR;
	char		**argv	= NULL;
	int		*argc;
	char		*argEnd;
	char		*argStart;
	bool		inQuotes;
	gssize		argSize;
	int		endParsing = FALSE;

	/* Test input parameters */
	if (!cmdLine || !iArgc || !iArgv)
		return NCLERR_PARSING_PARAMETERS;

	/* Splitting parameters list like argc/argv style */
	argv = iArgv;
	argc = iArgc;
	*argc = 0;
	inQuotes = false;

	argStart = argEnd = cmdLine;
	while ((*argc) < NB_MAX_PARAMETERS && *argEnd != '\0' &&
		endParsing == FALSE) {
		while (*argEnd != ' ' && *argEnd != '"' && *argEnd != '\0')
			argEnd++;
		if (*argEnd == '"') {
			if (inQuotes == false)
				argStart = argEnd + 1;
			inQuotes = !inQuotes;
		}

		if (inQuotes == false) {
			if (*argEnd == '\0')
				endParsing = TRUE;
			*argEnd = '\0';
			argSize = argEnd - argStart;
			if (argSize > 0)
				((char **)(argv))[(*argc)++] = argStart;
			argEnd++;
			argStart = argEnd;
		} else
			argEnd++;
	}

	return err;
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

NCLError ncl_exec(char *cmdName)
{
	NCLError		ret		= NCLERR_NOERROR;
	GError			*gerror		= NULL;
	ncl_cmd_func		funcList	= NULL;
	char			*cmd		= NULL;
	char			*argv[NB_MAX_PARAMETERS];
	int			argc;

	/* Duplicate Command line before split */
	cmd = g_strdup(cmdName);
	if (cmd == NULL)
		return NCLERR_MEM_ALLOC;

	/* Invoking 'list' command to display interpretor commands list */
	memset(argv, 0, sizeof(argv));
	ret = ncl_prv_split_cmdLine(cmd, &argc, argv);
	if (ret != NCLERR_NOERROR) {
		NCL_CMD_PRINTERR("Error while parsing '%s'\n", cmdName);
		goto error;
	}

	funcList = ncl_prv_find_func(argv[0]);
	if (funcList != NULL) {
		ret = (*funcList)(argc, argv);
		if (ret < NCLERR_NOERROR)
			NCL_CMD_PRINTERR(
				"Error command '%s' return err %d (%s)\n",
					 argv[0], ret, ncl_error_get_text(ret));
	} else {
		NCL_CMD_PRINTERR("Unknow NCL function '%s', trying shell...\n",
				 cmdName);
		g_spawn_command_line_async(cmdName, &gerror);
		if (gerror != NULL) {
			NCL_CMD_PRINTERR("Shell return error %d:%s\n",
					 gerror->code, gerror->message);
			g_error_free(gerror);
		}
	}
	g_free(cmd);

	return ret;

error:
	if (cmd != NULL)
		g_free(cmd);
	return ret;
}


static gboolean ncl_prv_kbinput_cb(GIOChannel *source, GIOCondition condition,
				   gpointer data)
{
	NCLContext	*nclCtx	= (NCLContext *) data;
	GError		*error	= NULL;

	switch (condition) {
	case G_IO_IN: {
		gsize		terminator_pos;
		GIOStatus	status;
		NCLCmdContext	*nclCmdCtx;

		nclCmdCtx = ncl_cmd_get_ctx();
		if (!nclCmdCtx)
			return FALSE;

		status = g_io_channel_read_line_string(source,
							nclCmdCtx->clBuf,
							&terminator_pos,
							&error);
		(void) status; /* Remove warning */
		if (nclCmdCtx->clBuf->str) {
			nclCmdCtx->clBuf->str[terminator_pos] = '\0';

			if (nclCmdCtx->clBuf->str[0] != '\0') {
				nclCtx->errOnExit = NCLERR_PARSING_PARAMETERS;
				nclCtx->errOnExit = ncl_exec(
							nclCmdCtx->clBuf->str);
			}
			g_string_erase(nclCmdCtx->clBuf, 0, -1);
			g_string_append_c(nclCmdCtx->clBuf, '\0');
		} else
			NCL_CMD_PRINTERR("buf is NULL!!!\n");
	}
	break;
	case G_IO_PRI:
	case G_IO_ERR:
	case G_IO_HUP:
	case G_IO_NVAL:
	default:
		NCL_CMD_PRINTERR("unhandled condition (%d)\n", condition);
	break;
	}
	ncl_prompt();

	return TRUE;
}

void ncl_prompt(void)
{
	ncl_cmd_print(stdout, PROMPT_PREFIX);
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

	if (gNclCtx.channel) {
		g_io_channel_unref(gNclCtx.channel);
		g_io_channel_shutdown(gNclCtx.channel, TRUE, NULL);
		g_source_remove(gNclCtx.tag);
	}
	if (gNclCtx.main_loop)
		g_main_loop_unref(gNclCtx.main_loop);
}

/*
static void signal_handler(int signum)
{
	NCL_CMD_PRINTERR("Receive signal %d\n", signum);
}
*/
static NCLError ncl_prv_init(char *execCmdLineStr)
{
/*	struct sigaction	sa;
	int			err = NCLERR_NOERROR; */

	/* Initialize Test App context... */
	memset(&gNclCtx, 0, sizeof(gNclCtx));

/*
	sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		NCL_CMD_PRINTERR("Unable to handle system signals\n");
		return NCLERR_INIT;
	}
*/

	gNclCtx.main_loop = g_main_loop_new(NULL, FALSE);

	/* Initialize command line interpretor context */
	return ncl_cmd_init(execCmdLineStr);
}

static void ncl_prv_parse_script_file(char *scriptFileStr)
{
	FILE	*scriptFile;
	char	*cmdLineStr	= NULL;
	size_t	cmdLineSize;
	ssize_t	nbRead;

	/* Opening file */
	scriptFile = fopen(scriptFileStr, "r");
	if (scriptFile == NULL) {
		gNclCtx.errOnExit = NCLERR_GLOBAL_ERROR;
		return;
	}

	do {
		/* Reading command line script file */
		nbRead = getline(&cmdLineStr, &cmdLineSize, scriptFile);
		if (nbRead > 0 && strlen(cmdLineStr) > 1) {
			cmdLineSize = strlen(cmdLineStr);
			if (cmdLineStr[0] != '#') {
				if (cmdLineStr[cmdLineSize - 1] == '\n')
					cmdLineStr[cmdLineSize - 1] = '\0';

				/* Executing command line */
				ncl_cmd_print(stdout,
					      "$$$$$$$$$$$$$$$$$$$$$$$$$'\n");
				ncl_cmd_print(stdout,
					      "Executing '%s'\n", cmdLineStr);
				ncl_cmd_print(stdout,
					      "$$$$$$$$$$$$$$$$$$$$$$$$$'\n");
				gNclCtx.errOnExit = ncl_exec(cmdLineStr);

				/* Main loop */
				do {
					g_main_context_iteration(NULL, false);
				} while (g_main_context_pending(NULL));
			}
		}
		/* Freeing command line */
		if (cmdLineStr != NULL) {
			free(cmdLineStr);
			cmdLineStr = NULL;
		}
	} while (nbRead > 0 && gNclCtx.errOnExit == NCLERR_NOERROR);
	g_free(scriptFile);
}

int main(int argc, char *argv[])
{
	NCLError	err;
	int		fd;
	GOptionContext	*context;
	GError		*error		= NULL;
	static char	*execCmdLineStr;
	static char	*scriptFileStr;
static GOptionEntry	options[] = {
		{ "exec"	, 'e', 0, G_OPTION_ARG_STRING, &execCmdLineStr,
		"Execute specific command line function", "Command Line" },
		{ "script", 's', 0, G_OPTION_ARG_STRING	, &scriptFileStr
				  , "Script file to execute", "filename" },
		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	execCmdLineStr	= NULL;
	scriptFileStr	= NULL;
	NCL_CMD_PRINTIN();
	NCL_CMD_PRINT("Compiled at %s : %s\n\n", __DATE__, __TIME__);

	context = g_option_context_new("");
	g_option_context_add_main_entries(context, options, "");
	if (g_option_context_parse(context, &argc, &argv, &error) == FALSE) {
		if (error != NULL) {
			NCL_CMD_PRINTERR("%s\n", error->message);
			g_error_free(error);
		} else
			NCL_CMD_PRINTERR("An unknown error occurred\n");
		return NCLERR_INIT;
	}
	g_option_context_free(context);

	err = ncl_prv_init(execCmdLineStr);
	if (err != NCLERR_NOERROR) {
		ncl_finalize();
		return NCLERR_INIT;
	}

	/* Do we have a command line in parameters list ? */
	if (scriptFileStr == NULL) {
		/* No, test application executed without a command line in
		parameter. Do we have a command in parameters list ? */
		if (execCmdLineStr == NULL) {
			/* No, test application executed without a command
			line in parameter */

			/* Wrap stdin (keyboard) on callback */
			fd = fileno(stdin);
			gNclCtx.channel = g_io_channel_unix_new(fd);
			g_io_channel_set_encoding(gNclCtx.channel, NULL, NULL);
			g_io_channel_set_buffered(gNclCtx.channel, TRUE);
			gNclCtx.tag = g_io_add_watch(gNclCtx.channel, G_IO_IN,
						(GIOFunc) ncl_prv_kbinput_cb,
						     &gNclCtx);

			/* Invoking 'help' command to display commands line
			 * list */
			ncl_exec(LISTCMD_NAME);
			ncl_prompt();

			/* Launch main-loop */
			g_main_loop_run(gNclCtx.main_loop);
		} else {
			int eventsCount = 0;

			/* Yes, test application executed with a command line
			 * in parameter */
			NCL_CMD_PRINTF("Executing command ('%s')...\n",
				       execCmdLineStr);
			gNclCtx.errOnExit = ncl_exec(execCmdLineStr);

			NCL_CMD_PRINT(
				"Command executed('%s'), processing events...",
				      execCmdLineStr);
			do {
				NCL_CMD_PRINT("*");
				eventsCount++;
			} while (g_main_context_iteration(NULL, false) == true);
			NCL_CMD_PRINT("\n");
			NCL_CMD_PRINTF("All events have been processed (%d).\n",
				       eventsCount);
			g_free(execCmdLineStr);
		}
	} else
		ncl_prv_parse_script_file(scriptFileStr);

	err = gNclCtx.errOnExit;

	ncl_finalize();

	if (err != NCLERR_NOERROR)
		NCL_CMD_PRINTERR("Exit with error %d\n", err);

	return err;
}
