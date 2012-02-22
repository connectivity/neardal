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

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <glib.h>
#include <glib-object.h>

#include "neardal.h"
#include "ncl.h"
#include "ncl_cmd.h"
#include <lib/neardal.h>
#include <glib-2.0/glib/goption.h>


static NCLCmdContext  sNclCmdCtx;
/* Some default values...
 * Max number of parameters provided in a command */
#define NB_MAX_PARAMETERS	20

typedef struct {
	uint32_t	idValue;
	gchar		*capStr;
	gchar		*helpStr;
} list_inf;

#define	DEFSTR(x)		(x, #x)


/* Local Utilities functions */
/******************************************************************************
 * Tool function : help command to dump parameters command
 *****************************************************************************/
static void ncl_cmd_prv_dumpOptions(GOptionEntry *options)
{
	GOptionEntry	*optP = options;
	char		long_nameTmp[20];

	while (optP->description != NULL) {
		snprintf(long_nameTmp, sizeof(long_nameTmp), "--%s",
			 optP->long_name);

		ncl_cmd_print(stdout, "-%c,\t%s=%s\t%20s\n", optP->short_name,
			      long_nameTmp, optP->arg_description,
			      optP->description);
		optP++;
	}
}


/******************************************************************************
 * Tool function : parse parameters command (like g_option_context_parse but
 * implicit exit() on '--help' disabled
 *****************************************************************************/
static NCLError ncl_cmd_prv_parseOptions(int *argc, char **argv[],
					 GOptionEntry *options)
{
	GOptionContext	*context;
	GError		*error		= NULL;
	NCLError	err		= NCLERR_NOERROR;
	char		**argvP;
	int		argn;
	int		helpRequested	= 0;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);
	/* disable '--help' parameter else function 'exit()' is implicitly
	 * called... */
	g_option_context_set_help_enabled(context, FALSE);

	/* Searching 'help' in arguments */
	argvP = *argv;
	argn  = *argc;
	while (argn && !helpRequested) {
		helpRequested = (strstr(*argvP, "help") != NULL);
		helpRequested |= (strstr(*argvP, "-h") != NULL);
		helpRequested |= (strstr(*argvP, "/?") != NULL);
		argn--;
		argvP++;
	}

	/* if help requested, generate a 'help' display */
	if (helpRequested) {
		ncl_cmd_prv_dumpOptions(options);
		err = NCLERR_NOERROR_HELP_DISP;
	} else {
		if (!g_option_context_parse(context, argc, argv, &error)) {
			if (error != NULL) {
				NCL_CMD_PRINTERR("%s\n", error->message);
				g_error_free(error);
			} else
				NCL_CMD_PRINTERR("Unknown error occurred\n");
			err = NCLERR_GLOBAL_ERROR;
		}
	}
	g_option_context_free(context);
	return err;
}



/* BEGIN : Interpretor commands array */
static NCLCmdInterpretor itFunc[];

/******************************************************************************
 * Display Interpretor command list
 *****************************************************************************/
NCLError ncl_cmd_list(int argc, char *argv[])
{
	int index;
	char funcName[50];
	int  nbCmd = ncl_cmd_get_nbCmd();

	(void) argc; /* remove warning */
	(void) argv; /* remove warning */

	ncl_cmd_print(stdout, "\nCommand line list\n");
	for (index = 0; index < nbCmd; index++) {
	if (0) { /* TODO: Remove */
		snprintf(funcName, sizeof(funcName), "%40s",
			 itFunc[index].cmdName);
		ncl_cmd_print(stdout, "%s :\t%s\n", funcName,
			      itFunc[index].helpStr);
	} else {
		ncl_cmd_print(stdout, "%s :\n\t%s\n\n", itFunc[index].cmdName,
			      itFunc[index].helpStr);
	}
	}

	return 0;
}


/******************************************************************************
 * Dump properties of an adapter
 *****************************************************************************/
