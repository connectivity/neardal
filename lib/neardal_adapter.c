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


/******************************************************************************
 * neardal_adp_prv_cb_tag_found: Callback called when a NFC tag is
 * found
 *****************************************************************************/
static void  neardal_adp_prv_cb_tag_found(orgNeardTag *proxy,
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
	err = neardal_tag_add((char *) arg_unnamed_arg0, adpProp);
	if (err == NEARDAL_SUCCESS) {
		tagProp = g_list_nth_data(adpProp->tagList, 0);
		neardal_tag_notify_tag_found(tagProp);
	}
	NEARDAL_TRACEF("NEARDAL LIB tagList contains %d elements\n",
		      g_list_length(adpProp->tagList));
}

/******************************************************************************
 * neardal_adp_prv_cb_tag_lost: Callback called when a NFC tag is
 * lost (removed)
 *****************************************************************************/
static void neardal_adp_prv_cb_tag_lost(orgNeardTag *proxy,
					   const gchar *arg_unnamed_arg0,
					   void *user_data)
{
	AdpProp		*adpProp	= user_data;
	TagProp		*tagProp	= NULL;
	errorCode_t	errCode;

	NEARDAL_TRACEIN();
	g_assert(arg_unnamed_arg0 != NULL);
	(void) proxy; /* remove warning */

	neardal_mgr_prv_get_adapter((char *) arg_unnamed_arg0, &adpProp);

	NEARDAL_TRACEF("Removing tag '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Tag Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	errCode = neardal_mgr_prv_get_tag(adpProp, (char *) arg_unnamed_arg0,
						  &tagProp);
	if (errCode == NEARDAL_SUCCESS) {
		if (neardalMgr.cb_tag_lost != NULL)
			(neardalMgr.cb_tag_lost)((char *) arg_unnamed_arg0,
					      neardalMgr.cb_tag_lost_ud);
		neardal_tag_remove(tagProp);
		NEARDAL_TRACEF("NEARDAL LIB tagList contains %d elements\n",
			      g_list_length(adpProp->tagList));
	}
}

/******************************************************************************
 * neardal_adp_prv_cb_property_changed: Callback called when a NFC tag
 * is found
 *****************************************************************************/
