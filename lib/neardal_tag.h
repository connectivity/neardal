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

#ifndef __NEARDAL_TAG_H
#define __NEARDAL_TAG_H

#include "neard_tag_proxy.h"
#include "neardal_record.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_TAGS_IF_NAME		"org.neard.Tag"
#define NEARD_TAG_SIG_PROPCHANGED	"PropertyChanged"

/* NEARDAL Tag Properties */
typedef struct {
	DBusGProxy	*proxy;	  /* proxy to Neard NEARDAL Tag interface */
	gchar		*name;	  /* DBus interface name (as identifier) */
	void		*parent;  /* parent (adapter ) */
	gboolean	notified; /* Already notified to client? */

	gchar		*type;

	GPtrArray	*rcdArray;	/* temporary storage */
	gsize		rcdLen;
	GList		*rcdList;	/* tag's records paths */

	gchar		**tagType;	/* array of tag types */
	gsize		tagTypeLen;
	gboolean	readOnly;	/* Read-Only flag */
} TagProp;

/******************************************************************************
 * neardal_tag_notify_tag_found: Invoke client callback for 'record found'
 * if present, and 'tag found' (if not already nofied)
 *****************************************************************************/
void neardal_tag_notify_tag_found(TagProp *tagProp);

/******************************************************************************
 * neardal_tag_add: add new NEARDAL tag, initialize DBus Proxy connection,
 * register tag signal
 *****************************************************************************/
errorCode_t neardal_tag_add(gchar *tagName, void * parent);

/******************************************************************************
 * neardal_tag_remove: remove NEARDAL tag, unref DBus Proxy connection,
 * unregister tag signal
 *****************************************************************************/
void neardal_tag_remove(TagProp *tagProp);

/******************************************************************************
 * neardal_tag_write: Creates and write NDEF record to be written to
 * an NFC tag
 *****************************************************************************/
errorCode_t neardal_tag_write(TagProp *tagProp, RcdProp *rcd);



#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_TAG_H */
