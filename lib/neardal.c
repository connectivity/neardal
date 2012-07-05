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
 *     This file contains all Public APIs
 */

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <dbus/dbus-glib.h>
#include "dbus/dbus.h"

#include "neard_manager_proxy.h"
#include "neard_adapter_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"

neardalCtx neardalMgr = {NULL, NULL, {NULL}, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL};

/*---------------------------------------------------------------------------
 * Context Management
 ---------------------------------------------------------------------------*/
/*****************************************************************************
 * neardal_prv_construct: create NEARDAL object instance, Neard Dbus
 * connection, register Neard's events
 ****************************************************************************/
void neardal_prv_construct(errorCode_t *ec)
{
	errorCode_t	err = NEARDAL_SUCCESS;

	if (neardalMgr.proxy != NULL)
		return;

	NEARDAL_TRACEIN();
	/* Create DBUS connection */
	g_type_init();
	neardalMgr.conn = dbus_g_bus_get(DBUS_BUS_SYSTEM,
					   &neardalMgr.gerror);
	if (neardalMgr.conn != NULL) {
		/* We have a DBUS connection, create proxy on Neard Manager */
		err =  neardal_mgr_create();
		if (err != NEARDAL_SUCCESS) {
			NEARDAL_TRACEF(
				"neardal_mgr_create() exit (err %d: %s)\n",
				err, neardal_error_get_text(err));

		}
		/* No Neard daemon, destroying neardal object... */
		if (err == NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY)
			neardal_tools_prv_free_gerror(&neardalMgr.gerror);
	} else {
		NEARDAL_TRACE_ERR("Unable to connect to dbus: %s\n",
				 neardalMgr.gerror->message);
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		err = NEARDAL_ERROR_DBUS;
	}

	if (ec != NULL)
		*ec = err;

	NEARDAL_TRACEF("Exit\n");
	return;
}


/*****************************************************************************
 * neardal_destroy: destroy NEARDAL object instance, Disconnect Neard Dbus
 * connection, unregister Neard's events
 ****************************************************************************/
void neardal_destroy(void)
{
	NEARDAL_TRACEIN();
	if (neardalMgr.proxy != NULL) {
		neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		neardal_mgr_destroy();
	}
}

/*****************************************************************************
 * neardal_set_cb_adapter_added: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_added = NULL to remove actual callback.
 ****************************************************************************/
errorCode_t neardal_set_cb_adapter_added(adapter_cb cb_adp_added,
					 void *user_data)
{
	neardalMgr.cb_adp_added		= cb_adp_added;
	neardalMgr.cb_adp_added_ud	= user_data;

	return NEARDAL_SUCCESS;
}

/*****************************************************************************
 * neardal_set_cb_adapter_removed: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_removed = NULL to remove actual callback.
 ****************************************************************************/
errorCode_t neardal_set_cb_adapter_removed(adapter_cb cb_adp_removed,
					   void *user_data)
{

	neardalMgr.cb_adp_removed	= cb_adp_removed;
	neardalMgr.cb_adp_removed_ud	= user_data;

	return NEARDAL_SUCCESS;
}

/*****************************************************************************
 * neardal_set_manager_cb_property_changed: setup a client callback for
 * 'NEARDAL Adapter Property Change'.
 * cb_mgr_adp_property_changed = NULL to remove actual callback.
 ****************************************************************************/
errorCode_t neardal_set_cb_adapter_property_changed(
					adapter_prop_cb cb_adp_property_changed,
					void *user_data)
{
	neardalMgr.cb_adp_prop_changed		= cb_adp_property_changed;
	neardalMgr.cb_adp_prop_changed_ud	= user_data;

	return NEARDAL_SUCCESS;
}

/*****************************************************************************
 * neardal_set_cb_adapter_added: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_added = NULL to remove actual callback.
 ****************************************************************************/
errorCode_t neardal_set_cb_tag_found(tag_cb cb_tag_found,
					void *user_data)
{
	neardalMgr.cb_tag_found		= cb_tag_found;
	neardalMgr.cb_tag_found_ud	= user_data;

	return NEARDAL_SUCCESS;
}

