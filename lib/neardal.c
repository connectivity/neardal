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

#include "neard_manager_proxy.h"
#include "neard_adapter_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"

#include <glib-2.0/glib/glist.h>
#include <glib-2.0/glib/gerror.h>

/******************************************************************************
 * neardal_set_cb_adapter_added: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_added = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_adapter_added(neardal_t neardalMgr,
					 adapter_cb cb_adp_added,
					 void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalMgr != NULL) {
		neardalMgr->cb_adp_added = cb_adp_added;
		neardalMgr->cb_adp_added_ud = user_data;
		err = NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_cb_adapter_removed: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_removed = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_adapter_removed(neardal_t neardalMgr,
					   adapter_cb cb_adp_removed,
					   void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalMgr != NULL) {
		neardalMgr->cb_adp_removed	= cb_adp_removed;
		neardalMgr->cb_adp_removed_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_manager_cb_property_changed: setup a client callback for
 * 'NEARDAL Adapter Property Change'.
 * cb_mgr_adp_property_changed = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_adapter_property_changed(neardal_t neardalMgr,
					adapter_prop_cb cb_adp_property_changed,
					void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalMgr != NULL) {
		neardalMgr->cb_adp_prop_changed = cb_adp_property_changed;
		neardalMgr->cb_adp_prop_changed_ud	= user_data;
		err					= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_cb_adapter_added: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_added = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_target_found(neardal_t neardalMgr,
					target_cb cb_tgt_found,
					void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalMgr != NULL) {
		neardalMgr->cb_tgt_found	= cb_tgt_found;
		neardalMgr->cb_tgt_found_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}

/******************************************************************************
 * neardal_set_cb_adapter_removed: setup a client callback for
 * 'NEARDAL adapter added'.
 * cb_adp_removed = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_target_lost(neardal_t neardalMgr,
				       target_cb cb_tgt_lost, void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalMgr != NULL) {
		neardalMgr->cb_tgt_lost		= cb_tgt_lost;
		neardalMgr->cb_tgt_lost_ud	= user_data;
		err				= NEARDAL_SUCCESS;
	}

	return err;
}


/******************************************************************************
 * neardal_set_cb_record_found: setup a client callback for
 * 'NEARDAL target record found'.
 * cb_rcd_found = NULL to remove actual callback.
 *****************************************************************************/
