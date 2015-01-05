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
#include <string.h>
#include <glib.h>
#include <glib-object.h>

#include "neard_manager_proxy.h"
#include "neard_adapter_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"

void neardal_adp_prv_cb_tag_found(OrgNeardTag *proxy,
			const gchar *arg_unnamed_arg0, void *user_data);
void neardal_adp_prv_cb_tag_lost(OrgNeardTag *proxy,
			const gchar *arg_unnamed_arg0, void *user_data);

TagProp *neardal_mgr_tag_search(const gchar *tag)
{
	char *adapter = NULL;
	AdpProp *adpProp = NULL;
	TagProp *tagProp = NULL;

	if (!(adapter = neardal_dirname(tag)))
		goto error;

	if (neardal_mgr_prv_get_adapter(adapter, &adpProp) != NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR("Adapter %s not found\n", adapter);
		goto error;
	}

	if (neardal_adp_prv_get_tag(adpProp, (gchar *) tag, &tagProp)
			!= NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR("Tag %s not found\n", tag);
		goto error;
	}
error:
	g_free(adapter);
	return tagProp;
}

TagProp *neardal_mgr_tag_search_by_record(const gchar *record)
{
	char *tag = NULL;
	TagProp *tagProp = NULL;

	if ((tag = neardal_dirname(record))) {
		tagProp = neardal_mgr_tag_search(tag);
		g_free(tag);
	} else
		NEARDAL_TRACE_ERR("No tag found for record=%s\n", record);

	return tagProp;
}

static void neardal_mgr_tag_add(const gchar *path, GVariant *tag)
{
	char *adapter = NULL;
	AdpProp *adpProp = NULL;

	NEARDAL_TRACEF("Tag: %s\n", g_variant_print(tag, TRUE));

	if (!g_variant_lookup(tag, "Adapter", "o", &adapter) ||
			neardal_mgr_prv_get_adapter(adapter, &adpProp) !=
				NEARDAL_SUCCESS) {
		NEARDAL_TRACE_ERR("Adapter not found\n");
		goto error;
	}

	NEARDAL_TRACEF("Adapter: %s\n", adapter);

	g_datalist_set_data_full(&(neardalMgr.dbus_data), path,
			g_variant_ref(tag), (GDestroyNotify) g_variant_unref);

	neardal_adp_prv_cb_tag_found(NULL, path, adpProp);
error:
	g_free(adapter);
}

void neardal_adp_prv_cb_dev_found(void *proxy,
				const gchar *arg_unnamed_arg0, void *user_data);
void neardal_adp_prv_cb_dev_lost(void *proxy,
				const gchar *arg_unnamed_arg0, void *user_data);

static AdpProp *neardal_adapter_find_by_child(const char *path)
{
	char *name = neardal_dirname(path);
	AdpProp *adapter = NULL;
	neardal_mgr_prv_get_adapter((char *) path, &adapter);
	g_free(name);
	return adapter;
}

static void neardal_mgr_interfaces_added(ObjectManager *om,
					const gchar *path, GVariant *interfaces)
{
	GVariant *v = NULL;

	NEARDAL_TRACEF("path=%s\n", path);
	NEARDAL_TRACEF("interfaces=%s\n", g_variant_print(interfaces, TRUE));

	if (g_variant_lookup(interfaces, "org.neard.Record", "*",
				(void *) &v)) {
		GVariant *record;
		if ((record = neardal_data_insert(path, "Record", v)))
			neardal_record_add(record);
		return;
	}

	if (g_variant_lookup(interfaces, "org.neard.Device", "*",
				(void *) &v)) {
		AdpProp *adp = neardal_adapter_find_by_child(path);
		if (adp)
			neardal_adp_prv_cb_dev_found(NULL, path, adp);
		return;
	}

	if (g_variant_lookup(interfaces, "org.neard.Tag", "*", (void *) &v)) {
		neardal_mgr_tag_add(path, v);
		return;
	}

	NEARDAL_TRACE_ERR("Unsupported interface change: path=%s, "
		"interface=%s\n", path, g_variant_print(interfaces, TRUE));
}

