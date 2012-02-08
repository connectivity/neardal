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
#include <glib-2.0/glib/gerror.h>

/******************************************************************************
 * neardal_set_cb_adapter_added: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_added = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_adapter_added(neardal_t neardalObj,
					 adapter_cb cb_adp_added,
					 void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalObj != NULL) {
		neardalObj->cb_adp_added = cb_adp_added;
		neardalObj->cb_adp_added_ud = user_data;
		err = NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_cb_adapter_removed: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_removed = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_adapter_removed(neardal_t neardalObj,
					   adapter_cb cb_adp_removed,
					   void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalObj != NULL) {
		neardalObj->cb_adp_removed	= cb_adp_removed;
		neardalObj->cb_adp_removed_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_manager_cb_property_changed: setup a client callback for
 * 'NEARDAL Adapter Property Change'.
 * cb_mgr_adp_property_changed = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_adapter_property_changed(neardal_t neardalObj,
					adapter_prop_cb cb_adp_property_changed,
					void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalObj != NULL) {
		neardalObj->cb_adp_prop_changed = cb_adp_property_changed;
		neardalObj->cb_adp_prop_changed_ud	= user_data;
		err					= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_cb_adapter_added: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_added = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_target_found(neardal_t neardalObj,
					target_cb cb_tgt_found,
					void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalObj != NULL) {
		neardalObj->cb_tgt_found	= cb_tgt_found;
		neardalObj->cb_tgt_found_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_cb_adapter_removed: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_removed = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_target_lost(neardal_t neardalObj,
				       target_cb cb_tgt_lost, void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalObj != NULL) {
		neardalObj->cb_tgt_lost		= cb_tgt_lost;
		neardalObj->cb_tgt_lost_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}


/******************************************************************************
 * neardal_set_cb_record_found: setup a client callback for
 * 'NEARDAL target record found'.
 * cb_rcd_found = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_record_found(neardal_t neardalObj,
					record_cb cb_rcd_found,
					void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalObj != NULL) {
		neardalObj->cb_rcd_found	= cb_rcd_found;
		neardalObj->cb_rcd_found_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_free_array: free adapters array, targets array or records array
 *****************************************************************************/
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


/******************************************************************************
 * neardal_start_poll: Request Neard to start polling
 *****************************************************************************/
void neardal_start_poll(neardal_t neardalObj, char *adpName, errorCode_t *ec)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;

	if (neardalObj == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalObj, adpName,
					  &adpProp);

	err = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp == NULL)
		goto exit;

	if (adpProp->dbusProxy == NULL)
		goto exit;

	if (!adpProp->polling) {
		org_neard_Adapter_start_poll(adpProp->dbusProxy,
					     &neardalObj->gerror);

		err = NEARDAL_SUCCESS;
		if (neardalObj->gerror != NULL) {
			NEARDAL_TRACE_ERR(
				"Error with neard dbus method (err:%d:'%s')\n"
					, neardalObj->gerror->code
					, neardalObj->gerror->message);
			err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;
			neardal_tools_prv_free_gerror(neardalObj);
		}
	} else
		err = NEARDAL_ERROR_POLLING_ALREADY_ACTIVE;

exit:
	if (ec != NULL)
		*ec = err;
}

/******************************************************************************
 * neardal_stop_poll: Request Neard to stop polling
 *****************************************************************************/
void neardal_stop_poll(neardal_t neardalObj, char *adpName, errorCode_t *ec)
{
	errorCode_t	err = NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;

	if (neardalObj == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalObj, adpName,
						   &adpProp);

	if (adpProp == NULL)
		goto exit;

	if (adpProp->dbusProxy == NULL)
		goto exit;

	if (adpProp->polling) {
		org_neard_Adapter_stop_poll(adpProp->dbusProxy,
					    &neardalObj->gerror);

		err = NEARDAL_SUCCESS;
		if (neardalObj->gerror != NULL) {
			NEARDAL_TRACE_ERR(
				"Error with neard dbus method (err:%d:'%s')\n"
					, neardalObj->gerror->code
					, neardalObj->gerror->message);
			err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;
			neardal_tools_prv_free_gerror(neardalObj);
		}
	}

exit:
	if (ec != NULL)
		*ec = err;
}

