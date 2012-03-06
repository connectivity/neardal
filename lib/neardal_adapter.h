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

#include "neard_adapter_proxy.h"
#include "neardal_target.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_ADP_IF_NAME				"org.neard.Adapter"
#define NEARD_ADP_SIG_PROPCHANGED			"property-changed"
#define NEARD_ADP_SIG_TGT_FOUND				"target-found"
#define NEARD_ADP_SIG_TGT_LOST				"target-lost"

/* NEARDAL Adapter Properties */
typedef struct {
	orgNeardAdp		*proxy;		/* The proxy connected to Neard
						Adapter interface */
	gchar			*name;		/* DBus interface name
						(as id) */
	neardal_t		parent;
	gboolean		polling;	/* adapter polling active ? */
	gboolean		powered;	/* adapter powered ? */
	gchar			**protocols;	/* protocols list */
	gsize			lenProtocols;
	gsize			tgtNb;
	GList			*tgtList;	/* Neard adapter targets list
						available */
} AdpProp;

/******************************************************************************
 * neardal_adp_prv_get_target: Get NEARDAL target from name
 *****************************************************************************/
errorCode_t neardal_adp_prv_get_target(AdpProp *adpProp, gchar *tgtName,
				       TgtProp **tgtProp);

/******************************************************************************
 * neardal_adp_add: add new NEARDAL adapter, initialize DBus Proxy
 * connection, register adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_add(gchar *adapterName);

/******************************************************************************
 * neardal_adp_remove: remove NEARDAL adapter, unref DBus Proxy
 * connection, unregister adapter signal
 *****************************************************************************/
errorCode_t neardal_adp_remove(AdpProp *adpProp);

/******************************************************************************
 * neardal_adp_publish: Creates and publish NDEF record to be written to
 * an NFC tag
 *****************************************************************************/
errorCode_t neardal_adp_publish(AdpProp *adpProp, RcdProp *rcd);


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_ADAPTER_H */
