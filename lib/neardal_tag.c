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

#include "neard_tag_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"


/*****************************************************************************
 * neardal_tag_prv_cb_property_changed: Callback called when a NFC tag
 * property is changed
 ****************************************************************************/
static void neardal_tag_prv_cb_property_changed(DBusGProxy *proxy,
					     const char	*arg_unnamed_arg0,
					     GValue	*gvalue,
					     void	*user_data)
{
	GPtrArray	*pathsGpa	= NULL;
	errorCode_t	err		= NEARDAL_ERROR_NO_TAG;
	TagProp		*tagProp	= user_data;

	(void) proxy; /* remove warning */

	NEARDAL_TRACEIN();

	if (tagProp == NULL || arg_unnamed_arg0 == NULL)
		return;

	NEARDAL_TRACEF("arg_unnamed_arg0='%s'\n", arg_unnamed_arg0);
	if (!strcmp(arg_unnamed_arg0, "Tags")) {
		if (!G_VALUE_HOLDS(gvalue, DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH)) {
			NEARDAL_TRACE_ERR("Unexpected type: %s",
					  G_VALUE_TYPE_NAME(&gvalue));
			err = NEARDAL_ERROR_DBUS;
			return;
		}

		/* Extract the tags arrays List from the GValue */
		err = NEARDAL_ERROR_NO_TAG;
		pathsGpa = g_value_get_boxed(gvalue);
		if (pathsGpa == NULL)
			goto error;

		if (pathsGpa->len <= 0)
			goto error;

		/* Getting last tag */
		gvalue = g_ptr_array_index(pathsGpa, pathsGpa->len - 1);
		if (gvalue != NULL)
			err = NEARDAL_SUCCESS;
		else
			err = NEARDAL_ERROR_NO_TAG;
	}

	return;

error:
	if (err != NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR("Exit with error code %d:%s\n", err,
				neardal_error_get_text(err));
		if (pathsGpa != NULL)
			g_boxed_free(G_TYPE_STRV, pathsGpa);
	}

	return;
}

/*****************************************************************************
 * neardal_tag_prv_read_properties: Get Neard Tag Properties
 ****************************************************************************/
