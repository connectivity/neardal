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

#include "neard_manager_proxy.h"

#include "neardal_agent.h"
#include "neardal_manager.h"
#include "neardal_tools.h"
#include "neardal_traces_prv.h"
#include "neardal.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/* NEARDAL Callbacks */
typedef struct {
	adapter_cb	adp_added;		/* Client callback for
							'adapter added' */
	void		*adp_added_ud;		/* User data for
							client callback
							'adapter added'*/
	adapter_cb	adp_removed;		/* Client callback for
							'adapter removed' */
	void		*adp_removed_ud;	/* User data for
							client callback
							'adapter removed'*/
	adapter_prop_cb	adp_prop_changed;	/* Client callback for
						'adapter property changed' */
	void		*adp_prop_changed_ud;	/* User data for
							client callback
						'adapter property changed' */

	tag_cb		tag_found;		/* Client callback for
							'tag found' */
	void		*tag_found_ud;		/* User data for
							client callback
							'tag found' */
	tag_cb		tag_lost;		/* Client callback for
							'tag lost' */
	void		*tag_lost_ud;		/* User data for
							client callback
							'tag lost' */

	dev_cb		dev_found;		/* Client callback for
							'device found' */
	void		*dev_found_ud;		/* User data for
							client callback
							'device found' */
	dev_cb		dev_lost;		/* Client callback for
							'device lost' */
	void		*dev_lost_ud;		/* User data for
							client callback
							'device lost' */

	record_cb	rcd_found;		/* Client callback for
						'	'tag record found'*/
	void		*rcd_found_ud;		/* User data for
							client callback
							'tag record found'*/
} neardalCb;

/* NEARDAL context */
typedef struct {
	neardalCb	cb;			/* Neardal Callbacks */
	GDBusConnection	*conn;			/* DBus connection */
	orgNeardMgr	*proxy;			/* Neard Mgr dbus proxy */
	MgrProp		prop;			/* Mgr Properties
							(adapter list) */
	guint		OwnerId;		/* dbus Id server side */
						/* (for neard agent Mgnt) */
	GDBusObjectManagerServer *agentMgr;	/* Object 'agent' Manager */

	errorCode_t	ec;		/* Lastest NEARDAL error */
	GError		*gerror;	/* Lastest GError if available */
} neardalCtx;

extern neardalCtx neardalMgr;

// DBUS TYPE
#define NEARDAL_DBUS_TYPE				G_BUS_TYPE_SYSTEM

// The well-known name to own
#define NEARDAL_DBUS_WELLKNOWN_NAME			"org.neardal"

/*! \fn neardal_t neardal_prv_construct(errorCode_t *ec)
*  \brief create NEARDAL object instance, Neard Dbus connection,
* register Neard's events
*  \param ec : optional, pointer to store error code
*  \return the NEARDAL context
*/
void neardal_prv_construct(errorCode_t *ec);



#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_PRV_H */
