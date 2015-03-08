/*
 *     NEARDAL Tester command line interpreter
 *
 *     Copyright 2014      Marvell International Ltd.
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

NCLError ncl_cmd_list(int argc, char *argv[]);

/* Local Utilities functions */
/*****************************************************************************
 * Get readable string of bytes
 ****************************************************************************/
static gchar* ncl_cmd_prv_bytes_to_str(GBytes* bytes)
{
	gchar* str = g_malloc0( 2*g_bytes_get_size(bytes) + 1 );
	const guint8* data = g_bytes_get_data(bytes, NULL);
	for(int i = 0 ; i < g_bytes_get_size(bytes); i++)
	{
		sprintf(&str[2*i], "%02X", data[i]);
	}
	return str;
}

/*****************************************************************************
 * Tool function : help command to dump parameters command
 ****************************************************************************/
static void ncl_cmd_prv_dumpOptions(GOptionEntry *options)
{
	GOptionEntry	*optP = options;
	char		long_nameTmp[20];

	while (optP->description != NULL) {
		snprintf(long_nameTmp, sizeof(long_nameTmp), "--%s",
			 optP->long_name);

		NCL_CMD_PRINT("-%c,\t%s=%s\t%20s\n", optP->short_name,
			      long_nameTmp, optP->arg_description,
			      optP->description);
		optP++;
	}
}


/*****************************************************************************
 * Tool function : parse parameters command (like g_option_context_parse but
 * implicit exit() on '--help' disabled
 ****************************************************************************/
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


/*****************************************************************************
 * Dump properties of an adapter
 ****************************************************************************/
static void ncl_cmd_prv_dump_adapter(neardal_adapter *adapter)
{
	char **protocols;
	char **tags;
	char **devs;

	NCL_CMD_PRINT("Adapter\n");
	NCL_CMD_PRINT(".. Name:\t\t'%s'\n", adapter->name);

	NCL_CMD_PRINT(".. Polling:\t\t'%s'\n",
		      adapter->polling ? "TRUE" : "FALSE");
	NCL_CMD_PRINT(".. Powered:\t\t'%s'\n",
		      adapter->powered ? "TRUE" : "FALSE");
	if (adapter->mode)
		NCL_CMD_PRINT(".. NFC Radio Mode:\t'%s'\n", adapter->mode);

	tags = adapter->tags;
	NCL_CMD_PRINT(".. Number of tags:\t%d\n", adapter->nbTags);
	NCL_CMD_PRINT(".. Tags[]:\t\t");
	if (adapter->nbTags > 0) {
		while ((*tags) != NULL) {
			NCL_CMD_PRINT("'%s', ", *tags);
			tags++;
		}
	} else
		NCL_CMD_PRINT("No tags!");
	NCL_CMD_PRINT("\n");

	devs = adapter->devs;
	NCL_CMD_PRINT(".. Number of devices:\t%d\n", adapter->nbDevs);
	NCL_CMD_PRINT(".. Devs[]:\t\t");
	if (adapter->nbDevs > 0) {
		while ((*devs) != NULL) {
			NCL_CMD_PRINT("'%s', ", *devs);
			devs++;
		}
	} else
		NCL_CMD_PRINT("No devs!");
	NCL_CMD_PRINT("\n");

	protocols = adapter->protocols;
	NCL_CMD_PRINT(".. Number of protocols:\t%d\n", adapter->nbProtocols);
	NCL_CMD_PRINT(".. Protocols[]:\t\t");
	if (adapter->nbProtocols > 0) {
		while ((*protocols) != NULL) {
			NCL_CMD_PRINT("'%s', ", *protocols);
			protocols++;
		}
	} else
		NCL_CMD_PRINT("No protocols!");
	NCL_CMD_PRINT("\n");
}

/*****************************************************************************
 * Dump properties of a tag
 ****************************************************************************/
