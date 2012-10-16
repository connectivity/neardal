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

#include "neard_adapter_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"


/*****************************************************************************
 * neardal_adp_prv_cb_tag_found: Callback called when a NFC tag is
 * found
 ****************************************************************************/
static void  neardal_adp_prv_cb_tag_found(DBusGProxy *proxy,
					     const gchar *arg_unnamed_arg0,
					     void        *user_data)
{
	AdpProp		*adpProp	= user_data;
	errorCode_t	err;
	TagProp		*tagProp;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */

	g_assert(arg_unnamed_arg0 != NULL);
	g_assert(adpProp != NULL);

	NEARDAL_TRACEF("Adding tag '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Tag Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_tag_prv_add((char *) arg_unnamed_arg0, adpProp);
	if (err == NEARDAL_SUCCESS) {
		tagProp = g_list_nth_data(adpProp->tagList, 0);
		neardal_tag_notify_tag_found(tagProp);
	}
	NEARDAL_TRACEF("NEARDAL LIB tagList contains %d elements\n",
		      g_list_length(adpProp->tagList));
}

/*****************************************************************************
 * neardal_adp_prv_cb_tag_lost: Callback called when a NFC tag is
 * lost (removed)
 ****************************************************************************/
static void neardal_adp_prv_cb_tag_lost(DBusGProxy *proxy,
					   const gchar *arg_unnamed_arg0,
					   void *user_data)
{
	AdpProp		*adpProp	= user_data;
	TagProp		*tagProp	= NULL;
	errorCode_t	err;

	NEARDAL_TRACEIN();
	g_assert(arg_unnamed_arg0 != NULL);
	(void) proxy; /* remove warning */

	neardal_mgr_prv_get_adapter((char *) arg_unnamed_arg0, &adpProp);

	NEARDAL_TRACEF("Removing tag '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Tag Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_mgr_prv_get_tag(adpProp, (char *) arg_unnamed_arg0,
						  &tagProp);
	if (err == NEARDAL_SUCCESS) {
		if (neardalMgr.cb_tag_lost != NULL)
			(neardalMgr.cb_tag_lost)((char *) arg_unnamed_arg0,
					      neardalMgr.cb_tag_lost_ud);
		neardal_tag_prv_remove(tagProp);
		NEARDAL_TRACEF("NEARDAL LIB tagList contains %d elements\n",
			      g_list_length(adpProp->tagList));
	}
}

/*****************************************************************************
 * neardal_adp_prv_cb_property_changed: Callback called when a NFC tag
 * is found
 ****************************************************************************/
static void neardal_adp_prv_cb_property_changed(DBusGProxy *proxy,
						const gchar *arg_unnamed_arg0,
						GValue      *gvalue,
						void        *user_data)
{
	AdpProp		*adpProp	= NULL;
	errorCode_t	err		= NEARDAL_ERROR_NO_TAG;
	char		*tagName	= NULL;
	void		*clientValue	= NULL;
	TagProp		*tagProp	= NULL;
	gchar		**tagArray	= NULL;
	GPtrArray	*pathsGpa		= NULL;

	(void) proxy; /* remove warning */
	(void) user_data; /* remove warning */
	NEARDAL_TRACEIN();
	g_assert(arg_unnamed_arg0 != NULL);

	neardal_mgr_prv_get_adapter_from_proxy(proxy, &adpProp);
	if (adpProp == NULL) {
		err = NEARDAL_ERROR_GENERAL_ERROR;
		goto exit;
	}


	if (!strcmp(arg_unnamed_arg0, "Polling")) {
		adpProp->polling =  g_value_get_boolean(gvalue);
		clientValue = GUINT_TO_POINTER(adpProp->polling);
		NEARDAL_TRACEF("neardalMgr.polling=%d\n", adpProp->polling);
	}

	if (!strcmp(arg_unnamed_arg0, "Tags")) {
		guint tmpLen;
		if (!G_IS_VALUE(gvalue)) {
			NEARDAL_TRACE_ERR(
			"Unexpected value returned getting adapters: %s",
					  G_VALUE_TYPE_NAME(&gvalue));
			err = NEARDAL_ERROR_DBUS;
			goto exit;
		}

		if (!G_VALUE_HOLDS(gvalue, DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH)) {
			NEARDAL_TRACE_ERR(
				"Unexpected type returned getting adapters: %s",
					  G_VALUE_TYPE_NAME(&gvalue));
			err = NEARDAL_ERROR_DBUS;
			goto exit;
		}

		/* Extract the tags arrays List from the GValue */
		pathsGpa = g_value_get_boxed(gvalue);
		err = NEARDAL_ERROR_NO_ADAPTER;
		NEARDAL_TRACE("pathsGpa=%p\n", pathsGpa);
		if (pathsGpa == NULL)
			goto exit;

		adpProp->tagNb = pathsGpa->len;
		if (adpProp->tagNb <= 0) {	/* Remove all tags */
			GList *node = NULL;
			NEARDAL_TRACEF(
				"Tag array empty! Removing all tags\n");
			while (g_list_length(adpProp->tagList)) {
				node = g_list_first(adpProp->tagList);
				tagProp = (TagProp *) node->data;
				neardal_adp_prv_cb_tag_lost(tagProp->proxy,
							       tagProp->name,
							       tagProp->parent);
			}
			g_strfreev(tagArray);

			err = NEARDAL_SUCCESS;
			goto exit;
		}

		/* Extract the tags arrays List from the GValue */
		err = NEARDAL_ERROR_NO_ADAPTER;
		tmpLen = 0;
		while (tmpLen < adpProp->tagNb) {
			/* Getting last tag (tags list not updated with
			 * tags lost */
			gvalue = g_ptr_array_index(pathsGpa, tmpLen++);

			if (gvalue == NULL)
				goto exit;

			tagName = g_strdup((const gchar *)gvalue);

			/* TODO : for Neard Workaround, emulate 'TagFound'
			 * signals */
			err = neardal_mgr_prv_get_tag(adpProp,
							     tagName,
							     &tagProp);
			if (err == NEARDAL_ERROR_NO_TAG) {
				neardal_adp_prv_cb_tag_found(NULL,
								tagName,
								adpProp);
				err = NEARDAL_SUCCESS;
			}
		}
		g_strfreev(tagArray);
	}

	if (neardalMgr.cb_adp_prop_changed != NULL)
		(neardalMgr.cb_adp_prop_changed)(adpProp->name,
						  (char *) arg_unnamed_arg0,
						  clientValue,
					neardalMgr.cb_adp_prop_changed_ud);
	return;

exit:
	if (err != NEARDAL_SUCCESS)
		NEARDAL_TRACEF("Exit with error code %d:%s\n", err,
			      neardal_error_get_text(err));
	return;
}

/*****************************************************************************
 * neardal_adp_prv_read_properties: Get Neard Adapter Properties
 ****************************************************************************/
static errorCode_t neardal_adp_prv_read_properties(AdpProp *adpProp)
{
	errorCode_t	err			= NEARDAL_SUCCESS;
	GError		*gerror			= NULL;
	void		*tmp			= NULL;
	GHashTable	*neardAdapterPropHash	= NULL;
	GPtrArray	*tagArray		= NULL;
	gsize		len;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);
	g_assert(adpProp->proxy != NULL);

	org_neard_Adapter_get_properties(adpProp->proxy,
					 &neardAdapterPropHash,
					 &gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read adapter's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}

	err = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
						"Tags",
					DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
					&tagArray);
	if (err == NEARDAL_SUCCESS && tagArray != NULL && tagArray->len > 0) {
		adpProp->tagNb = tagArray->len;
		if (adpProp->tagNb == 0) {
			g_ptr_array_unref(tagArray);
			tagArray = NULL;
		} else {
			len = 0;
			char *tagName;

			while (len < adpProp->tagNb &&
			err == NEARDAL_SUCCESS) {
				tagName = g_ptr_array_index(tagArray,
							    len++);
				err = neardal_tag_prv_add(tagName,
							  adpProp);
			}
		}
	}

	err = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
					      "Polling", G_TYPE_BOOLEAN, &tmp);
	if (err == NEARDAL_SUCCESS)
		adpProp->polling = (gboolean) tmp;

	err = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
					      "Powered", G_TYPE_BOOLEAN, &tmp);
	if (err == NEARDAL_SUCCESS)
		adpProp->powered = (gboolean) tmp;

	err = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
					      "Protocols", G_TYPE_STRV, &tmp);
	if (err == NEARDAL_SUCCESS) {
		adpProp->protocols = g_boxed_copy(G_TYPE_STRV, tmp);
		len = 0;
		while (adpProp->protocols[len] != NULL) 
			len++;
		adpProp->lenProtocols = len - 1;
		if (adpProp->lenProtocols == 0) {
			g_strfreev(adpProp->protocols);
			adpProp->protocols = NULL;
		}
	}
	g_hash_table_destroy(neardAdapterPropHash);

