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
#include <glib-2.0/glib/gerror.h>
#include <glib-2.0/glib/glist.h>


/******************************************************************************
 * neardal_adp_prv_cb_target_found: Callback called when a NFC target is
 * found
 *****************************************************************************/
static void  neardal_adp_prv_cb_target_found(orgNeardTgt *proxy,
					     const gchar *arg_unnamed_arg0,
					     void        *user_data)
{
	AdpProp		*adpProp	= user_data;
	errorCode_t	err;
	neardal_t	neardalMgr;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */
	
	g_assert(arg_unnamed_arg0 != NULL);
	g_assert(adpProp != NULL);
	neardalMgr = adpProp->parent;
	g_assert(neardalMgr != NULL);
	

	NEARDAL_TRACEF("Adding target '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Target Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	err = neardal_tgt_add(neardalMgr, adpProp,
			      (char *) arg_unnamed_arg0);
	if (err != NEARDAL_SUCCESS) {
		if (neardalMgr->cb_tgt_found != NULL)
			(neardalMgr->cb_tgt_found)((char *) arg_unnamed_arg0,
					      neardalMgr->cb_tgt_found_ud);
	}
	NEARDAL_TRACEF("NEARDAL LIB targetList contains %d elements\n",
		      g_list_length(adpProp->tgtList));
}

/******************************************************************************
 * neardal_adp_prv_cb_target_lost: Callback called when a NFC target is
 * lost (removed)
 *****************************************************************************/
static void neardal_adp_prv_cb_target_lost(orgNeardTgt *proxy,
					   const gchar *arg_unnamed_arg0,
					   void *user_data)
{
	AdpProp		*adpProp	= user_data;
	neardal_t	neardalMgr	= adpProp->parent;
	TgtProp		*tgtProp	= NULL;
	errorCode_t	errCode;

	NEARDAL_TRACEIN();
	g_assert(neardalMgr != NULL);
	g_assert(arg_unnamed_arg0 != NULL);
	(void) proxy; /* remove warning */

	neardal_mgr_prv_get_adapter(neardalMgr, (char *) arg_unnamed_arg0,
				    &adpProp);

	NEARDAL_TRACEF("Removing target '%s'\n", arg_unnamed_arg0);
	/* Invoking Callback 'Target Found' before adding it (otherwise
	 * callback 'Record Found' would be called before ) */
	errCode = neardal_mgr_prv_get_target(adpProp, (char *) arg_unnamed_arg0,
						  &tgtProp);
	if (errCode == NEARDAL_SUCCESS) {
		if (neardalMgr->cb_tgt_lost != NULL)
			(neardalMgr->cb_tgt_lost)((char *) arg_unnamed_arg0,
					      neardalMgr->cb_tgt_lost_ud);
		neardal_tgt_remove(tgtProp);
		NEARDAL_TRACEF("NEARDAL LIB targetList contains %d elements\n",
			      g_list_length(adpProp->tgtList));
	}
}

/******************************************************************************
 * neardal_adp_prv_cb_property_changed: Callback called when a NFC target
 * is found
 *****************************************************************************/