static void ncl_cmd_prv_dump_adapter(neardal_adapter adapter)
{
	char **protocols;
	char **targets;

	NCL_CMD_PRINT("Adapter\n");
	NCL_CMD_PRINT(".. Name:\t\t'%s'\n", adapter.name);

	NCL_CMD_PRINT(".. Polling:\t\t'%s'\n",
		      adapter.polling ? "TRUE" : "FALSE");
	NCL_CMD_PRINT(".. Powered:\t\t'%s'\n",
		      adapter.powered ? "TRUE" : "FALSE");

	targets = adapter.targets;
	NCL_CMD_PRINT(".. Number of targets:\t%d\n", adapter.nbTargets);
	NCL_CMD_PRINT(".. Targets[]:\t\t");
	if (adapter.nbTargets > 0) {
		while ((*targets) != NULL) {
			NCL_CMD_PRINT("'%s', ", *targets);
			targets++;
		}
		neardal_free_array(&adapter.targets);
	} else
		NCL_CMD_PRINT("No targets!");
	NCL_CMD_PRINT("\n");

	protocols = adapter.protocols;
	NCL_CMD_PRINT(".. Number of protocols:\t%d\n", adapter.nbProtocols);
	NCL_CMD_PRINT(".. Protocols[]:\t\t");
	if (adapter.nbProtocols > 0) {
		while ((*protocols) != NULL) {
			NCL_CMD_PRINT("'%s', ", *protocols);
			protocols++;
		}
		neardal_free_array(&adapter.protocols);
	} else
		NCL_CMD_PRINT("No protocols!");
	NCL_CMD_PRINT("\n");
}

/******************************************************************************
 * Dump properties of a target
 *****************************************************************************/
static void ncl_cmd_prv_dump_target(neardal_target target)
{
	char **records;
	char **tagTypes;

	NCL_CMD_PRINT("Target:\n");
	NCL_CMD_PRINT(".. Name:\t\t'%s'\n", target.name);

	NCL_CMD_PRINT(".. Type:\t\t'%s'\n", target.type);

	NCL_CMD_PRINT(".. Number of 'Tag Type':%d\n", target.nbTagTypes);
	tagTypes = target.tagType;
	if (target.nbTagTypes > 0) {
		NCL_CMD_PRINT(".. Tags type[]:\t\t");
		while ((*tagTypes) != NULL) {
			NCL_CMD_PRINT("'%s', ", *tagTypes);
			tagTypes++;
		}
		NCL_CMD_PRINT("\n");
		neardal_free_array(&target.tagType);
	}

	records = target.records;
	NCL_CMD_PRINT(".. Number of records:\t%d\n", target.nbRecords);
	NCL_CMD_PRINT(".. Records[]:\t\t");
	if (records != NULL) {
		while ((*records) != NULL) {
			NCL_CMD_PRINT("'%s', ", *records);
			records++;
		}
		neardal_free_array(&target.records);
	} else
		NCL_CMD_PRINT("No records!");

	NCL_CMD_PRINT("\n");
	NCL_CMD_PRINT(".. ReadOnly:\t\t%s\n"	,
		      target.readOnly ? "TRUE" : "FALSE");
}

/******************************************************************************
 * Dump properties of a record
 *****************************************************************************/
static void ncl_cmd_prv_dump_record(neardal_record record)
{
	NCL_CMD_PRINT("Record\n");
	NCL_CMD_PRINT(".. Name:\t\t%s\n"	, record.name);
	NCL_CMD_PRINT(".. Encoding:\t\t%s\n"	, record.encoding);
	NCL_CMD_PRINT(".. HandOver:\t\t%s\n"	,
		      record.handOver ? "TRUE" : "FALSE");
	NCL_CMD_PRINT(".. Language:\t\t%s\n"	, record.language);
	NCL_CMD_PRINT(".. SmartPoster:\t\t%s\n"	,
		      record.smartPoster ? "TRUE" : "FALSE");
	NCL_CMD_PRINT(".. Action:\t\t%s\n"	, record.action);

	NCL_CMD_PRINT(".. Type:\t\t%s\n"	, record.type);
	NCL_CMD_PRINT(".. Representation:\t%s\n", record.representation);
	NCL_CMD_PRINT(".. URI:\t\t\t%s\n"	, record.uri);
	NCL_CMD_PRINT(".. MIME:\t\t%s\n"	, record.mime);
}

