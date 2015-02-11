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

#ifndef NEARDAL_AGENT_H
#define NEARDAL_AGENT_H

#include "neardal.h"

#define AGENT_PREFIX	"/neardal"


typedef struct {
	gchar			*objPath;		/* agent object path */
	gchar			*tagType;		/* tag Type to register
							(for NDEF agent only )
							*/

	gint			pid;			/* process pid */

	ndef_agent_cb		cb_ndef_agent;		/* client callback to
							retrieve raw NDEF data
							and records object path
							*/

	ndef_agent_free_cb	cb_ndef_release_agent;	/* client callback gets
							called when Neard
							unregisters the agent.
							Can be used to cleanup
							tasks. There is no need
							to unregister the agent,
							because when this
							callback gets called it
							has already been
							unregistered.*/
	gpointer		user_data;
} neardal_ndef_agent_t;

typedef struct {
	gchar			*objPath;		/* agent object path */
	gchar			*carrierType;	/* carrier type: "wifi" or "bluetooth" */
	gint			pid;			/* process pid */

	oob_req_agent_cb	cb_oob_req_agent;	/* client callback to
							get Out Of Band data
							from the handover agent
							*/

	oob_push_agent_cb	cb_oob_push_agent;	/* client callback to
							pass remote Out Of Band
							data to agent to start
							handover */

	oob_agent_free_cb	cb_oob_release_agent;	/* client callback gets
							called when Neard
							unregisters the agent.
							Can be used to cleanup
							tasks. There is no need
							to unregister the agent,
							because when this
							callback gets called it
							has already been
							unregistered.*/
							
	gpointer		user_data;
} neardal_handover_agent_t;

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
 * unregister it with neardal object manager and Neard for NDEF data
 ****************************************************************************/
errorCode_t neardal_ndefagent_prv_manage(neardal_ndef_agent_t agentData);

/*****************************************************************************
 * neardal_handoveragent_prv_manage: create or release an agent and register
 * or unregister it with neardal object manager and Neard for handover message
 * (request / answer)
 ****************************************************************************/
errorCode_t neardal_handoveragent_prv_manage(
					neardal_handover_agent_t agentData);

#endif /* NEARDAL_AGENT_H */
