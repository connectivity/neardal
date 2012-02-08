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

#include "neard-target-proxy.h"

#include "neardal.h"
#include "neardal_prv.h"
#include <glib-2.0/glib/gerror.h>
#include <glib-2.0/glib/glist.h>
#include <glib-2.0/glib/garray.h>


/******************************************************************************
 * neardal_tgt_prv_cb_property_changed: Callback called when a NFC target
 * property is changed
 *****************************************************************************/
static void neardal_tgt_prv_cb_property_changed(DBusGProxy	*proxy,
					       const char	*str0,
					       GValue		*gvalue,
					       void		*user_data)
{
	GPtrArray	*pathsGpa	= NULL;
	errorCode_t	errCode		= NEARDAL_ERROR_NO_TARGET;
	TgtProp		*tgtProp	= NULL;

	(void) proxy; /* remove warning */
	(void) user_data; /* remove warning */

	NEARDAL_TRACEIN();

	if (tgtProp == NULL || str0 == NULL)
		return;

	NEARDAL_TRACEF("str0='%s'\n", str0);
	if (!strcmp(str0, "Targets")) {
		if (!G_VALUE_HOLDS(gvalue, DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH)) {
			NEARDAL_TRACE_ERR("Unexpected type: %s",
					  G_VALUE_TYPE_NAME(&gvalue));
			errCode = NEARDAL_ERROR_DBUS;
			return;
		}

		/* Extract the targets arrays List from the GValue */
		errCode = NEARDAL_ERROR_NO_TARGET;
		pathsGpa = g_value_get_boxed(gvalue);
		if (pathsGpa == NULL)
			goto error;

		if (pathsGpa->len <= 0)
			goto error;

		/* Getting last target */
		gvalue = g_ptr_array_index(pathsGpa, pathsGpa->len - 1);
		if (gvalue != NULL)
			errCode = NEARDAL_SUCCESS;
		else
			errCode = NEARDAL_ERROR_NO_TARGET;
	}

	return;

error:
	if (errCode != NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR("Exit with error code %d:%s\n", errCode,
				neardal_error_get_text(errCode));
		if (pathsGpa != NULL)
			g_boxed_free(G_TYPE_STRV, pathsGpa);
	}

	return;
}

/******************************************************************************
 * neardal_tgt_prv_read_properties: Get Neard Target Properties
 *****************************************************************************/
static errorCode_t neardal_tgt_prv_read_properties(TgtProp *tgtProp)
{
	errorCode_t	errCode			= NEARDAL_SUCCESS;
	GHashTable	*neardTargetPropHash	= NULL;
	GError		*gerror			= NULL;
	void		*tmp			= NULL;

	NEARDAL_TRACEIN();
	g_assert(tgtProp != NULL);
	g_assert(tgtProp->dbusProxy != NULL);

	org_neard_Target_get_properties(tgtProp->dbusProxy,
					&neardTargetPropHash,
					&gerror);
	if (gerror != NULL) {
		errCode = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR(
			"Unable to read target's properties (%d:%s)\n",
				 gerror->code, gerror->message);
		g_error_free(gerror);
		goto exit;
	}

	errCode = neardal_tools_prv_hashtable_get(neardTargetPropHash,
						   "Records",
					    DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH,
					    &tmp);
	if (errCode == NEARDAL_SUCCESS)
		neardal_tools_prv_g_ptr_array_copy(&tgtProp->rcdArray, tmp);

	errCode = neardal_tools_prv_hashtable_get(neardTargetPropHash,
						   "TagType",
						    G_TYPE_STRV, &tmp);
	if (errCode == NEARDAL_SUCCESS)
		tgtProp->tagType = g_boxed_copy(G_TYPE_STRV, tmp);

	errCode = neardal_tools_prv_hashtable_get(neardTargetPropHash, "Type",
						   G_TYPE_STRING, &tmp);
	if (errCode == NEARDAL_SUCCESS)
		tgtProp->type = g_strdup(tmp);

	errCode = neardal_tools_prv_hashtable_get(neardTargetPropHash,
						   "ReadOnly", G_TYPE_BOOLEAN,
						   &tmp);
	if (errCode == NEARDAL_SUCCESS)
		tgtProp->readOnly = (gboolean) tmp;

	g_hash_table_destroy(neardTargetPropHash);

exit:
	return errCode;
}