/******************************************************************************
 * neardal_construct : BEGIN
 * Instanciate NFC object, create Neard Dbus connection, register Neard's event
 *****************************************************************************/
static void ncl_cmd_cb_adapter_added(const char *adpName, void *user_data)
{
	neardal_t	neardalMgr = user_data;
	errorCode_t	ec;
	neardal_adapter	adapter;

	NCL_CMD_PRINTF("NFC Adapter added '%s'\n", adpName);
	ec = neardal_get_adapter_properties(neardalMgr, adpName, &adapter);
	if (ec == NEARDAL_SUCCESS)
		ncl_cmd_prv_dump_adapter(adapter);
	else
		NCL_CMD_PRINTF(
		"Unable to read adapter properties. (error:%d='%s'). exit...\n",
			       ec, neardal_error_get_text(ec));

	return;
}

static void ncl_cmd_cb_adapter_removed(const char *adpName, void * user_data)
{
	neardal_t	neardalMgr = user_data;

	(void) neardalMgr;

	NCL_CMD_PRINTF("NFC Adapter removed '%s'\n", adpName);
}

static void ncl_cmd_cb_adapter_prop_changed(char *adpName, char *propName,
					    void *value, void *user_data)
{
	neardal_t	neardalMgr = user_data;
	int		polling;

	(void) neardalMgr;
	if (!strcmp(propName, "Polling")) {
		polling = (int)value;
		NCL_CMD_PRINTF("Polling=%d\n", polling);
	} else
		NCL_CMD_PRINTF("Adapter '%s' -> Property=%s=0x%X\n", adpName,
			       propName, value);

	NCL_CMD_PRINT("\n");
}

static void ncl_cmd_cb_target_found(const char *tgtName, void *user_data)
{
	neardal_t	neardalMgr = user_data;
	neardal_target	target;
	errorCode_t	ec;

	NCL_CMD_PRINTF("NFC Target found (%s)\n", tgtName);

	ec = neardal_get_target_properties(neardalMgr, tgtName, &target);
	if (ec == NEARDAL_SUCCESS)
		ncl_cmd_prv_dump_target(target);
	else
		NCL_CMD_PRINTF(
		"Unable to read target properties. (error:%d='%s'). exit...\n",
			       ec, neardal_error_get_text(ec));
	return;
}

static void ncl_cmd_cb_target_lost(const char *tgtName, void *user_data)
{
	neardal_t	neardalMgr = user_data;

	NCL_CMD_PRINTF("NFC Target lost (%s)\n", tgtName);
	(void) neardalMgr;

}

static void ncl_cmd_cb_record_found(const char *rcdName, void *user_data)
{
	neardal_t	neardalMgr = user_data;
	errorCode_t	ec;
	neardal_record	record;

	NCL_CMD_PRINTF("Target Record found (%s)\n", rcdName);
	ec = neardal_get_record_properties(neardalMgr, rcdName, &record);
	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_record(record);
/*		NCL_CMD_PRINTF("(Re)Start Poll\n");
		sleep(1);
		neardal_start_poll(neardalMgr, (char *) rcdName, NULL); */
	} else
		NCL_CMD_PRINTF("Read record error. (error:%d='%s').\n", ec,
			       neardal_error_get_text(ec));

	return;
}