static void neardal_adp_prv_cb_property_changed(orgNeardAdp *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void        *user_data)
{
	neardal_t	neardalMgr	= user_data;
	AdpProp		*adpProp	= NULL;
	errorCode_t	errCode		= NEARDAL_ERROR_NO_TARGET;
	char		*tgtName	= NULL;
	void		*clientValue	= NULL;
	TgtProp		*tgtProp	= NULL;
	GVariant	*tmp		= NULL;
	gchar		**tgtArray	= NULL;

	(void) proxy; /* remove warning */
	NEARDAL_TRACEIN();
	g_assert(arg_unnamed_arg0 != NULL);

	neardal_mgr_prv_get_adapter_from_proxy(neardalMgr, proxy, &adpProp);
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
		adpProp->polling = g_variant_get_boolean  (tmp);		
		clientValue = GUINT_TO_POINTER(adpProp->polling);
		NEARDAL_TRACEF("neardalMgr->polling=%d\n", adpProp->polling);
	}

	if (!strcmp(arg_unnamed_arg0, "Targets")) {
		guint tmpLen;

		tgtArray = g_variant_dup_objv (tmp, &tmpLen);
		adpProp->tgtNb = tmpLen;
		if (tmpLen == 0) {
			GList *node = NULL;
			NEARDAL_TRACEF(
				"Target array empty! Removing all targets\n");
			while (g_list_length(adpProp->tgtList)) {
				node = g_list_first(adpProp->tgtList);
				tgtProp = (TgtProp *) node->data;
				neardal_adp_prv_cb_target_lost(tgtProp->proxy,
							       tgtProp->name,
							       tgtProp->parent);
			}
			g_strfreev(tgtArray);
			
			errCode = NEARDAL_SUCCESS;
			goto exit;
		}
		
		/* Extract the targets arrays List from the GValue */
		errCode = NEARDAL_ERROR_NO_ADAPTER;
		tmpLen = 0;
		while (tmpLen < adpProp->tgtNb) {
			/* Getting last target (targets list not updated with
			 * targets lost */
			
 			tgtName = g_strdup(tgtArray[tmpLen++]);
			
			/* TODO : for Neard Workaround, emulate 'TargetFound'
			 * signals */
			errCode = neardal_mgr_prv_get_target(adpProp,
							     tgtName,
							     &tgtProp);
			if (errCode == NEARDAL_ERROR_NO_TARGET) {
				neardal_adp_prv_cb_target_found(NULL,
								tgtName,
								adpProp);
				errCode = NEARDAL_SUCCESS;
			}
		}
		g_strfreev(tgtArray);
	}

	if (neardalMgr->cb_adp_prop_changed != NULL)
		(neardalMgr->cb_adp_prop_changed)(adpProp->name,
						  (char *) arg_unnamed_arg0,
						  clientValue,
					neardalMgr->cb_adp_prop_changed_ud);
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
	gchar		**tgtArray		= NULL;
	gsize		len;
	neardal_t	neardalMgr		= NULL;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);
	g_assert(adpProp->proxy != NULL);
	neardalMgr = adpProp->parent;
	g_assert(neardalMgr != NULL);

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

	NEARDAL_TRACEF("GVariant=%s\n", g_variant_print (tmp, TRUE));
	tmpOut = g_variant_lookup_value(tmp, "Targets", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		tgtArray = g_variant_dup_objv (tmpOut, &len);
		adpProp->tgtNb = len;
		if (len == 0) {
			g_strfreev(tgtArray);
			tgtArray = NULL;
		} else {
			guint len = 0;
			char *tgtName;

			while (len < adpProp->tgtNb &&
				errCode == NEARDAL_SUCCESS) {
				tgtName = tgtArray[len++];
				errCode = neardal_tgt_add(neardalMgr, adpProp,
							tgtName);
			}
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "Polling", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		adpProp->polling = g_variant_get_boolean  (tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Powered", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		adpProp->powered = g_variant_get_boolean  (tmpOut);

	tmpOut = g_variant_lookup_value(tmp, "Protocols",
					G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		adpProp->protocols = g_variant_dup_strv (tmpOut, &len);
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
static errorCode_t neardal_adp_prv_init(neardal_t neardalMgr,
					     AdpProp *adpProp)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(adpProp != NULL);

	if (adpProp->proxy != NULL) {
		g_signal_handlers_disconnect_by_func(adpProp->proxy,
				G_CALLBACK(neardal_adp_prv_cb_property_changed),
						     NULL);
		g_signal_handlers_disconnect_by_func(adpProp->proxy,
				G_CALLBACK(neardal_adp_prv_cb_target_found),
						     NULL);
		g_signal_handlers_disconnect_by_func(adpProp->proxy,
				G_CALLBACK(neardal_adp_prv_cb_target_lost),
						     NULL);
		g_object_unref(adpProp->proxy);
	}
	adpProp->proxy = NULL;

	errCode = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp->name == NULL)
		return errCode;

	adpProp->proxy = org_neard_adp__proxy_new_sync(neardalMgr->conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							adpProp->name,
							NULL, /* GCancellable */
							&neardalMgr->gerror);

	if (neardalMgr->gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Adapter Proxy (%d:%s)\n",
				 neardalMgr->gerror->code,
				neardalMgr->gerror->message);
		neardal_tools_prv_free_gerror(neardalMgr);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	errCode = neardal_adp_prv_read_properties(adpProp);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'PropertyChanged'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_PROPCHANGED,
			G_CALLBACK (neardal_adp_prv_cb_property_changed),
			  neardalMgr);

	/* Register 'TargetFound', 'TargetLost' */
	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TargetFound'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_TGT_FOUND,
			G_CALLBACK (neardal_adp_prv_cb_target_found),
			  adpProp);

	NEARDAL_TRACEF("Register Neard-Adapter Signal ");
	NEARDAL_TRACE("'TargetLost'\n");
	g_signal_connect(adpProp->proxy, NEARD_ADP_SIG_TGT_LOST,
			G_CALLBACK (neardal_adp_prv_cb_target_lost),
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
				G_CALLBACK(neardal_adp_prv_cb_target_found),
						     NULL);
		g_signal_handlers_disconnect_by_func((*adpProp)->proxy,
				G_CALLBACK(neardal_adp_prv_cb_target_lost),
						     NULL);
		g_object_unref((*adpProp)->proxy);
		(*adpProp)->proxy = NULL;
	}
	g_free((*adpProp)->name);