static void neardal_adp_prv_cb_property_changed(orgNeardAdp *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void        *user_data)
{
	AdpProp		*adpProp	= NULL;
	errorCode_t	errCode		= NEARDAL_ERROR_NO_TARGET;
	char		*tagName	= NULL;
	void		*clientValue	= NULL;
	TagProp		*tagProp	= NULL;
	GVariant	*tmp		= NULL;
	gchar		**tagArray	= NULL;

	(void) proxy; /* remove warning */
	(void) user_data; /* remove warning */
	NEARDAL_TRACEIN();
	g_assert(arg_unnamed_arg0 != NULL);

	neardal_mgr_prv_get_adapter_from_proxy(proxy, &adpProp);
	if (adpProp == NULL) {
		errCode = NEARDAL_ERROR_GENERAL_ERROR;
		goto exit;
	}

	tmp = g_variant_get_variant(arg_unnamed_arg1);
	if (tmp == NULL) {
		errCode = NEARDAL_ERROR_GENERAL_ERROR;
		goto exit;
	}

	if (!strcmp(arg_unnamed_arg0, "Polling")) {
		adpProp->polling = g_variant_get_boolean(tmp);
		clientValue = GUINT_TO_POINTER(adpProp->polling);
		NEARDAL_TRACEF("neardalMgr.polling=%d\n", adpProp->polling);
	}

	if (!strcmp(arg_unnamed_arg0, "Tags")) {
		guint tmpLen;

		tagArray = g_variant_dup_objv(tmp, &tmpLen);
		adpProp->tagNb = tmpLen;
		if (tmpLen == 0) {
			NEARDAL_TRACEF(
				"Tag array empty! Removing all tags\n");
			while (tmpLen < g_list_length(adpProp->tagList)) {
				tagProp = g_list_nth_data(adpProp->tagList,
							  tmpLen++);
				neardal_adp_prv_cb_tag_lost(tagProp->proxy,
							       tagProp->name,
							       tagProp->parent);
			}
			g_strfreev(tagArray);

			errCode = NEARDAL_SUCCESS;
			goto exit;
		}

		/* Extract the tags arrays List from the GValue */
		errCode = NEARDAL_ERROR_NO_ADAPTER;
		tmpLen = 0;
		while (tmpLen < adpProp->tagNb) {
			/* Getting last tag (tags list not updated with
			 * tags lost */
			tagName = g_strdup(tagArray[tmpLen++]);

			/* TODO : for Neard Workaround, emulate 'TagFound'
			 * signals */
			errCode = neardal_mgr_prv_get_tag(adpProp,
							     tagName,
							     &tagProp);
			if (errCode == NEARDAL_ERROR_NO_TARGET) {
				neardal_adp_prv_cb_tag_found(NULL,
								tagName,
								adpProp);
				errCode = NEARDAL_SUCCESS;
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
	if (errCode != NEARDAL_SUCCESS)
		NEARDAL_TRACEF("Exit with error code %d:%s\n", errCode,
			      neardal_error_get_text(errCode));
	return;
}

/******************************************************************************
 * neardal_adp_prv_read_properties: Get Neard Adapter Properties
 *****************************************************************************/
static errorCode_t neardal_adp_prv_read_properties(AdpProp *adpProp)
{
	errorCode_t	errCode			= NEARDAL_SUCCESS;
	GError		*gerror			= NULL;
	GVariant	*tmp			= NULL;
	GVariant	*tmpOut			= NULL;
	gchar		**tagArray		= NULL;
	gsize		len;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);
	g_assert(adpProp->proxy != NULL);

	org_neard_adp__call_get_properties_sync(adpProp->proxy, &tmp,
						NULL, &gerror);
	if (gerror != NULL) {
		errCode = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read adapter's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}

	NEARDAL_TRACE_LOG("Reading:\n%s\n", g_variant_print(tmp, TRUE));
	tmpOut = g_variant_lookup_value(tmp, "Tags", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		tagArray = g_variant_dup_objv(tmpOut, &len);
		adpProp->tagNb = len;
		if (len == 0) {
			g_strfreev(tagArray);
			tagArray = NULL;
		} else {
			guint len = 0;
			char *tagName;

			while (len < adpProp->tagNb &&
				errCode == NEARDAL_SUCCESS) {
				tagName = tagArray[len++];
				errCode = neardal_tag_add(tagName, adpProp);
			}
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "Polling", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		adpProp->polling = g_variant_get_boolean(tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Powered", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		adpProp->powered = g_variant_get_boolean(tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Protocols",
					G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		adpProp->protocols = g_variant_dup_strv(tmpOut, &len);
		adpProp->lenProtocols = len;
		if (adpProp->lenProtocols == 0) {
			g_strfreev(adpProp->protocols);
			adpProp->protocols = NULL;
		}
	}

exit:
	return errCode;
}

/******************************************************************************
 * neardal_adp_prv_get_current_tag: Get current NFC tag
 *****************************************************************************/
errorCode_t neardal_adp_prv_get_tag(AdpProp *adpProp, gchar *tagName,
				       TagProp **tagProp)
{
	errorCode_t	errCode	= NEARDAL_ERROR_NO_TARGET;
	guint		len = 0;
	TagProp		*tag;

	g_assert(adpProp != NULL);
	g_assert(tagProp != NULL);

	while (len < g_list_length(adpProp->tagList)) {
		tag = g_list_nth_data(adpProp->tagList, len);
		if (tag != NULL) {
			if (!strncmp(tag->name, tagName, strlen(tag->name))) {
				*tagProp = tag;
				errCode = NEARDAL_SUCCESS;
				break;
			}
		}
		len++;
	}

	return errCode;
}

/******************************************************************************
 * neardal_adp_init: Get Neard Manager Properties = NFC Adapters list.
 * Create a DBus proxy for the first one NFC adapter if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
static errorCode_t neardal_adp_prv_init(AdpProp *adpProp)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);

	if (adpProp->proxy != NULL) {
		g_signal_handlers_disconnect_by_func(adpProp->proxy,
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

	errCode = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp->name == NULL)
		return errCode;

	adpProp->proxy = org_neard_adp__proxy_new_sync(neardalMgr.conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							adpProp->name,
							NULL, /* GCancellable */
							&neardalMgr.gerror);

	if (neardalMgr.gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Adapter Proxy (%d:%s)\n",
				 neardalMgr.gerror->code,
				neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	errCode = neardal_adp_prv_read_properties(adpProp);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'PropertyChanged'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_PROPCHANGED,
			G_CALLBACK(neardal_adp_prv_cb_property_changed),
			  NULL);

	/* Register 'TagFound', 'TagLost' */
	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TagFound'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_TGT_FOUND,
			G_CALLBACK(neardal_adp_prv_cb_tag_found),
			  adpProp);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TagLost'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_TGT_LOST,
			G_CALLBACK(neardal_adp_prv_cb_tag_lost),
			  adpProp);

	return errCode;
}

/******************************************************************************
 * neardal_adp_prv_free: unref DBus proxy, disconnect Neard Adapter signals
 *****************************************************************************/
static void neardal_adp_prv_free(AdpProp **adpProp)
{
	NEARDAL_TRACEIN();
	if ((*adpProp)->proxy != NULL) {
		g_signal_handlers_disconnect_by_func((*adpProp)->proxy,
				G_CALLBACK(neardal_adp_prv_cb_property_changed),
						     NULL);
		g_signal_handlers_disconnect_by_func((*adpProp)->proxy,
				G_CALLBACK(neardal_adp_prv_cb_tag_found),
						     NULL);
		g_signal_handlers_disconnect_by_func((*adpProp)->proxy,
				G_CALLBACK(neardal_adp_prv_cb_tag_lost),
						     NULL);
		g_object_unref((*adpProp)->proxy);
		(*adpProp)->proxy = NULL;
	}
	g_free((*adpProp)->name);
	if ((*adpProp)->protocols != NULL)
		g_strfreev((*adpProp)->protocols);
	g_free((*adpProp));
	(*adpProp) = NULL;
}

/******************************************************************************
 * neardal_get_adapters: get an array of NFC adapters (adpName) present
 *****************************************************************************/
errorCode_t neardal_get_adapters(char ***array, int *len)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	int		adpNb		= 0;
	int		ct		= 0;	/* counter */
	char		**adps		= NULL;
	AdpProp		*adapter	= NULL;
	gsize		size;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	adpNb = g_list_length(neardalMgr.prop.adpList);
	if (adpNb > 0) {
		err = NEARDAL_ERROR_NO_MEMORY;
		size = (adpNb + 1) * sizeof(char *);
		adps = g_try_malloc0(size);
		if (adps != NULL) {
			GList	*list;
			while (ct < adpNb) {
				list = neardalMgr.prop.adpList;
				adapter = g_list_nth_data(list, ct);
				if (adapter != NULL)
					adps[ct++] = g_strdup(adapter->name);
			}
			err = NEARDAL_SUCCESS;
		}
	} else
		err = NEARDAL_ERROR_NO_ADAPTER;

	if (len != NULL)
		*len = adpNb;
	*array	= adps;

	return err;
}

/******************************************************************************
 * neardal_adp_add: add new NFC adapter, initialize DBus Proxy connection,
 * register adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_add(gchar *adapterName)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;
	AdpProp		*adpProp = NULL;
	GList		**adpList;
	TagProp		*tagProp;
	gsize		len;

	NEARDAL_TRACEF("Adding adapter:%s\n", adapterName);

	adpProp = g_try_malloc0(sizeof(AdpProp));
	if (adpProp == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	adpProp->name = g_strdup(adapterName);
	adpProp->parent = &neardalMgr;

	adpList = &neardalMgr.prop.adpList;
	*adpList = g_list_prepend(*adpList, (gpointer) adpProp);
	errCode = neardal_adp_prv_init(adpProp);

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
	return errCode;
}

/******************************************************************************
 * neardal_adp_remove: remove one NFC adapter, unref DBus Proxy connection,
 * unregister adapter signal
 *****************************************************************************/
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
		neardal_tag_remove(tagProp);
	}

	adpList = &neardalMgr.prop.adpList;
	(*adpList) = g_list_remove((*adpList), (gconstpointer) adpProp);
	neardal_adp_prv_free(&adpProp);

	return NEARDAL_SUCCESS;
}

