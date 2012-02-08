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
#include <string.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <glib-object.h>
#include "dbus/dbus.h"

#include "neard-record-proxy.h"

#include "neardal.h"
#include "neardal_prv.h"
#include <glib-2.0/glib/gerror.h>
#include <glib-2.0/glib/glist.h>

/******************************************************************************
 * neardal_rcd_prv_read_std_properties: Get Neard Record Properties for non
 * smartPoster tag
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_std_properties(GHashTable *rcdHash,
						      RcdProp *rcd)
{
	errorCode_t	errCode;
	char		*tmp	= NULL;

	errCode = neardal_tools_prv_hashtable_get(rcdHash, "Type",
					      G_TYPE_STRING, &tmp);
	if (errCode == NEARDAL_SUCCESS)
		rcd->type = g_strdup(tmp);

	if (!strcmp(rcd->type, "Text")) {
		errCode = neardal_tools_prv_hashtable_get(rcdHash,
						      "Representation",
						      G_TYPE_STRING, &tmp);
		if (errCode == NEARDAL_SUCCESS)
			rcd->representation = g_strdup(tmp);
	}

	if (!strcmp(rcd->type, "URI")) {
		errCode = neardal_tools_prv_hashtable_get(rcdHash, "URI",
						      G_TYPE_STRING, &tmp);
		if (errCode == NEARDAL_SUCCESS)
			rcd->uri = g_strdup(tmp);
	}

	if (!strcmp(rcd->type, "MIME Type (RFC 2046)")) {
		errCode = neardal_tools_prv_hashtable_get(rcdHash, "MIME",
						      G_TYPE_STRING, &tmp);
		if (errCode == NEARDAL_SUCCESS)
			rcd->mime = g_strdup(tmp);
	}
	return errCode;
}

/******************************************************************************
 * neardal_rcd_prv_read_sp_properties: Get Neard Record Properties
 * for smartPoster tag
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_sp_properties(GHashTable *rcdHash,
						       RcdProp *rcd)
{
	/* TODO */
	(void) rcdHash; /* remove warning */
	(void) rcd; /* remove warning */

	return NEARDAL_ERROR_GENERAL_ERROR;
}

/******************************************************************************
 * neardal_rcd_prv_read_properties: Get Neard Record Properties
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_properties(RcdProp *rcd)
{
	errorCode_t	errCode		= NEARDAL_SUCCESS;
	GHashTable	*rcdHash	= NULL;
	GError		*gerror		= NULL;
	char		*tmp		= NULL;
	gboolean	dbusCall;


	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);
	g_assert(rcd->dbusProxy != NULL);

	dbusCall = org_neard_Record_get_properties(rcd->dbusProxy, &rcdHash,
						   &gerror);
	if (dbusCall != TRUE || gerror != NULL) {
		errCode = NEARDAL_ERROR_DBUS;
		NEARDAL_TRACE_ERR(
			"Unable to read record's properties (%d:%s)\n",
				   gerror->code, gerror->message);
		g_error_free(gerror);
		return errCode;
	}

	errCode = neardal_tools_prv_hashtable_get(rcdHash, "Encoding",
					      G_TYPE_STRING, &tmp);
	if (errCode == NEARDAL_SUCCESS)
		rcd->encoding = g_strdup(tmp);
	errCode = neardal_tools_prv_hashtable_get(rcdHash, "HandOver",
					      G_TYPE_BOOLEAN,
					      &rcd->handOver);
	errCode = neardal_tools_prv_hashtable_get(rcdHash, "Language",
					      G_TYPE_STRING, &tmp);
	if (errCode == NEARDAL_SUCCESS)
		rcd->language = g_strdup(tmp);
	errCode = neardal_tools_prv_hashtable_get(rcdHash, "SmartPoster",
					      G_TYPE_BOOLEAN,
					      &rcd->smartPoster);
	if (rcd->smartPoster == FALSE)
		errCode = neardal_rcd_prv_read_std_properties(rcdHash,
								  rcd);
	else
		errCode = neardal_rcd_prv_read_sp_properties(rcdHash,
								       rcd);

	g_hash_table_destroy(rcdHash);

	return errCode;
}

/******************************************************************************
 * neardal_rcd_init: Get Neard Manager Properties = NFC Records list.
 * Create a DBus proxy for the first one NFC record if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_init(neardal_t neardalObj,
					    RcdProp *rcd)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);

	if (rcd->dbusProxy != NULL)
		g_object_unref(rcd->dbusProxy);
	rcd->dbusProxy = NULL;

	errCode = neardal_tools_prv_create_proxy(neardalObj->conn,
						  &rcd->dbusProxy,
						  rcd->name,
						  NEARD_RECORDS_IF_NAME);
	if (errCode == NEARDAL_SUCCESS)
		errCode = neardal_rcd_prv_read_properties(rcd);

	return errCode;
}

/******************************************************************************
 * neardal_rcd_release: unref DBus proxy, disconnect Neard Record signals
 *****************************************************************************/
