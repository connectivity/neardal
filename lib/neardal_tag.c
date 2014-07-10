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

#include "neard_tag_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"


/*****************************************************************************
 * neardal_tag_prv_cb_property_changed: Callback called when a NFC tag
 * property is changed
 ****************************************************************************/
static void neardal_tag_prv_cb_property_changed(OrgNeardTag *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void		*user_data)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	TagProp		*tagProp	= user_data;

	(void) proxy; /* remove warning */
	(void) arg_unnamed_arg1; /* remove warning */

	NEARDAL_TRACEIN();

	if (tagProp == NULL || arg_unnamed_arg0 == NULL)
		return;

	NEARDAL_TRACEF("str0='%s'\n", arg_unnamed_arg0);
	NEARDAL_TRACEF("arg_unnamed_arg1=%s (%s)\n",
		       g_variant_print (arg_unnamed_arg1, TRUE),
		       g_variant_get_type_string(arg_unnamed_arg1));


	if (err != NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR("Exit with error code %d:%s\n", err,
				neardal_error_get_text(err));
	}

	return;
}

/*****************************************************************************
 * neardal_tag_prv_read_properties: Get Neard Tag Properties
 ****************************************************************************/
static errorCode_t neardal_tag_prv_read_properties(TagProp *tagProp)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	GError		*gerror		= NULL;
	GVariant	*tmp		= NULL;
	GVariant	*tmpOut		= NULL;
	gsize		len;
	gchar		**rcdArray	= NULL;

	NEARDAL_TRACEIN();
	NEARDAL_ASSERT_RET(tagProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);
	NEARDAL_ASSERT_RET(tagProp->proxy != NULL
			  , NEARDAL_ERROR_GENERAL_ERROR);

	org_neard_tag_call_get_properties_sync(tagProp->proxy, &tmp, NULL,
						&gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read tag's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}
	NEARDAL_TRACEF("Reading:\n%s\n", g_variant_print(tmp, TRUE));
	tmpOut = g_variant_lookup_value(tmp, "Records", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		rcdArray = g_variant_dup_objv(tmpOut, &len);
		tagProp->rcdLen = len;
		if (len == 0) {
			g_strfreev(rcdArray);
			rcdArray = NULL;
		} else {
			guint len = 0;
			char *rcdName;

			while (len < tagProp->rcdLen &&
				err == NEARDAL_SUCCESS) {
				rcdName = rcdArray[len++];
				err = neardal_rcd_add(rcdName, tagProp);
			}
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "TagType", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		tagProp->tagType = g_variant_dup_strv(tmpOut, &len);
		tagProp->tagTypeLen = len;
		if (len == 0) {
			g_strfreev(tagProp->tagType);
			tagProp->tagType = NULL;
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "Type", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		tagProp->type = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "ReadOnly",
					G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		tagProp->readOnly = g_variant_get_boolean(tmpOut);

exit:
	return err;
}

/*****************************************************************************
 * neardal_tag_init: Get Neard Manager Properties = NFC Tags list.
 * Create a DBus proxy for the first one NFC tag if present
 * Register Neard Manager signals ('PropertyChanged')
 ****************************************************************************/
static errorCode_t neardal_tag_prv_init(TagProp *tagProp)
{
	errorCode_t	err = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	NEARDAL_ASSERT_RET(tagProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

	if (tagProp->proxy != NULL) {
		g_signal_handlers_disconnect_by_func(tagProp->proxy,
			NEARDAL_G_CALLBACK(neardal_tag_prv_cb_property_changed),
						     NULL);
		g_object_unref(tagProp->proxy);
		tagProp->proxy = NULL;
	}
	tagProp->proxy = NULL;

	tagProp->proxy = org_neard_tag_proxy_new_sync(neardalMgr.conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							tagProp->name,
							NULL, /* GCancellable */
							&neardalMgr.gerror);
	if (neardalMgr.gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Tag Proxy (%d:%s)\n",
				  neardalMgr.gerror->code,
				  neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	/* Populate Tag datas... */
	err = neardal_tag_prv_read_properties(tagProp);
	if (err != NEARDAL_SUCCESS)
		return err;

	NEARDAL_TRACEF("Register Neard-Tag Signal 'PropertyChanged'\n");
	g_signal_connect(tagProp->proxy, NEARD_TAG_SIG_PROPCHANGED,
			G_CALLBACK(neardal_tag_prv_cb_property_changed),
			  tagProp);

	return err;
}

/*****************************************************************************
 * neardal_tag_prv_get_record: Get specific record from tag
 ****************************************************************************/
errorCode_t neardal_tag_prv_get_record(TagProp *tagProp, gchar *rcdName,
				       RcdProp **rcdProp)
{
	errorCode_t	err	= NEARDAL_ERROR_NO_RECORD;
	guint		len;
	RcdProp	*rcd	= NULL;

	NEARDAL_ASSERT_RET((tagProp != NULL) && (rcdName != NULL)
			   && (rcdProp != NULL)
			   , NEARDAL_ERROR_INVALID_PARAMETER);
	
	len = 0;
	while (len < g_list_length(tagProp->rcdList)) {
		rcd = g_list_nth_data(tagProp->rcdList, len);
		if (neardal_tools_prv_cmp_path(rcd->name, rcdName)) {
			*rcdProp = rcd;
			err = NEARDAL_SUCCESS;
			break;
		}
		len++;
	}

	return err;
}



/*****************************************************************************
 * neardal_tag_prv_free: unref DBus proxy, disconnect Neard Tag signals
 ****************************************************************************/
static void neardal_tag_prv_free(TagProp **tagProp)
{
	NEARDAL_TRACEIN();
	if ((*tagProp)->proxy != NULL) {
		g_signal_handlers_disconnect_by_func((*tagProp)->proxy,
			NEARDAL_G_CALLBACK(neardal_tag_prv_cb_property_changed),
						     NULL);
		g_object_unref((*tagProp)->proxy);
		(*tagProp)->proxy = NULL;
	}
	g_free((*tagProp)->name);
	g_free((*tagProp)->type);
	g_strfreev((*tagProp)->tagType);
	g_free((*tagProp));
	(*tagProp) = NULL;
}

/*****************************************************************************
 * neardal_tag_notify_tag_found: Invoke client callback for 'record found'
 * if present, and 'tag found' (if not already nofied)
 ****************************************************************************/
void neardal_tag_notify_tag_found(TagProp *tagProp)
{
	RcdProp *rcdProp;
	gsize	len;

	NEARDAL_ASSERT(tagProp != NULL);

	if (tagProp->notified == FALSE && neardalMgr.cb.tag_found != NULL) {
		(neardalMgr.cb.tag_found)(tagProp->name,
					   neardalMgr.cb.tag_found_ud);
		tagProp->notified = TRUE;
	}

	len = 0;
	if (neardalMgr.cb.rcd_found != NULL)
		while (len < g_list_length(tagProp->rcdList)) {
			rcdProp = g_list_nth_data(tagProp->rcdList, len++);
			if (rcdProp->notified == FALSE) {
				(neardalMgr.cb.rcd_found)(rcdProp->name,
						neardalMgr.cb.rcd_found_ud);
				rcdProp->notified = TRUE;
			}
		}
}

/*****************************************************************************
 * neardal_tag_prv_write: Creates and write NDEF record to be written to a NFC
 * tag
 ****************************************************************************/
errorCode_t neardal_tag_prv_write(TagProp *tagProp, RcdProp *rcd)
{
	GVariantBuilder	*dictBuilder = NULL;
	GVariant	*in;
	errorCode_t	err;
	GError		*gerror	= NULL;

	NEARDAL_ASSERT_RET(tagProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

	dictBuilder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
	if (dictBuilder == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	err = neardal_rcd_prv_format(dictBuilder, rcd);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	in = g_variant_builder_end(dictBuilder);
	NEARDAL_TRACEF("Sending:\n%s\n", g_variant_print(in, TRUE));
	org_neard_tag_call_write_sync(tagProp->proxy, in, NULL, &gerror);

exit:
	if (gerror != NULL) {
		NEARDAL_TRACE_ERR("Unable to write tag record (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		err = NEARDAL_ERROR_DBUS;
	}
	g_variant_builder_unref(dictBuilder);

	return err;
}

/*****************************************************************************
 * neardal_tag_prv_add: add new NFC tag, initialize DBus Proxy connection,
 * register tag signal
 ****************************************************************************/
errorCode_t neardal_tag_prv_add(gchar *tagName, void *parent)
{
	errorCode_t	err		= NEARDAL_ERROR_NO_MEMORY;
	TagProp		*tagProp	= NULL;
	AdpProp		*adpProp	= parent;

	NEARDAL_ASSERT_RET((tagName != NULL) && (parent != NULL)
			   , NEARDAL_ERROR_INVALID_PARAMETER);

	NEARDAL_TRACEF("Adding tag:%s\n", tagName);
	tagProp = g_try_malloc0(sizeof(TagProp));
	if (tagProp == NULL)
		goto error;

	tagProp->name	= g_strdup(tagName);
	tagProp->parent	= adpProp;

	adpProp->tagList = g_list_prepend(adpProp->tagList, tagProp);
	err = neardal_tag_prv_init(tagProp);

	NEARDAL_TRACEF("NEARDAL LIB tagList contains %d elements\n",
		      g_list_length(adpProp->tagList));

	return err;

error:
	if (tagProp->name != NULL)
		g_free(tagProp->name);
	if (tagProp != NULL)
		g_free(tagProp);

	return err;
}

/*****************************************************************************
 * neardal_tag_prv_remove: remove one NFC tag, unref DBus Proxy connection,
 * unregister tag signal
 ****************************************************************************/
void neardal_tag_prv_remove(TagProp *tagProp)
{
	RcdProp		*rcdProp	= NULL;
	GList		*node;
	AdpProp		*adpProp;

	NEARDAL_ASSERT(tagProp != NULL);

	NEARDAL_TRACEF("Removing tag:%s\n", tagProp->name);
	/* Remove all tags */
	while (g_list_length(tagProp->rcdList)) {
		node = g_list_first(tagProp->rcdList);
		rcdProp = (RcdProp *) node->data;
		neardal_rcd_remove(rcdProp);
	}
	adpProp = tagProp->parent;
	adpProp->tagList = g_list_remove(adpProp->tagList,
					 (gconstpointer) tagProp);

	neardal_tag_prv_free(&tagProp);
}
