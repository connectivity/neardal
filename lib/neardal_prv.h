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

#ifndef __NEARDAL_PRV_H
#define __NEARDAL_PRV_H

#include "neardal_manager.h"
#include "neardal_tools.h"
#include "neardal_traces_prv.h"
#include "neardal.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/* GType for Dict(string, variant) */
#define DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH    (dbus_g_type_get_collection( \
						"GPtrArray", \
						DBUS_TYPE_G_OBJECT_PATH))

/* NEARDAL context */
typedef struct neardalCtx {
	DBusGConnection	*conn;			/* The DBus connection */
	DBusGProxy	*mgrProxy;		/* dbus proxy to Neard Mgr */
	MgrProp		mgrProp;		/* Mgr Properties
						(adapter list) */
	adapter_cb	cb_adp_added;		/* Client callback for
						'NEARDAL adapter added' */
	void		*cb_adp_added_ud;	/* User data for client
						callback 'adapter added'*/
	adapter_cb	cb_adp_removed;		/* Client callback for
						'NEARDAL adapter removed' */
	void		*cb_adp_removed_ud;	/* User data for client
						callback 'adapter removed'*/
	adapter_prop_cb	cb_adp_prop_changed;	/* Client callback for
						'adapter property changed' */
	void		*cb_adp_prop_changed_ud;/* User data for client callback
						'adapter property changed' */

	target_cb	cb_tgt_found;		/* Client callback for
						'NEARDAL target found' */
	void		*cb_tgt_found_ud;	/* User data for client callback
						'NEARDAL target found' */
	target_cb	cb_tgt_lost;		/* Client callback for
						'NEARDAL target lost' */
	void		*cb_tgt_lost_ud;	/* User data for client callback
						'NEARDAL target lost' */

	record_cb	cb_rcd_found;		/* Client callback for
						'NEARDAL target record found'*/
	void		*cb_rcd_found_ud;	/* User data for client callback
						'NEARDAL target record found'*/

	errorCode_t	ec;			/* The last NEARDAL Library
						error code */
	GError		*gerror;		/* The last GError when
						available */
} neardalCtx;

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_PRV_H */
