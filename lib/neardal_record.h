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

#ifndef NEARDAL_RECORD_H
#define NEARDAL_RECORD_H

#include "neard_record_proxy.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* NEARDAL Record Properties */
typedef struct {
	orgNeardRcd	*proxy;	/* proxy to Neard NFC Record interface */
	gchar		*name;	/* DBus interface name (as identifier) */
	void		*parent; /* parent (tag) */
	gboolean	notified; /* Already notified to client? */

	gchar		*encoding;
	gboolean	handOver;
	gchar		*language;
	gboolean	smartPoster;
	gchar		*action;
	gchar		*type;
	gchar		*representation;
	gchar		*uri;
	gsize		uriObjSize;
	gchar		*mime;
} RcdProp;

/*****************************************************************************
 * neardal_rcd_add: add new NFC record, initialize DBus Proxy connection,
 * register record signal
 *****************************************************************************/
errorCode_t neardal_rcd_add(char *rcdName, void *parent);

/*****************************************************************************
 * neardal_rcd_remove: remove NFC record, unref DBus Proxy connection,
 * unregister record signal
 *****************************************************************************/
void neardal_rcd_remove(RcdProp *rcdProp);

/*****************************************************************************
 * neardal_rcd_prv_format: Insert key/value in a GHashTable
 *****************************************************************************/
errorCode_t neardal_rcd_prv_format(GVariantBuilder *builder, RcdProp *rcd);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* NEARDAL_RECORD_H */