// TODO remove	if ((*adpProp)->tgtArray != NULL)
// 		g_strfreev((*adpProp)->tgtArray);
	if ((*adpProp)->protocols != NULL)
		g_strfreev((*adpProp)->protocols);
	g_free((*adpProp));
	(*adpProp) = NULL;
}

/******************************************************************************
 * neardal_get_adapters: get an array of NFC adapters (adpName) present
 *****************************************************************************/
errorCode_t neardal_get_adapters(neardal_t neardalMgr, char ***array,
				  int *len)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_ADAPTER;
	int		adpNb		= 0;
	int		ct		= 0;	/* counter */
	char		**adps		= NULL;
	AdpProp		*adapter	= NULL;
	gsize		size;

	if (neardalMgr == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	adpNb = g_list_length(neardalMgr->prop.adpList);
	if (adpNb > 0) {
		errCode = NEARDAL_ERROR_NO_MEMORY;
		size = (adpNb + 1) * sizeof(char *);
		adps = g_try_malloc0(size);
		if (adps != NULL) {
			GList	*list;
			while (ct < adpNb) {
				list = neardalMgr->prop.adpList;
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
errorCode_t neardal_adp_add(neardal_t neardalMgr, char *adapterName)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;
	AdpProp		*adpProp = NULL;
	GList		**adpList;

	g_assert(neardalMgr != NULL);
	NEARDAL_TRACEF("Adding adapter:%s\n", adapterName);

	adpProp = g_try_malloc0(sizeof(AdpProp));
	if (adpProp == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	adpProp->name = g_strdup(adapterName);
	adpProp->parent = neardalMgr;
	
	adpList = &neardalMgr->prop.adpList;
	*adpList = g_list_prepend(*adpList, (gpointer) adpProp);
	errCode = neardal_adp_prv_init(neardalMgr, adpProp);

	NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
		      g_list_length(*adpList));

	/* Invoke client cb 'adapter added' */
	if (neardalMgr->cb_adp_added != NULL)
			(neardalMgr->cb_adp_added)((char *) adapterName,
					       neardalMgr->cb_adp_added_ud);

	return errCode;
}

/******************************************************************************
 * neardal_adp_remove: remove one NFC adapter, unref DBus Proxy connection,
 * unregister adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_remove(neardal_t neardalMgr, AdpProp *adpProp)
{
	TgtProp		*tgtProp;
	GList		*node = NULL;
	GList		**adpList;

	g_assert(neardalMgr != NULL);
	g_assert(adpProp != NULL);

	NEARDAL_TRACEF("Removing adapter:%s\n", adpProp->name);

	/* Remove all targets */
	while (g_list_length(adpProp->tgtList)) {
		node = g_list_first(adpProp->tgtList);
		tgtProp = (TgtProp *) node->data;
		neardal_tgt_remove(tgtProp);
	}

	adpList = &neardalMgr->prop.adpList;
	(*adpList) = g_list_remove((*adpList), (gconstpointer) adpProp);
	neardal_adp_prv_free(&adpProp);

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

// TODO	org_neard_Adapter_publish(adpProp->proxy, hash, &gerror);

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
