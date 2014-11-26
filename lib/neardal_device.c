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
#include <glib.h>

#include "neardal.h"
#include "neardal_prv.h"

static void neardal_dev_prv_free(DevProp **devProp)
{
	NEARDAL_TRACEIN();
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

	NEARDAL_ASSERT(devProp != NULL);

	if (devProp->notified == FALSE && neardalMgr.cb.dev_found != NULL) {
		(neardalMgr.cb.dev_found)(devProp->name,
					   neardalMgr.cb.dev_found_ud);
		devProp->notified = TRUE;
	}

	len = 0;
	if (neardalMgr.cb.rcd_found != NULL)
		while (len < g_list_length(devProp->rcdList)) {
			rcdProp = g_list_nth_data(devProp->rcdList, len++);
			if (rcdProp->notified == FALSE) {
				(neardalMgr.cb.rcd_found)(rcdProp->name,
						neardalMgr.cb.rcd_found_ud);
				rcdProp->notified = TRUE;
			}
		}
}

errorCode_t neardal_dev_push(neardal_record *record)
{
	GError		*gerror	= NULL;
	errorCode_t	err;
	GVariant	*in;

	neardal_prv_construct(&err);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	in = neardal_record_to_g_variant(record);

	g_dbus_connection_call_sync(neardalMgr.conn,
					"org.neard",
                                        record->name,
                                        "org.neard.Device",
                                        "Push",
					g_variant_new("(@a{sv})", in),
					NULL,
                                        G_DBUS_CALL_FLAGS_NONE,
                                        3000, /* 3 secs */
                                        NULL,
                                        &gerror);
	if (gerror) {
		NEARDAL_TRACE_ERR("Can't push record: %s\n", gerror->message);
		g_error_free(gerror);
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
	}
exit:
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

	NEARDAL_ASSERT_RET( ((adpProp != NULL) && (devName != NULL))
			  , NEARDAL_ERROR_INVALID_PARAMETER);

	NEARDAL_TRACEF("Adding dev:%s\n", devName);
	devProp = g_try_malloc0(sizeof(DevProp));
	if (devProp == NULL)
		goto error;

	devProp->name	= g_strdup(devName);
	devProp->parent	= adpProp;

	adpProp->devList = g_list_prepend(adpProp->devList, devProp);

	NEARDAL_TRACEF("NEARDAL LIB devList contains %d elements\n",
		      g_list_length(adpProp->devList));

	return NEARDAL_SUCCESS;

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

	NEARDAL_ASSERT(devProp != NULL);

	NEARDAL_TRACEF("Removing dev:%s\n", devProp->name);

	adpProp = devProp->parent;
	adpProp->devList = g_list_remove(adpProp->devList,
					 (gconstpointer) devProp);

	neardal_dev_prv_free(&devProp);
}