static void neardal_rcd_prv_release(RcdProp *rcd)
{
	NEARDAL_TRACEIN();
	if (rcd->dbusProxy != NULL)
		g_object_unref(rcd->dbusProxy);
	rcd->dbusProxy = NULL;
	g_free(rcd->name);
	g_free(rcd->language);
	g_free(rcd->encoding);
	g_free(rcd->mime);
	g_free(rcd->representation);
	g_free(rcd->type);
	g_free(rcd->uri);
	g_free(rcd);
}
/******************************************************************************
 * neardal_rcd_prv_read_std_properties: Insert key/value in a GHashTable for
 * non smartPoster tag
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_format_std_properties(GHashTable **hash,
							  RcdProp *rcd)
{
	if (rcd->action != NULL)
		neardal_tools_add_dict_entry(*hash, "Action", rcd->action);

	if (rcd->encoding != NULL)
		neardal_tools_add_dict_entry(*hash, "Encoding", rcd->encoding);

	if (rcd->language != NULL)
		neardal_tools_add_dict_entry(*hash, "Language", rcd->language);

	if (rcd->mime != NULL)
		neardal_tools_add_dict_entry(*hash, "MIME", rcd->mime);

	if (rcd->representation != NULL)
		neardal_tools_add_dict_entry(*hash, "Representation",
					      rcd->representation);

	if (rcd->type != NULL)
		neardal_tools_add_dict_entry(*hash, "Type", rcd->type);

	if (rcd->uri != NULL)
		neardal_tools_add_dict_entry(*hash, "URI", rcd->uri);
	
	return NEARDAL_SUCCESS;
}

/******************************************************************************
 * neardal_rcd_prv_read_sp_properties: Insert key/value in a GHashTable for
 * smartPoster tag
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_format_sp_properties(GHashTable **hash,
							 RcdProp *rcd)
{
	/* TODO */
	(void) hash; /* remove warning */
	(void) rcd; /* remove warning */

	return NEARDAL_ERROR_GENERAL_ERROR;
}


/******************************************************************************
 * neardal_rcd_prv_format: Insert key/value in a GHashTable
 *****************************************************************************/
errorCode_t neardal_rcd_prv_format(GHashTable **hash, RcdProp *rcd)
{
	errorCode_t	errCode		= NEARDAL_SUCCESS;


	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);
	g_assert(rcd->dbusProxy != NULL);

	if (rcd->smartPoster == FALSE)
		errCode = neardal_rcd_prv_format_std_properties(hash, rcd);
	else
		errCode = neardal_rcd_prv_format_sp_properties(hash, rcd);

	return errCode;
}


/******************************************************************************
 * neardal_get_records: get an array of target records
 *****************************************************************************/
errorCode_t neardal_get_records(neardal_t neardalObj, char *tgtName,
				 char ***array, int *len)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_RECORD;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	int		rcdNb		= 0;
	int		ct		= 0;	/* counter */
	char		**rcds		= NULL;
	RcdProp		*rcd		= NULL;


	if (neardalObj == NULL || tgtName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	errCode = neardal_mgr_prv_get_adapter(neardalObj, tgtName,
						   &adpProp);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	errCode = neardal_adp_prv_get_target(adpProp, tgtName, &tgtProp);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	errCode		= NEARDAL_ERROR_NO_RECORD;
	rcdNb = g_list_length(tgtProp->rcdList);
	if (rcdNb <= 0)
		goto exit;

	errCode = NEARDAL_ERROR_NO_MEMORY;
	rcds = g_try_malloc0((rcdNb + 1) * sizeof(char *));
	if (rcds == NULL)
		goto exit;

	while (ct < rcdNb) {
		rcd = g_list_nth_data(tgtProp->rcdList, ct);
		if (rcd != NULL)
			rcds[ct++] = g_strdup(rcd->name);
	}
	errCode = NEARDAL_SUCCESS;

exit:
	if (len != NULL)
		*len = rcdNb;
	*array	= rcds;

	return errCode;
}

/******************************************************************************
 * neardal_rcd_add: add new NFC record, initialize DBus Proxy connection,
 * register record signal
 *****************************************************************************/
errorCode_t neardal_rcd_add(neardal_t neardalObj, char *rcdName)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_MEMORY;
	AdpProp		*adpProp	= NULL;
	RcdProp		*rcd	= NULL;
	TgtProp		*tgtProp	= NULL;

	g_assert(neardalObj != NULL);
	g_assert(rcdName != NULL);

	NEARDAL_TRACEF("Adding record:%s\n", rcdName);
	rcd = g_try_malloc0(sizeof(RcdProp));
	if (rcd == NULL)
		goto exit;

	rcd->name	= g_strdup(rcdName);
	if (rcd->name == NULL)
		goto exit;

	errCode = neardal_rcd_prv_init(neardalObj, rcd);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	errCode = neardal_mgr_prv_get_adapter(neardalObj, rcdName,
						   &adpProp);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	errCode = neardal_adp_prv_get_target(adpProp, rcdName, &tgtProp);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	tgtProp->rcdList = g_list_prepend(tgtProp->rcdList, (gpointer) rcd);
	NEARDAL_TRACEF("NEARDAL LIB recordList contains %d elements\n",
		      g_list_length(tgtProp->rcdList));
	errCode = NEARDAL_SUCCESS;

exit:
	if (errCode != NEARDAL_SUCCESS) {
		if (rcd->name != NULL) {
			g_free(rcd->name);
			rcd->name = NULL;
		}
		if (rcd != NULL)
			g_free(rcd);
	}

	return errCode;
}

/******************************************************************************
 * neardal_rcd_remove: remove NFC record, unref DBus Proxy connection,
 * unregister record signal
 *****************************************************************************/
void neardal_rcd_remove(RcdProp *rcd)
{
	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);

	NEARDAL_TRACEF("Removing record:%s\n", rcd->name);
	/* Remove all records */
	neardal_rcd_prv_release(rcd);
}