static NCLError ncl_cmd_neardal_construct(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;

	(void) argc; /* remove warning */
	(void) argv; /* remove warning */

	/* Construct NEARDAL object */
	sNclCmdCtx.neardalMgr = neardal_construct(&ec);
	if (sNclCmdCtx.neardalMgr != NULL)
		NCL_CMD_PRINTF("NFC object constructed");
	else
		NCL_CMD_PRINTF("NFC object not constructed");
	if (ec != NEARDAL_SUCCESS)
		NCL_CMD_PRINT(" with error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
	NCL_CMD_PRINT("\n");

	if (sNclCmdCtx.neardalMgr == NULL)
		goto exit;

	nclErr = NCLERR_NOERROR;
	neardal_set_cb_adapter_added(sNclCmdCtx.neardalMgr,
				      ncl_cmd_cb_adapter_added,
				      sNclCmdCtx.neardalMgr);
	neardal_set_cb_adapter_removed(sNclCmdCtx.neardalMgr,
					ncl_cmd_cb_adapter_removed,
					sNclCmdCtx.neardalMgr);
	neardal_set_cb_adapter_property_changed(sNclCmdCtx.neardalMgr,
					ncl_cmd_cb_adapter_prop_changed,
						sNclCmdCtx.neardalMgr);
	NCL_CMD_PRINTF("NFC adapter callback registered\n");
	neardal_set_cb_target_found(sNclCmdCtx.neardalMgr,
				     ncl_cmd_cb_target_found,
				     sNclCmdCtx.neardalMgr);
	neardal_set_cb_target_lost(sNclCmdCtx.neardalMgr,
				    ncl_cmd_cb_target_lost,
				    sNclCmdCtx.neardalMgr);
	NCL_CMD_PRINTF("NFC target registered\n");
	neardal_set_cb_record_found(sNclCmdCtx.neardalMgr,
				     ncl_cmd_cb_record_found,
				     sNclCmdCtx.neardalMgr);
	NCL_CMD_PRINTF("NFC record callback registered\n");

exit:
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	if (ec == NEARDAL_SUCCESS)
		nclErr =  NCLERR_NOERROR ;
	else
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/******************************************************************************
 * neardal_construct : END
 *****************************************************************************/

/******************************************************************************
 * neardal_get_adapters : BEGIN
 * Get adapters List
 *****************************************************************************/
static NCLError ncl_cmd_neardal_get_adapters(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**adpArray = NULL;
	int		adpLen;
	int		adpOff;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINT("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	ec = neardal_get_adapters(sNclCmdCtx.neardalMgr, &adpArray,
					&adpLen);
	if (ec == NEARDAL_SUCCESS) {
		adpOff = 0;
		/* For each adapter */
		while (adpArray[adpOff] != NULL)
			NCL_CMD_PRINT(".. Adapter '%s'\n",
					adpArray[adpOff++]);

		neardal_free_array(&adpArray);
	} else
		NCL_CMD_PRINTF("No adapter\n");

	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	if (ec == NEARDAL_SUCCESS)
		nclErr =  NCLERR_NOERROR ;
	else
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/******************************************************************************
 * neardal_get_adapters : END
 *****************************************************************************/

/******************************************************************************
 * ncl_cmd_neardal_get_adapter_properties : BEGIN
 * Read adapter properties
 *****************************************************************************/
static NCLError ncl_cmd_neardal_get_adapter_properties(int argc, char *argv[])
{
	errorCode_t ec;
	NCLError	nclErr;
	char		*adapterName	= NULL;
	neardal_adapter	adapter;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	adapterName = argv[1];
	ec = neardal_get_adapter_properties(sNclCmdCtx.neardalMgr,
					     adapterName, &adapter);

	if (ec == NEARDAL_SUCCESS)
		ncl_cmd_prv_dump_adapter(adapter);
	else {
		NCL_CMD_PRINTF("Read adapter properties error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/******************************************************************************
 * ncl_cmd_neardal_get_adapter_properties : END
 *****************************************************************************/

/******************************************************************************
 * neardal_get_targets : BEGIN
 * Get targets List
 *****************************************************************************/
static NCLError ncl_cmd_neardal_get_targets(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**tgtArray = NULL;
	int		tgtLen;
	int		tgtOff;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	ec = neardal_get_targets(sNclCmdCtx.neardalMgr, argv[1],
					&tgtArray, &tgtLen);
	if (ec == NEARDAL_SUCCESS) {
		tgtOff = 0;
			/* For each target */
		while (tgtArray[tgtOff] != NULL)
			NCL_CMD_PRINT(".. Target '%s'\n",
					tgtArray[tgtOff++]);

		neardal_free_array(&tgtArray);
	} else
		NCL_CMD_PRINTF("No target\n");

	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	if (ec == NEARDAL_SUCCESS)
		nclErr =  NCLERR_NOERROR ;
	else
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/******************************************************************************
 * neardal_get_targets : END
 *****************************************************************************/

/******************************************************************************
 * ncl_cmd_neardal_get_target_properties : BEGIN
 * Read target properties
 *****************************************************************************/
static NCLError ncl_cmd_neardal_get_target_properties(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		*targetName	= NULL;
	neardal_target	target;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;


	targetName = argv[1];
	ec = neardal_get_target_properties(sNclCmdCtx.neardalMgr, targetName,
					    &target);

	if (ec == NEARDAL_SUCCESS)
		ncl_cmd_prv_dump_target(target);
	else {
		NCL_CMD_PRINTF("Read target properties error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/******************************************************************************
 * ncl_cmd_neardal_get_target_properties : END
 *****************************************************************************/

/******************************************************************************
 * neardal_get_records : BEGIN
 * Get records List
 *****************************************************************************/
static NCLError ncl_cmd_neardal_get_records(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**rcdArray = NULL;
	int		rcdLen;
	int		rcdOff;


	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	ec = neardal_get_records(sNclCmdCtx.neardalMgr, argv[1],
					&rcdArray, &rcdLen);
	if (ec == NEARDAL_SUCCESS) {
		rcdOff = 0;
		/* For each record */
		while (rcdArray[rcdOff] != NULL)
			NCL_CMD_PRINT(".. Record '%s'\n",
					rcdArray[rcdOff++]);
		neardal_free_array(&rcdArray);
	} else
		NCL_CMD_PRINTF("No record\n");

	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	if (ec == NEARDAL_SUCCESS)
		nclErr =  NCLERR_NOERROR ;
	else
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/******************************************************************************
 * neardal_get_records : END
 *****************************************************************************/

/******************************************************************************
 * ncl_cmd_neardal_get_record_properties : BEGIN
 * Read a specific target
 *****************************************************************************/
static NCLError ncl_cmd_neardal_get_record_properties(int argc, char *argv[])
{
	errorCode_t ec;
	NCLError	nclErr;
	char		*recordName	= NULL;
	neardal_record	record;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	recordName = argv[1];
	ec = neardal_get_record_properties(sNclCmdCtx.neardalMgr, recordName,
					    &record);
	if (ec == NEARDAL_SUCCESS)
		ncl_cmd_prv_dump_record(record);
	else {
		NCL_CMD_PRINTF("Read record error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/******************************************************************************
 * ncl_cmd_neardal_get_record_properties : END
 *****************************************************************************/

/******************************************************************************
 * ncl_cmd_neardal_publish : BEGIN
 * write NDEF record to tag
 *****************************************************************************/
static NCLError ncl_cmd_neardal_publish(int argc, char *argv[])
{
	errorCode_t		ec = NEARDAL_SUCCESS;
	NCLError		nclErr;
	static neardal_record	rcd;
	static int		smartPoster;

static GOptionEntry options[] = {
		{ "act", 'c', 0, G_OPTION_ARG_STRING, &rcd.action
				  , "Action", "save"},

		{ "adp", 'a', 0, G_OPTION_ARG_STRING, &rcd.name
				  , "Adapter name", "/org/neard/nfc0"},

		{ "encoding", 'e', 0, G_OPTION_ARG_STRING, &rcd.encoding
				, "Encoding", "UTF-8" },

		{ "lang", 'l', 0, G_OPTION_ARG_STRING	, &rcd.language
				, "Language", "en" },

		{ "mime", 'm', 0, G_OPTION_ARG_STRING	, &rcd.mime
				, "Mime-type", "audio/mp3"},

		{ "rep"	, 'r', 0, G_OPTION_ARG_STRING , &rcd.representation
				, "Representation", "sample text" },

		{ "smt"	, 's', 0, G_OPTION_ARG_INT , &smartPoster
				, "SmartPoster", "0 or <>0" },

		{ "type", 't', 0, G_OPTION_ARG_STRING, &rcd.type
				  , "Record type (Text, URI,...", "Text" },

		{ "uri", 'u', 0, G_OPTION_ARG_STRING, &rcd.uri
				  , "URI", "http://www.intel.com" },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Parse options */
	memset(&rcd, 0, sizeof(neardal_record));
	nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	if (nclErr != NCLERR_NOERROR)
		goto exit;
	rcd.smartPoster = (smartPoster != 0) ? true : false;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		goto exit;

	ec = neardal_publish(sNclCmdCtx.neardalMgr, &rcd);

exit:
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));
	if (rcd.action != NULL)
		g_free((gchar *) rcd.action);
	if (rcd.name != NULL)
		g_free((gchar *) rcd.name);
	if (rcd.encoding != NULL)
		g_free((gchar *) rcd.encoding);
	if (rcd.language != NULL)
		g_free((gchar *) rcd.language);
	if (rcd.mime != NULL)
		g_free((gchar *) rcd.mime);
	if (rcd.representation != NULL)
		g_free((gchar *) rcd.representation);
	if (rcd.type != NULL)
		g_free((gchar *) rcd.type);
	if (rcd.uri != NULL)
		g_free((gchar *) rcd.uri);

	return nclErr;
}
/******************************************************************************
 * ncl_cmd_neardal_publish : END
 *****************************************************************************/


/******************************************************************************
 * ncl_cmd_neardal_start_poll : BEGIN
 * Request Neard to start polling
 *****************************************************************************/
static NCLError ncl_cmd_neardal_start_poll(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	char		*adpName	= NULL;
	NCLError	nclErr;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) && (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	/* Start polling if adapter present */
	adpName = argv[1];
	neardal_start_poll(sNclCmdCtx.neardalMgr, adpName, &ec);
	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("NFC polling activation error:%d='%s'\n",
				ec, neardal_error_get_text(ec));
		if (ec == NEARDAL_ERROR_POLLING_ALREADY_ACTIVE)
			ec = NEARDAL_SUCCESS;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/******************************************************************************
 * ncl_cmd_neardal_start_poll : END
 *****************************************************************************/

/******************************************************************************
 * ncl_cmd_neardal_stop_poll : BEGIN
 * Request Neard to stop polling
 *****************************************************************************/
static NCLError ncl_cmd_neardal_stop_poll(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr		= NCLERR_NOERROR;
	char		*adpName	= NULL;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Check if NEARDAL object exist */
	if (sNclCmdCtx.neardalMgr == NULL) {
		NCL_CMD_PRINTERR("Construct NEARDAL object...\n");
		nclErr = ncl_cmd_neardal_construct(argc, argv);
	}
	if ((nclErr != NCLERR_NOERROR) || (sNclCmdCtx.neardalMgr == NULL))
		return nclErr;

	adpName = argv[1];
	neardal_stop_poll(sNclCmdCtx.neardalMgr, adpName, &ec);
	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("Stop NFC polling error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/******************************************************************************
 * ncl_cmd_neardal_stop_poll : END
 *****************************************************************************/

/******************************************************************************
 * test parameter type (sample code) : BEGIN
 *****************************************************************************/
static gboolean test_cb(const gchar *opt, const gchar *arg, gpointer data,
			GError **err)
{
	gboolean success = TRUE;

	NCL_CMD_PRINT("Callback invoked from g_option_context_parse()\n");
	NCL_CMD_PRINT("opt = '%s'\n", opt);
	NCL_CMD_PRINT("arg = '%s'\n", arg);
	NCL_CMD_PRINT("data = 0x%08X\n", data);
	NCL_CMD_PRINT("err = 0x%08X\n", err);
	if (*err) {
		NCL_CMD_PRINT("*err->domain = %d\n", (*err)->domain);
		NCL_CMD_PRINT("*err->code = %d\n", (*err)->code);
		NCL_CMD_PRINT("*err->message = '%s'\n", (*err)->message);
	}

	/* If no parameter, emulate an error */
	if (arg[0] == 0) {
		if (err) {
			NCL_CMD_PRINTERR(
				"missing argument with parameter '%s'\n", opt);
			NCL_CMD_PRINTERR("Emulating parsing error\n");
			g_set_error(err, G_OPTION_ERROR,
				    G_OPTION_ERROR_UNKNOWN_OPTION,
				"Missing option (False Error, emulation)");

			success = FALSE;
		}
	}

	return success;
}

/* Parameters Command line test */
static NCLError ncl_cmd_test_parameters(int argc, char *argv[])
{
	static int		intTmp;
	static char		*stringTmp;
	static double		doubleTmp;
	static long long	int64Tmp;
	NCLError		err		= NCLERR_NOERROR;
static GOptionEntry options[] = {
		{ "int"	, 'i', 0, G_OPTION_ARG_INT		, &intTmp
				  , "Integer parameter", "9999" },

		{ "string", 's', 0, G_OPTION_ARG_STRING	, &stringTmp
				, "String parameter", "STRING" },

		{ "double", 'd', 0, G_OPTION_ARG_DOUBLE	, &doubleTmp
				, "Double parameter", "9.99" },

		{ "int64"	, 'l', 0, G_OPTION_ARG_INT64 , &int64Tmp
				, "Integer64 parameter", "9999" },
		{ "cb"	, 'c', 0, G_OPTION_ARG_CALLBACK	, test_cb
				, "Callback test", "9999" },
		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	/* Parse input parameters... */
	intTmp		= 0;
	stringTmp	= NULL;
	doubleTmp	= 0;
	int64Tmp	= 0ll;
	err = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	if (err != NCLERR_NOERROR)
		return err;

	/* Check 'int' parameter type... */
	NCL_CMD_PRINT("Integer parameter = %d\n", intTmp);

	/* Check 'string' parameter type... */
	if (stringTmp) {
		NCL_CMD_PRINT("String parameter = %s\n", stringTmp);
		g_free(stringTmp); stringTmp = NULL;
	}

	/* Check 'double' parameter type... */
	NCL_CMD_PRINT("Double parameter = %f\n", doubleTmp);

	/* Check 'int64' parameter type... */
	NCL_CMD_PRINT("Int64 parameter = %lld\n", int64Tmp);

	NCL_CMD_PRINT("The following type have not been tested :  ");
	NCL_CMD_PRINT("G_OPTION_ARG_FILENAME, G_OPTION_ARG_STRING_ARRAY");
	NCL_CMD_PRINT("G_OPTION_ARG_FILENAME_ARRAY.\n");
	NCL_CMD_PRINT("enjoy...\n");

	return err;
}
/******************************************************************************
 * test parameter type : END
 *****************************************************************************/


/******************************************************************************
 *
 *****************************************************************************/
/* Exiting from command line interpretor */
static NCLError ncl_cmd_quit(int argc, char *argv[])
{
	NCLError	err		= NCLERR_NOERROR;
	NCLContext	*nclCtxP	= NULL;

	(void) argc; /* remove warning */
	(void) argv; /* remove warning */
	nclCtxP = ncl_get_ctx();

	/* Release NEARDAL object */
	neardal_destroy(sNclCmdCtx.neardalMgr);
	sNclCmdCtx.neardalMgr = NULL;

	/* Quit Main Loop */
	if (nclCtxP)
		g_main_loop_quit(nclCtxP->main_loop);
	else
		err = NCLERR_GLOBAL_ERROR;

	return err;
}
/* END : Interpretor commands */


/******************************************************************************
 *
 *****************************************************************************/
/* Array of command line functions interpretor (alphabetical order) */
static NCLCmdInterpretor itFunc[] = {
	{ "get_adapters",
	ncl_cmd_neardal_get_adapters,
	"Get adapters list"},

	{ "get_adapter_properties",
	ncl_cmd_neardal_get_adapter_properties,
	"Get adapter properties (1st parameter is adapter name)"},

	{ "get_records",
	ncl_cmd_neardal_get_records,
	"Get records list (1st parameter is target name)"},

	{ "get_record_properties",
	ncl_cmd_neardal_get_record_properties,
	"Read a specific record. (1st parameter is record name)"},

	{ "get_targets",
	ncl_cmd_neardal_get_targets,
	"Get targets list (1st parameter is adapter name)"},

	{ "get_target_properties",
	ncl_cmd_neardal_get_target_properties,
	"Get target properties (1st parameter is target name)"},

	{ LISTCMD_NAME,
	ncl_cmd_list,
	"List all available commands. 'cmd' --help -h /? for a specific help" },

	{ "publish",
	ncl_cmd_neardal_publish,
	"Creates a NDEF record from parametersto be written to an NFC tag"},

	{ "quit",
	ncl_cmd_quit,
	"Exit from command line interpretor" },

	{ "start_poll",
	ncl_cmd_neardal_start_poll,
	"Request Neard to start polling (1st parameter is adapter name)"},

	{ "stop_poll",
	ncl_cmd_neardal_stop_poll,
	"Request Neard to stop polling (1st parameter is adapter name)"},

	{ "test_parameters",
	ncl_cmd_test_parameters,
	"Simple test to parse input parameters"}
};
#define NB_CL_FUNC		(sizeof(itFunc) / sizeof(NCLCmdInterpretor))


/******************************************************************************
 *
 *****************************************************************************/
void ncl_cmd_print(FILE *stream, char *format, ...)
{
	gchar	*bufTrace;
	va_list ap;

	va_start(ap, format);

	bufTrace = g_strdup_vprintf(format, ap);
	if (bufTrace != NULL) {
		fprintf(stream, "%s", bufTrace);
		fflush(stream);
	}
	va_end(ap);
	g_free(bufTrace);
}


/******************************************************************************
 *
 *****************************************************************************/
NCLCmdInterpretor *ncl_cmd_get_list(int *nbCmd)
{
	if (nbCmd != NULL)
		*nbCmd = NB_CL_FUNC;
	return itFunc;
}


/******************************************************************************
 *
 *****************************************************************************/
int ncl_cmd_get_nbCmd(void)
{
	return NB_CL_FUNC;
}


/******************************************************************************
 *
 *****************************************************************************/
NCLCmdContext *ncl_cmd_get_ctx(void)
{
	return &sNclCmdCtx;
}

/******************************************************************************
 *
 *****************************************************************************/
NCLError ncl_cmd_init(char *execCmdLineStr)
{
	memset(&sNclCmdCtx, 0, sizeof(sNclCmdCtx));

	if (!execCmdLineStr)
		sNclCmdCtx.clBuf = g_string_sized_new(CL_BUF_SIZE);
	else
		sNclCmdCtx.clBuf = g_string_new(execCmdLineStr);

	if (sNclCmdCtx.clBuf == NULL) {
		NCL_CMD_PRINTERR("Unable to allocate %d bytes\n", CL_BUF_SIZE);
		return NCLERR_INIT;
	}

	return NCLERR_NOERROR;
}


/******************************************************************************
 *
 *****************************************************************************/
void ncl_cmd_finalize(void)
{

	if (sNclCmdCtx.clBuf != NULL)
		g_string_free(sNclCmdCtx.clBuf, TRUE);

	/* Release NFC object */
	if (sNclCmdCtx.neardalMgr != NULL)
		neardal_destroy(sNclCmdCtx.neardalMgr);

}
