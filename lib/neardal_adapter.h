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

#ifndef __NEARDAL_ADAPTER_H
#define __NEARDAL_ADAPTER_H

#include "neardal_target.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_ADAPTERS_IF_NAME				"org.neard.Adapter"
#define NEARD_ADP_SIG_PROPCHANGED			"PropertyChanged"
#define NEARD_ADP_SIG_TGT_FOUND				"TargetFound"
#define NEARD_ADP_SIG_TGT_LOST				"TargetLost"

/* NEARDAL Adapter Properties */
typedef struct {
	DBusGProxy		*dbusProxy;	/* The proxy connected to Neard
						Adapter interface */
	char			*name;		/* DBus interface name
						(as id) */
	gboolean		polling;	/* adapter polling active ? */
	gboolean		powered;	/* adapter powered ? */
	char			**protocols;	/* protocols list */
	GPtrArray		*tgtArray;	/* temporary storage */
	GList			*tgtList;	/* Neard adapter targets list
						available */
} AdpProp;

/******************************************************************************
 * neardal_adp_prv_get_target: Get NEARDAL target from name
 *****************************************************************************/
errorCode_t neardal_adp_prv_get_target(AdpProp *adpProp, char *tgtName,
				       TgtProp **tgtProp);

/******************************************************************************
 * neardal_adp_add: add new NEARDAL adapter, initialize DBus Proxy
 * connection, register adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_add(neardal_t neardalObj, char *adapterName);

/******************************************************************************
 * neardal_adp_remove: remove NEARDAL adapter, unref DBus Proxy
 * connection, unregister adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_remove(neardal_t neardalObj, AdpProp *adpProp);

/******************************************************************************
 * neardal_adp_publish: Creates and publish NDEF record to be written to
 * an NFC tag
 *****************************************************************************/
errorCode_t neardal_adp_publish(AdpProp *adpProp, RcdProp *rcd);


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_ADAPTER_H */