static void neardal_mgr_tag_remove(const gchar *tag)
{
	GVariant *v = g_datalist_get_data(&(neardalMgr.dbus_data), tag);
	char *adapter = NULL;
	AdpProp *adpProp = NULL;

	if (v == NULL) {
		NEARDAL_TRACE_ERR("Tag %s not found\n", tag);
		return;
	}

	NEARDAL_TRACEF("Tag's objects: %s\n", g_variant_print(v, TRUE));

	if (!g_variant_lookup(v, "Adapter", "o", &adapter) ||
			neardal_mgr_prv_get_adapter(adapter, &adpProp)
				!= NEARDAL_SUCCESS)
		return;

	NEARDAL_TRACEF("Adapter: %s=%p\n", adapter, (void *) adpProp);

	neardal_adp_prv_cb_tag_lost(NULL, tag, adpProp);

	g_datalist_remove_data(&(neardalMgr.dbus_data), tag);

	g_free(adapter);
}

static void neardal_mgr_interfaces_removed(ObjectManager *om,
						const gchar *path,
						const gchar *const *interfaces)
{
	char *s = g_strjoinv("' '", (gchar **)interfaces);
	int i = 0;

	NEARDAL_TRACEF("path=%s\n", path);
	NEARDAL_TRACEF("interfaces='%s'\n", s);

	g_free(s);

	while ((s = (char *) interfaces[i++])) {
		if (strcmp(s, "org.neard.Record") == 0) {
			GVariant *record;
			if ((record = neardal_data_search(path))) {
				neardal_record_remove(record);
				neardal_data_remove(record);
			}
			continue;
		}

		if (strcmp(s, "org.neard.Tag") == 0) {
			neardal_mgr_tag_remove(path);
			continue;
		}

		if (strcmp(s, "org.neard.Device") == 0) {
			AdpProp *adp = neardal_adapter_find_by_child(path);
			if (adp)
				neardal_adp_prv_cb_dev_lost(NULL, path, adp);
			continue;
		}

		if (strcmp(s, "org.freedesktop.DBus.Properties") == 0)
			continue;
		if (strcmp(s, "org.freedesktop.DBus.Introspectable") == 0)
			continue;

		NEARDAL_TRACE_ERR("Unsupported interface change: "
					"path=%s, data=%s\n", path, s);
	}
}

/*****************************************************************************
 * neardal_mgr_prv_cb_property_changed: Callback called when a NFC Manager
 * Property is changed
 ****************************************************************************/
static void neardal_mgr_prv_cb_property_changed(OrgNeardManager *proxy,
						const gchar *arg_unnamed_arg0,
						GVariant *arg_unnamed_arg1,
						void        *user_data)
{
	NEARDAL_TRACEIN();

	(void) proxy; /* remove warning */
	(void) arg_unnamed_arg1; /* remove warning */
	(void) user_data; /* remove warning */
	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);

	NEARDAL_TRACEF("arg_unnamed_arg0='%s'\n", arg_unnamed_arg0);
	/* Adapters List ignored... */
}

/*****************************************************************************
 * neardal_mgr_prv_cb_adapter_added: Callback called when a NFC adapter is
 * added
 ****************************************************************************/
static void neardal_mgr_prv_cb_adapter_added(OrgNeardManager *proxy,
					     const gchar *arg_unnamed_arg0,
					     void        *user_data)
{
	errorCode_t	err = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */
	(void) user_data; /* remove warning */

	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);
	
	err = neardal_adp_add((char *) arg_unnamed_arg0);
	if (err != NEARDAL_SUCCESS)
		return;

	NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
		      g_list_length(neardalMgr.prop.adpList));
}