static void ncl_cmd_prv_dump_tag(neardal_tag *tag)
{
	char **records;
	char **tagTypes;

	NCL_CMD_PRINT("Tag:\n");
	NCL_CMD_PRINT(".. Name:\t\t'%s'\n", tag->name);

	NCL_CMD_PRINT(".. Type:\t\t'%s'\n", tag->type);

	NCL_CMD_PRINT(".. Number of 'Tag Type':%d\n", tag->nbTagTypes);
	tagTypes = tag->tagType;
	if (tag->nbTagTypes > 0) {
		NCL_CMD_PRINT(".. Tags type[]:\t\t");
		while ((*tagTypes) != NULL) {
			NCL_CMD_PRINT("'%s', ", *tagTypes);
			tagTypes++;
		}
		NCL_CMD_PRINT("\n");
	}

	records = tag->records;
	NCL_CMD_PRINT(".. Number of records:\t%d\n", tag->nbRecords);
	NCL_CMD_PRINT(".. Records[]:\t\t");
	if (records != NULL) {
		while ((*records) != NULL) {
			NCL_CMD_PRINT("'%s', ", *records);
			records++;
		}
	} else
		NCL_CMD_PRINT("No records!");

	NCL_CMD_PRINT("\n");
	NCL_CMD_PRINT(".. ReadOnly:\t\t%s\n"	,
		      tag->readOnly ? "TRUE" : "FALSE");

	if(tag->iso14443aAtqa != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->iso14443aAtqa);
		NCL_CMD_PRINT(".. ISO14443A ATQA:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->iso14443aSak != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->iso14443aSak);
		NCL_CMD_PRINT(".. ISO14443A SAK:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->iso14443aUid != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->iso14443aUid);
		NCL_CMD_PRINT(".. ISO14443A UID:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaManufacturer != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->felicaManufacturer);
		NCL_CMD_PRINT(".. Felica Manufacturer:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaCid != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->felicaCid);
		NCL_CMD_PRINT(".. Felica CID:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaIc != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->felicaIc);
		NCL_CMD_PRINT(".. Felica IC Code:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaMaxRespTimes != NULL)
	{
		gchar *str = ncl_cmd_prv_bytes_to_str(tag->felicaMaxRespTimes);
		NCL_CMD_PRINT(".. Felica Maximum Response times:\t\t'%s'\n", str);
		g_free(str);
	}
}

/*****************************************************************************
 * Dump properties of a device
 ****************************************************************************/
static void ncl_cmd_prv_dump_dev(neardal_dev *dev)
{
	char **records;

	NCL_CMD_PRINT("Dev:\n");
	NCL_CMD_PRINT(".. Name:\t\t'%s'\n", dev->name);


	records = dev->records;
	NCL_CMD_PRINT(".. Number of records:\t%d\n", dev->nbRecords);
	NCL_CMD_PRINT(".. Records[]:\t\t");
	if (records != NULL) {
		while ((*records) != NULL) {
			NCL_CMD_PRINT("'%s', ", *records);
			records++;
		}
	} else
		NCL_CMD_PRINT("No records!");

	NCL_CMD_PRINT("\n");

}

static void ncl_cmd_prv_dump_record(neardal_record *record)
{
	neardal_g_variant_dump(neardal_record_to_g_variant(record));
}

/*****************************************************************************
 * neardal callbacks : BEGIN
 ****************************************************************************/
static void ncl_cmd_cb_adapter_added(const char *adpName, void *user_data)
{
	errorCode_t	ec;
	neardal_adapter	*adapter;

	(void) user_data; /* Remove warning */

	NCL_CMD_PRINTF("NFC Adapter added '%s'\n", adpName);
	ec = neardal_get_adapter_properties(adpName, &adapter);
	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_adapter(adapter);
		neardal_free_adapter(adapter);
	} else
		NCL_CMD_PRINTF(
		"Unable to read adapter properties. (error:%d='%s'). exit...\n",
			       ec, neardal_error_get_text(ec));

	return;
}

static void ncl_cmd_cb_adapter_removed(const char *adpName, void *user_data)
{
	(void) user_data; /* remove warning */

	NCL_CMD_PRINTF("NFC Adapter removed '%s'\n", adpName);
}

static void ncl_cmd_cb_adapter_prop_changed(char *adpName, char *propName,
					    void *value, void *user_data)
{
	int		polling;

	(void) user_data; /* remove warning */

	if (!strcmp(propName, "Polling")) {
		polling = *(int*)&value;
		NCL_CMD_PRINTF("Polling=%d\n", polling);
	} else
		NCL_CMD_PRINTF("Adapter '%s' -> Property=%s=0x%X\n", adpName,
				propName, GPOINTER_TO_UINT(value));

	NCL_CMD_PRINT("\n");
}

static void ncl_cmd_cb_tag_found(const char *tagName, void *user_data)
{
	neardal_tag	*tag;
	errorCode_t	ec;

	(void) user_data; /* remove warning */

	NCL_CMD_PRINTF("NFC Tag found (%s)\n", tagName);

	ec = neardal_get_tag_properties(tagName, &tag);
	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_tag(tag);
		neardal_free_tag(tag);
	} else
		NCL_CMD_PRINTF(
		"Unable to read tag properties. (error:%d='%s'). exit...\n",
			       ec, neardal_error_get_text(ec));
	return;
}

static void ncl_cmd_cb_tag_lost(const char *tagName, void *user_data)
{
	(void) user_data; /* remove warning */
	NCL_CMD_PRINTF("NFC Tag lost (%s)\n", tagName);
}

static void ncl_cmd_cb_dev_found(const char *devName, void *user_data)
{
	neardal_dev	*dev;
	errorCode_t	ec;

	(void) user_data; /* remove warning */

	NCL_CMD_PRINTF("NFC Device found (%s)\n", devName);

	ec = neardal_get_dev_properties(devName, &dev);
	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_dev(dev);
		neardal_free_device(dev);
	} else
		NCL_CMD_PRINTF(
		"Unable to read device properties. (error:%d='%s'). exit...\n",
			       ec, neardal_error_get_text(ec));
	return;
}

static void ncl_cmd_cb_dev_lost(const char *devName, void *user_data)
{
	(void) user_data; /* remove warning */
	NCL_CMD_PRINTF("NFC Dev lost (%s)\n", devName);
}

static void ncl_cmd_cb_record_found(const char *rcdName, void *user_data)
{
	errorCode_t	ec;
	neardal_record	*record;

	(void) user_data; /* remove warning */

	NCL_CMD_PRINTF("Tag Record found (%s)\n", rcdName);
	ec = neardal_get_record_properties(rcdName, &record);
	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_record(record);
		neardal_free_record(record);
	} else
		NCL_CMD_PRINTF("Read record error. (error:%d='%s').\n", ec,
			       neardal_error_get_text(ec));

	return;
}

/*****************************************************************************
 * neardal callbacks : END
 ****************************************************************************/

static void ncl_cmd_install_callback(void)
{
	neardal_set_cb_adapter_added(ncl_cmd_cb_adapter_added, NULL);
	neardal_set_cb_adapter_removed(ncl_cmd_cb_adapter_removed, NULL);
	neardal_set_cb_adapter_property_changed(ncl_cmd_cb_adapter_prop_changed,
						NULL);
	NCL_CMD_PRINTF("NFC adapter callback registered\n");
	neardal_set_cb_tag_found(ncl_cmd_cb_tag_found, NULL);
	neardal_set_cb_tag_lost(ncl_cmd_cb_tag_lost, NULL);
	NCL_CMD_PRINTF("NFC tag registered\n");
	neardal_set_cb_dev_found(ncl_cmd_cb_dev_found, NULL);
	neardal_set_cb_dev_lost(ncl_cmd_cb_dev_lost, NULL);
	NCL_CMD_PRINTF("NFC dev registered\n");
	neardal_set_cb_record_found(ncl_cmd_cb_record_found, NULL);
	NCL_CMD_PRINTF("NFC record callback registered\n\n");
	sNclCmdCtx.cb_initialized = true;
}

/*****************************************************************************
 * neardal_get_adapters : BEGIN
 * Get adapters List
 ****************************************************************************/
static NCLError ncl_cmd_get_adapters(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**adpArray = NULL;
	int		adpLen;
	int		adpOff;

	(void) argc; /* Remove warning */
	(void) argv; /* Remove warning */

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_get_adapters(&adpArray, &adpLen);
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
/*****************************************************************************
 * neardal_get_adapters : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_get_adapter_properties : BEGIN
 * Read adapter properties
 ****************************************************************************/
static NCLError ncl_cmd_get_adapter_properties(int argc, char *argv[])
{
	errorCode_t	ec;
	char		*adapterName	= NULL;
	neardal_adapter	*adapter;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	adapterName = argv[1];
	ec = neardal_get_adapter_properties(adapterName, &adapter);

	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_adapter(adapter);
		neardal_free_adapter(adapter);
	} else {
		NCL_CMD_PRINTF("Read adapter properties error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/*****************************************************************************
 * ncl_cmd_get_adapter_properties : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_get_devices : BEGIN
 * Get devices List
 ****************************************************************************/
static NCLError ncl_cmd_get_devices(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**devArray = NULL;
	int		devLen;
	int		devOff;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_get_devices(argv[1], &devArray, &devLen);
	if (ec == NEARDAL_SUCCESS) {
		devOff = 0;
			/* For each dev */
		while (devArray[devOff] != NULL)
			NCL_CMD_PRINT(".. Dev '%s'\n",
					devArray[devOff++]);

		neardal_free_array(&devArray);
	} else
		NCL_CMD_PRINTF("No dev\n");

	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	if (ec == NEARDAL_SUCCESS)
		nclErr =  NCLERR_NOERROR ;
	else
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/*****************************************************************************
 * ncl_cmd_get_devices : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_get_device_properties : BEGIN
 * Read device properties
 ****************************************************************************/
static NCLError ncl_cmd_get_device_properties(int argc, char *argv[])
{
	errorCode_t	ec;
	char		*devName	= NULL;
	neardal_dev	*dev;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	devName = argv[1];
	ec = neardal_get_dev_properties(devName, &dev);

	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_dev(dev);
		neardal_free_device(dev);
	} else {
		NCL_CMD_PRINTF("Read dev properties error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/*****************************************************************************
 * ncl_cmd_get_device_properties : END
 ****************************************************************************/

/*****************************************************************************
 * neardal_get_tags : BEGIN
 * Get tags List
 ****************************************************************************/
static NCLError ncl_cmd_get_tags(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**tagArray = NULL;
	int		tagLen;
	int		tagOff;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_get_tags(argv[1], &tagArray, &tagLen);
	if (ec == NEARDAL_SUCCESS) {
		tagOff = 0;
			/* For each tag */
		while (tagArray[tagOff] != NULL)
			NCL_CMD_PRINT(".. Tag '%s'\n",
					tagArray[tagOff++]);

		neardal_free_array(&tagArray);
	} else
		NCL_CMD_PRINTF("No tag\n");

	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	if (ec == NEARDAL_SUCCESS)
		nclErr =  NCLERR_NOERROR ;
	else
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/*****************************************************************************
 * neardal_get_tags : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_get_tag_properties : BEGIN
 * Read tag properties
 ****************************************************************************/
static NCLError ncl_cmd_get_tag_properties(int argc, char *argv[])
{
	errorCode_t	ec;
	char		*tagName	= NULL;
	neardal_tag	*tag;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	tagName = argv[1];
	ec = neardal_get_tag_properties(tagName, &tag);

	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_tag(tag);
		neardal_free_tag(tag);
	} else {
		NCL_CMD_PRINTF("Read tag properties error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/*****************************************************************************
 * ncl_cmd_get_tag_properties : END
 ****************************************************************************/

/*****************************************************************************
 * neardal_get_records : BEGIN
 * Get records List
 ****************************************************************************/
static NCLError ncl_cmd_get_records(int argc, char *argv[])
{
	errorCode_t	ec;
	NCLError	nclErr;
	char		**rcdArray = NULL;
	int		rcdLen;
	int		rcdOff;


	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_get_records(argv[1], &rcdArray, &rcdLen);
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
/*****************************************************************************
 * neardal_get_records : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_get_record_properties : BEGIN
 * Read a specific tag
 ****************************************************************************/
static NCLError ncl_cmd_get_record_properties(int argc, char *argv[])
{
	errorCode_t ec;
	char		*recordName	= NULL;
	neardal_record	*record;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	recordName = argv[1];
	ec = neardal_get_record_properties(recordName, &record);
	if (ec == NEARDAL_SUCCESS) {
		ncl_cmd_prv_dump_record(record);
		neardal_free_record(record);
	} else {
		NCL_CMD_PRINTF("Read record error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/*****************************************************************************
 * ncl_cmd_get_record_properties : END
 ****************************************************************************/
/*****************************************************************************
 * ncl_cmd_push : BEGIN
 * Push NDEF record to device
 ****************************************************************************/
static NCLError ncl_cmd_push(int argc, char *argv[])
{
	errorCode_t		ec = NEARDAL_SUCCESS;
	NCLError		nclErr;
	static neardal_record	rcd;
	static int		smartPoster;

static GOptionEntry options[] = {
		{ "act", 'c', 0, G_OPTION_ARG_STRING, &rcd.action
				  , "Action", "save"},

		{ "dev", 'a', 0, G_OPTION_ARG_STRING, &rcd.name
				  , "Device name", "/org/neard/nfc0/device0"},

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

		{ "carrier", 'u', 0, G_OPTION_ARG_STRING, &rcd.carrier
				  , "Carrier", "bluetooth" },

		{ "ssid", 'd', 0, G_OPTION_ARG_STRING, &rcd.ssid
				  , "SSID", "ssid" },

		{ "passphrase", 'p', 0, G_OPTION_ARG_STRING, &rcd.passphrase
				  , "Passphrase", "psk" },

		{ "encryption", 'c', 0, G_OPTION_ARG_STRING , &rcd.encryption
                                  , "List separated by ','"
		                  , "NONE,WEP,TKIP,AES"},

		{ "authentication", 'z', 0, G_OPTION_ARG_STRING
		                  , &rcd.authentication
		                  , "List separated by ','"
		                  , "OPEN,WPA-Personal,Shared,WPA-Enterprise,"
		                    "WPA2-Enterprise,WPA2-Personal" },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc > 1) {
		/* Parse options */
		memset(&rcd, 0, sizeof(neardal_record));
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR) {
		NCL_CMD_PRINT("Sample (Type 'Text'):");
		NCL_CMD_PRINT("e.g. < push --type Text --lang en-US \
--encoding UTF-8 --rep \"Simple text\" --dev /org/neard/nfc0/device0 >\n");
		NCL_CMD_PRINT("Sample (Type 'URI'):");
		NCL_CMD_PRINT("e.g. < push --type URI \
--uri=http://www.nfc-forum.com  --dev /org/neard/nfc0/device0 >\n");
		NCL_CMD_PRINT("Sample (Type 'SmartPoster'):");
		NCL_CMD_PRINT("e.g. < push --type=SmartPoster \
--uri=http://www.nfc-forum.com > --dev /org/neard/nfc0/device0 >\n");
	}

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_dev_push(&rcd);

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
	if (rcd.carrier != NULL)
		g_free((gchar *) rcd.carrier);
	if (rcd.ssid != NULL)
		g_free((gchar *) rcd.ssid);
	if (rcd.passphrase != NULL)
		g_free((gchar *) rcd.passphrase);
	if (rcd.authentication != NULL)
		g_free((gchar *) rcd.authentication);
	if (rcd.encryption != NULL)
		g_free((gchar *) rcd.encryption);

	return nclErr;
}
/*****************************************************************************
 * ncl_cmd_push : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_write : BEGIN
 * write NDEF record to tag
 ****************************************************************************/
static NCLError ncl_cmd_write(int argc, char *argv[])
{
	errorCode_t		ec = NEARDAL_SUCCESS;
	NCLError		nclErr;
	static neardal_record	rcd;
	static int		smartPoster;

	static GOptionEntry options[] = {
		{ "act", 'c', 0, G_OPTION_ARG_STRING, &rcd.action
				  , "Action", "save"},

		{ "tag", 'a', 0, G_OPTION_ARG_STRING, &rcd.name
				  , "Tag name", "/org/neard/nfc0/tag0"},

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

		{ "carrier", 'u', 0, G_OPTION_ARG_STRING, &rcd.carrier
				  , "Carrier", "bluetooth" },

		{ "ssid", 'd', 0, G_OPTION_ARG_STRING, &rcd.ssid
				  , "SSID", "ssid" },

		{ "passphrase", 'p', 0, G_OPTION_ARG_STRING, &rcd.passphrase
				  , "Passphrase", "psk" },

		{ "encryption", 'c', 0, G_OPTION_ARG_STRING , &rcd.encryption
                                  , "List separated by ','"
		                  , "NONE,WEP,TKIP,AES"},

		{ "authentication", 'z', 0, G_OPTION_ARG_STRING
		                  , &rcd.authentication
		                  , "List separated by ','"
		                  , "OPEN,WPA-Personal,Shared,WPA-Enterprise,"
		                    "WPA2-Enterprise,WPA2-Personal" },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	memset(&rcd, 0, sizeof(neardal_record));
	if (argc > 1) {
		/* Parse options */
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR) {
		NCL_CMD_PRINT("Sample (Type 'Text'):");
		NCL_CMD_PRINT("e.g. < write --type Text --lang en-US \
--encoding UTF-8 --rep \"Simple text\" --tag /org/neard/nfc0/tag0 >\n");
		NCL_CMD_PRINT("Sample (Type 'URI'):");
		NCL_CMD_PRINT("e.g. < write --type URI \
--uri=http://www.nfc-forum.com  --tag /org/neard/nfc0/tag0 >\n");
		NCL_CMD_PRINT("Sample (Type 'SmartPoster'):");
		NCL_CMD_PRINT("e.g. < write --type=SmartPoster \
--uri=http://www.nfc-forum.com > --tag /org/neard/nfc0/tag0 >\n");
	}

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_tag_write(&rcd);

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
	if (rcd.ssid != NULL)
		g_free((gchar *) rcd.ssid);
	if (rcd.passphrase != NULL)
		g_free((gchar *) rcd.passphrase);
	if (rcd.authentication != NULL)
		g_free((gchar *) rcd.authentication);
	if (rcd.encryption != NULL)
		g_free((gchar *) rcd.encryption);


	return nclErr;
}
/*****************************************************************************
 * ncl_cmd_write : END
 ****************************************************************************/


/*****************************************************************************
 * ncl_cmd_set_adp_property : BEGIN
 * write NDEF record to tag
 ****************************************************************************/
static NCLError ncl_cmd_set_adapter_property(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr;
	static int	powered		= -1;
	char		*adapterName	= NULL;

static GOptionEntry options[] = {
		{ "powered", 's', 0, G_OPTION_ARG_INT , &powered
				, "Set Adapter power ON/OFF", "<>0 or =0" },
		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc > 2) {
		/* Parse options */
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR) {
		NCL_CMD_PRINT("e.g. < %s /org/neard/nfc0 --powered=1 \
>\n", argv[0]);
	}

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	adapterName = argv[1];
	if (powered >= 0)
		ec = neardal_set_adapter_property(adapterName,
						NEARD_ADP_PROP_POWERED,
						GINT_TO_POINTER(powered));

exit:
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return nclErr;
}
/*****************************************************************************
 * ncl_cmd_set_adp_property : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_start_poll : BEGIN
 * Request Neard to start polling
 ****************************************************************************/
static NCLError ncl_cmd_start_poll(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr;
	char		*adpName	= NULL;
	static char	*strMode;

static GOptionEntry options[] = {
		{ "mode", 's', 0, G_OPTION_ARG_STRING , &strMode
				, "Set Adapter mode initiator/target/dual",
				"'initiator, target, dual'" },
		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	if (argc > 1) {
		/* Parse options */
		strMode = NULL;
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Start polling if adapter present */
	adpName = argv[1];
	if (strMode != NULL) {
		if (!strcmp(strMode, "initiator"))
			ec = neardal_start_poll_loop(adpName,
						     NEARD_ADP_MODE_INITIATOR);
		else if (!strcmp(strMode, "target"))
			ec = neardal_start_poll_loop(adpName,
						     NEARD_ADP_MODE_TARGET);
		else if (!strcmp(strMode, "dual"))
			ec = neardal_start_poll_loop(adpName,
						     NEARD_ADP_MODE_DUAL);
		else
			ec = neardal_start_poll_loop(adpName,
						     NEARD_ADP_MODE_INITIATOR);
	} else
		ec = neardal_start_poll(adpName);

	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("NFC polling activation error:%d='%s'\n",
				ec, neardal_error_get_text(ec));
		if (ec == NEARDAL_ERROR_POLLING_ALREADY_ACTIVE)
			ec = NEARDAL_SUCCESS;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

exit:
	if (strMode != NULL)
		g_free(strMode);

	if (ec != NEARDAL_SUCCESS)
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/*****************************************************************************
 * ncl_cmd_start_poll : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_stop_poll : BEGIN
 * Request Neard to stop polling
 ****************************************************************************/
static NCLError ncl_cmd_stop_poll(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	char		*adpName	= NULL;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	adpName = argv[1];
	ec = neardal_stop_poll(adpName);
	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("Stop NFC polling error:%d='%s'.\n", ec,
			       neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

	return NCLERR_NOERROR;
}
/*****************************************************************************
 * ncl_cmd_stop_poll : END
 ****************************************************************************/


/*****************************************************************************
 * ncl_cmd_(un)register_NDEF_agent : BEGIN
 * Handle a record macthing a registered tag type
 ****************************************************************************/
void ncl_cmd_ndef_agent_cb(unsigned char **rcdArray, unsigned int rcdLen
		      , unsigned char *ndefArray, unsigned int ndefLen
		      , void *user_data)
{
	unsigned int i;

	(void) user_data;

	NCL_CMD_PRINTF("Received %d records :\n"
		      , rcdLen);
	for (i = 0; i < rcdLen; i++) {
		NCL_CMD_PRINT("Record : '%s'\n", rcdArray[i]);
	}

	NCL_CMD_PRINT("\n");
	NCL_CMD_PRINTF("%d bytes of NDEF raw data Received :\n"
		      , ndefLen);
	NCL_CMD_DUMP(ndefArray, ndefLen);
	NCL_CMD_PRINT("\n");

	NCL_CMD_PRINTF("user_data= %s\n", (char *) user_data);

}

void ncl_cmd_ndef_agent_release_cb(void *user_data)
{
	NCL_CMD_PRINTF("Release user_data=%s\n", (char *) user_data);
	g_free(user_data);
}

static NCLError ncl_cmd_register_NDEF_agent(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr;
	static char	*tagType;
	char		*test_user_data;

static GOptionEntry options[] = {
		{ "tagType", 's', 0, G_OPTION_ARG_STRING , &tagType
				, "tag Type to register",
				"'urn:nfc:wkt:U'..." },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc > 1) {
		/* Parse options */
		tagType = NULL;
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR) {
		NCL_CMD_PRINT("Sample (Type 'URI'):");
		NCL_CMD_PRINT("e.g. < %s --tagType urn:nfc:wkt:U >\n"
			     , argv[0]);
	}
	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	test_user_data = g_strdup("test NDEF user data");
	ec = neardal_agent_set_NDEF_cb(tagType, ncl_cmd_ndef_agent_cb
				       , ncl_cmd_ndef_agent_release_cb
				       , test_user_data);
	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("Set NDEF callback failed! error:%d='%s'.\n",
			       ec, neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

exit:
	g_free(tagType);
	tagType = NULL;

	return NCLERR_NOERROR;
}
static NCLError ncl_cmd_unregister_NDEF_agent(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr;
	static char	*tagType;

	if (argc <= 1)
		return NCLERR_PARSING_PARAMETERS;

static GOptionEntry options[] = {
		{ "tagType", 's', 0, G_OPTION_ARG_STRING , &tagType
				, "tag Type to unregister",
				"'Text', 'URI'..." },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc > 1) {
		/* Parse options */
		tagType = NULL;
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	ec = neardal_agent_set_NDEF_cb(tagType, NULL, NULL, NULL);
	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("Set NDEF callback failed! error:%d='%s'.\n",
			       ec, neardal_error_get_text(ec));
		return NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

exit:
	g_free(tagType);
	tagType = NULL;

	return NCLERR_NOERROR;
}

/*****************************************************************************
 * ncl_cmd_(un)register_NDEF_agent : END
 ****************************************************************************/

/*****************************************************************************
 * ncl_cmd_(un)register_handover_agent : BEGIN
 * Handle a record macthing a registered tag type
 ****************************************************************************/
void ncl_cmd_handover_req_bluetooth_agent_cb(unsigned char *blobEIR
				   , unsigned int blobSize
				   , unsigned char ** oobData
				   , unsigned int * oobDataSize
				   , freeFunc *freeF
				   , void *user_data)
{
	const unsigned char test_data[] = {0x16,0x00,0x01,0x02,0x03,0x04,
					    0x05,0x06,0x08,0x09,0x41,0x72,
					    0x72,0x61,0x6b,0x69,0x73,0x04,
					    0x0d,0x6e,0x01,0x00};

	(void) user_data;

	NCL_CMD_PRINTF("Received blobEIR = \n");
	NCL_CMD_DUMP(blobEIR, blobSize);

	NCL_CMD_PRINTF("user_data= %s\n", (char *) user_data);

	*oobData = g_try_malloc0(sizeof(test_data));
	memcpy(*oobData , test_data, sizeof(test_data));
	*oobDataSize = sizeof(test_data);
	*freeF = g_free;
	NCL_CMD_PRINTF("return oobData = \n");
	NCL_CMD_DUMP(*oobData, *oobDataSize);
}

void ncl_cmd_handover_push_bluetooth_agent_cb (unsigned char *blobEIR
				     , unsigned int blobSize
				     , void *user_data)
{
	NCL_CMD_PRINTF("Received blobEIR = \n");
	NCL_CMD_DUMP(blobEIR, blobSize);
	(void) user_data;

	NCL_CMD_PRINTF("\n");
}

void ncl_cmd_handover_req_wifi_agent_cb(unsigned char *blobWSC
				   , unsigned int blobSize
				   , unsigned char ** oobData
				   , unsigned int * oobDataSize
				   , freeFunc *freeF
				   , void *user_data)
{
	const unsigned char test_data[] = {0x10,0x4A,0x00,0x01,0x10,
			 	 	 	0x10,0x45,0x00,0x08,
			 	 	 	0x74,0x65,0x73,0x74,0x73,0x73,0x69,0x64,
			 	 	 	0x10,0x27,0x00,0x06,
			 	 	 	0x62,0x6C,0x61,0x62,0x6C,0x61};

	(void) user_data;

	NCL_CMD_PRINTF("Received blobWSC = \n");
	NCL_CMD_DUMP(blobWSC, blobSize);

	NCL_CMD_PRINTF("user_data= %s\n", (char *) user_data);

	*oobData = g_try_malloc0(sizeof(test_data));
	memcpy(*oobData , test_data, sizeof(test_data));
	*oobDataSize = sizeof(test_data);
	*freeF = g_free;
	NCL_CMD_PRINTF("return oobData = \n");
	NCL_CMD_DUMP(*oobData, *oobDataSize);
}

void ncl_cmd_handover_push_wifi_agent_cb (unsigned char *blobWSC
				     , unsigned int blobSize
				     , void *user_data)
{
	NCL_CMD_PRINTF("Received blobWSC = \n");
	NCL_CMD_DUMP(blobWSC, blobSize);
	(void) user_data;

	NCL_CMD_PRINTF("\n");
}


void ncl_cmd_handover_release_agent_cb(void *user_data)
{
	NCL_CMD_PRINTF("Release user_data= %s\n", (char *) user_data);
	g_free(user_data);
}


static NCLError ncl_cmd_register_handover_agent(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr;
	static char		*carrier = NULL;
	char		*test_user_data = NULL;

	static GOptionEntry options[] = {
		{ "carrier", 's', 0, G_OPTION_ARG_STRING , &carrier
				, "carrier to register",
				"'bluetooth, wifi'" },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc > 1) {
		/* Parse options */
		carrier = NULL;
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	if (!strcmp(carrier, "bluetooth")) {
		test_user_data = g_strdup("test HANDOVER user data");

		ec = neardal_agent_set_handover_cb(
							 carrier
						   , ncl_cmd_handover_push_bluetooth_agent_cb
						   , ncl_cmd_handover_req_bluetooth_agent_cb
						   , ncl_cmd_handover_release_agent_cb
						   , test_user_data);
	} else if (!strcmp(carrier, "wifi")) {
		test_user_data = g_strdup("test HANDOVER user data");

		ec = neardal_agent_set_handover_cb(
							 carrier
						   , ncl_cmd_handover_push_wifi_agent_cb
						   , ncl_cmd_handover_req_wifi_agent_cb
						   , ncl_cmd_handover_release_agent_cb
						   , test_user_data);
	}
	else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("Set handover callback failed! error:%d='%s'.\n",
			       ec, neardal_error_get_text(ec));
		nclErr = NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

exit:
	if (carrier != NULL)
		g_free(carrier);

	if (test_user_data != NULL)
		g_free(test_user_data);

	if (nclErr != NCLERR_NOERROR) {
		NCL_CMD_PRINT("Sample (WiFi agent):");
		NCL_CMD_PRINT("e.g. < %s --carrier wifi >\n"
				 , argv[0]);
	}

	if (ec != NEARDAL_SUCCESS)
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
static NCLError ncl_cmd_unregister_handover_agent(int argc, char *argv[])
{
	errorCode_t	ec		= NEARDAL_SUCCESS;
	NCLError	nclErr;
	static char		*carrier = NULL;
	char		*test_user_data = NULL;

	static GOptionEntry options[] = {
		{ "carrier", 's', 0, G_OPTION_ARG_STRING , &carrier
				, "carrier to unregister",
				"'bluetooth, wifi'" },

		{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
	};

	if (argc > 1) {
		/* Parse options */
		carrier = NULL;
		nclErr = ncl_cmd_prv_parseOptions(&argc, &argv, options);
	} else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (nclErr != NCLERR_NOERROR)
		goto exit;

	/* Install Neardal Callback*/
	if (sNclCmdCtx.cb_initialized == false)
		ncl_cmd_install_callback();

	if (!strcmp(carrier, "bluetooth")) {
		ec = neardal_agent_set_handover_cb(
							 carrier
						   , NULL
						   , NULL
						   , NULL
						   , NULL);
	} else if (!strcmp(carrier, "wifi")) {
		ec = neardal_agent_set_handover_cb(
							 carrier
						   , NULL
						   , NULL
						   , NULL
						   , NULL);
	}
	else
		nclErr = NCLERR_PARSING_PARAMETERS;

	if (ec != NEARDAL_SUCCESS) {
		NCL_CMD_PRINTF("Set handover callback failed! error:%d='%s'.\n",
			       ec, neardal_error_get_text(ec));
		nclErr = NCLERR_LIB_ERROR;
	}
	NCL_CMD_PRINT("\nExit with error code %d:%s\n", ec,
		      neardal_error_get_text(ec));

exit:
	if (carrier != NULL)
		g_free(carrier);

	if (test_user_data != NULL)
		g_free(test_user_data);

	if (nclErr != NCLERR_NOERROR) {
		NCL_CMD_PRINT("Sample (WiFi agent):");
		NCL_CMD_PRINT("e.g. < %s --carrier wifi >\n"
				 , argv[0]);
	}

	if (ec != NEARDAL_SUCCESS)
		nclErr = NCLERR_LIB_ERROR;

	return nclErr;
}
/*****************************************************************************
 * ncl_cmd_(un)register_handover_agent : END
 ****************************************************************************/


/*****************************************************************************
 * test parameter type (sample code) : BEGIN
 ****************************************************************************/
static gboolean test_cb(const gchar *opt, const gchar *arg, gpointer data,
			GError **err)
{
	gboolean success = TRUE;

	NCL_CMD_PRINT("Callback invoked from g_option_context_parse()\n");
	NCL_CMD_PRINT("opt = '%s'\n", opt);
	NCL_CMD_PRINT("arg = '%s'\n", arg);
	NCL_CMD_PRINT("data = 0x%08X\n", GPOINTER_TO_UINT(data));
	NCL_CMD_PRINT("err = 0x%08X\n", GPOINTER_TO_UINT(err));
	if (err && *err) {
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

static inline gpointer ncl_g_callback(GCallback gc)
{
	union {
		gpointer gp;
		GCallback gc;
	} p;
	p.gc = gc;
	return p.gp;
}

#define NCL_G_CALLBACK(cb) ncl_g_callback(G_CALLBACK(cb))

/* Parameters Command line test */
static NCLError ncl_cmd_test_parameters(int argc, char *argv[])
{
	static int		intTmp;
	static char		*stringTmp;
	static double		doubleTmp;
	static long long	int64Tmp;
	NCLError		err		= NCLERR_NOERROR;
	GOptionEntry options[] = {
		{ "int"	, 'i', 0, G_OPTION_ARG_INT		, &intTmp
				  , "Integer parameter", "9999" },

		{ "string", 's', 0, G_OPTION_ARG_STRING	, &stringTmp
				, "String parameter", "STRING" },

		{ "double", 'd', 0, G_OPTION_ARG_DOUBLE	, &doubleTmp
				, "Double parameter", "9.99" },

		{ "int64"	, 'l', 0, G_OPTION_ARG_INT64 , &int64Tmp
				, "Integer64 parameter", "9999" },
		{ "cb"	, 'c', 0, G_OPTION_ARG_CALLBACK, NCL_G_CALLBACK(test_cb)
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
/*****************************************************************************
 * test parameter type : END
 ****************************************************************************/


/*****************************************************************************
 *
 ****************************************************************************/
/* Exiting from command line interpretor */
static NCLError ncl_cmd_exit(int argc, char *argv[])
{
	NCLError	err		= NCLERR_NOERROR;
	NCLContext	*nclCtxP	= NULL;

	(void) argc; /* remove warning */
	(void) argv; /* remove warning */
	nclCtxP = ncl_get_ctx();

	/* Release NEARDAL object */
	neardal_destroy();

	/* Quit Main Loop */
	if (nclCtxP)
		g_main_loop_quit(nclCtxP->main_loop);
	else
		err = NCLERR_GLOBAL_ERROR;

	return err;
}
/* END : Interpretor commands */


/*****************************************************************************
 *
 ****************************************************************************/
/* Array of command line functions interpretor (alphabetical order) */
static NCLCmdInterpretor itFunc[] = {
	{ "q",
	ncl_cmd_exit,
	"Exit from command line interpretor" },

	{ "quit",
	ncl_cmd_exit,
	"Exit from command line interpretor" },

	{ "exit",
	ncl_cmd_exit,
	"Exit from command line interpretor" },

	{ "get_adapters",
	ncl_cmd_get_adapters,
	"Get adapters list"},

	{ "get_adapter_properties",
	ncl_cmd_get_adapter_properties,
	"Get adapter properties (1st parameter is adapter name)"},

	{ "get_devices",
	ncl_cmd_get_devices,
	"Get devices list (1st parameter is adapter name)"},

	{ "get_device_properties",
	ncl_cmd_get_device_properties,
	"Get device properties (1st parameter is device name)"},

	{ "get_records",
	ncl_cmd_get_records,
	"Get records list (1st parameter is tag name)"},

	{ "get_record_properties",
	ncl_cmd_get_record_properties,
	"Read a specific record. (1st parameter is record name)"},

	{ "get_tags",
	ncl_cmd_get_tags,
	"Get tags list (1st parameter is adapter name)"},

	{ "get_tag_properties",
	ncl_cmd_get_tag_properties,
	"Get tag properties (1st parameter is tag name)"},

	{ LISTCMD_NAME,
	ncl_cmd_list,
	"List all available commands. 'cmd' --help -h /? for a specific help" },

	{ "push",
	ncl_cmd_push,
	"Creates and push a NDEF record to a NFC device"},

	{ "registerHandover",
	ncl_cmd_register_handover_agent,
	"register a handler for handover connection"},

	{ "registerNDEFtype",
	ncl_cmd_register_NDEF_agent,
	"register a handler for a specific NDEF tag type"},

	{ "set_adp_property",
	ncl_cmd_set_adapter_property,
	"Request Neard to set a proprety on defined adapter"},

	{ "start_poll",
	ncl_cmd_start_poll,
	"Request Neard to start polling (1st parameter is adapter name)"},

	{ "stop_poll",
	ncl_cmd_stop_poll,
	"Request Neard to stop polling (1st parameter is adapter name)"},

	{ "test_parameters",
	ncl_cmd_test_parameters,
	"Simple test to parse input parameters"},

	{ "unregisterHandover",
	ncl_cmd_unregister_handover_agent,
	"unregister a handler for handover connection"},

	{ "unregisterNDEFtype",
	ncl_cmd_unregister_NDEF_agent,
	"unregister a handler for a specific NDEF tag type"},

	{ "write",
	ncl_cmd_write,
	"Creates and write a NDEF record to a NFC tag"}
};
#define NB_CL_FUNC		(sizeof(itFunc) / sizeof(NCLCmdInterpretor))


/*****************************************************************************
 * Display Interpretor command list
 ****************************************************************************/
NCLError ncl_cmd_list(int argc, char *argv[])
{
	int index;
	int  nbCmd = ncl_cmd_get_nbCmd();

	(void) argc; /* remove warning */
	(void) argv; /* remove warning */

	NCL_CMD_PRINT("\nCommand line list\n");
	for (index = 0; index < nbCmd; index++) {
		NCL_CMD_PRINT("%s :\n\t%s\n\n", itFunc[index].cmdName,
			      itFunc[index].helpStr);
	}

	return 0;
}

/*****************************************************************************
 *
 ****************************************************************************/
NCLCmdInterpretor *ncl_cmd_get_list(int *nbCmd)
{
	if (nbCmd != NULL)
		*nbCmd = NB_CL_FUNC;
	return itFunc;
}


/*****************************************************************************
 *
 ****************************************************************************/
int ncl_cmd_get_nbCmd(void)
{
	return NB_CL_FUNC;
}


/*****************************************************************************
 *
 ****************************************************************************/
NCLCmdContext *ncl_cmd_get_ctx(void)
{
	return &sNclCmdCtx;
}

/*****************************************************************************
 *
 ****************************************************************************/
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


/*****************************************************************************
 *
 ****************************************************************************/
void ncl_cmd_finalize(void)
{

	if (sNclCmdCtx.clBuf != NULL)
		g_string_free(sNclCmdCtx.clBuf, TRUE);

	/* Release NFC object */
	neardal_destroy();
}
