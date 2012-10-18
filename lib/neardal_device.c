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

#include "neard_device_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"


/*****************************************************************************
 * neardal_dev_prv_cb_property_changed: Callback called when a NFC dev
 * property is changed
 ****************************************************************************/
static void neardal_dev_prv_cb_property_changed(DBusGProxy *proxy,
					     const char	*arg_unnamed_arg0,
					     GValue	*arg_unnamed_arg1,
					     void	*user_data)
{
	DevProp		*devProp	= user_data;

	(void) proxy; /* remove warning */
	(void) arg_unnamed_arg1; /* remove warning */

	NEARDAL_TRACEIN();

	if (devProp == NULL || arg_unnamed_arg0 == NULL)
		return;

	NEARDAL_TRACEF("arg_unnamed_arg0='%s'\n", arg_unnamed_arg0);

	return;
}

/*****************************************************************************
 * neardal_dev_prv_read_properties: Get Neard Dev Properties
 ****************************************************************************/
static errorCode_t neardal_dev_prv_read_properties(DevProp *devProp)
{
	errorCode_t	err			= NEARDAL_SUCCESS;
	GHashTable	*neardDevPropHash	= NULL;
	GError		*gerror		= NULL;
	gsize		len;
	GPtrArray	*rcdArray		= NULL;

	NEARDAL_TRACEIN();
	g_assert(devProp != NULL);
	g_assert(devProp->proxy != NULL);

	org_neard_Device_get_properties(devProp->proxy,
					&neardDevPropHash,
						&gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read dev's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}
	err = neardal_tools_prv_hashtable_get(neardDevPropHash,
						   "Records",
					    DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
					    &rcdArray);
	if (err == NEARDAL_SUCCESS) {
			char *rcdName;
//TODO 		neardal_tools_prv_g_ptr_array_copy(&devProp->rcdArray,
// 						   rcdArray);

		devProp->rcdLen = rcdArray->len;
		while ((len < devProp->rcdLen) && (err == NEARDAL_SUCCESS)) {
			rcdName = g_ptr_array_index(rcdArray, len++);
			err = neardal_rcd_add(rcdName, devProp);
		}
	}

	g_hash_table_destroy(neardDevPropHash);

exit:
	return err;
}

/*****************************************************************************
 * neardal_dev_init: Get Neard Manager Properties = NFC Devs list.
 * Create a DBus proxy for the first one NFC dev if present
 * Register Neard Manager signals ('PropertyChanged')
 ****************************************************************************/
static errorCode_t neardal_dev_prv_init(DevProp *devProp)
{
	errorCode_t	err = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(devProp != NULL);

	if (devProp->proxy != NULL) {
		dbus_g_proxy_disconnect_signal(devProp->proxy,
					       NEARD_DEV_SIG_PROPCHANGED,
				G_CALLBACK(neardal_dev_prv_cb_property_changed),
						     NULL);
		g_object_unref(devProp->proxy);
		devProp->proxy = NULL;
	}
	devProp->proxy = NULL;

	err = neardal_tools_prv_create_proxy(neardalMgr.conn,
						  &devProp->proxy,
							devProp->name,
						  NEARD_DEVS_IF_NAME);
	if (err != NEARDAL_SUCCESS)
		return err;

	/* Populate Dev datas... */
	err = neardal_dev_prv_read_properties(devProp);
	if (err != NEARDAL_SUCCESS)
		return err;

	/* Register Marshaller for signals (String,Variant) */
	dbus_g_object_register_marshaller(neardal_marshal_VOID__STRING_BOXED,
					   G_TYPE_NONE, G_TYPE_STRING,
					   G_TYPE_VALUE, G_TYPE_INVALID);

	NEARDAL_TRACEF("Register Neard-Dev Signal 'PropertyChanged'\n");
	dbus_g_proxy_add_signal(devProp->proxy, NEARD_DEV_SIG_PROPCHANGED,
				 G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(devProp->proxy,
				    NEARD_DEV_SIG_PROPCHANGED,
			G_CALLBACK(neardal_dev_prv_cb_property_changed),
				    devProp, NULL);

	return err;
}

/*****************************************************************************
 * neardal_dev_prv_free: unref DBus proxy, disconnect Neard Dev signals
 ****************************************************************************/
static void neardal_dev_prv_free(DevProp **devProp)
{
	NEARDAL_TRACEIN();
	if ((*devProp)->proxy != NULL) {
		dbus_g_proxy_disconnect_signal((*devProp)->proxy,
					       NEARD_DEV_SIG_PROPCHANGED,
				G_CALLBACK(neardal_dev_prv_cb_property_changed),
						     NULL);
		g_object_unref((*devProp)->proxy);
		(*devProp)->proxy = NULL;
	}
	g_free((*devProp)->name);
}

/*****************************************************************************
 * neardal_dev_notify_dev_found: Invoke client callback for 'record found'
 * if present, and 'dev found' (if not already nofied)
 ****************************************************************************/
void neardal_dev_notify_dev_found(DevProp *devProp)
{
	RcdProp *rcdProp;
	gsize	len;

	g_assert(devProp != NULL);

	if (devProp->notified == FALSE && neardalMgr.cb_dev_found != NULL) {
		(neardalMgr.cb_dev_found)(devProp->name,
					   neardalMgr.cb_dev_found_ud);
		devProp->notified = TRUE;
	}

	len = 0;
	if (neardalMgr.cb_rcd_found != NULL)
		while (len < g_list_length(devProp->rcdList)) {
			rcdProp = g_list_nth_data(devProp->rcdList, len++);
			if (rcdProp->notified == FALSE) {
				(neardalMgr.cb_rcd_found)(rcdProp->name,
						neardalMgr.cb_rcd_found_ud);
				rcdProp->notified = TRUE;
			}
		}
}
/*****************************************************************************
 * neardal_dev_prv_write: Creates and write NDEF record to be written to
 * an NFC dev
 ****************************************************************************/
errorCode_t neardal_dev_prv_push(DevProp *devProp, RcdProp *rcd)
{
	errorCode_t	err;
	GError		*gerror	= NULL;
	GHashTable	*hash;

	g_assert(devProp != NULL);

	hash = neardal_tools_prv_create_dict();
	if (hash != NULL)
		err = neardal_rcd_prv_format(&hash, rcd);
	else
		err = NEARDAL_ERROR_NO_MEMORY;

	if (err != NEARDAL_SUCCESS)
		goto exit;
	org_neard_Device_push(devProp->proxy, hash, &gerror);

exit:
	if (gerror != NULL) {
		NEARDAL_TRACE_ERR("Unable to write dev record (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		err = NEARDAL_ERROR_DBUS;
	}

	return err;
}

/*****************************************************************************
 * neardal_dev_prv_add: add new NFC dev, initialize DBus Proxy connection,
 * register dev signal
 ****************************************************************************/
errorCode_t neardal_dev_prv_add(gchar *devName, void *parent)
{
	errorCode_t	err		= NEARDAL_ERROR_NO_MEMORY;
	DevProp		*devProp	= NULL;
	AdpProp		*adpProp	= parent;

	g_assert(devName != NULL);

	NEARDAL_TRACEF("Adding dev:%s\n", devName);
	devProp = g_try_malloc0(sizeof(DevProp));
	if (devProp == NULL)
		goto error;

	devProp->name	= g_strdup(devName);
	devProp->parent	= adpProp;

	adpProp->devList = g_list_prepend(adpProp->devList, devProp);
	err = neardal_dev_prv_init(devProp);

	NEARDAL_TRACEF("NEARDAL LIB devList contains %d elements\n",
		      g_list_length(adpProp->devList));

	return err;

error:
	if (devProp->name != NULL)
		g_free(devProp->name);
	if (devProp != NULL)
		g_free(devProp);

	return err;
}

/*****************************************************************************
 * neardal_dev_prv_remove: remove one NFC dev, unref DBus Proxy connection,
 * unregister dev signal
 ****************************************************************************/
void neardal_dev_prv_remove(DevProp *devProp)
{
	RcdProp		*rcdProp	= NULL;
	GList		*node;
	AdpProp		*adpProp;

	g_assert(devProp != NULL);

	NEARDAL_TRACEF("Removing dev:%s\n", devProp->name);
	/* Remove all devs */
	while (g_list_length(devProp->rcdList)) {
		node = g_list_first(devProp->rcdList);
		rcdProp = (RcdProp *) node->data;
		neardal_rcd_remove(rcdProp);
	}
	adpProp = devProp->parent;
	adpProp->devList = g_list_remove(adpProp->devList,
					 (gconstpointer) devProp);

	neardal_dev_prv_free(&devProp);
}