static errorCode_t neardal_tag_prv_read_properties(TagProp *tagProp)
{
	errorCode_t	err			= NEARDAL_SUCCESS;
	GHashTable	*neardTagPropHash	= NULL;
	GError		*gerror		= NULL;
	GVariant	*tmp		= NULL;
	gsize		len;
	GPtrArray	*rcdArray		= NULL;

	NEARDAL_TRACEIN();
	g_assert(tagProp != NULL);
	g_assert(tagProp->proxy != NULL);

	org_neard_Tag_get_properties(tagProp->proxy,
					&neardTagPropHash,
						&gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read tag's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}
	err = neardal_tools_prv_hashtable_get(neardTagPropHash,
						   "Records",
					    DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
					    &rcdArray);
	if (err == NEARDAL_SUCCESS) {
			char *rcdName;
		neardal_tools_prv_g_ptr_array_copy(&tagProp->rcdArray,
						   rcdArray);

		tagProp->rcdLen = rcdArray->len;
		while ((len < tagProp->rcdLen) && (err == NEARDAL_SUCCESS)) {
			rcdName = g_ptr_array_index(rcdArray, len++);
			err = neardal_rcd_add(rcdName, tagProp);
		}
	}

	err = neardal_tools_prv_hashtable_get(neardTagPropHash,
						   "TagType",
						    G_TYPE_STRV, &tmp);
	if (err == NEARDAL_SUCCESS)
		tagProp->tagType = g_boxed_copy(G_TYPE_STRV, tmp);

	err = neardal_tools_prv_hashtable_get(neardTagPropHash, "Type",
						   G_TYPE_STRING, &tmp);
	if (err == NEARDAL_SUCCESS)
		tagProp->type = g_strdup((const gchar*) tmp);

	err = neardal_tools_prv_hashtable_get(neardTagPropHash,
						   "ReadOnly", G_TYPE_BOOLEAN,
						   &tmp);
	if (err == NEARDAL_SUCCESS)
		tagProp->readOnly = (gboolean) tmp;

	g_hash_table_destroy(neardTagPropHash);

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
	g_assert(tagProp != NULL);

	if (tagProp->proxy != NULL) {
		dbus_g_proxy_disconnect_signal(tagProp->proxy,
					       NEARD_TAG_SIG_PROPCHANGED,
				G_CALLBACK(neardal_tag_prv_cb_property_changed),
						     NULL);
		g_object_unref(tagProp->proxy);
		tagProp->proxy = NULL;
	}
	tagProp->proxy = NULL;

	err = neardal_tools_prv_create_proxy(neardalMgr.conn,
						  &tagProp->proxy,
							tagProp->name,
						  NEARD_TAGS_IF_NAME);
	if (err != NEARDAL_SUCCESS)
		return err;

	/* Populate Tag datas... */
	err = neardal_tag_prv_read_properties(tagProp);
	if (err != NEARDAL_SUCCESS)
		return err;

	/* Register Marshaller for signals (String,Variant) */
	dbus_g_object_register_marshaller(neardal_marshal_VOID__STRING_BOXED,
					   G_TYPE_NONE, G_TYPE_STRING,
					   G_TYPE_VALUE, G_TYPE_INVALID);

	NEARDAL_TRACEF("Register Neard-Tag Signal 'PropertyChanged'\n");
	dbus_g_proxy_add_signal(tagProp->proxy, NEARD_TAG_SIG_PROPCHANGED,
				 G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(tagProp->proxy,
				    NEARD_TAG_SIG_PROPCHANGED,
			G_CALLBACK(neardal_tag_prv_cb_property_changed),
				    tagProp, NULL);

	return err;
}

/*****************************************************************************
 * neardal_tag_prv_free: unref DBus proxy, disconnect Neard Tag signals
 ****************************************************************************/
static void neardal_tag_prv_free(TagProp **tagProp)
{
	NEARDAL_TRACEIN();
	if ((*tagProp)->proxy != NULL) {
		dbus_g_proxy_disconnect_signal((*tagProp)->proxy,
					       NEARD_TAG_SIG_PROPCHANGED,
				G_CALLBACK(neardal_tag_prv_cb_property_changed),
						     NULL);
		g_object_unref((*tagProp)->proxy);
		(*tagProp)->proxy = NULL;
	}
	g_free((*tagProp)->name);
	g_free((*tagProp)->type);
	neardal_tools_prv_g_ptr_array_free((*tagProp)->rcdArray);
	if ((*tagProp)->tagType != NULL)
		g_boxed_free(G_TYPE_STRV, (*tagProp)->tagType);
}

/*****************************************************************************
 * neardal_tag_notify_tag_found: Invoke client callback for 'record found'
 * if present, and 'tag found' (if not already nofied)
 ****************************************************************************/
void neardal_tag_notify_tag_found(TagProp *tagProp)
{
	RcdProp *rcdProp;
	gsize	len;

	g_assert(tagProp != NULL);

	if (tagProp->notified == FALSE && neardalMgr.cb_tag_found != NULL) {
		(neardalMgr.cb_tag_found)(tagProp->name,
					   neardalMgr.cb_tag_found_ud);
		tagProp->notified = TRUE;
	}

	len = 0;
	if (neardalMgr.cb_rcd_found != NULL)
		while (len < g_list_length(tagProp->rcdList)) {
			rcdProp = g_list_nth_data(tagProp->rcdList, len++);
			if (rcdProp->notified == FALSE) {
				(neardalMgr.cb_rcd_found)(rcdProp->name,
						neardalMgr.cb_rcd_found_ud);
				rcdProp->notified = TRUE;
			}
		}
}
/*****************************************************************************
 * neardal_tag_prv_write: Creates and write NDEF record to be written to
 * an NFC tag
 ****************************************************************************/
errorCode_t neardal_tag_prv_write(TagProp *tagProp, RcdProp *rcd)
{
	errorCode_t	err;
	GError		*gerror	= NULL;
	GHashTable	*hash;

	g_assert(tagProp != NULL);

	hash = neardal_tools_prv_create_dict();
	if (hash != NULL)
		err = neardal_rcd_prv_format(&hash, rcd);
	else
		err = NEARDAL_ERROR_NO_MEMORY;

	if (err != NEARDAL_SUCCESS)
		goto exit;
	org_neard_Tag_write(tagProp->proxy, hash, &gerror);

exit:
	if (gerror != NULL) {
		NEARDAL_TRACE_ERR("Unable to write tag record (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		err = NEARDAL_ERROR_DBUS;
	}

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

	g_assert(tagName != NULL);

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

	g_assert(tagProp != NULL);

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
