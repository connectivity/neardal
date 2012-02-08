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

#include "neard-manager-proxy.h"
#include "neard-adapter-proxy.h"

#include "neardal.h"
#include "neardal_prv.h"
#include <glib-2.0/glib/glist.h>
#include <glib-2.0/glib/garray.h>

/******************************************************************************
 * neardal_mgr_prv_cb_property_changed: Callback called when a NFC Manager
 * Property is changed
 *****************************************************************************/
static void neardal_mgr_prv_cb_property_changed(DBusGProxy  *proxy,
						     const char  *str0,
						     GValue      *gvalue,
						     void        *user_data)
{
	neardal_t	neardalObj	= user_data;

	NEARDAL_TRACEIN();

	g_assert(neardalObj != NULL);
	g_assert(str0 != NULL);
	(void) proxy; /* remove warning */
	(void) gvalue; /* remove warning */

	NEARDAL_TRACEF("str0='%s'\n", str0);
	/* Adapters List ignored... */
}

/******************************************************************************
 * neardal_mgr_prv_cb_adapter_added: Callback called when a NFC adapter is
 * added
 *****************************************************************************/
static void neardal_mgr_prv_cb_adapter_added(DBusGProxy  *proxy,
						  const char  *str0,
						  void        *user_data)
{
	neardal_t	neardalObj	= user_data;
	errorCode_t	errCode = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(neardalObj != NULL);
	g_assert(str0 != NULL);
	(void) proxy; /* remove warning */

	errCode = neardal_adp_add(neardalObj, (char *) str0);
	if (errCode != NEARDAL_SUCCESS)
		return;

	/* Invoke client cb 'adapter added' */
	if (neardalObj->cb_adp_added != NULL)
			(neardalObj->cb_adp_added)((char *) str0,
					       neardalObj->cb_adp_added_ud);
	NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
		      g_list_length(neardalObj->mgrProp.adpList));
}

/******************************************************************************
 * neardal_mgr_prv_cb_adapter_removed: Callback called when a NFC adapter
 * is removed
 *****************************************************************************/
static void neardal_mgr_prv_cb_adapter_removed(DBusGProxy  *proxy,
						    const char  *str0,
						    void *user_data)
{
	neardal_t	neardalObj	= user_data;
	GList	*node	= NULL;

	NEARDAL_TRACEIN();
	g_assert(neardalObj != NULL);
	g_assert(str0 != NULL);
	(void) proxy; /* remove warning */

	node = g_list_first(neardalObj->mgrProp.adpList);
	if (node == NULL) {
		NEARDAL_TRACE_ERR("NFC adapter not found! (%s)\n", str0);
		return;
	}

	neardal_adp_remove(neardalObj, ((AdpProp *)node->data));
	/* Invoke client cb 'adapter removed' */
	if (neardalObj->cb_adp_removed != NULL)
		(neardalObj->cb_adp_removed)((char *) str0,
					 neardalObj->cb_adp_removed_ud);

	NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
		      g_list_length(neardalObj->mgrProp.adpList));
}

/******************************************************************************
 * neardal_mgr_prv_get_all_adapters: Check if neard has an adapter
 *****************************************************************************/
static errorCode_t neardal_mgr_prv_get_all_adapters(neardal_t neardalObj,
							 GPtrArray **adpArray)
{
	GHashTable	*neardAdapterHash	= NULL;
	GPtrArray	*pathsGpa		= NULL;
	errorCode_t	errCode			= NEARDAL_ERROR_NO_ADAPTER;

	g_assert(neardalObj != NULL);
	g_assert(adpArray != NULL);

	/* Invoking method 'GetProperties' on Neard Manager */
	if (org_neard_Manager_get_properties(neardalObj->mgrProxy,
					     &neardAdapterHash,
					     &neardalObj->gerror)) {
		/* Receiving a GPtrArray of GList */
		NEARDAL_TRACEF("Parsing neard adapters...\n");
		errCode = neardal_tools_prv_hashtable_get(neardAdapterHash,
						   NEARD_MGR_SECTION_ADAPTERS,
					DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
					&pathsGpa);
		if (errCode != NEARDAL_SUCCESS || pathsGpa->len <= 0)
			errCode = NEARDAL_ERROR_NO_ADAPTER;
		else
			neardal_tools_prv_g_ptr_array_copy(adpArray, pathsGpa);

		g_hash_table_destroy(neardAdapterHash);
	} else {
		errCode = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR("%d:%s\n", neardalObj->gerror->code,
				 neardalObj->gerror->message);
		neardal_tools_prv_free_gerror(neardalObj);
	}

	return errCode;
}


