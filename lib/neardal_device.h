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

#ifndef __NEARDAL_DEV_H
#define __NEARDAL_DEV_H

#include "neard_device_proxy.h"
#include "neardal_record.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_DEVS_IF_NAME		"org.neard.Dev"
#define NEARD_DEV_SIG_PROPCHANGED	"property-changed"

/* NEARDAL Dev Properties */
typedef struct {
	orgNeardDev	*proxy;	  /* proxy to Neard NEARDAL Dev interface */
	gchar		*name;	  /* DBus interface name (as identifier) */
	void		*parent;  /* parent (adapter ) */
	gboolean	notified; /* Already notified to client? */

	gsize		rcdLen;
	GList		*rcdList;	/* dev's records paths */
} DevProp;

/*****************************************************************************
 * neardal_dev_notify_dev_found: Invoke client callback for 'record found'
 * if present, and 'dev found' (if not already nofied)
 ****************************************************************************/
void neardal_dev_notify_dev_found(DevProp *devProp);

/******************************************************************************
 * neardal_dev_prv_add: add new NEARDAL dev, initialize DBus Proxy connection,
 * register dev signal
 *****************************************************************************/
errorCode_t neardal_dev_prv_add(gchar *devName, void *parent);

/******************************************************************************
 * neardal_dev_remove: remove NEARDAL dev, unref DBus Proxy connection,
 * unregister dev signal
 *****************************************************************************/
void neardal_dev_prv_remove(DevProp *devProp);

/******************************************************************************
 * neardal_dev_prv_push: Creates and write NDEF record to be written to
 * an NFC dev
 *****************************************************************************/
errorCode_t neardal_dev_prv_push(DevProp *devProp, RcdProp *rcd);



#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_DEV_H */
