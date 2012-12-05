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
#include "neardal_device.h"
#include "neardal_tag.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_ADP_SIG_PROPCHANGED			"property-changed"
#define NEARD_ADP_SIG_TAG_FOUND				"tag-found"
#define NEARD_ADP_SIG_TAG_LOST				"tag-lost"

/* NEARDAL Adapter Properties */
typedef struct {
	orgNeardAdp		*proxy;		/* The proxy connected to Neard
						Adapter interface */
	gchar			*name;		/* DBus interface name
						(as id) */
	void			*parent;
	gboolean		polling;	/* adapter polling active ? */
	gboolean		powered;	/* adapter powered ? */
	gchar			**protocols;	/* protocols list */
	gsize			lenProtocols;
	gsize			tagNb;
	GList			*tagList;	/* Neard adapter tags list
						available */
	gsize			devNb;
	GList			*devList;	/* Neard adapter devices list
						available */
} AdpProp;

/*****************************************************************************
 * neardal_adp_prv_get_tag: Get NEARDAL tag from name
 ****************************************************************************/
errorCode_t neardal_adp_prv_get_tag(AdpProp * adpProp, gchar *tagName,
				       TagProp * *tagProp);

/*****************************************************************************
 * neardal_adp_prv_get_dev: Get NEARDAL dev from name
 ****************************************************************************/
errorCode_t neardal_adp_prv_get_dev(AdpProp * adpProp, gchar *devName,
				       DevProp * *devProp);

/*****************************************************************************
 * neardal_adp_add: add new NEARDAL adapter, initialize DBus Proxy
 * connection, register adapter signal
 ****************************************************************************/
errorCode_t neardal_adp_add(gchar *adapterName);

/*****************************************************************************
 * neardal_adp_remove: remove NEARDAL adapter, unref DBus Proxy
 * connection, unregister adapter signal
 ****************************************************************************/
errorCode_t neardal_adp_remove(AdpProp *adpProp);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_ADAPTER_H */
