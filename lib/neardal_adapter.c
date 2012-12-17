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
static void  neardal_adp_prv_cb_tag_found(orgNeardTag *proxy,
					     const gchar *arg_unnamed_arg0,
					     void        *user_data)
{
	AdpProp		*adpProp	= user_data;
	errorCode_t	err;
	TagProp		*tagProp;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */

	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);
	NEARDAL_ASSERT(adpProp != NULL);

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
static void neardal_adp_prv_cb_tag_lost(orgNeardTag *proxy,
					   const gchar *arg_unnamed_arg0,
					   void *user_data)
{
	AdpProp		*adpProp	= user_data;
	TagProp		*tagProp	= NULL;
	errorCode_t	err;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */
	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);

	neardal_mgr_prv_get_adapter((char *) arg_unnamed_arg0, &adpProp);

	NEARDAL_TRACEF("Removing tag '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Tag Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_adp_prv_get_tag(adpProp, (char *) arg_unnamed_arg0,
						  &tagProp);
	if (err == NEARDAL_SUCCESS) {
		if (neardalMgr.cb.tag_lost != NULL)
			(neardalMgr.cb.tag_lost)((char *) arg_unnamed_arg0,
					      neardalMgr.cb.tag_lost_ud);
		neardal_tag_prv_remove(tagProp);
		NEARDAL_TRACEF("NEARDAL LIB tagList contains %d elements\n",
			      g_list_length(adpProp->tagList));
	}
}

/*****************************************************************************
 * neardal_adp_prv_cb_dev_found: Callback called when a NFC dev is
 * found
 ****************************************************************************/
static void  neardal_adp_prv_cb_dev_found(orgNeardDev *proxy,
					     const gchar *arg_unnamed_arg0,
					     void        *user_data)
{
	AdpProp		*adpProp	= user_data;
	errorCode_t	err;
	DevProp		*devProp;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */

	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);
	NEARDAL_ASSERT(adpProp != NULL);

	NEARDAL_TRACEF("Adding device '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Dev Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_dev_prv_add((char *) arg_unnamed_arg0, adpProp);
	if (err == NEARDAL_SUCCESS) {
		devProp = g_list_nth_data(adpProp->devList, 0);
		neardal_dev_notify_dev_found(devProp);
	}
	NEARDAL_TRACEF("NEARDAL LIB devList contains %d elements\n",
		      g_list_length(adpProp->devList));
}

/*****************************************************************************
 * neardal_adp_prv_cb_dev_lost: Callback called when a NFC dev is
 * lost (removed)
 ****************************************************************************/
static void neardal_adp_prv_cb_dev_lost(orgNeardDev *proxy,
					   const gchar *arg_unnamed_arg0,
					   void *user_data)
{
	AdpProp		*adpProp	= user_data;
	DevProp		*devProp	= NULL;
	errorCode_t	err;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */
	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);

	neardal_mgr_prv_get_adapter((char *) arg_unnamed_arg0, &adpProp);

	NEARDAL_TRACEF("Removing dev '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Dev Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_adp_prv_get_dev(adpProp, (char *) arg_unnamed_arg0,
						  &devProp);
	if (err == NEARDAL_SUCCESS) {
		if (neardalMgr.cb.dev_lost != NULL)
			(neardalMgr.cb.dev_lost)((char *) arg_unnamed_arg0,
					      neardalMgr.cb.dev_lost_ud);
		neardal_dev_prv_remove(devProp);
		NEARDAL_TRACEF("NEARDAL LIB devList contains %d elements\n",
			      g_list_length(adpProp->devList));
	}
}

/*****************************************************************************
 * neardal_adp_prv_cb_property_changed: Callback called when a NFC tag
 * is found
 ****************************************************************************/