errorCode_t neardal_set_cb_record_found(neardal_t neardalMgr,
					record_cb cb_rcd_found,
					void *user_data)
{
	errorCode_t err = NEARDAL_ERROR_INVALID_PARAMETER;

	if (neardalMgr != NULL) {
		neardalMgr->cb_rcd_found	= cb_rcd_found;
		neardalMgr->cb_rcd_found_ud	= user_data;
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
void neardal_start_poll(neardal_t neardalMgr, char *adpName, errorCode_t *ec)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;

	if (neardalMgr == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalMgr, adpName,
					  &adpProp);

	err = NEARDAL_ERROR_NO_ADAPTER;
	if (adpProp == NULL)
		goto exit;

	if (adpProp->proxy == NULL)
		goto exit;

	if (!adpProp->polling) {
		org_neard_adp__call_start_poll_sync(adpProp->proxy, NULL, 
						&neardalMgr->gerror);

		err = NEARDAL_SUCCESS;
		if (neardalMgr->gerror != NULL) {
			NEARDAL_TRACE_ERR(
				"Error with neard dbus method (err:%d:'%s')\n"
					, neardalMgr->gerror->code
					, neardalMgr->gerror->message);
			err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;
			neardal_tools_prv_free_gerror(neardalMgr);
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
void neardal_stop_poll(neardal_t neardalMgr, char *adpName, errorCode_t *ec)
{
	errorCode_t	err = NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;

	if (neardalMgr == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalMgr, adpName,
						   &adpProp);

	if (adpProp == NULL)
		goto exit;

	if (adpProp->proxy == NULL)
		goto exit;

	if (adpProp->polling) {
		org_neard_adp__call_stop_poll_sync(adpProp->proxy, NULL,
						   &neardalMgr->gerror);

		err = NEARDAL_SUCCESS;
		if (neardalMgr->gerror != NULL) {
			NEARDAL_TRACE_ERR(
				"Error with neard dbus method (err:%d:'%s')\n"
					, neardalMgr->gerror->code
					, neardalMgr->gerror->message);
			err = NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR;
			neardal_tools_prv_free_gerror(neardalMgr);
		}
	}

exit:
	if (ec != NULL)
		*ec = err;
}

/******************************************************************************
 * neardal_publish: Write NDEF record to an NFC tag
 *****************************************************************************/
errorCode_t neardal_publish(neardal_t neardalMgr, neardal_record *record)
{
	errorCode_t	err	= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp;
	RcdProp		rcd;

	if (neardalMgr == NULL || record == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalMgr, record->name,
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
errorCode_t neardal_get_adapter_properties(neardal_t neardalMgr,
					   const char *adpName,
					   neardal_adapter *adapter)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;
	TgtProp		*target		= NULL;
	int		ct		= 0;	/* counter */
	gsize		size;

	if (neardalMgr == NULL || adpName == NULL || adapter == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalMgr, adpName,
					  &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	adapter->name		= adpProp->name;
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
				adapter->protocols[ct++] = tmp;
			}
			err = NEARDAL_SUCCESS;
		}
	}

	adapter->nbTargets	= adpProp->tgtNb;
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
errorCode_t neardal_get_target_properties(neardal_t neardalMgr,
					  const char *tgtName,
					  neardal_target *target)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	int		ct		= 0;	/* counter */
	RcdProp		*record		= NULL;
	gsize		size;

	if (neardalMgr == NULL || tgtName == NULL || target == NULL)
		goto exit;

	target->records	= NULL;
	target->tagType	= NULL;
	err = neardal_mgr_prv_get_adapter(neardalMgr, tgtName,
						   &adpProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	err = neardal_mgr_prv_get_target(adpProp, tgtName, &tgtProp);
	if (err != NEARDAL_SUCCESS)
		goto exit;

	target->name		= tgtProp->name;
	target->type		= tgtProp->type;
	target->readOnly	= tgtProp->readOnly;
	target->nbRecords	= tgtProp->rcdLen;
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
	target->nbTagTypes = tgtProp->tagTypeLen;

	if (target->nbTagTypes <= 0)
		goto exit;

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
errorCode_t neardal_get_record_properties(neardal_t neardalMgr,
					  const char *recordName,
					  neardal_record *record)
{
	errorCode_t	err		= NEARDAL_ERROR_INVALID_PARAMETER;
	AdpProp		*adpProp	= NULL;
	TgtProp		*tgtProp	= NULL;
	RcdProp		*rcdProp	= NULL;

	if (neardalMgr == NULL || recordName == NULL || record == NULL)
		goto exit;

	err = neardal_mgr_prv_get_adapter(neardalMgr, recordName,
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
	neardal_t	neardalMgr	= NULL;
	errorCode_t	err	= NEARDAL_ERROR_NO_MEMORY;

	NEARDAL_TRACEIN();
	/* Allocate NEARDAL context */
	neardalMgr = g_try_malloc0(sizeof(neardalCtx));
	if (neardalMgr == NULL)
		goto exit;

	/* Create DBUS connection */
	g_type_init();
	neardalMgr->conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL,
					   &neardalMgr->gerror);
	if (neardalMgr->conn != NULL) {
		/* We have a DBUS connection, create proxy on Neard Manager */
		err =  neardal_mgr_create(neardalMgr);
		if (err != NEARDAL_SUCCESS) {
			NEARDAL_TRACEF(
				"neardal_mgr_create() exit (err %d: %s)\n",
				err, neardal_error_get_text(err));

			/* No Neard daemon, destroying neardal object... */
			if (err == NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY) {
				neardal_tools_prv_free_gerror(neardalMgr);
				g_free(neardalMgr);
				neardalMgr = NULL;
			}
		}
	} else {
		NEARDAL_TRACE_ERR("Unable to connect to dbus: %s\n",
				 neardalMgr->gerror->message);
		neardal_tools_prv_free_gerror(neardalMgr);
		err = NEARDAL_ERROR_DBUS;
		g_free(neardalMgr);
		neardalMgr = NULL;
	}

exit:
	if (ec != NULL)
		*ec = err;

	NEARDAL_TRACEF("Exit\n");
	return neardalMgr;
}


/******************************************************************************
 * neardal_destroy: destroy NEARDAL object instance, Disconnect Neard Dbus
 * connection, unregister Neard's events
 *****************************************************************************/
void neardal_destroy(neardal_t neardalMgr)
{
	NEARDAL_TRACEIN();
	if (neardalMgr != NULL) {
		neardal_tools_prv_free_gerror(neardalMgr);
		neardal_mgr_destroy(&neardalMgr);
	}
}