/*****************************************************************************
 * neardal_set_cb_adapter_removed: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_removed = NULL to remove actual callback.
 ****************************************************************************/
errorCode_t neardal_set_cb_tag_lost(tag_cb cb_tag_lost,
				       void *user_data)
{
	neardalMgr.cb_tag_lost		= cb_tag_lost;
	neardalMgr.cb_tag_lost_ud	= user_data;

	return NEARDAL_SUCCESS;
}


/*****************************************************************************
 * neardal_set_cb_record_found: setup a client callback for
 * 'NEARDAL tag record found'.
 * cb_rcd_found = NULL to remove actual callback.
 ****************************************************************************/
errorCode_t neardal_set_cb_record_found(record_cb cb_rcd_found,
					void *user_data)
{
	neardalMgr.cb_rcd_found		= cb_rcd_found;
	neardalMgr.cb_rcd_found_ud	= user_data;

	return NEARDAL_SUCCESS;
}

/*****************************************************************************
 * neardal_free_array: free adapters array, tags array or records array
 ****************************************************************************/
errorCode_t neardal_free_array(char ***array)
{
	errorCode_t	err = NEARDAL_SUCCESS;
	char		**adps;

	if (array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	adps = *array;
	while ((*adps) != NULL) {
		g_free(*adps);
		adps++;
	}
	g_free(*array);
	*array = NULL;

	return err;
}

char *neardal_error_get_text(errorCode_t ec)
{
	switch (ec) {
	case NEARDAL_SUCCESS:
		return "Success";

	case NEARDAL_ERROR_GENERAL_ERROR:
		return "General error";

	case NEARDAL_ERROR_INVALID_PARAMETER:
		return "Invalid parameter";

	case NEARDAL_ERROR_NO_MEMORY:
		return "Memory allocation error";

	case NEARDAL_ERROR_DBUS:
		return "DBUS general error";

	case NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY:
		return "Can not create a DBUS proxy";

	case NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD:
		return "Can not invoke a DBUS method";

	case NEARDAL_ERROR_NO_ADAPTER:
		return "No NFC adapter found...";

	case NEARDAL_ERROR_NO_TAG:
		return "No NFC tag found...";

	case NEARDAL_ERROR_NO_RECORD:
		return "No tag record found...";

	case NEARDAL_ERROR_POLLING_ALREADY_ACTIVE:
		return "Polling already active";

	case NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR:
		return "Error while invoking method";
	}

	return "UNKNOWN ERROR !!!";
}


/*---------------------------------------------------------------------------
 * NFC Adapter Management
 ---------------------------------------------------------------------------*/
/*****************************************************************************
 * neardal_get_adapters: get an array of NFC adapters (adpName) present
 ****************************************************************************/
errorCode_t neardal_get_adapters(char ***array, int *len)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	int		adpNb		= 0;
	int		ct		= 0;	/* counter */
	char		**adps		= NULL;
	AdpProp		*adapter	= NULL;
	gsize		size;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	adpNb = g_list_length(neardalMgr.prop.adpList);
	if (adpNb > 0) {
		err = NEARDAL_ERROR_NO_MEMORY;
		size = (adpNb + 1) * sizeof(char *);
		adps = g_try_malloc0(size);
		if (adps != NULL) {
			GList	*list;
			while (ct < adpNb) {
				list = neardalMgr.prop.adpList;
				adapter = g_list_nth_data(list, ct);
				if (adapter != NULL)
					adps[ct++] = g_strdup(adapter->name);
			}
			err = NEARDAL_SUCCESS;
		}
	} else
		err = NEARDAL_ERROR_NO_ADAPTER;

	if (len != NULL)
		*len = adpNb;
	*array	= adps;

	return err;
}

/*****************************************************************************
 * neardal_get_adapter_properties: Get properties of a specific NEARDAL adapter
 ****************************************************************************/
