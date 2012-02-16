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

#ifndef __NEARDAL_TARGET_H
#define __NEARDAL_TARGET_H

#include "neard_target_proxy.h"
#include "neardal_record.h"
// #include <glib-2.0/glib/gtypes.h>

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define NEARD_TARGETS_IF_NAME		"org.neard.Target"
#define NEARD_TGT_SIG_PROPCHANGED	"property-changed"

/* NEARDAL Target Properties */
typedef struct {
	orgNeardTgt	*proxy;		/* proxy to Neard NEARDAL Target
					interface */
	char		*name;		/* DBus interface name
					(as identifier) */
	void		*parent;	/* parent (adapter ) */
	
	gchar		*type;
	
	gsize		rcdLen;
	GList		*rcdList;	/* target's records paths */
	
	gchar		**tagsType;	/* array of tag types */
	gsize		tagsTypeLen;
	gboolean	readOnly;	/* Read-Only flag */
} TgtProp;

/******************************************************************************
 * neardal_tgt_add: add new NEARDAL target, initialize DBus Proxy connection,
 * register target signal
 *****************************************************************************/
errorCode_t neardal_tgt_add(neardal_t neardalMgr, void *parent,
			    gchar *tgtName);

/******************************************************************************
 * neardal_tgt_remove: remove NEARDAL target, unref DBus Proxy connection,
 * unregister target signal
 *****************************************************************************/
void neardal_tgt_remove(TgtProp *tgtProp);


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_TARGET_H */
