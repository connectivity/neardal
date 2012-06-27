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
#include <glib-object.h>

#include "neard_record_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"


/******************************************************************************
 * neardal_rcd_prv_read_properties: Get Neard Record Properties
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_properties(RcdProp *rcd)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	GHashTable	*rcdHash	= NULL;
	GError		*gerror		= NULL;
	char		*tmp		= NULL;

	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);
	g_assert(rcd->proxy != NULL);

	org_neard_Record_get_properties(rcd->proxy, &rcdHash, &gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS;
		NEARDAL_TRACE_ERR(
			"Unable to read record's properties (%d:%s)\n",
				   gerror->code, gerror->message);
		g_error_free(gerror);
		return err;
	}

	err = neardal_tools_prv_hashtable_get(rcdHash, "Type",
					      G_TYPE_STRING, &tmp);
	if (err == NEARDAL_SUCCESS)
		rcd->type = g_strdup(tmp);

	if (!strcmp(rcd->type, "Text")) {
		err = neardal_tools_prv_hashtable_get(rcdHash,
						      "Representation",
						      G_TYPE_STRING, &tmp);
		if (err == NEARDAL_SUCCESS)
			rcd->representation = g_strdup(tmp);
	}

	err = neardal_tools_prv_hashtable_get(rcdHash, "Encoding",
					      G_TYPE_STRING, &tmp);
	if (err == NEARDAL_SUCCESS)
		rcd->encoding = g_strdup(tmp);

	err = neardal_tools_prv_hashtable_get(rcdHash, "Language",
					      G_TYPE_STRING, &tmp);
	if (err == NEARDAL_SUCCESS)
		rcd->language = g_strdup(tmp);

	if (!strcmp(rcd->type, "MIME Type (RFC 2046)")) {
		err = neardal_tools_prv_hashtable_get(rcdHash, "MIME",
						      G_TYPE_STRING, &tmp);
		if (err == NEARDAL_SUCCESS)
			rcd->mime = g_strdup(tmp);
	}

	if (!strcmp(rcd->type, "URI")) {
		err = neardal_tools_prv_hashtable_get(rcdHash, "URI",
						      G_TYPE_STRING, &tmp);
		if (err == NEARDAL_SUCCESS)
			rcd->uri = g_strdup(tmp);
	}

	err = neardal_tools_prv_hashtable_get(rcdHash, "HandOver",
					      G_TYPE_BOOLEAN,
					      &rcd->handOver);
	err = neardal_tools_prv_hashtable_get(rcdHash, "SmartPoster",
					      G_TYPE_BOOLEAN,
					      &rcd->smartPoster);
	g_hash_table_destroy(rcdHash);

	return NEARDAL_SUCCESS;
}

/******************************************************************************
 * neardal_rcd_init: Get Neard Manager Properties = NFC Records list.
 * Create a DBus proxy for the first one NFC record if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_init(RcdProp *rcd)
{
	errorCode_t err;
	
	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);

	if (rcd->proxy != NULL)
		g_object_unref(rcd->proxy);
	rcd->proxy = NULL;

	err = neardal_tools_prv_create_proxy(neardalMgr.conn,
						  &rcd->proxy,
							rcd->name,
						  NEARD_RECORDS_IF_NAME);
	if (err != NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Record Proxy (%d:%s)\n",
				 neardalMgr.gerror->code,
				neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	return neardal_rcd_prv_read_properties(rcd);
}

/******************************************************************************
 * neardal_rcd_prv_free: unref DBus proxy, disconnect Neard Record signals
 *****************************************************************************/
static void neardal_rcd_prv_free(RcdProp **rcd)
{
	NEARDAL_TRACEIN();
	if ((*rcd)->proxy != NULL)
		g_object_unref((*rcd)->proxy);
	(*rcd)->proxy = NULL;
	g_free((*rcd)->name);
	g_free((*rcd)->language);
	g_free((*rcd)->encoding);
	g_free((*rcd)->mime);
	g_free((*rcd)->representation);
	g_free((*rcd)->type);
	g_free((*rcd)->uri);
	g_free((*rcd));
	(*rcd) = NULL;
}

