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

#ifndef NEARDAL_TAG_H
#define NEARDAL_TAG_H

#include "neard_tag_proxy.h"
#include "neardal_record.h"

#define NEARD_TAG_SIG_PROPCHANGED	"property-changed"

/* NEARDAL Tag Properties */
typedef struct {
	OrgNeardTag	*proxy;	  /* proxy to Neard NEARDAL Tag interface */
	gchar		*name;	  /* DBus interface name (as identifier) */
	void		*parent;  /* parent (adapter ) */
	gboolean	notified; /* Already notified to client? */

	gchar		*type;

	gsize		rcdLen;
	GList		*rcdList;	/* tag's records paths */

	gchar		**tagType;	/* array of tag types */
	gsize		tagTypeLen;
	gboolean	readOnly;	/* Read-Only flag */

	GBytes		*iso14443aAtqa;	/* ISO14443A ATQA */
	GBytes		*iso14443aSak;	/* ISO14443A SAK */
	GBytes		*iso14443aUid;	/* ISO14443A UID */

	GBytes		*felicaManufacturer;	/* Felica Manufacturer info */
	GBytes		*felicaCid;	/* Felica CID */
	GBytes		*felicaIc;	/* Felica IC code */
	GBytes		*felicaMaxRespTimes;	/* Felica Max response times */
} TagProp;

/*****************************************************************************
 * neardal_tag_notify_tag_found: Invoke client callback for 'record found'
 * if present, and 'tag found' (if not already nofied)
 ****************************************************************************/
void neardal_tag_notify_tag_found(TagProp *tagProp);

/******************************************************************************
 * neardal_tag_prv_add: add new NEARDAL tag, initialize DBus Proxy connection,
 * register tag signal
 *****************************************************************************/
errorCode_t neardal_tag_prv_add(gchar *tagName, void *parent);

/******************************************************************************
 * neardal_tag_prv_remove: remove NEARDAL tag, unref DBus Proxy connection,
 * unregister tag signal
 *****************************************************************************/
void neardal_tag_prv_remove(TagProp *tagProp);

#endif /* NEARDAL_TAG_H */