errorCode_t neardal_get_adapter_properties(const char *adpName,
					   neardal_adapter *adapter)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;
	TagProp		*tag		= NULL;
	int		ct		= 0;	/* counter */
	gsize		size;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || adpName == NULL || adapter == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter((gchar *) adpName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	adapter->name		= (char *) adpProp->name;
	adapter->polling	= (short) adpProp->polling;
	adapter->powered	= (short) adpProp->powered;

	adapter->nbProtocols	= adpProp->lenProtocols;
	adapter->protocols	= NULL;


	if (adapter->nbProtocols > 0) {
		err = NEARDAL_ERROR_NO_MEMORY;
		size = (adapter->nbProtocols + 1) * sizeof(char *);
		adapter->protocols = g_try_malloc0(size);
		if (adapter->protocols != NULL) {
			ct = 0;
			while (ct < adapter->nbProtocols) {
				gchar *tmp = g_strdup(adpProp->protocols[ct]);
				adapter->protocols[ct++] = (char *) tmp;
			}
			err = NEARDAL_SUCCESS;
		}
	}

	adapter->nbTags	= (int) adpProp->tagNb;
	adapter->tags	= NULL;
	if (adapter->nbTags <= 0)
		goto exit;

	err = NEARDAL_ERROR_NO_MEMORY;
	size = (adapter->nbTags + 1) * sizeof(char *);
	adapter->tags = g_try_malloc0(size);
	if (adapter->tags == NULL)
		goto exit;

	ct = 0;
	while (ct < adapter->nbTags) {
		tag = g_list_nth_data(adpProp->tagList, ct);
		if (tag != NULL)
			adapter->tags[ct++] = g_strdup(tag->name);
	}
	err = NEARDAL_SUCCESS;

exit:
	return err;
}

/*****************************************************************************
 * neardal_set_adapter_property: Set a property on a specific NEARDAL adapter
 ****************************************************************************/
errorCode_t neardal_set_adapter_property(const char *adpName,
					   int adpPropId, void *value)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;
	GValue		*gProperty	= NULL;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || adpName == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter((gchar *) adpName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;


	gProperty = g_try_malloc0(sizeof(GValue));
	switch (adpPropId) {
	case NEARD_ADP_PROP_POWERED:
		g_value_init(gProperty, G_TYPE_BOOLEAN);
		g_value_set_boolean(gProperty, (gboolean) value);
		org_neard_Adapter_set_property(adpProp->proxy, "Powered",
					       gProperty, &neardalMgr.gerror);
		break;
	case NEARD_ADP_PROP_MODE:
		g_value_init(gProperty, G_TYPE_STRING);
		g_value_set_string(gProperty, (gchar *) value);
		org_neard_Adapter_set_property(adpProp->proxy, "Mode",
					       gProperty, &neardalMgr.gerror);
		break;
	default:
		break;
	}


	if (neardalMgr.gerror == NULL)
		err = NEARDAL_SUCCESS;
	else
		err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;

exit:
	neardal_tools_prv_free_gerror(&neardalMgr.gerror);
	neardalMgr.gerror = NULL;
	return err;
}

/*****************************************************************************
 * neardal_start_poll: Request Neard to start polling
 ****************************************************************************/
errorCode_t neardal_start_poll(char *adpName)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);
	if (err != NEARDAL_SUCCESS)
		return err;

	err = neardal_mgr_prv_get_adapter(adpName, &adpProp);

	err = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp == NULL)
		goto exit;

	if (adpProp->proxy == NULL)
		goto exit;

	if (!adpProp->polling) {
		org_neard_Adapter_start_poll(adpProp->proxy,
						    &neardalMgr.gerror);

		err = NEARDAL_SUCCESS;
		if (neardalMgr.gerror != NULL) {
			NEARDAL_TRACE_ERR(
				"Error with neard dbus method (err:%d:'%s')\n"
					, neardalMgr.gerror->code
					, neardalMgr.gerror->message);
			err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;
			neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		}
	} else
		err = NEARDAL_ERROR_POLLING_ALREADY_ACTIVE;

exit:
	return err;
}

/*****************************************************************************
 * neardal_stop_poll: Request Neard to stop polling
 ****************************************************************************/
