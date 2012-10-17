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
 * neardal_dev_prv_cb_property_changed: Callback called when a NFC device
 * property is changed
 ****************************************************************************/
static void neardal_dev_prv_cb_property_changed(orgNeardDev *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void		*user_data)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	DevProp		*devProp	= user_data;

	(void) proxy; /* remove warning */
	(void) arg_unnamed_arg1; /* remove warning */

	NEARDAL_TRACEIN();

	if (devProp == NULL || arg_unnamed_arg0 == NULL)
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
 * neardal_dev_prv_read_properties: Get Neard Dev Properties
 ****************************************************************************/
static errorCode_t neardal_dev_prv_read_properties(DevProp *devProp)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	GError		*gerror		= NULL;
	GVariant	*tmp		= NULL;
	GVariant	*tmpOut		= NULL;
	gsize		len;
	gchar		**rcdArray	= NULL;

	NEARDAL_TRACEIN();
	g_assert(devProp != NULL);
	g_assert(devProp->proxy != NULL);

	org_neard_dev__call_get_properties_sync(devProp->proxy, &tmp, NULL,
						&gerror);
	if (gerror != NULL) {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read dev's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}
	NEARDAL_TRACEF("Reading:\n%s\n", g_variant_print(tmp, TRUE));
	tmpOut = g_variant_lookup_value(tmp, "Records", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		rcdArray = g_variant_dup_objv(tmpOut, &len);
		devProp->rcdLen = len;
		if (len == 0) {
			g_strfreev(rcdArray);
			rcdArray = NULL;
		} else {
			guint len = 0;
			char *rcdName;

			while (len < devProp->rcdLen &&
				err == NEARDAL_SUCCESS) {
				rcdName = rcdArray[len++];
				err = neardal_rcd_add(rcdName, devProp);
			}
		}
	}

exit:
	return err;
}

/*****************************************************************************
 * neardal_dev_init: Get Neard Manager Properties = NFC Devs list.
 * Create a DBus proxy for the first one NFC device if present
 * Register Neard Manager signals ('PropertyChanged')
 ****************************************************************************/
static errorCode_t neardal_dev_prv_init(DevProp *devProp)
{
	errorCode_t	err = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(devProp != NULL);

	if (devProp->proxy != NULL) {
		g_signal_handlers_disconnect_by_func(devProp->proxy,
				G_CALLBACK(neardal_dev_prv_cb_property_changed),
						     NULL);
		g_object_unref(devProp->proxy);
		devProp->proxy = NULL;
	}
	devProp->proxy = NULL;

	devProp->proxy = org_neard_dev__proxy_new_sync(neardalMgr.conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							devProp->name,
							NULL, /* GCancellable */
							&neardalMgr.gerror);
	if (neardalMgr.gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Dev Proxy (%d:%s)\n",
				  neardalMgr.gerror->code,
				  neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	/* Populate Dev datas... */
	err = neardal_dev_prv_read_properties(devProp);
	if (err != NEARDAL_SUCCESS)
		return err;

	NEARDAL_TRACEF("Register Neard-Dev Signal 'PropertyChanged'\n");
	g_signal_connect(devProp->proxy, NEARD_DEV_SIG_PROPCHANGED,
			G_CALLBACK(neardal_dev_prv_cb_property_changed),
			  devProp);

	return err;
}

/*****************************************************************************
 * neardal_dev_prv_free: unref DBus proxy, disconnect Neard Dev signals
 ****************************************************************************/
static void neardal_dev_prv_free(DevProp **devProp)
{
	NEARDAL_TRACEIN();
	if ((*devProp)->proxy != NULL) {
		g_signal_handlers_disconnect_by_func((*devProp)->proxy,
				G_CALLBACK(neardal_dev_prv_cb_property_changed),
						     NULL);
		g_object_unref((*devProp)->proxy);
		(*devProp)->proxy = NULL;
	}
	g_free((*devProp)->name);
	g_free((*devProp));
	(*devProp) = NULL;
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
 * neardal_dev_prv_push: Creates and write NDEF record to be pushed to
 * an NFC device
 ****************************************************************************/
errorCode_t neardal_dev_prv_push(DevProp *devProp, RcdProp *rcd)
{
	GVariantBuilder	*builder = NULL;
	GVariant	*in;
	errorCode_t	err;
	GError		*gerror	= NULL;

	g_assert(devProp != NULL);

	builder = g_variant_builder_new(G_VARIANT_TYPE_ARRAY);
	if (builder == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	g_variant_builder_init(builder,  G_VARIANT_TYPE_ARRAY);
	err = neardal_rcd_prv_format(builder, rcd);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	in = g_variant_builder_end(builder);
	NEARDAL_TRACE_LOG("Sending:\n%s\n", g_variant_print(in, TRUE));
	org_neard_dev__call_push_sync(devProp->proxy, in, NULL, &gerror);

exit:
	if (gerror != NULL) {
		NEARDAL_TRACE_ERR("Unable to write dev record (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		err = NEARDAL_ERROR_DBUS;
	}
	if (builder != NULL)
		g_variant_builder_unref(builder);

	return err;
}

/*****************************************************************************
 * neardal_dev_prv_add: add new NFC device, initialize DBus Proxy connection,
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
 * neardal_dev_prv_remove: remove one NFC device, unref DBus Proxy connection,
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