/******************************************************************************
 * neardal_tgt_init: Get Neard Manager Properties = NFC Targets list.
 * Create a DBus proxy for the first one NFC target if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
static errorCode_t neardal_tgt_prv_init(neardal_t neardalObj,
					    TgtProp *tgtProp)
{
	errorCode_t	errCode = NEARDAL_SUCCESS;
	guint		len = 0;
	char		*rcdName;

	NEARDAL_TRACEIN();
	g_assert(tgtProp != NULL);

	if (tgtProp->dbusProxy != NULL) {
		dbus_g_proxy_disconnect_signal(tgtProp->dbusProxy,
					       NEARD_TGT_SIG_PROPCHANGED,
			G_CALLBACK(neardal_tgt_prv_cb_property_changed),
					       NULL);
		g_object_unref(tgtProp->dbusProxy);
	}
	tgtProp->dbusProxy = NULL;

	errCode = neardal_tools_prv_create_proxy(neardalObj->conn,
						  &tgtProp->dbusProxy,
						  tgtProp->name,
						  NEARD_TARGETS_IF_NAME);
	if (errCode != NEARDAL_SUCCESS)
		return errCode;

	/* Populate Target datas... */
	errCode = neardal_tgt_prv_read_properties(tgtProp);
	if (errCode != NEARDAL_SUCCESS)
		return errCode;

	if (tgtProp->rcdArray == NULL)
		return NEARDAL_ERROR_GENERAL_ERROR;

	if (tgtProp->rcdArray->len <= 0)
		return NEARDAL_ERROR_NO_TARGET;

	len = 0;
	while (len < tgtProp->rcdArray->len && errCode == NEARDAL_SUCCESS) {
		rcdName = tgtProp->rcdArray->pdata[len];
		errCode = neardal_rcd_add(neardalObj, rcdName);
		if (errCode == NEARDAL_SUCCESS &&
			neardalObj->cb_tgt_found != NULL)
			(neardalObj->cb_tgt_found)(tgtProp->name,
					       neardalObj->cb_tgt_found_ud);
		if (neardalObj->cb_rcd_found != NULL)
			(neardalObj->cb_rcd_found)(rcdName,
					       neardalObj->cb_rcd_found_ud);
		len++;
	}

	/* Register Marshaller for signals (String,Variant) */
	dbus_g_object_register_marshaller(neardal_marshal_VOID__STRING_BOXED,
					   G_TYPE_NONE, G_TYPE_STRING,
					   G_TYPE_VALUE, G_TYPE_INVALID);

	NEARDAL_TRACEF("Register Neard-Target Signal 'PropertyChanged'\n");
	dbus_g_proxy_add_signal(tgtProp->dbusProxy, NEARD_TGT_SIG_PROPCHANGED,
				 G_TYPE_STRING, G_TYPE_VALUE, G_TYPE_INVALID);
	dbus_g_proxy_connect_signal(tgtProp->dbusProxy,
				    NEARD_TGT_SIG_PROPCHANGED,
			  G_CALLBACK(neardal_tgt_prv_cb_property_changed),
				    neardalObj, NULL);

	return errCode;
}

/******************************************************************************
 * neardal_tgt_release: unref DBus proxy, disconnect Neard Target signals
 *****************************************************************************/
static void neardal_tgt_prv_release(TgtProp *tgtProp)
{
	NEARDAL_TRACEIN();
	if (tgtProp->dbusProxy != NULL) {
		dbus_g_proxy_disconnect_signal(tgtProp->dbusProxy,
					       NEARD_TGT_SIG_PROPCHANGED,
			G_CALLBACK(neardal_tgt_prv_cb_property_changed),
					       NULL);
		g_object_unref(tgtProp->dbusProxy);
		tgtProp->dbusProxy = NULL;
	}
	g_free(tgtProp->name);
	g_free(tgtProp->type);
	neardal_tools_prv_g_ptr_array_free(tgtProp->rcdArray);
	g_boxed_free(G_TYPE_STRV, tgtProp->tagType);
}

/******************************************************************************
 * neardal_get_targets: get an array of NFC targets present
 *****************************************************************************/
errorCode_t neardal_get_targets(neardal_t neardalObj, char *adpName,
				 char ***array, int *len)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_TARGET;
	AdpProp		*adpProp	= NULL;
	int		tgtNb		= 0;
	int		ct		= 0;	/* counter */
	char		**tgts		= NULL;
	TgtProp		*target		= NULL;


	if (neardalObj == NULL || adpName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	errCode = neardal_mgr_prv_get_adapter(neardalObj, adpName,
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
errorCode_t neardal_tgt_add(neardal_t neardalObj, char *tgtName)
{
	errorCode_t	errCode		= NEARDAL_ERROR_NO_MEMORY;
	TgtProp		*tgtProp	= NULL;
	AdpProp		*adpProp;

	g_assert(neardalObj != NULL);
	g_assert(tgtName != NULL);

	NEARDAL_TRACEF("Adding target:%s\n", tgtName);
	tgtProp = g_try_malloc0(sizeof(TgtProp));
	if (tgtProp == NULL)
		goto error;

	tgtProp->name	= g_strdup(tgtName);
	errCode = neardal_mgr_prv_get_adapter(neardalObj, tgtName,
						   &adpProp);
	if (errCode != NEARDAL_SUCCESS)
		goto error;

	adpProp->tgtList = g_list_prepend(adpProp->tgtList, tgtProp);
	errCode = neardal_tgt_prv_init(neardalObj, tgtProp);
	if (errCode != NEARDAL_SUCCESS) {
		adpProp->tgtList = g_list_remove(adpProp->tgtList, tgtProp);
		goto error;
	}

	NEARDAL_TRACEF("NEARDAL LIB targetList contains %d elements\n",
		      g_list_length(adpProp->tgtList));

	return errCode;

error:
	if (tgtProp->name != NULL)
		g_free(tgtProp->name);
	if (tgtProp != NULL)
		g_free(tgtProp);

	return errCode;
}

/******************************************************************************
 * neardal_tgt_remove: remove NFC target, unref DBus Proxy connection,
 * unregister target signal
 *****************************************************************************/
void neardal_tgt_remove(TgtProp *tgtProp)
{
	RcdProp		*rcdProp	= NULL;
	GList		*node;

	g_assert(tgtProp != NULL);

	NEARDAL_TRACEF("Removing target:%s\n", tgtProp->name);
	/* Remove all targets */
	while (g_list_length(tgtProp->rcdList)) {
		node = g_list_first(tgtProp->rcdList);
		rcdProp = (RcdProp *) node->data;
		tgtProp->rcdList = g_list_remove(tgtProp->rcdList,
						 (gconstpointer) rcdProp);
		neardal_rcd_remove(rcdProp);
	}
	neardal_tgt_prv_release(tgtProp);
}