exit:
	return err;
}

/*****************************************************************************
 * neardal_adp_prv_get_tag: Get current NFC tag
 ****************************************************************************/
errorCode_t neardal_adp_prv_get_tag(AdpProp *adpProp, gchar *tagName,
				       TagProp **tagProp)
{
	errorCode_t	err	= NEARDAL_ERROR_NO_TAG;
	guint		len = 0;
	TagProp		*tag;

	g_assert(adpProp != NULL);
	g_assert(tagProp != NULL);

	while (len < g_list_length(adpProp->tagList)) {
		tag = g_list_nth_data(adpProp->tagList, len);
		if (tag != NULL) {
			if (!strncmp(tag->name, tagName, strlen(tag->name))) {
				*tagProp = tag;
				err = NEARDAL_SUCCESS;
				break;
			}
		}
		len++;
	}

	return err;
}

/*****************************************************************************
 * neardal_adp_init: Get Neard Manager Properties = NFC Adapters list.
 * Create a DBus proxy for the first one NFC adapter if present
 * Register Neard Manager signals ('PropertyChanged')
 ****************************************************************************/
static errorCode_t neardal_adp_prv_init(AdpProp *adpProp)
{
	errorCode_t	err = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);

	if (adpProp->proxy != NULL) {
		dbus_g_proxy_disconnect_signal(adpProp->proxy,
					       NEARD_ADP_SIG_PROPCHANGED,
				G_CALLBACK(neardal_adp_prv_cb_property_changed),
						     NULL);
		g_signal_handlers_disconnect_by_func(adpProp->proxy,
				G_CALLBACK(neardal_adp_prv_cb_tag_found),
						     NULL);
		g_signal_handlers_disconnect_by_func(adpProp->proxy,
				G_CALLBACK(neardal_adp_prv_cb_tag_lost),
						     NULL);
		g_object_unref(adpProp->proxy);
	}
	adpProp->proxy = NULL;

	err = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp->name == NULL)
		return err;

	err = neardal_tools_prv_create_proxy(neardalMgr.conn,
						  &adpProp->proxy,
							adpProp->name,
						  NEARD_ADP_IF_NAME);
	if (err != NEARDAL_SUCCESS)
		return err;

	err = neardal_adp_prv_read_properties(adpProp);

	/* Register Marshaller for signals (String,Variant) */
	dbus_g_object_register_marshaller(neardal_marshal_VOID__STRING_BOXED,
						G_TYPE_NONE, G_TYPE_STRING,
						G_TYPE_VALUE, G_TYPE_INVALID);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'PropertyChanged'\n");
	dbus_g_proxy_add_signal(adpProp->proxy, NEARD_ADP_SIG_PROPCHANGED,
				G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(adpProp->proxy,
				    NEARD_ADP_SIG_PROPCHANGED,
			G_CALLBACK(neardal_adp_prv_cb_property_changed),
				    &neardalMgr, NULL);

	/* Register 'TagFound', 'TagLost' */
	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TagFound'\n");
	dbus_g_proxy_add_signal(adpProp->proxy, NEARD_ADP_SIG_TAG_FOUND,
				DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(adpProp->proxy, NEARD_ADP_SIG_TAG_FOUND,
		G_CALLBACK(neardal_adp_prv_cb_tag_found), &neardalMgr,
				    NULL);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TagLost'\n");
	dbus_g_proxy_add_signal(adpProp->proxy, NEARD_ADP_SIG_TAG_LOST,
				DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(adpProp->proxy, NEARD_ADP_SIG_TAG_LOST,
			G_CALLBACK(neardal_adp_prv_cb_tag_lost),
					&neardalMgr, NULL);

	return err;
}

/*****************************************************************************
 * neardal_adp_prv_free: unref DBus proxy, disconnect Neard Adapter signals
 ****************************************************************************/
static void neardal_adp_prv_free(AdpProp **adpProp)
{
	NEARDAL_TRACEIN();
	if ((*adpProp)->proxy != NULL) {
		dbus_g_proxy_disconnect_signal((*adpProp)->proxy,
					       NEARD_ADP_SIG_PROPCHANGED,
				G_CALLBACK(neardal_adp_prv_cb_property_changed),
						     NULL);
		dbus_g_proxy_disconnect_signal((*adpProp)->proxy,
					       NEARD_ADP_SIG_TAG_FOUND,
				G_CALLBACK(neardal_adp_prv_cb_tag_found),
						     NULL);
		dbus_g_proxy_disconnect_signal((*adpProp)->proxy,
					       NEARD_ADP_SIG_TAG_LOST,
				G_CALLBACK(neardal_adp_prv_cb_tag_lost),
						     NULL);
		g_object_unref((*adpProp)->proxy);
		(*adpProp)->proxy = NULL;
	}
	g_free((*adpProp)->name);
	if ((*adpProp)->protocols != NULL)
		g_boxed_free(G_TYPE_STRV, (*adpProp)->protocols);
	g_free((*adpProp));
	(*adpProp) = NULL;
}

/*****************************************************************************
 * neardal_adp_add: add new NFC adapter, initialize DBus Proxy connection,
 * register adapter signal
 ****************************************************************************/
errorCode_t neardal_adp_add(gchar *adapterName)
{
	errorCode_t	err = NEARDAL_SUCCESS;
	AdpProp		*adpProp = NULL;
	GList		**adpList;
	TagProp		*tagProp;
	gsize		len;

	// Check if adapter already exist in list...
	err = neardal_mgr_prv_get_adapter(adapterName, NULL);
	if (err != NEARDAL_SUCCESS) {
		NEARDAL_TRACEF("Adding adapter:%s\n", adapterName);

		adpProp = g_try_malloc0(sizeof(AdpProp));
		if (adpProp == NULL)
			return NEARDAL_ERROR_NO_MEMORY;

		adpProp->name = g_strdup(adapterName);
		adpProp->parent = &neardalMgr;

	adpList = &neardalMgr.prop.adpList;
	*adpList = g_list_prepend(*adpList, (gpointer) adpProp);
	err = neardal_adp_prv_init(adpProp);

		NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
			g_list_length(*adpList));

		/* Invoke client cb 'adapter added' */
		if (neardalMgr.cb_adp_added != NULL)
				(neardalMgr.cb_adp_added)((char *) adapterName,
						neardalMgr.cb_adp_added_ud);

		/* Notify 'Tag Found' */
		len = 0;
		while (len < g_list_length(adpProp->tagList)) {
			tagProp = g_list_nth_data(adpProp->tagList, len++);
			neardal_tag_notify_tag_found(tagProp);
			len++;
		}
	} else
		NEARDAL_TRACEF("Adapter '%s' already added\n", adapterName);
		
	return err;
}

/*****************************************************************************
 * neardal_adp_remove: remove one NFC adapter, unref DBus Proxy connection,
 * unregister adapter signal
 ****************************************************************************/
errorCode_t neardal_adp_remove(AdpProp *adpProp)
{
	TagProp		*tagProp;
	GList		*node = NULL;
	GList		**adpList;

	g_assert(adpProp != NULL);

	NEARDAL_TRACEF("Removing adapter:%s\n", adpProp->name);

	/* Remove all tags */
	while (g_list_length(adpProp->tagList)) {
		node = g_list_first(adpProp->tagList);
		tagProp = (TagProp *) node->data;
		neardal_tag_prv_remove(tagProp);
	}

	adpList = &neardalMgr.prop.adpList;
	(*adpList) = g_list_remove((*adpList), (gconstpointer) adpProp);
	neardal_adp_prv_free(&adpProp);

	return NEARDAL_SUCCESS;
}