/******************************************************************************
 * neardal_rcd_prv_format: Insert key/value in a GHashTable
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_format_std_properties(GHashTable **hash,
							  RcdProp *rcd)
{
	errorCode_t	err		= NEARDAL_SUCCESS;


	// Type

	if (rcd->type != NULL)
		neardal_tools_add_dict_entry(*hash, "Type", rcd->type);

	// Encoding
	if (rcd->encoding != NULL)
		neardal_tools_add_dict_entry(*hash, "Encoding", rcd->encoding);

	if (rcd->language != NULL)
		neardal_tools_add_dict_entry(*hash, "Language", rcd->language);

	if (rcd->mime != NULL)
		neardal_tools_add_dict_entry(*hash, "MIME", rcd->mime);

	if (rcd->representation != NULL)
		neardal_tools_add_dict_entry(*hash, "Representation",
					      rcd->representation);

	if (rcd->uri != NULL) {
		neardal_tools_add_dict_entry(*hash, "URI", rcd->uri);
//		neardal_tools_add_dict_entry(*hash, "Size", rcd->uriObjSize);

	}
	if (rcd->mime != NULL)
		neardal_tools_add_dict_entry(*hash, "MIME", rcd->mime);

	if (rcd->action != NULL)
		neardal_tools_add_dict_entry(*hash, "Action", rcd->action);
	return err;
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
errorCode_t neardal_rcd_prv_format(GHashTable ** hash, RcdProp *rcd)
{
	errorCode_t	err		= NEARDAL_SUCCESS;


	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);
	g_assert(rcd->proxy != NULL);

	if (rcd->smartPoster == FALSE)
		err = neardal_rcd_prv_format_std_properties(hash, rcd);
	else
		err = neardal_rcd_prv_format_sp_properties(hash, rcd);

	return err;
}


/******************************************************************************
 * neardal_get_records: get an array of tag records
 *****************************************************************************/
errorCode_t neardal_get_records(char *tagName, char ***array, int *len)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;
	TagProp		*tagProp	= NULL;
	int		rcdLen		= 0;
	int		ct		= 0;	/* counter */
	char		**rcds		= NULL;
	RcdProp		*rcd		= NULL;


	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || tagName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	err = neardal_mgr_prv_get_adapter(tagName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_adp_prv_get_tag(adpProp, tagName, &tagProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err		= NEARDAL_ERROR_NO_RECORD;
	rcdLen = g_list_length(tagProp->rcdList);
	if (rcdLen <= 0)
		goto exit;

	err = NEARDAL_ERROR_NO_MEMORY;
	rcds = g_try_malloc0((rcdLen + 1) * sizeof(char *));
	if (rcds == NULL)
		goto exit;

	while (ct < rcdLen) {
		rcd = g_list_nth_data(tagProp->rcdList, ct);
		if (rcd != NULL)
			rcds[ct++] = g_strdup(rcd->name);
	}
	err = NEARDAL_SUCCESS;

exit:
	if (len != NULL)
		*len = rcdLen;
	*array	= rcds;

	return err;
}

/******************************************************************************
 * neardal_rcd_add: add new NFC record, initialize DBus Proxy connection,
 * register record signal
 *****************************************************************************/
errorCode_t neardal_rcd_add(char *rcdName, void *parent)
{
	errorCode_t	err		= NEARDAL_ERROR_NO_MEMORY;
	RcdProp		*rcd	= NULL;
	TagProp		*tagProp = parent;

	g_assert(rcdName != NULL);
	g_assert(parent != NULL);

	NEARDAL_TRACEF("Adding record:%s\n", rcdName);
	rcd = g_try_malloc0(sizeof(RcdProp));
	if (rcd == NULL)
		goto exit;

	rcd->name = g_strdup(rcdName);
	if (rcd->name == NULL)
		goto exit;

	rcd->parent = tagProp;

	tagProp->rcdList = g_list_prepend(tagProp->rcdList, (gpointer) rcd);
	err = neardal_rcd_prv_init(rcd);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	NEARDAL_TRACEF("NEARDAL LIB recordList contains %d elements\n",
		      g_list_length(tagProp->rcdList));

	err = NEARDAL_SUCCESS;

exit:
	if (err != NEARDAL_SUCCESS) {
		tagProp->rcdList = g_list_remove(tagProp->rcdList,
						 (gpointer) rcd);
		neardal_rcd_prv_free(&rcd);
	}

	return err;
}

/******************************************************************************
 * neardal_rcd_remove: remove NFC record, unref DBus Proxy connection,
 * unregister record signal
 *****************************************************************************/
void neardal_rcd_remove(RcdProp *rcd)
{
	TagProp		*tagProp;

	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);

	tagProp = rcd->parent;
	NEARDAL_TRACEF("Removing record:%s\n", rcd->name);
	tagProp->rcdList = g_list_remove(tagProp->rcdList,
					 (gconstpointer) rcd);
	/* Remove all records */
	neardal_rcd_prv_free(&rcd);
}