/******************************************************************************
 * neardal_mgr_prv_get_adapter: Get NFC Adapter from name
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_adapter(neardal_t neardalObj,
					     const char *adpName,
					     AdpProp **adpProp)
{
	errorCode_t	errCode	= NEARDAL_ERROR_NO_ADAPTER;
	guint		len	= 0;
	AdpProp		*adapter;
	GList		*tmpList;

	g_assert(neardalObj != NULL);
	g_assert(adpProp != NULL);

	tmpList = neardalObj->mgrProp.adpList;
	while (len < g_list_length(tmpList)) {
		adapter = g_list_nth_data(tmpList, len);
		if (adapter != NULL) {
			if (neardal_tools_prv_cmp_path(adapter->name,
							adpName)) {
				*adpProp = adapter;
				errCode = NEARDAL_SUCCESS;
				break;
			}
		}
		len++;
	}

	return errCode;
}

/******************************************************************************
 * neardal_mgr_prv_get_adapter_from_proxy: Get NFC Adapter from proxy
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_adapter_from_proxy(neardal_t neardalObj,
							DBusGProxy *adpProxy,
							AdpProp **adpProp)
{
	errorCode_t	errCode	= NEARDAL_ERROR_NO_ADAPTER;
	guint		len = 0;
	AdpProp		*adapter;
	GList		*tmpList;

	g_assert(neardalObj != NULL);
	g_assert(adpProp != NULL);

	tmpList = neardalObj->mgrProp.adpList;
	while (len < g_list_length(tmpList)) {
		adapter = g_list_nth_data(tmpList, len);
		if (adapter != NULL) {
			if (adapter->dbusProxy == adpProxy) {
				*adpProp = adapter;
				errCode = NEARDAL_SUCCESS;
				break;
			}
		}
		len++;
	}

	return errCode;
}

/******************************************************************************
 * neardal_mgr_prv_get_target: Get specific target from adapter
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_target(AdpProp *adpProp,
					    const char *tgtName,
					    TgtProp **tgtProp)
{
	errorCode_t	errCode	= NEARDAL_ERROR_NO_TARGET;
	guint		len;
	TgtProp		*tgt	= NULL;
	GList		*tmpList;

	g_assert(adpProp != NULL);
	g_assert(tgtName != NULL);
	g_assert(tgtProp != NULL);

	len = 0;
	tmpList = adpProp->tgtList;
	while (len < g_list_length(tmpList)) {
		tgt = g_list_nth_data(tmpList, len);
		if (neardal_tools_prv_cmp_path(tgt->name, tgtName)) {
			*tgtProp = tgt;
			errCode = NEARDAL_SUCCESS;
			break;
		}
		len++;
	}

	return errCode;
}

/******************************************************************************
 * neardal_mgr_prv_get_record: Get specific record from target
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_record(TgtProp *tgtProp,
					    const char *rcdName,
					    RcdProp **rcdProp)
{
	errorCode_t	errCode	= NEARDAL_ERROR_NO_RECORD;
	guint		len;
	RcdProp	*rcd	= NULL;

	g_assert(tgtProp != NULL);
	g_assert(rcdName != NULL);
	g_assert(rcdProp != NULL);

	len = 0;
	while (len < g_list_length(tgtProp->rcdList)) {
		rcd = g_list_nth_data(tgtProp->rcdList, len);
		if (neardal_tools_prv_cmp_path(rcd->name, rcdName)) {
			*rcdProp = rcd;
			errCode = NEARDAL_SUCCESS;
			break;
		}
		len++;
	}

	return errCode;
}


/******************************************************************************
 * neardal_mgr_init: Get Neard Manager Properties = NFC Adapters list.
 * Create a DBus proxy for the first one NFC adapter if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
errorCode_t neardal_mgr_init(neardal_t neardalObj)
{
	errorCode_t	errCode;
	GPtrArray	*adpArray = NULL;
	char		*adpName;
	guint		len;
	DBusGProxy	*proxyTmp;

	NEARDAL_TRACEIN();
	errCode = neardal_tools_prv_create_proxy(neardalObj->conn,
						  &neardalObj->mgrProxy,
						  "/", NEARD_MGR_IF_NAME);

	if (errCode != NEARDAL_SUCCESS)
		return errCode;

	/* Check if a NFC adapter is present */
	errCode = neardal_mgr_prv_get_all_adapters(neardalObj, &adpArray);
	if (adpArray != NULL && adpArray->len > 0) {
		len = 0;
		while (len < adpArray->len && errCode == NEARDAL_SUCCESS) {
			adpName =  g_ptr_array_index(adpArray, len++);
			errCode = neardal_adp_add(neardalObj, adpName);
		}
		neardal_tools_prv_g_ptr_array_free(adpArray);
	}

	/* Register Marshaller for signals (String,Variant) */
	dbus_g_object_register_marshaller(neardal_marshal_VOID__STRING_BOXED,
					  G_TYPE_NONE, G_TYPE_STRING,
					  G_TYPE_VALUE, G_TYPE_INVALID);


	/* Register for manager signals 'PropertyChanged(String,Variant)' */
	proxyTmp = neardalObj->mgrProxy;
	NEARDAL_TRACEF("Register Neard-Manager Signal 'PropertyChanged'\n");
	dbus_g_proxy_add_signal(proxyTmp, NEARD_MGR_SIG_PROPCHANGED,
				G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(proxyTmp, NEARD_MGR_SIG_PROPCHANGED,
			  G_CALLBACK(neardal_mgr_prv_cb_property_changed),
				   neardalObj, NULL);


	/* Register for manager signals 'AdapterAdded(ObjectPath)' */
	NEARDAL_TRACEF("Register Neard-Manager Signal 'AdapterAdded'\n");
	dbus_g_proxy_add_signal(proxyTmp, NEARD_MGR_SIG_ADP_ADDED,
				DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(proxyTmp, NEARD_MGR_SIG_ADP_ADDED,
			G_CALLBACK(neardal_mgr_prv_cb_adapter_added),
				    neardalObj, NULL);

	/* Register for manager signals 'AdapterRemoved(ObjectPath)' */
	NEARDAL_TRACEF("Register Neard-Manager Signal 'AdapterRemoved'\n");
	dbus_g_proxy_add_signal(proxyTmp, NEARD_MGR_SIG_ADP_RM,
				DBUS_TYPE_G_OBJECT_PATH, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(proxyTmp, NEARD_MGR_SIG_ADP_RM,
			G_CALLBACK(neardal_mgr_prv_cb_adapter_removed),
				    neardalObj, NULL);

	return errCode;
}

/******************************************************************************
 * neardal_mgr_release: unref DBus proxy, disconnect Neard Manager signals
 *****************************************************************************/
void neardal_mgr_release(neardal_t neardalObj)
{
	GList	*node;
	GList	*tmpList;

	NEARDAL_TRACEIN();
	/* Remove all adapters */
	tmpList = neardalObj->mgrProp.adpList;
	while (g_list_length(tmpList)) {
		node = g_list_first(tmpList);
		neardal_adp_remove(neardalObj, ((AdpProp *)node->data));
		tmpList = g_list_remove(tmpList, node->data);
	}
	neardalObj->mgrProp.adpList = tmpList;

	if (neardalObj->mgrProxy == NULL)
		return;

	dbus_g_proxy_disconnect_signal(neardalObj->mgrProxy,
				       NEARD_MGR_SIG_PROPCHANGED,
			G_CALLBACK(neardal_mgr_prv_cb_property_changed),
				       NULL);
	dbus_g_proxy_disconnect_signal(neardalObj->mgrProxy,
				       NEARD_MGR_SIG_ADP_ADDED,
			G_CALLBACK(neardal_mgr_prv_cb_adapter_added),
				       NULL);
	dbus_g_proxy_disconnect_signal(neardalObj->mgrProxy,
				       NEARD_MGR_SIG_ADP_RM,
			G_CALLBACK(neardal_mgr_prv_cb_adapter_removed),
				       NULL);
	g_object_unref(neardalObj->mgrProxy);
	neardalObj->mgrProxy = NULL;
}
