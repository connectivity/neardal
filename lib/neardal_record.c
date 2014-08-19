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

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include "neardal.h"
#include "neardal_prv.h"

GVariant *neardal_record_to_g_variant(neardal_record *in)
{
	GVariantBuilder b;
	GVariant *out;

	g_variant_builder_init(&b, G_VARIANT_TYPE_ARRAY);

	NEARDAL_G_VARIANT_IN(&b, "{'Action', <%s>}", in->action);
	NEARDAL_G_VARIANT_IN(&b, "{'Carrier', <%s>}", in->carrier);
	NEARDAL_G_VARIANT_IN(&b, "{'Encoding', <%s>}", in->encoding);
	NEARDAL_G_VARIANT_IN(&b, "{'Language', <%s>}", in->language);
	NEARDAL_G_VARIANT_IN(&b, "{'MIME', <%s>}", in->mime);
	NEARDAL_G_VARIANT_IN(&b, "{'Name', <%s>}", in->name);
	NEARDAL_G_VARIANT_IN(&b, "{'Representation', <%s>}",
				in->representation);
	NEARDAL_G_VARIANT_IN(&b, "{'Size', <%u>}", in->uriObjSize);
	NEARDAL_G_VARIANT_IN(&b, "{'Type', <%s>}", in->type);
	NEARDAL_G_VARIANT_IN(&b, "{'URI', <%s>}", in->uri);

	out = g_variant_builder_end(&b);

	return out;
}

neardal_record *neardal_g_variant_to_record(GVariant *in)
{
	neardal_record *out = g_new0(neardal_record, 1);

	NEARDAL_G_VARIANT_OUT(in, "Action", "s", &out->action);
	NEARDAL_G_VARIANT_OUT(in, "Carrier", "s", &out->carrier);
	NEARDAL_G_VARIANT_OUT(in, "Encoding", "s", &out->encoding);
	NEARDAL_G_VARIANT_OUT(in, "Language", "s", &out->language);
	NEARDAL_G_VARIANT_OUT(in, "MIME", "s", &out->mime);
	NEARDAL_G_VARIANT_OUT(in, "Name", "s", &out->name);
	NEARDAL_G_VARIANT_OUT(in, "Representation", "s", &out->representation);
	NEARDAL_G_VARIANT_OUT(in, "Size", "u", &out->uriObjSize);
	NEARDAL_G_VARIANT_OUT(in, "Type", "s", &out->type);
	NEARDAL_G_VARIANT_OUT(in, "URI", "s", &out->uri);

	return out;
}

static void neardal_rcd_notify(RcdProp *rcd)
{
	if (!rcd->notified) {
		neardalMgr.cb.rcd_found(rcd->name, neardalMgr.cb.rcd_found_ud);
		rcd->notified = TRUE;
	}
}

/*****************************************************************************
 * neardal_rcd_prv_read_properties: Get Neard Record Properties
 ****************************************************************************/
static errorCode_t neardal_rcd_prv_read_properties(RcdProp *rcd)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	GVariant	*tmp		= NULL;
	GVariant	*tmpOut		= NULL;

	NEARDAL_TRACEIN();
	NEARDAL_ASSERT_RET(rcd != NULL, NEARDAL_ERROR_INVALID_PARAMETER);
	NEARDAL_ASSERT_RET(rcd->proxy != NULL,
				NEARDAL_ERROR_INVALID_PARAMETER);

	if ((tmp = g_datalist_get_data(&(neardalMgr.dbus_data), rcd->name)))
		goto parse_properties;

	if (properties_call_get_all_sync(rcd->proxy,
				"org.freedesktop.DBus.Properties", &tmp, NULL,
					&neardalMgr.gerror) == FALSE) {
		NEARDAL_TRACE_ERR("Can't read properties on path=%s: %s\n",
					rcd->name, neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
	}
parse_properties:
	NEARDAL_TRACEF("Reading:\n%s\n", g_variant_print(tmp, TRUE));

	tmpOut = g_variant_lookup_value(tmp, "Type", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->type = g_variant_dup_string(tmpOut, NULL);
	else
		goto error;

	tmpOut = g_variant_lookup_value(tmp, "Carrier", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->type = g_variant_dup_string(tmpOut, NULL);
	
	tmpOut = g_variant_lookup_value(tmp, "Representation",
					G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->representation = g_variant_dup_string(tmpOut,
								NULL);

	tmpOut = g_variant_lookup_value(tmp, "Encoding", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->encoding = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "Language", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->language = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "MIME",
					G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->mime = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "URI",
					G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		rcd->uri = g_variant_dup_string(tmpOut, NULL);

	neardal_rcd_notify(rcd);

	return err;
error:
	/* due to error, record content will be destroyed later */
	return NEARDAL_ERROR_INVALID_RECORD;
}

/*****************************************************************************
 * neardal_rcd_init: Get Neard Manager Properties = NFC Records list.
 * Create a DBus proxy for the first one NFC record if present
 * Register Neard Manager signals ('PropertyChanged')
 ****************************************************************************/
static errorCode_t neardal_rcd_prv_init(RcdProp *rcd)
{
	NEARDAL_TRACEIN();
	NEARDAL_ASSERT_RET(rcd != NULL, NEARDAL_ERROR_INVALID_PARAMETER);
	NEARDAL_ASSERT_RET(rcd->proxy == NULL,
				NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY);

	rcd->proxy = properties_proxy_new_sync(neardalMgr.conn, 0,
				NEARD_DBUS_SERVICE, rcd->name, NULL,
				&neardalMgr.gerror);

	if (neardalMgr.gerror != NULL) {
		NEARDAL_TRACE_ERR("Error creating proxy for path=%s: %s\n",
					rcd->name, neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	return neardal_rcd_prv_read_properties(rcd);
}

/*****************************************************************************
 * neardal_rcd_prv_free: unref DBus proxy, disconnect Neard Record signals
 ****************************************************************************/
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
	g_free((*rcd)->carrier);
	g_free((*rcd)->uri);
	g_free((*rcd));
	(*rcd) = NULL;
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

	NEARDAL_ASSERT_RET((rcdName != NULL) && (parent != NULL)
			  , NEARDAL_ERROR_INVALID_PARAMETER);

	NEARDAL_TRACEF("Adding record:%s\n", rcdName);
	rcd = g_try_malloc0(sizeof(RcdProp));
	if (rcd == NULL)
		goto exit;

	rcd->name = g_strdup(rcdName);
	if (rcd->name == NULL)
		goto exit;

	rcd->parent = tagProp;

	tagProp->rcdList = g_list_prepend(tagProp->rcdList, (gpointer) rcd);
	tagProp->rcdLen++;

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

/*****************************************************************************
 * neardal_rcd_remove: remove NFC record, unref DBus Proxy connection,
 * unregister record signal
 ****************************************************************************/
void neardal_rcd_remove(RcdProp *rcd)
{
	TagProp		*tagProp;

	NEARDAL_TRACEIN();
	NEARDAL_ASSERT(rcd != NULL);

	tagProp = rcd->parent;
	NEARDAL_TRACEF("Removing record:%s\n", rcd->name);
	tagProp->rcdList = g_list_remove(tagProp->rcdList,
					 (gconstpointer) rcd);
	/* Remove all records */
	neardal_rcd_prv_free(&rcd);
}
