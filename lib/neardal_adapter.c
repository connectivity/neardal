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
#include <dbus/dbus-glib.h>
#include <glib-object.h>
#include "dbus/dbus.h"

#include "neard-adapter-proxy.h"

#include "neardal.h"
#include "neardal_prv.h"
#include <glib-2.0/glib/gerror.h>
#include <glib-2.0/glib/glist.h>


/******************************************************************************
 * neardal_adp_prv_cb_target_found: Callback called when a NFC target is
 * found
 *****************************************************************************/
static void  neardal_adp_prv_cb_target_found(DBusGProxy  *proxy,
					     const char *str0,
					     void        *user_data)
{
	neardal_t	neardalObj	= user_data;
	AdpProp		*adpProp	= NULL;
	errorCode_t	err;

	NEARDAL_TRACEIN();
	g_assert(str0 != NULL);
	(void) proxy; /* remove warning */

	neardal_mgr_prv_get_adapter(neardalObj, (char *) str0, &adpProp);

	NEARDAL_TRACEF("Adding target '%s'\n", str0);
	/* Invoking Callback 'Target Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_tgt_add(neardalObj, (char *) str0);
	if (err != NEARDAL_SUCCESS) {
		if (neardalObj->cb_tgt_found != NULL)
			(neardalObj->cb_tgt_found)((char *) str0,
					      neardalObj->cb_tgt_found_ud);
	}
	NEARDAL_TRACEF("NEARDAL LIB targetList contains %d elements\n",
		      g_list_length(adpProp->tgtList));
}

/******************************************************************************
 * neardal_adp_prv_cb_target_lost: Callback called when a NFC target is
 * lost (removed)
 *****************************************************************************/
static void neardal_adp_prv_cb_target_lost(DBusGProxy  *proxy,
						const char *str0,
						void *user_data)
{
	neardal_t	neardalObj	= user_data;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	errorCode_t	errCode;

	NEARDAL_TRACEIN();
	g_assert(neardalObj != NULL);
	g_assert(str0 != NULL);
	(void) proxy; /* remove warning */

	neardal_mgr_prv_get_adapter(neardalObj, (char *) str0, &adpProp);

	NEARDAL_TRACEF("Removing target '%s'\n", str0);
	/* Invoking Callback 'Target Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	errCode = neardal_mgr_prv_get_target(adpProp, (char *) str0,
						  &tgtProp);
	if (errCode == NEARDAL_SUCCESS) {
		if (neardalObj->cb_tgt_lost != NULL)
			(neardalObj->cb_tgt_lost)((char *) str0,
					      neardalObj->cb_tgt_lost_ud);
		neardal_tgt_remove(tgtProp);
		adpProp->tgtList = g_list_remove(adpProp->tgtList,
						     (gconstpointer) tgtProp);
		NEARDAL_TRACEF("NEARDAL LIB targetList contains %d elements\n",
			      g_list_length(adpProp->tgtList));
	}
}

/******************************************************************************
 * neardal_adp_prv_cb_property_changed: Callback called when a NFC target
 * is found
 *****************************************************************************/
static void neardal_adp_prv_cb_property_changed(DBusGProxy *proxy,
						const char *str0,
						GValue      *gvalue,
						void        *user_data)
{
	neardal_t		neardalObj		= user_data;
	AdpProp		*adpProp	= user_data;
	GPtrArray	*pathsGpa	= NULL;
	errorCode_t	errCode		= NEARDAL_ERROR_NO_TARGET;
	char		*tgtName	= NULL;
	void		*clientValue	= NULL;
	TgtProp		*tgtProp	= NULL;

	NEARDAL_TRACEIN();
	g_assert(neardalObj != NULL);
	g_assert(str0 != NULL);

	NEARDAL_TRACEF("str0='%s'\n", str0);
	errCode = neardal_mgr_prv_get_adapter_from_proxy(neardalObj,
							      proxy, &adpProp);
	if (errCode != NEARDAL_SUCCESS)
		goto error;

	if (!strcmp(str0, "Polling")) {
		adpProp->polling =  g_value_get_boolean(gvalue);
		clientValue = GUINT_TO_POINTER(adpProp->polling);
		NEARDAL_TRACEF("neardalObj->polling=%d\n", adpProp->polling);
	}

	if (!strcmp(str0, "Targets")) {
		guint tmpLen;
		if (!G_IS_VALUE(gvalue)) {
			NEARDAL_TRACE_ERR(
			"Unexpected value returned getting adapters: %s",
					  G_VALUE_TYPE_NAME(&gvalue));
			errCode = NEARDAL_ERROR_DBUS;
			goto error;
		}

		if (!G_VALUE_HOLDS(gvalue, DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH)) {
			NEARDAL_TRACE_ERR(
				"Unexpected type returned getting adapters: %s",
					  G_VALUE_TYPE_NAME(&gvalue));
			errCode = NEARDAL_ERROR_DBUS;
			goto error;
		}

		/* Extract the targets arrays List from the GValue */
		pathsGpa = g_value_get_boxed(gvalue);
		errCode = NEARDAL_ERROR_NO_ADAPTER;
		if (pathsGpa == NULL)
			goto error;

		if (pathsGpa->len <= 0) {	/* Remove all targets */
			GList *node = NULL;
			NEARDAL_TRACEF(
				"Target array empty! Removing all targets\n");
			while (g_list_length(adpProp->tgtList)) {
				node = g_list_first(adpProp->tgtList);
				tgtProp = (TgtProp *) node->data;
				neardal_adp_prv_cb_target_lost(proxy,
							       tgtProp->name,
								user_data);
			}
			errCode = NEARDAL_SUCCESS;
			goto exit;
		}

		tmpLen = 0;
		while (tmpLen < pathsGpa->len) {
			/* Getting last target (targets list not updated with
			 * targets lost */
			gvalue = g_ptr_array_index(pathsGpa, tmpLen++);

			if (gvalue == NULL)
				goto error;

			tgtName = g_strdup((const gchar *)gvalue);

			/* TODO : for Neard Workaround, emulate 'TargetFound'
			 * signals */
			errCode = neardal_mgr_prv_get_adapter(neardalObj,
								   tgtName,
								   &adpProp);
			if (errCode != NEARDAL_SUCCESS)
				goto error;

			errCode = neardal_mgr_prv_get_target(adpProp,
								  tgtName,
								  &tgtProp);
			if (errCode == NEARDAL_ERROR_NO_TARGET) {
				neardal_adp_prv_cb_target_found(proxy,
								     tgtName,
								     user_data);
				errCode = NEARDAL_SUCCESS;
			}
		}
	}

	if (neardalObj->cb_adp_prop_changed != NULL)
		(neardalObj->cb_adp_prop_changed)(adpProp->name, (char *) str0,
					      clientValue,
					  neardalObj->cb_adp_prop_changed_ud);
	return;

exit:
error:
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
	GHashTable	*neardAdapterPropHash	= NULL;
	GError		*gerror			= NULL;
	void		*tmp			= NULL;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);
	g_assert(adpProp->dbusProxy != NULL);

	org_neard_Adapter_get_properties(adpProp->dbusProxy,
					 &neardAdapterPropHash,
					 &gerror);
	if (gerror != NULL) {
		errCode = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read adapter's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}

	errCode = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
						"Targets",
					DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
					&tmp);
	if (errCode == NEARDAL_SUCCESS) {
		neardal_tools_prv_g_ptr_array_copy(&adpProp->tgtArray,
						tmp);

	}

	errCode = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
						"Polling",
						G_TYPE_BOOLEAN,
						&tmp);
	if (errCode == NEARDAL_SUCCESS)
		adpProp->polling = (gboolean) tmp;

	errCode = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
						"Powered",
						G_TYPE_BOOLEAN,
						&tmp);
	if (errCode == NEARDAL_SUCCESS)
		adpProp->powered = (gboolean) tmp;

	errCode = neardal_tools_prv_hashtable_get(neardAdapterPropHash,
						"Protocols",
						G_TYPE_STRV,
						&tmp);
	if (errCode == NEARDAL_SUCCESS)
		adpProp->protocols = g_boxed_copy(G_TYPE_STRV, tmp);

	g_hash_table_destroy(neardAdapterPropHash);