/******************************************************************************
 * neardal_publish: Write NDEF record to an NFC tag
 *****************************************************************************/
errorCode_t neardal_publish(neardal_t neardalObj, neardal_record *record)
{
	errorCode_t	err	= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp;
	RcdProp		rcd;

	if (neardalObj == NULL || record == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalObj, record->name,
						   &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;
	rcd.name		= (char *) record->name;
	rcd.action		= (char *) record->action;
	rcd.encoding		= (char *) record->encoding;
	rcd.handOver		= record->handOver;
	rcd.language		= (char *) record->language;
	rcd.type		= (char *) record->type;
	rcd.representation	= (char *) record->representation;
	rcd.uri			= (char *) record->uri;
	rcd.mime		= (char *) record->mime;
	rcd.smartPoster		= record->smartPoster;

	 neardal_adp_publish(adpProp, &rcd);
exit:
	return err;
}

/******************************************************************************
 * neardal_get_adapter_properties: Get properties of a specific NEARDAL adapter
 *****************************************************************************/
errorCode_t neardal_get_adapter_properties(neardal_t neardalObj,
					   const char *adpName,
					   neardal_adapter *adapter)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;
	TgtProp		*target		= NULL;
	int		ct		= 0;	/* counter */
	gsize		size;

	if (neardalObj == NULL || adpName == NULL || adapter == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalObj, adpName,
					  &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	adapter->name		= adpProp->name;
	adapter->polling	= (short) adpProp->polling;
	adapter->powered	= (short) adpProp->powered;

	adapter->nbProtocols	= 0;
	adapter->protocols	= NULL;

	/* Count protocols */
	while (adpProp->protocols[adapter->nbProtocols++] != NULL);

	if (adapter->nbProtocols > 0) {
		adapter->nbProtocols--;
		err = NEARDAL_ERROR_NO_MEMORY;
		size = (adapter->nbProtocols + 1) * sizeof(char *);
		adapter->protocols = g_try_malloc0(size);
		if (adapter->protocols != NULL) {
			ct = 0;
			while (ct < adapter->nbProtocols) {
				gchar *tmp = g_strdup(adpProp->protocols[ct]);
				adapter->protocols[ct] = tmp;
				ct++;
			}
			err = NEARDAL_SUCCESS;
		}
	}

	adapter->nbTargets	= g_list_length(adpProp->tgtList);
	adapter->targets	= NULL;
	if (adapter->nbTargets <= 0)
		goto exit;

	err = NEARDAL_ERROR_NO_MEMORY;
	size = (adapter->nbTargets + 1) * sizeof(char *);
	adapter->targets = g_try_malloc0(size);
	if (adapter->targets == NULL)
		goto exit;

	ct = 0;
	while (ct < adapter->nbTargets) {
		target = g_list_nth_data(adpProp->tgtList, ct);
		if (target != NULL)
			adapter->targets[ct++] = g_strdup(target->name);
	}
	err = NEARDAL_SUCCESS;

exit:
	return err;
}

/******************************************************************************
 * neardal_get_adapter_properties: Get properties of a specific NEARDAL adapter
 *****************************************************************************/
errorCode_t neardal_get_target_properties(neardal_t neardalObj,
					  const char *tgtName,
					  neardal_target *target)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	int		ct		= 0;	/* counter */
	RcdProp		*record		= NULL;
	gsize		size;

	if (neardalObj == NULL || tgtName == NULL || target == NULL)
		goto exit;

	target->records	= NULL;
	target->tagType	= NULL;
	err = neardal_mgr_prv_get_adapter(neardalObj, tgtName,
						   &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_target(adpProp, tgtName, &tgtProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	target->name		= tgtProp->name;
	target->type		= tgtProp->type;
	target->readOnly	= tgtProp->readOnly;
	target->nbRecords	= g_list_length(tgtProp->rcdList);
	if (target->nbRecords > 0) {
		err = NEARDAL_ERROR_NO_MEMORY;
		size = (target->nbRecords + 1) * sizeof(char *);
		target->records = g_try_malloc0(size);
		if (target->records == NULL)
			goto exit;

		ct = 0;
		while (ct < target->nbRecords) {
			record = g_list_nth_data(tgtProp->rcdList, ct);
			if (record != NULL)
				target->records[ct++] = g_strdup(record->name);
		}
		err = NEARDAL_SUCCESS;
	}

	target->nbTagTypes = 0;
	target->tagType = NULL;
	/* Count TagTypes */
	while (tgtProp->tagType[target->nbTagTypes++] != NULL);

	if (target->nbTagTypes <= 0)
		goto exit;

	target->nbTagTypes--;
	err = NEARDAL_ERROR_NO_MEMORY;
	size = (target->nbTagTypes + 1) * sizeof(char *);
	target->tagType = g_try_malloc0(size);
	if (target->tagType == NULL)
		goto exit;

	ct = 0;
	while (ct < target->nbTagTypes) {
		target->tagType[ct] = g_strdup(tgtProp->tagType[ct]);
		ct++;
	}
	err = NEARDAL_SUCCESS;

exit:
	return err;
}

 /******************************************************************************
 * neardal_get_record_properties: Get values of a specific target record
  *****************************************************************************/
errorCode_t neardal_get_record_properties(neardal_t neardalObj,
					  const char *recordName,
					  neardal_record *record)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	RcdProp		*rcdProp	= NULL;

	if (neardalObj == NULL || recordName == NULL || record == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalObj, recordName,
						   &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_target(adpProp, recordName
						, &tgtProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_record(tgtProp, recordName
					, &rcdProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	record->name		= rcdProp->name;
	record->encoding	= rcdProp->encoding;
	record->handOver	= (short) rcdProp->handOver;
	record->language	= rcdProp->language;
	record->smartPoster	= (short) rcdProp->smartPoster;
	record->action		= rcdProp->action;

	record->type		= rcdProp->type;
	record->representation	= rcdProp->representation;
	record->uri		= rcdProp->uri;
	record->mime		= rcdProp->mime;

exit:
	return err;
}


/******************************************************************************
 * neardal_construct: create NEARDAL object instance, Neard Dbus connection,
 * register Neard's events
 *****************************************************************************/
neardal_t neardal_construct(errorCode_t *ec)
{
	neardal_t	neardalObj	= NULL;
	errorCode_t	err	= NEARDAL_ERROR_NO_MEMORY;

	NEARDAL_TRACEIN();
	/* Allocate NEARDAL context */
	neardalObj = g_try_malloc0(sizeof(neardalCtx));
	if (neardalObj == NULL)
		goto exit;

	/* Create DBUS connection */
	g_type_init();
	neardalObj->conn = dbus_g_bus_get(DBUS_BUS_SYSTEM,
					   &neardalObj->gerror);
	if (neardalObj->conn != NULL) {
		/* We have a DBUS connection, create proxy on Neard Manager */
		err =  neardal_mgr_init(neardalObj);
		if (err != NEARDAL_SUCCESS) {
			NEARDAL_TRACEF(
				"neardal_mgr_init() exit (err %d: %s)\n",
				err, neardal_error_get_text(err));

			/* No Neard daemon, destroying neardal object... */
			if (err == NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY) {
				neardal_tools_prv_free_gerror(neardalObj);
				g_free(neardalObj);
				neardalObj = NULL;
			}
		}
	} else {
		NEARDAL_TRACE_ERR("Unable to connect to dbus: %s\n",
				 neardalObj->gerror->message);
		neardal_tools_prv_free_gerror(neardalObj);
		err = NEARDAL_ERROR_DBUS;
		g_free(neardalObj);
		neardalObj = NULL;
	}

exit:
	if (ec != NULL)
		*ec = err;

	NEARDAL_TRACEF("Exit\n");
	return neardalObj;
}


/******************************************************************************
 * neardal_destroy: destroy NEARDAL object instance, Disconnect Neard Dbus
 * connection, unregister Neard's events
 *****************************************************************************/
void neardal_destroy(neardal_t neardalObj)
{
	NEARDAL_TRACEIN();
	if (neardalObj != NULL) {
		neardal_mgr_release(neardalObj);
		neardal_tools_prv_free_gerror(neardalObj);
		g_free(neardalObj);
	}
}
