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

#ifndef __NEARDAL_MANAGER_H
#define __NEARDAL_MANAGER_H

#include "neardal_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_DBUS_SERVICE			"org.neard"
#define NEARD_MGR_PATH				"/"
#define NEARD_MGR_IF_NAME			"org.neard.Manager"
#define NEARD_MGR_SECTION_ADAPTERS		"Adapters"
#define NEARD_MGR_SIG_PROPCHANGED		"PropertyChanged"
#define NEARD_MGR_SIG_ADP_ADDED			"AdapterAdded"
#define NEARD_MGR_SIG_ADP_RM			"AdapterRemoved"

/* NEARDAL Manager Properties */
typedef struct {
	GList	*adpList;	/* List of available adapter (AdpProp*) */
} MgrProp;

/******************************************************************************
 * neardal_mgr_prv_get_adapter: Get NEARDAL Adapter from name
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_adapter(gchar *adpName, AdpProp **adpProp);

/******************************************************************************
 * neardal_mgr_prv_get_adapter_from_proxy: Get NEARDAL Adapter from proxy
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_adapter_from_proxy(DBusGProxy * adpProxy,
							AdpProp **adpProp);

/******************************************************************************
 * neardal_mgr_prv_get_tag: Get specific tag from adapter
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_tag(AdpProp *adpProp, gchar *tagName,
				       TagProp * *tagProp);

/******************************************************************************
 * neardal_mgr_prv_get_record: Get specific record from tag
 *****************************************************************************/
errorCode_t neardal_mgr_prv_get_record(TagProp * tagProp, gchar *rcdName,
				       RcdProp * *rcdProp);

/******************************************************************************
 * neardal_mgr_create: Get Neard Manager Properties = NEARDAL Adapters list.
 * Create a DBus proxy for the first one NEARDAL adapter if present
 * Register Neard Manager signals ('PropertyChanged')
 *****************************************************************************/
errorCode_t neardal_mgr_create(void);

/******************************************************************************
 * neardal_mgr_destroy: unref DBus proxy, disconnect Neard Manager signals
 *****************************************************************************/
void neardal_mgr_destroy(void);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_MANAGER_H */
