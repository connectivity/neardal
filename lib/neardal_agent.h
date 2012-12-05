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

#ifndef __NEARDAL_AGENT_H
#define __NEARDAL_AGENT_H

#include "neardal_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#define AGENT_PREFIX	"/neardal"


typedef struct {
	gchar		*objPath;	/* agent object path */
	gchar		*tagType;	/* tag Type to register */
	gint		pid;
	agent_cb	cb_agent;
	gpointer	user_data;
} neardal_agent_t;

/*****************************************************************************
 * neardal_agent_acquire_dbus_name: acquire dbus name for management of neard
 *  agent feature
 ****************************************************************************/
errorCode_t neardal_agent_acquire_dbus_name(void);

/*****************************************************************************
 * neardal_agent_stop_owning_dbus_name: Stops owning a dbus name
 ****************************************************************************/
void neardal_agent_stop_owning_dbus_name(void);

/*****************************************************************************
 * neardal_ndefagent_prv_manage: create or release an agent and register or
 * unregister it with neardal object manager
 ****************************************************************************/
errorCode_t neardal_ndefagent_prv_manage(neardal_agent_t agentData);


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_AGENT_H */
