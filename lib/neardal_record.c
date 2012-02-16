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
#include <glib-2.0/glib/gerror.h>
#include <glib-2.0/glib/glist.h>

/******************************************************************************
 * neardal_rcd_prv_read_std_properties: Get Neard Record Properties for non
 * smartPoster tag
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_std_properties(GVariant *value,
						      RcdProp *rcd)
{
	errorCode_t	errCode	= NEARDAL_SUCCESS;
	GVariant	*tmpOut	= NULL;

	tmpOut = g_variant_lookup_value(value, "Type", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->type = g_variant_dup_string(tmpOut, NULL);
	
	if (!strcmp(rcd->type, "Text")) {
		tmpOut = g_variant_lookup_value(value, "Representation",
						G_VARIANT_TYPE_STRING);
		if (tmpOut != NULL)
			rcd->representation = g_variant_dup_string(tmpOut,
								   NULL);
	}

	if (!strcmp(rcd->type, "URI")) {
		tmpOut = g_variant_lookup_value(value, "URI",
						G_VARIANT_TYPE_STRING);
		if (tmpOut != NULL)
			rcd->uri = g_variant_dup_string(tmpOut, NULL);
	}

	if (!strcmp(rcd->type, "MIME Type (RFC 2046)")) {
		tmpOut = g_variant_lookup_value(value, "MIME",
						G_VARIANT_TYPE_STRING);
		if (tmpOut != NULL)
			rcd->mime = g_variant_dup_string(tmpOut, NULL);
	}
	return errCode;
}

/******************************************************************************
 * neardal_rcd_prv_read_sp_properties: Get Neard Record Properties
 * for smartPoster tag
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_sp_properties(GVariant *value,
						       RcdProp *rcd)
{
	/* TODO */
	(void) value; /* remove warning */
	(void) rcd; /* remove warning */

	return NEARDAL_ERROR_GENERAL_ERROR;
}

/******************************************************************************
 * neardal_rcd_prv_read_properties: Get Neard Record Properties
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_read_properties(RcdProp *rcd)
{
	errorCode_t	errCode		= NEARDAL_SUCCESS;
	GError		*gerror		= NULL;
	GVariant	*tmp		= NULL;
	GVariant	*tmpOut		= NULL;

	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);
	g_assert(rcd->proxy != NULL);

	org_neard_rcd__call_get_properties_sync(rcd->proxy, &tmp, NULL,
						&gerror);
	if (gerror != NULL) {
		errCode = NEARDAL_ERROR_DBUS;
		NEARDAL_TRACE_ERR(
			"Unable to read record's properties (%d:%s)\n",
				   gerror->code, gerror->message);
		g_error_free(gerror);
		return errCode;
	}
	NEARDAL_TRACEF("GVariant=%s\n", g_variant_print (tmp, TRUE));

	tmpOut = g_variant_lookup_value(tmp, "Encoding", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->encoding = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "HandOver", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		rcd->handOver = g_variant_get_boolean  (tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Language", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->language = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "SmartPoster",
					G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		rcd->smartPoster = g_variant_get_boolean  (tmpOut);

	if (rcd->smartPoster == FALSE)
		errCode = neardal_rcd_prv_read_std_properties(tmp, rcd);
	else
		errCode = neardal_rcd_prv_read_sp_properties(tmp, rcd);

	return errCode;
}

/******************************************************************************
 * neardal_rcd_init: Get Neard Manager Properties = NFC Records list.
 * Create a DBus proxy for the first one NFC record if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
static errorCode_t neardal_rcd_prv_init(neardal_t neardalMgr,
					    RcdProp *rcd)
{
	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);

	if (rcd->proxy != NULL)
		g_object_unref(rcd->proxy);
	rcd->proxy = NULL;

	rcd->proxy = org_neard_rcd__proxy_new_sync(neardalMgr->conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							rcd->name,
							NULL, /* GCancellable */
							&neardalMgr->gerror);
	if (neardalMgr->gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Record Proxy (%d:%s)\n",
				 neardalMgr->gerror->code,
				neardalMgr->gerror->message);
		neardal_tools_prv_free_gerror(neardalMgr);
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
	g_assert(rcd->proxy != NULL);

	if (rcd->smartPoster == FALSE)
		errCode = neardal_rcd_prv_format_std_properties(hash, rcd);
	else
		errCode = neardal_rcd_prv_format_sp_properties(hash, rcd);

	return errCode;
}