static void neardal_adp_prv_cb_property_changed(orgNeardAdp *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void        *user_data)
{
	AdpProp		*adpProp	= NULL;
	errorCode_t	err		= NEARDAL_ERROR_NO_TAG;
	char		*dbusObjPath	= NULL;
	void		*clientValue	= NULL;
	TagProp		*tagProp	= NULL;
	DevProp		*devProp	= NULL;
	gchar		**array		= NULL;
	GVariant	*gvalue		= NULL;
	gsize		mode_len;

	(void) proxy; /* remove warning */
	(void) user_data; /* remove warning */
	NEARDAL_TRACEIN();
	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);

	neardal_mgr_prv_get_adapter_from_proxy(proxy, &adpProp);
	if (adpProp == NULL) {
		err = NEARDAL_ERROR_GENERAL_ERROR;
		goto exit;
	}

	gvalue = g_variant_get_variant(arg_unnamed_arg1);
	if (gvalue == NULL) {
		err = NEARDAL_ERROR_GENERAL_ERROR;
		goto exit;
	}

	NEARDAL_TRACEF(" arg_unnamed_arg0 : %s\n", arg_unnamed_arg0);

	if (!strcmp(arg_unnamed_arg0, "Mode")) {
		if (adpProp->mode != NULL) {
			g_free(adpProp->mode);
			adpProp->mode = NULL;
		}

		adpProp->mode = g_strdup(g_variant_get_string(gvalue, &mode_len));
		clientValue = adpProp->mode;
		NEARDAL_TRACEF("neardalMgr.mode=%s\n", adpProp->mode);
	}

	if (!strcmp(arg_unnamed_arg0, "Polling")) {
		adpProp->polling = g_variant_get_boolean(gvalue);
		clientValue = GUINT_TO_POINTER(adpProp->polling);
		NEARDAL_TRACEF("neardalMgr.polling=%d\n", adpProp->polling);
	}

	if (!strcmp(arg_unnamed_arg0, "Tags")) {
		guint tmpLen;

		array = g_variant_dup_objv(gvalue, &tmpLen);
		adpProp->tagNb = tmpLen;
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
			g_strfreev(array);

			err = NEARDAL_SUCCESS;
			goto exit;
		}

		/* Extract the tags arrays List from the GValue */
		err = NEARDAL_ERROR_NO_ADAPTER;
		tmpLen = 0;
		while (tmpLen < adpProp->tagNb) {
			/* Getting last tag (tags list not updated with
			 * tags lost */
			dbusObjPath = g_strdup(array[tmpLen++]);

			/* TODO : for Neard Workaround, emulate 'TagFound'
			 * signals */
			err = neardal_adp_prv_get_tag(adpProp,
							     dbusObjPath,
							     &tagProp);
			clientValue = dbusObjPath;
			if (err == NEARDAL_ERROR_NO_TAG) {
				neardal_adp_prv_cb_tag_found(NULL,
								dbusObjPath,
								adpProp);
				err = NEARDAL_SUCCESS;
			}
		}
		g_strfreev(array);
		array = NULL;
	}

	if (!strcmp(arg_unnamed_arg0, "Devices")) {
		guint tmpLen;

		array = g_variant_dup_objv(gvalue, &tmpLen);
		adpProp->devNb = tmpLen;
		if (adpProp->devNb <= 0) {	/* Remove all devs */
			GList *node = NULL;
			NEARDAL_TRACEF(
				"Dev array empty! Removing all devs\n");
			while (g_list_length(adpProp->devList)) {
				node = g_list_first(adpProp->devList);
				devProp = (DevProp *) node->data;
				neardal_adp_prv_cb_dev_lost(devProp->proxy,
							       devProp->name,
							       devProp->parent);
			}
			g_strfreev(array);

			err = NEARDAL_SUCCESS;
			goto exit;
		}

		/* Extract the devs arrays List from the GValue */
		err = NEARDAL_ERROR_NO_ADAPTER;
		tmpLen = 0;
		while (tmpLen < adpProp->devNb) {
			/* Getting last dev (devs list not updated with
			 * devs lost */
			dbusObjPath = g_strdup(array[tmpLen++]);

			/* TODO : for Neard Workaround, emulate 'DevFound'
			 * signals */
			err = neardal_adp_prv_get_dev(adpProp,
						      dbusObjPath,
						      &devProp);
			if (err == NEARDAL_ERROR_NO_DEV) {
				clientValue = dbusObjPath;
				neardal_adp_prv_cb_dev_found(NULL,
							     dbusObjPath,
							     adpProp);
				err = NEARDAL_SUCCESS;
			}
		}
		g_strfreev(array);
		array = NULL;
	}

	if (neardalMgr.cb.adp_prop_changed != NULL)
		(neardalMgr.cb.adp_prop_changed)(adpProp->name,
						  (char *) arg_unnamed_arg0,
						  clientValue,
					neardalMgr.cb.adp_prop_changed_ud);
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
	errorCode_t	err	= NEARDAL_SUCCESS;
	GError		*gerror	= NULL;
	GVariant	*tmp	= NULL;
	GVariant	*tmpOut	= NULL;
	gchar		**array	= NULL;
	gsize		len	= 0;

	NEARDAL_TRACEIN();
	NEARDAL_ASSERT_RET(adpProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);
	NEARDAL_ASSERT_RET(adpProp->proxy != NULL
			  , NEARDAL_ERROR_INVALID_PARAMETER);

	org_neard_adp__call_get_properties_sync(adpProp->proxy, &tmp,
						NULL, &gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read adapter's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}

	NEARDAL_TRACEF("Reading:\n%s\n", g_variant_print(tmp, TRUE));
	tmpOut = g_variant_lookup_value(tmp, "Tags", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		array = g_variant_dup_objv(tmpOut, &len);
		adpProp->tagNb = len;
		if (adpProp->tagNb == 0) {
			g_strfreev(array);
			array = NULL;
		} else {
			len = 0;
			char *tagName;

			while (len < adpProp->tagNb &&
				err == NEARDAL_SUCCESS) {
				tagName = array[len++];
				err = neardal_tag_prv_add(tagName,
							  adpProp);
			}
			g_strfreev(array);
			array = NULL;
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "Devices", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		array = g_variant_dup_objv(tmpOut, &len);
		adpProp->devNb = len;
		if (adpProp->devNb == 0) {
			g_strfreev(array);
			array = NULL;
		} else {
			len = 0;
			char *devName;

			while (len < adpProp->devNb &&
				err == NEARDAL_SUCCESS) {
				devName = array[len++];
				err = neardal_dev_prv_add(devName,
							  adpProp);
			}
			g_strfreev(array);
			array = NULL;
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "Polling", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		adpProp->polling = g_variant_get_boolean(tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Powered", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		adpProp->powered = g_variant_get_boolean(tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Mode", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		adpProp->mode = g_variant_dup_string(tmpOut, &len);

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
	g_variant_unref(tmp);
	g_variant_unref(tmpOut);
	return err;
}

/*****************************************************************************
 * neardal_adp_prv_get_tag: Get NFC tag from adapter
 ****************************************************************************/
errorCode_t neardal_adp_prv_get_tag(AdpProp *adpProp, gchar *tagName,
                                    TagProp **tagProp)
{
	errorCode_t	err	= NEARDAL_ERROR_NO_TAG;
	guint		len = 0;
	TagProp		*tag;

	NEARDAL_ASSERT_RET(adpProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);
	NEARDAL_ASSERT_RET(tagProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

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
 * neardal_adp_prv_get_dev: Get NFC device from adapter
 ****************************************************************************/
errorCode_t neardal_adp_prv_get_dev(AdpProp *adpProp, gchar *devName,
				       DevProp **devProp)
{
	errorCode_t	err	= NEARDAL_ERROR_NO_DEV;
	guint		len = 0;
	DevProp		*dev;

	NEARDAL_ASSERT_RET(adpProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);
	NEARDAL_ASSERT_RET(devProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

	while (len < g_list_length(adpProp->devList)) {
		dev = g_list_nth_data(adpProp->devList, len);
		if (dev != NULL) {
			if (!strncmp(dev->name, devName, strlen(dev->name))) {
				*devProp = dev;
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
	NEARDAL_ASSERT_RET(adpProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

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

	err = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp->name == NULL)
		return err;

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

	err = neardal_adp_prv_read_properties(adpProp);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'PropertyChanged'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_PROPCHANGED,
			G_CALLBACK(neardal_adp_prv_cb_property_changed),
			  NULL);

	/* Register 'TagFound', 'TagLost' */
	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TagFound'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_TAG_FOUND,
			G_CALLBACK(neardal_adp_prv_cb_tag_found),
			  adpProp);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TagLost'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_TAG_LOST,
			G_CALLBACK(neardal_adp_prv_cb_tag_lost),
			  adpProp);

	return err;
}

/*****************************************************************************
 * neardal_adp_prv_free: unref DBus proxy, disconnect Neard Adapter signals
 ****************************************************************************/
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
	if ((*adpProp)->mode != NULL)
		g_free((*adpProp)->mode);
	if ((*adpProp)->protocols != NULL)
		g_strfreev((*adpProp)->protocols);
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

	/* Check if adapter already exist in list... */
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
		if (neardalMgr.cb.adp_added != NULL)
				(neardalMgr.cb.adp_added)((char *) adapterName,
						neardalMgr.cb.adp_added_ud);

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

	NEARDAL_ASSERT_RET(adpProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

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