errorCode_t neardal_stop_poll(char *adpName)
{
	errorCode_t	err = NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err == NEARDAL_SUCCESS)
		err = neardal_mgr_prv_get_adapter(adpName, &adpProp);

	if (adpProp == NULL)
		goto exit;

	if (adpProp->proxy == NULL)
		goto exit;

	if (adpProp->polling) {
		org_neard_Adapter_stop_poll(adpProp->proxy,
					    &neardalMgr.gerror);

		err = NEARDAL_SUCCESS;
		if (neardalMgr.gerror != NULL) {
			NEARDAL_TRACE_ERR(
				"Error with neard dbus method (err:%d:'%s')\n"
					, neardalMgr.gerror->code
					, neardalMgr.gerror->message);
			err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;
			neardal_tools_prv_free_gerror(&neardalMgr.gerror);
		}
	}

exit:
	return err;
}


/*---------------------------------------------------------------------------
 * NFC Tag Management
 ---------------------------------------------------------------------------*/
/*****************************************************************************
 * neardal_get_tags: get an array of NFC tags present
 ****************************************************************************/
errorCode_t neardal_get_tags(char *adpName, char ***array, int *len)
{
	errorCode_t	err		= NEARDAL_ERROR_NO_TAG;
	AdpProp		*adpProp	= NULL;
	int		tagNb		= 0;
	int		ct		= 0;	/* counter */
	char		**tags		= NULL;
	TagProp		*tag		= NULL;


	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || adpName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	err = neardal_mgr_prv_get_adapter(adpName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		return err;

	tagNb = g_list_length(adpProp->tagList);
	if (tagNb <= 0)
		return NEARDAL_ERROR_NO_TAG;

	err = NEARDAL_ERROR_NO_MEMORY;
	tags = g_try_malloc0((tagNb + 1) * sizeof(char *));

	if (tags == NULL)
		return NEARDAL_ERROR_NO_MEMORY;

	while (ct < tagNb) {
		tag = g_list_nth_data(adpProp->tagList, ct);
		if (tag != NULL)
			tags[ct++] = g_strdup(tag->name);
	}
	err = NEARDAL_SUCCESS;

	if (len != NULL)
		*len = tagNb;
	*array	= tags;

	return err;
}

/*****************************************************************************
 * neardal_get_tag_properties: Get properties of a specific NEARDAL
 * tag
 ****************************************************************************/
errorCode_t neardal_get_tag_properties(const char *tagName,
					  neardal_tag *tag)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;
	TagProp		*tagProp	= NULL;
	int		ct		= 0;	/* counter */
	RcdProp		*record		= NULL;
	gsize		size;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || tagName == NULL || tag == NULL)
		goto exit;

	tag->records	= NULL;
	tag->tagType	= NULL;
	err = neardal_mgr_prv_get_adapter((gchar *) tagName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_tag(adpProp, (gchar *) tagName, &tagProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	tag->name		= (const char *) tagProp->name;
	tag->type		= (const char *) tagProp->type;
	tag->readOnly	= (short) tagProp->readOnly;
	tag->nbRecords	= (int) tagProp->rcdLen;
	if (tag->nbRecords > 0) {
		err = NEARDAL_ERROR_NO_MEMORY;
		size = (tag->nbRecords + 1) * sizeof(char *);
		tag->records = g_try_malloc0(size);
		if (tag->records == NULL)
			goto exit;

		ct = 0;
		while (ct < tag->nbRecords) {
			record = g_list_nth_data(tagProp->rcdList, ct);
			if (record != NULL)
				tag->records[ct++] = g_strdup(record->name);
		}
		err = NEARDAL_SUCCESS;
	}

	tag->nbTagTypes = 0;
	tag->tagType = NULL;
	/* Count TagTypes */
	tag->nbTagTypes = (int) tagProp->tagTypeLen;

	if (tag->nbTagTypes <= 0)
		goto exit;

	err = NEARDAL_ERROR_NO_MEMORY;
	size = (tag->nbTagTypes + 1) * sizeof(char *);
	tag->tagType = g_try_malloc0(size);
	if (tag->tagType == NULL)
		goto exit;

	ct = 0;
	while (ct < tag->nbTagTypes) {
		tag->tagType[ct] = g_strdup(tagProp->tagType[ct]);
		ct++;
	}
	err = NEARDAL_SUCCESS;

exit:
	return err;
}

/*****************************************************************************
 * neardal_write: Write NDEF record to an NFC tag
 ****************************************************************************/
errorCode_t neardal_write(neardal_record *record)
{
	errorCode_t	err	= NEARDAL_SUCCESS;
	AdpProp		*adpProp;
	TagProp		*tagProp;
	RcdProp		rcd;


	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || record == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter((gchar *) record->name, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;
	err = neardal_mgr_prv_get_tag(adpProp, (gchar *) record->name,
				      &tagProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	rcd.name		= (gchar *) record->name;
	rcd.action		= (gchar *) record->action;
	rcd.encoding		= (gchar *) record->encoding;
	rcd.language		= (gchar *) record->language;
	rcd.type		= (gchar *) record->type;
	rcd.representation	= (gchar *) record->representation;
	rcd.uri			= (gchar *) record->uri;
	rcd.uriObjSize		= record->uriObjSize;
	rcd.mime		= (gchar *) record->mime;

	neardal_tag_prv_write(tagProp, &rcd);
exit:
	return err;
}

/*---------------------------------------------------------------------------
 * NFC Record Management
 ---------------------------------------------------------------------------*/
/******************************************************************************
 * neardal_get_records: get an array of tag records
 *****************************************************************************/
errorCode_t neardal_get_records(char *tagName, char ***array, int *len)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;
	TagProp		*tagProp	= NULL;
	int		rcdLen		= 0;
	int		ct		= 0;	/* counter */
	char		**rcds		= NULL;
	RcdProp		*rcd		= NULL;


	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS || tagName == NULL || array == NULL)
		return NEARDAL_ERROR_INVALID_PARAMETER;

	err = neardal_mgr_prv_get_adapter(tagName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_adp_prv_get_tag(adpProp, tagName, &tagProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err		= NEARDAL_ERROR_NO_RECORD;
	rcdLen = g_list_length(tagProp->rcdList);
	if (rcdLen <= 0)
		goto exit;

	err = NEARDAL_ERROR_NO_MEMORY;
	rcds = g_try_malloc0((rcdLen + 1) * sizeof(char *));
	if (rcds == NULL)
		goto exit;

	while (ct < rcdLen) {
		rcd = g_list_nth_data(tagProp->rcdList, ct);
		if (rcd != NULL)
			rcds[ct++] = g_strdup(rcd->name);
	}
	err = NEARDAL_SUCCESS;

exit:
	if (len != NULL)
		*len = rcdLen;
	*array	= rcds;

	return err;
}

/*****************************************************************************
 * neardal_get_record_properties: Get values of a specific tag record
 ****************************************************************************/
errorCode_t neardal_get_record_properties(const char *recordName,
					  neardal_record *record)
{
	errorCode_t	err		= NEARDAL_SUCCESS;
	AdpProp		*adpProp	= NULL;
	TagProp		*tagProp	= NULL;
	RcdProp		*rcdProp	= NULL;

	if (recordName == NULL || record == NULL)
		goto exit;

	if (neardalMgr.proxy == NULL)
		neardal_prv_construct(&err);

	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_adapter((gchar *) recordName, &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_tag(adpProp, (gchar *) recordName,
					 &tagProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_record(tagProp, (gchar *) recordName,
					 &rcdProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	record->name		= (const char *) rcdProp->name;
	record->encoding	= (const char *) rcdProp->encoding;
	record->language	= (const char *) rcdProp->language;
	record->action		= (const char *) rcdProp->action;

	record->type		= (const char *) rcdProp->type;
	record->representation	= (const char *) rcdProp->representation;
	record->uri		= (const char *) rcdProp->uri;
	record->uriObjSize	= (unsigned int) rcdProp->uriObjSize;
	record->mime		= (const char *) rcdProp->mime;

exit:
	return err;
}