/******************************************************************************
 * neardal_get_records: get an array of target records
 *****************************************************************************/
errorCode_t neardal_get_records(neardal_t neardalMgr, char *tgtName,
				 char ***array, int *len)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_RECORD;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	int		rcdLen		= 0;
	int		ct		= 0;	/* counter */
	char		**rcds		= NULL;
	RcdProp		*rcd		= NULL;


	if (neardalMgr == NULL || tgtName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	errCode = neardal_mgr_prv_get_adapter(neardalMgr, tgtName,
						   &adpProp);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	errCode = neardal_adp_prv_get_target(adpProp, tgtName, &tgtProp);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	errCode		= NEARDAL_ERROR_NO_RECORD;
	rcdLen = g_list_length(tgtProp->rcdList);
	if (rcdLen <= 0)
		goto exit;

	errCode = NEARDAL_ERROR_NO_MEMORY;
	rcds = g_try_malloc0((rcdLen + 1) * sizeof(char *));
	if (rcds == NULL)
		goto exit;

	while (ct < rcdLen) {
		rcd = g_list_nth_data(tgtProp->rcdList, ct);
		if (rcd != NULL)
			rcds[ct++] = g_strdup(rcd->name);
	}
	errCode = NEARDAL_SUCCESS;

exit:
	if (len != NULL)
		*len = rcdLen;
	*array	= rcds;

	return errCode;
}

/******************************************************************************
 * neardal_rcd_add: add new NFC record, initialize DBus Proxy connection,
 * register record signal
 *****************************************************************************/
errorCode_t neardal_rcd_add(neardal_t neardalMgr, void *parent,
			    char *rcdName)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_MEMORY;
	RcdProp		*rcd	= NULL;
	TgtProp		*tgtProp = parent;
	
	g_assert(neardalMgr != NULL);
	g_assert(rcdName != NULL);
	g_assert(parent != NULL);

	NEARDAL_TRACEF("Adding record:%s\n", rcdName);
	rcd = g_try_malloc0(sizeof(RcdProp));
	if (rcd == NULL)
		goto exit;

	rcd->name = g_strdup(rcdName);
	if (rcd->name == NULL)
		goto exit;

	rcd->parent = tgtProp;
	
	tgtProp->rcdList = g_list_prepend(tgtProp->rcdList, (gpointer) rcd);
	errCode = neardal_rcd_prv_init(neardalMgr, rcd);
	if (errCode != NEARDAL_SUCCESS)
		goto exit;

	NEARDAL_TRACEF("NEARDAL LIB recordList contains %d elements\n",
		      g_list_length(tgtProp->rcdList));
	
	if (neardalMgr->cb_rcd_found != NULL)
		(neardalMgr->cb_rcd_found)(rcdName,
					   neardalMgr->cb_rcd_found_ud);
	errCode = NEARDAL_SUCCESS;

exit:
	if (errCode != NEARDAL_SUCCESS) {
		tgtProp->rcdList = g_list_remove(tgtProp->rcdList,
						 (gpointer) rcd);
		neardal_rcd_prv_free(&rcd);
	}

	return errCode;
}

/******************************************************************************
 * neardal_rcd_remove: remove NFC record, unref DBus Proxy connection,
 * unregister record signal
 *****************************************************************************/
void neardal_rcd_remove(RcdProp *rcd)
{
	TgtProp		*tgtProp;
	
	NEARDAL_TRACEIN();
	g_assert(rcd != NULL);

	tgtProp = rcd->parent;
	NEARDAL_TRACEF("Removing record:%s\n", rcd->name);
	tgtProp->rcdList = g_list_remove(tgtProp->rcdList,
					 (gconstpointer) rcd);
	/* Remove all records */
	neardal_rcd_prv_free(&rcd);
}