/*****************************************************************************
 * neardal_mgr_prv_cb_adapter_removed: Callback called when a NFC adapter
 * is removed
 ****************************************************************************/
static void neardal_mgr_prv_cb_adapter_removed(OrgNeardManager *proxy,
					       const gchar *arg_unnamed_arg0,
					       void *user_data)
{
	GList	*node	= NULL;

	NEARDAL_TRACEIN();
	(void) proxy; /* remove warning */
	(void) user_data; /* remove warning */

	NEARDAL_ASSERT(arg_unnamed_arg0 != NULL);

	node = g_list_first(neardalMgr.prop.adpList);
	if (node == NULL) {
		NEARDAL_TRACE_ERR("NFC adapter not found! (%s)\n",
				  arg_unnamed_arg0);
		return;
	}

	/* Invoke client cb 'adapter removed' */
	if (neardalMgr.cb.adp_removed != NULL)
		(neardalMgr.cb.adp_removed)((char *) arg_unnamed_arg0,
					 neardalMgr.cb.adp_removed_ud);

	neardal_adp_remove(((AdpProp *)node->data));

	NEARDAL_TRACEF("NEARDAL LIB adapterList contains %d elements\n",
		      g_list_length(neardalMgr.prop.adpList));
}

static void neardal_mgr_adapters_parse(GVariant *v, char ***adps, gsize *nadps)
{
	char *s = NULL;
	char *t = NULL;
	GVariantIter iter, *iter2;

	*adps = g_new0(char *, g_variant_n_children(v) + 1);
	*nadps = 0;

	g_variant_iter_init(&iter, v);

	while (g_variant_iter_loop(&iter, "{oa{sa{sv}}}", &s, &iter2, NULL)) {
		while (g_variant_iter_loop(iter2, "{s*}", &t, NULL)) {
			if(!strcmp(t, "org.neard.Adapter"))
			{
				NEARDAL_TRACEF("Found adapter: %s\n", s);
				(*adps)[(*nadps)++] = s;
				s = NULL;
				g_free(t);
				break;
			}
		}
	}

	if (!*nadps) {
		g_free(*adps);
		return;
	}

	NEARDAL_TRACEF("Found %lu adapter(s)\n", *nadps);
}

/*****************************************************************************
 * neardal_mgr_prv_get_all_adapters: Check if neard has an adapter
 ****************************************************************************/
