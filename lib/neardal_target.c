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

#include "neard_target_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"
#include <glib-2.0/glib/gerror.h>
#include <glib-2.0/glib/glist.h>
#include <glib-2.0/glib/garray.h>


/******************************************************************************
 * neardal_tgt_prv_cb_property_changed: Callback called when a NFC target
 * property is changed
 *****************************************************************************/
static void neardal_tgt_prv_cb_property_changed(orgNeardTgt *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void		*user_data)
{
	errorCode_t	errCode		= NEARDAL_SUCCESS;
	TgtProp		*tgtProp	= user_data;

	(void) proxy; /* remove warning */
	(void) arg_unnamed_arg1; /* remove warning */

	NEARDAL_TRACEIN();

	if (tgtProp == NULL || arg_unnamed_arg0 == NULL)
		return;

	NEARDAL_TRACEF("str0='%s'\n", arg_unnamed_arg0);
	NEARDAL_TRACEF("arg_unnamed_arg1=%s (%s)\n",
		       g_variant_print (arg_unnamed_arg1, TRUE),
		       g_variant_get_type_string(arg_unnamed_arg1));

	NEARDAL_TRACE_ERR("Exit with error code %d:%s\n", errCode,
			  neardal_error_get_text(errCode));

	return;
}

/******************************************************************************
 * neardal_tgt_prv_read_properties: Get Neard Target Properties
 *****************************************************************************/
static errorCode_t neardal_tgt_prv_read_properties(TgtProp *tgtProp)
{
	errorCode_t	errCode		= NEARDAL_SUCCESS;
	GError		*gerror		= NULL;
	GVariant	*tmp		= NULL;
	GVariant	*tmpOut		= NULL;
	gsize		len;
	gchar		**rcdArray	= NULL;

	NEARDAL_TRACEIN();
	g_assert(tgtProp != NULL);
	g_assert(tgtProp->proxy != NULL);

	org_neard_tgt__call_get_properties_sync(tgtProp->proxy, &tmp, NULL,
						&gerror);
	if (gerror != NULL) {
		errCode = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read target's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}
	NEARDAL_TRACEF("GVariant=%s\n", g_variant_print (tmp, TRUE));

	tmpOut = g_variant_lookup_value(tmp, "Records", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		rcdArray = g_variant_dup_objv (tmpOut, &len);
		tgtProp->rcdLen = len;
		if (len == 0) {
			g_strfreev(rcdArray);
			rcdArray = NULL;
		} else {
			guint len = 0;
			char *rcdName;
			AdpProp	*adpProp;
			neardal_t neardalMgr;

			adpProp = tgtProp->parent;
			neardalMgr = adpProp->parent;
			while (len < tgtProp->rcdLen && 
				errCode == NEARDAL_SUCCESS) {
				rcdName = rcdArray[len++];
				errCode = neardal_rcd_add(neardalMgr, tgtProp,
							rcdName);
			}
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "TagType", G_VARIANT_TYPE_ARRAY);
	if (tmpOut != NULL) {
		tgtProp->tagsType = g_variant_dup_strv (tmpOut, &len);
		tgtProp->tagsTypeLen = len;
		if (len == 0) {
			g_strfreev(tgtProp->tagsType);
			tgtProp->tagsType = NULL;
		}
	}

	tmpOut = g_variant_lookup_value(tmp, "Type", G_VARIANT_TYPE_STRING);
	if (tmpOut != NULL)
		tgtProp->type = g_variant_dup_string(tmpOut, NULL);

	tmpOut = g_variant_lookup_value(tmp, "ReadOnly", G_VARIANT_TYPE_BOOLEAN);
	if (tmpOut != NULL)
		tgtProp->readOnly = g_variant_get_boolean  (tmpOut);

exit:
	return errCode;
}

/******************************************************************************
 * neardal_tgt_init: Get Neard Manager Properties = NFC Targets list.
 * Create a DBus proxy for the first one NFC target if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
static errorCode_t neardal_tgt_prv_init(neardal_t neardalMgr,
					    TgtProp *tgtProp)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	g_assert(tgtProp != NULL);

	if (tgtProp->proxy != NULL) {
		g_signal_handlers_disconnect_by_func(tgtProp->proxy,
				G_CALLBACK(neardal_tgt_prv_cb_property_changed),
						     NULL);
		g_object_unref(tgtProp->proxy);
		tgtProp->proxy = NULL;
	}
	tgtProp->proxy = NULL;

	tgtProp->proxy = org_neard_tgt__proxy_new_sync(neardalMgr->conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							tgtProp->name,
							NULL, /* GCancellable */
							&neardalMgr->gerror);
	if (neardalMgr->gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Target Proxy (%d:%s)\n",
				  neardalMgr->gerror->code,
				  neardalMgr->gerror->message);
		neardal_tools_prv_free_gerror(neardalMgr);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	/* Populate Target datas... */
	errCode = neardal_tgt_prv_read_properties(tgtProp);
	if (errCode != NEARDAL_SUCCESS)
		return errCode;

	NEARDAL_TRACEF("Register Neard-Target Signal 'PropertyChanged'\n");
	g_signal_connect(tgtProp->proxy, NEARD_TGT_SIG_PROPCHANGED,
			G_CALLBACK (neardal_tgt_prv_cb_property_changed),
			  tgtProp);

	return errCode;
}