exit:
	return errCode;
}

/******************************************************************************
 * neardal_adp_prv_get_current_target: Get current NFC target
 *****************************************************************************/
errorCode_t neardal_adp_prv_get_target(AdpProp *adpProp, char *tgtName,
				       TgtProp **tgtProp)
{
	errorCode_t	errCode	= NEARDAL_ERROR_NO_TARGET;
	guint		len = 0;
	TgtProp		*tgt;

	g_assert(adpProp != NULL);
	g_assert(tgtProp != NULL);

	while (len < g_list_length(adpProp->tgtList)) {
		tgt = g_list_nth_data(adpProp->tgtList, len);
		if (tgt != NULL) {
			if (!strncmp(tgt->name, tgtName, strlen(tgt->name))) {
				*tgtProp = tgt;
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
static errorCode_t neardal_adp_prv_init(neardal_t neardalObj,
					     AdpProp *adpProp)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;
	char		*tgtName;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);

	if (adpProp->dbusProxy != NULL) {
		dbus_g_proxy_disconnect_signal(adpProp->dbusProxy,
					       NEARD_ADP_SIG_PROPCHANGED,
			G_CALLBACK(neardal_adp_prv_cb_property_changed),
					       NULL);
		g_object_unref(adpProp->dbusProxy);
	}
	adpProp->dbusProxy = NULL;

	errCode = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp->name == NULL)
		return errCode;

	errCode = neardal_tools_prv_create_proxy(neardalObj->conn,
						  &adpProp->dbusProxy,
						  adpProp->name,
						  NEARD_ADAPTERS_IF_NAME);
	if (errCode != NEARDAL_SUCCESS)
		return errCode;

	errCode = neardal_adp_prv_read_properties(adpProp);
	if (errCode == NEARDAL_SUCCESS && adpProp->tgtArray->len > 0) {
		guint len = 0;

		while (len < adpProp->tgtArray->len &&
			errCode == NEARDAL_SUCCESS) {
			tgtName = g_ptr_array_index(adpProp->tgtArray, len++);
			errCode = neardal_tgt_add(neardalObj, tgtName);
		}
	}

	/* Register Marshaller for signals (String,Variant) */
	dbus_g_object_register_marshaller(neardal_marshal_VOID__STRING_BOXED,
						G_TYPE_NONE, G_TYPE_STRING,
						G_TYPE_VALUE, G_TYPE_INVALID);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'PropertyChanged'\n");
	dbus_g_proxy_add_signal(adpProp->dbusProxy, NEARD_ADP_SIG_PROPCHANGED,
				G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(adpProp->dbusProxy,
				    NEARD_ADP_SIG_PROPCHANGED,
			 G_CALLBACK(neardal_adp_prv_cb_property_changed),
				    neardalObj, NULL);

	/* Register 'TargetFound', 'TargetLost' */
	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TargetFound'\n");
	dbus_g_proxy_add_signal(adpProp->dbusProxy, NEARD_ADP_SIG_TGT_FOUND,
				DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(adpProp->dbusProxy, NEARD_ADP_SIG_TGT_FOUND,
		G_CALLBACK(neardal_adp_prv_cb_target_found), neardalObj,
				    NULL);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TargetLost'\n");
	dbus_g_proxy_add_signal(adpProp->dbusProxy, NEARD_ADP_SIG_TGT_LOST,
				DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(adpProp->dbusProxy, NEARD_ADP_SIG_TGT_LOST,
				G_CALLBACK(neardal_adp_prv_cb_target_lost),
					neardalObj, NULL);

	return errCode;
}

/******************************************************************************
 * neardal_adp_release: unref DBus proxy, disconnect Neard Adapter signals
 *****************************************************************************/
static void neardal_adp_prv_release(AdpProp *adpProp)
{
	NEARDAL_TRACEIN();
	if (adpProp->dbusProxy != NULL) {
		dbus_g_proxy_disconnect_signal(adpProp->dbusProxy,
					       NEARD_ADP_SIG_PROPCHANGED,
			G_CALLBACK(neardal_adp_prv_cb_property_changed),
					       NULL);
		dbus_g_proxy_disconnect_signal(adpProp->dbusProxy,
					       NEARD_ADP_SIG_TGT_FOUND,
			G_CALLBACK(neardal_adp_prv_cb_target_found),
					       NULL);
		dbus_g_proxy_disconnect_signal(adpProp->dbusProxy,
					       NEARD_ADP_SIG_TGT_LOST,
			G_CALLBACK(neardal_adp_prv_cb_target_lost),
					       NULL);
		g_object_unref(adpProp->dbusProxy);
		adpProp->dbusProxy = NULL;
	}
	g_free(adpProp->name);
	if (adpProp->tgtArray != NULL)
		neardal_tools_prv_g_ptr_array_free(adpProp->tgtArray);
	g_boxed_free(G_TYPE_STRV, adpProp->protocols);
	g_free(adpProp);
}

/******************************************************************************
 * neardal_get_adapters: get an array of NFC adapters (adpName) present
 *****************************************************************************/
errorCode_t neardal_get_adapters(neardal_t neardalObj, char ***array,
				  int *len)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_ADAPTER;
	int		adpNb		= 0;
	int		ct		= 0;	/* counter */
	char		**adps		= NULL;
	AdpProp		*adapter	= NULL;
	gsize		size;

	if (neardalObj == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	adpNb = g_list_length(neardalObj->mgrProp.adpList);
	if (adpNb > 0) {
		errCode = NEARDAL_ERROR_NO_MEMORY;
		size = (adpNb + 1) * sizeof(char *);
		adps = g_try_malloc0(size);
		if (adps != NULL) {
			GList	*list;
			while (ct < adpNb) {
				list = neardalObj->mgrProp.adpList;
				adapter = g_list_nth_data(list, ct);
				if (adapter != NULL)
					adps[ct++] = g_strdup(adapter->name);
			}
			errCode = NEARDAL_SUCCESS;
		}
	}
	if (len != NULL)
		*len = adpNb;
	*array	= adps;

	return errCode;
}

/******************************************************************************
 * neardal_adp_add: add new NFC adapter, initialize DBus Proxy connection,
 * register adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_add(neardal_t neardalObj, char *adapterName)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;
	AdpProp		*adpProp = NULL;

	g_assert(neardalObj != NULL);
	NEARDAL_TRACEF("Adding adapter:%s\n", adapterName);

	adpProp = g_try_malloc0(sizeof(AdpProp));
	if (adpProp == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	adpProp->name = g_strdup(adapterName);
	neardalObj->mgrProp.adpList = g_list_prepend(neardalObj->mgrProp.adpList,
						     (gpointer) adpProp);
	NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
		      g_list_length(neardalObj->mgrProp.adpList));
	errCode = neardal_adp_prv_init(neardalObj, adpProp);

	return errCode;
}

/******************************************************************************
 * neardal_adp_remove: remove NFC adapter, unref DBus Proxy connection,
 * unregister adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_remove(neardal_t neardalObj, AdpProp *adpProp)
{
	TgtProp		*tgtProp;
	GList		*node = NULL;

	g_assert(neardalObj != NULL);
	g_assert(adpProp != NULL);

	NEARDAL_TRACEF("Removing adapter:%s\n", adpProp->name);

	/* Remove all targets */
	while (g_list_length(adpProp->tgtList)) {
		node = g_list_first(adpProp->tgtList);
		tgtProp = (TgtProp *) node->data;
		neardal_tgt_remove(tgtProp);
		adpProp->tgtList = g_list_remove(adpProp->tgtList,
						 (gconstpointer) tgtProp);
	}

	neardal_adp_prv_release(adpProp);
	neardalObj->mgrProp.adpList = g_list_remove(neardalObj->mgrProp.adpList,
						    (gconstpointer) adpProp);

	return NEARDAL_SUCCESS;
}

/******************************************************************************
 * neardal_adp_publish: Creates and publish NDEF record to be written to
 * an NFC tag
 *****************************************************************************/
errorCode_t neardal_adp_publish(AdpProp *adpProp, RcdProp *rcd)
{
	GHashTable	*hash = NULL;
	errorCode_t	err;
	GError		*gerror = NULL;
	
	g_assert(adpProp != NULL);

	hash = neardal_tools_create_dict();
	if (hash == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	err = neardal_rcd_prv_format(&hash, rcd);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	org_neard_Adapter_publish(adpProp->dbusProxy, hash, &gerror);

exit:
	if (hash != NULL)
		g_hash_table_destroy(hash);
	if (gerror != NULL) {
		NEARDAL_TRACE_ERR("Unable to publish tag record (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		err = NEARDAL_ERROR_DBUS;
	}
	
	return err;
}