static errorCode_t neardal_mgr_prv_get_all_adapters(gchar ***adpArray,
						    gsize *len)
{
	errorCode_t	err		= NEARDAL_ERROR_NO_ADAPTER;

	NEARDAL_ASSERT_RET(adpArray != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

	if (object_manager_call_get_managed_objects_sync(neardalMgr.dbus_om,
			&neardalMgr.dbus_objs, NULL, &neardalMgr.gerror)) {
		NEARDAL_TRACEF("Reading:\n%s\n",
				g_variant_print(neardalMgr.dbus_objs, TRUE));
		NEARDAL_TRACEF("Parsing neard adapters...\n");

		neardal_mgr_adapters_parse(neardalMgr.dbus_objs, adpArray, len);

		err = *len ? NEARDAL_SUCCESS : NEARDAL_ERROR_NO_ADAPTER;

	} else {
		err = NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD;
		NEARDAL_TRACE_ERR("%d:%s\n", neardalMgr.gerror->code,
				 neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
	}

	return err;
}


/*****************************************************************************
 * neardal_mgr_prv_get_adapter: Get NFC Adapter from name
 ****************************************************************************/
errorCode_t neardal_mgr_prv_get_adapter(gchar *adpName, AdpProp **adpProp)
{
	errorCode_t	err	= NEARDAL_ERROR_NO_ADAPTER;
	guint		len	= 0;
	AdpProp		*adapter;
	GList		*tmpList;

	tmpList = neardalMgr.prop.adpList;
	while (len < g_list_length(tmpList)) {
		adapter = g_list_nth_data(tmpList, len);
		if (adapter != NULL) {
			if (neardal_tools_prv_cmp_path(adapter->name,
							adpName)) {
				if (adpProp != NULL)
					*adpProp = adapter;
				err = NEARDAL_SUCCESS;
				break;
			}
		}
		len++;
	}

	return err;
}

/*****************************************************************************
 * neardal_mgr_prv_get_adapter_from_proxy: Get NFC Adapter from proxy
 ****************************************************************************/
errorCode_t neardal_mgr_prv_get_adapter_from_proxy(OrgNeardAdapter *adpProxy,
						   AdpProp **adpProp)
{
	errorCode_t	err	= NEARDAL_ERROR_NO_ADAPTER;
	guint		len = 0;
	AdpProp		*adapter;
	GList		*tmpList;

	NEARDAL_ASSERT_RET(adpProp != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

	tmpList = neardalMgr.prop.adpList;
	while (len < g_list_length(tmpList)) {
		adapter = g_list_nth_data(tmpList, len);
		if (adapter != NULL) {
			if (adapter->proxy == adpProxy) {
				*adpProp = adapter;
				err = NEARDAL_SUCCESS;
				break;
			}
		}
		len++;
	}

	return err;
}

/*****************************************************************************
 * neardal_mgr_create: Get Neard Manager Properties = NFC Adapters list.
 * Create a DBus proxy for the first one NFC adapter if present
 * Register Neard Manager signals ('PropertyChanged')
 ****************************************************************************/
errorCode_t neardal_mgr_create(void)
{
	errorCode_t	err;
	gchar		**adpArray = NULL;
	gsize		adpArrayLen;
	char		*adpName;
	guint		len;

	NEARDAL_TRACEIN();
	if (neardalMgr.proxy != NULL) {
		g_signal_handlers_disconnect_by_func(neardalMgr.proxy,
			NEARDAL_G_CALLBACK(neardal_mgr_prv_cb_property_changed),
							NULL);
		g_signal_handlers_disconnect_by_func(neardalMgr.proxy,
			NEARDAL_G_CALLBACK(neardal_mgr_prv_cb_adapter_added),
							NULL);
		g_signal_handlers_disconnect_by_func(neardalMgr.proxy,
			NEARDAL_G_CALLBACK(neardal_mgr_prv_cb_adapter_removed),
							NULL);
		g_object_unref(neardalMgr.proxy);
		neardalMgr.proxy = NULL;
	}

	neardalMgr.proxy = org_neard_manager_proxy_new_sync(neardalMgr.conn,
					G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
							NEARD_DBUS_SERVICE,
							NEARD_MGR_PATH,
							NULL, /* GCancellable */
							&neardalMgr.gerror);

	if (neardalMgr.gerror != NULL) {
		NEARDAL_TRACE_ERR(
			"Unable to create Neard Manager Proxy (%d:%s)\n",
				 neardalMgr.gerror->code,
				neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	g_datalist_init(&(neardalMgr.dbus_data));

	if (neardalMgr.dbus_om != NULL) {
		g_signal_handlers_disconnect_by_func(neardalMgr.dbus_om,
			NEARDAL_G_CALLBACK(neardal_mgr_interfaces_added), NULL);

		g_signal_handlers_disconnect_by_func(neardalMgr.dbus_om,
			NEARDAL_G_CALLBACK(neardal_mgr_interfaces_removed),
							NULL);
	}

	neardalMgr.gerror = NULL;

	neardalMgr.dbus_om = object_manager_proxy_new_sync(neardalMgr.conn, 0,
				NEARD_DBUS_SERVICE, NEARD_MGR_PATH, NULL,
				&neardalMgr.gerror);
	if (neardalMgr.gerror) {
		NEARDAL_TRACE_ERR("Error creating ObjectManager proxy: %s\n",
					neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		g_object_unref(neardalMgr.proxy);
		neardalMgr.proxy = NULL;
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}

	/* Get and store NFC adapters (is present) */
	err = neardal_mgr_prv_get_all_adapters(&adpArray, &adpArrayLen);
	if (adpArray != NULL && adpArrayLen > 0) {
		len = 0;
		while (len < adpArrayLen && err == NEARDAL_SUCCESS) {
			adpName =  adpArray[len++];
			err = neardal_adp_add(adpName);
		}
		g_strfreev(adpArray);
	}

	/* Register for manager signals 'PropertyChanged(String,Variant)' */
	NEARDAL_TRACEF("Register Neard-Manager Signal 'PropertyChanged'\n");
	g_signal_connect(neardalMgr.proxy,
			 NEARD_MGR_SIG_PROPCHANGED,
			 G_CALLBACK(neardal_mgr_prv_cb_property_changed),
			 NULL);

	/* Register for manager signals 'AdapterAdded(ObjectPath)' */
	NEARDAL_TRACEF("Register Neard-Manager Signal 'AdapterAdded'\n");
	g_signal_connect(neardalMgr.proxy,
			 NEARD_MGR_SIG_ADP_ADDED,
			 G_CALLBACK(neardal_mgr_prv_cb_adapter_added),
			 NULL);

	/* Register for manager signals 'AdapterRemoved(ObjectPath)' */
	NEARDAL_TRACEF("Register Neard-Manager Signal 'AdapterRemoved'\n");
	g_signal_connect(neardalMgr.proxy,
			 NEARD_MGR_SIG_ADP_RM,
			 G_CALLBACK(neardal_mgr_prv_cb_adapter_removed),
			 NULL);

	g_signal_connect(neardalMgr.dbus_om, "interfaces-added",
		G_CALLBACK(neardal_mgr_interfaces_added), NULL);

	g_signal_connect(neardalMgr.dbus_om, "interfaces-removed",
		G_CALLBACK(neardal_mgr_interfaces_removed), NULL);

	return err;
}

/*****************************************************************************
 * neardal_mgr_destroy: unref DBus proxy, disconnect Neard Manager signals
 ****************************************************************************/
void neardal_mgr_destroy(void)
{
	GList	*node;
	GList	**tmpList;

	NEARDAL_TRACEIN();
	/* Remove all adapters */
	tmpList = &neardalMgr.prop.adpList;
	while (g_list_length((*tmpList))) {
		node = g_list_first((*tmpList));
		neardal_adp_remove(((AdpProp *)node->data));
	}
	neardalMgr.prop.adpList = (*tmpList);

	if (neardalMgr.proxy == NULL)
		return;

	g_signal_handlers_disconnect_by_func(neardalMgr.proxy,
			NEARDAL_G_CALLBACK(neardal_mgr_prv_cb_property_changed),
						NULL);
	g_signal_handlers_disconnect_by_func(neardalMgr.proxy,
			NEARDAL_G_CALLBACK(neardal_mgr_prv_cb_adapter_added),
						NULL);
	g_signal_handlers_disconnect_by_func(neardalMgr.proxy,
			NEARDAL_G_CALLBACK(neardal_mgr_prv_cb_adapter_removed),
						NULL);
	g_object_unref(neardalMgr.proxy);
	neardalMgr.proxy = NULL;

	g_signal_handlers_disconnect_by_func(neardalMgr.dbus_om,
		NEARDAL_G_CALLBACK(neardal_mgr_interfaces_added), NULL);

	g_signal_handlers_disconnect_by_func(neardalMgr.dbus_om,
		NEARDAL_G_CALLBACK(neardal_mgr_interfaces_removed), NULL);

	g_datalist_clear(&(neardalMgr.dbus_data));
	neardalMgr.dbus_data = NULL;

	g_variant_unref(neardalMgr.dbus_objs);
	neardalMgr.dbus_objs = NULL;

	g_object_unref(neardalMgr.dbus_om);
	neardalMgr.dbus_om = NULL;
}