/******************************************************************************
 * neardal_tgt_prv_free: unref DBus proxy, disconnect Neard Target signals
 *****************************************************************************/
static void neardal_tgt_prv_free(TgtProp **tgtProp)
{
	NEARDAL_TRACEIN();
	if ((*tgtProp)->proxy != NULL) {
		g_signal_handlers_disconnect_by_func((*tgtProp)->proxy,
				G_CALLBACK(neardal_tgt_prv_cb_property_changed),
						     NULL);
		g_object_unref((*tgtProp)->proxy);
		(*tgtProp)->proxy = NULL;
	}
	g_free((*tgtProp)->name);
	g_free((*tgtProp)->type);
	g_strfreev((*tgtProp)->tagsType);
	g_free((*tgtProp));
	(*tgtProp) = NULL;
}

/******************************************************************************
 * neardal_get_targets: get an array of NFC targets present
 *****************************************************************************/
errorCode_t neardal_get_targets(neardal_t neardalMgr, char *adpName,
				 char ***array, int *len)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_TARGET;
	AdpProp		*adpProp	= NULL;
	int		tgtNb		= 0;
	int		ct		= 0;	/* counter */
	char		**tgts		= NULL;
	TgtProp		*target		= NULL;


	if (neardalMgr == NULL || adpName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	errCode = neardal_mgr_prv_get_adapter(neardalMgr, adpName,
						   &adpProp);
	if (errCode != NEARDAL_SUCCESS)
		return errCode;

	tgtNb = g_list_length(adpProp->tgtList);
	if (tgtNb <= 0)
		return NEARDAL_ERROR_NO_TARGET;

	errCode = NEARDAL_ERROR_NO_MEMORY;
	tgts = g_try_malloc0((tgtNb + 1) * sizeof(char *));

	if (tgts == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	while (ct < tgtNb) {
		target = g_list_nth_data(adpProp->tgtList, ct);
		if (target != NULL)
			tgts[ct++] = g_strdup(target->name);
	}
	errCode = NEARDAL_SUCCESS;

	if (len != NULL)
		*len = tgtNb;
	*array	= tgts;

	return errCode;
}

/******************************************************************************
 * neardal_tgt_add: add new NFC target, initialize DBus Proxy connection,
 * register target signal
 *****************************************************************************/
errorCode_t neardal_tgt_add(neardal_t neardalMgr, void * parent,
			    gchar *tgtName)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_MEMORY;
	TgtProp		*tgtProp	= NULL;
	AdpProp		*adpProp	= parent;

	g_assert(neardalMgr != NULL);
	g_assert(tgtName != NULL);

	NEARDAL_TRACEF("Adding target:%s\n", tgtName);
	tgtProp = g_try_malloc0(sizeof(TgtProp));
	if (tgtProp == NULL)
		goto error;

	tgtProp->name	= g_strdup(tgtName);
	tgtProp->parent	= adpProp;

	adpProp->tgtList = g_list_prepend(adpProp->tgtList, tgtProp);
	errCode = neardal_tgt_prv_init(neardalMgr, tgtProp);

	NEARDAL_TRACEF("NEARDAL LIB targetList contains %d elements\n",
		      g_list_length(adpProp->tgtList));
	
	if (neardalMgr->cb_tgt_found != NULL)
		(neardalMgr->cb_tgt_found)(tgtProp->name,
					   neardalMgr->cb_tgt_found_ud);

	return errCode;

error:
	if (tgtProp->name != NULL)
		g_free(tgtProp->name);
	if (tgtProp != NULL)
		g_free(tgtProp);

	return errCode;
}

/******************************************************************************
 * neardal_tgt_remove: remove one NFC target, unref DBus Proxy connection,
 * unregister target signal
 *****************************************************************************/
void neardal_tgt_remove(TgtProp *tgtProp)
{
	RcdProp		*rcdProp	= NULL;
	GList		*node;
	AdpProp		*adpProp;

	g_assert(tgtProp != NULL);

	NEARDAL_TRACEF("Removing target:%s\n", tgtProp->name);
	/* Remove all targets */
	while (g_list_length(tgtProp->rcdList)) {
		node = g_list_first(tgtProp->rcdList);
		rcdProp = (RcdProp *) node->data;
		neardal_rcd_remove(rcdProp);
	}
	adpProp = tgtProp->parent;
	adpProp->tgtList = g_list_remove(adpProp->tgtList,
					 (gconstpointer) tgtProp);
	
	neardal_tgt_prv_free(&tgtProp);
}
