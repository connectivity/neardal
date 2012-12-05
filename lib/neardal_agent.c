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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "neardal.h"
#include "neardal_prv.h"
#include "neardal_ndef_agent.h"

static gboolean neardal_agent_prv_remove(gchar *objPath)
{
	g_assert(objPath != NULL);
	
	NEARDAL_TRACEIN();
	return g_dbus_object_manager_server_unexport(neardalMgr.agentMgr
						     , objPath);
}

static gboolean on_get_ndef(neardalNDEFAgent		*ndefAgent
			    , GDBusMethodInvocation	*invocation
			    , GVariant			*out
			    , gpointer			user_data)
{
	neardal_agent_t *agent_data = user_data;
	gchar		**rcdArray	= NULL;
	gsize		rcdLen		= 0;
	gchar		*ndefArray	= NULL;
	gsize		ndefLen		= 0;
	gconstpointer  value;

	(void) ndefAgent;	/* Avoid warning */
	(void) invocation;	/* Avoid warning */
	
	NEARDAL_TRACEIN();
	NEARDAL_TRACEF("%s\n", g_variant_print(out, TRUE));

	
	if (agent_data != NULL) {
		NEARDAL_TRACEF("ndefAgent pid=%d, obj path is : %s\n"
			      , agent_data->pid
			      , agent_data->objPath);
		if (agent_data->cb_agent != NULL) {
			GVariant	*tmpOut		= NULL;

			tmpOut = g_variant_lookup_value(out, "Records",
							G_VARIANT_TYPE_ARRAY);
			if (tmpOut != NULL) {
				rcdArray = (gchar**) g_variant_dup_strv(tmpOut
								, &rcdLen);
				
				if (rcdLen == 0) {
					g_strfreev(rcdArray);
					rcdArray = NULL;
				}
			}
			tmpOut = g_variant_lookup_value(out, "NDEF",
							G_VARIANT_TYPE_ARRAY);
			if (tmpOut != NULL) {
				value = g_variant_get_data(tmpOut);
				ndefLen = g_variant_get_size(tmpOut);

				if (ndefLen > 0) {
					ndefArray = g_try_malloc0(ndefLen);
					if (ndefArray != NULL)
						memcpy(ndefArray, value
						      , ndefLen);
				}
			}
			(agent_data->cb_agent)(
					(unsigned char **) rcdArray
					, rcdLen
					, (unsigned char *) ndefArray
					, ndefLen
					, agent_data->user_data);
		}
	}
		
//	neardal_ndefagent_complete_get_ndef(ndefAgent, invocation, out);
	return TRUE;
}

static gboolean on_release( neardalNDEFAgent		*ndefAgent
			   , GDBusMethodInvocation	*invocation
			   , gpointer			user_data)
{
	neardal_agent_t	*agent_data = user_data;
	NEARDAL_TRACEIN();

	if (invocation != NULL)
		neardal_ndefagent_complete_release(ndefAgent, invocation);
	if (agent_data != NULL) {
		NEARDAL_TRACEF("agent '%s' ",agent_data->objPath);

		if (neardal_agent_prv_remove(agent_data->objPath) == TRUE)
			NEARDAL_TRACE("removed\n");
		else
			NEARDAL_TRACE("not removed!\n");
		g_free(agent_data->objPath);
		g_free(agent_data->tagType);
		g_free(agent_data);
	}

	return TRUE;
}

static void on_object_removed( GDBusObjectManager *manager
			      , GDBusObject        *object
			      , gpointer            user_data)
{
	NEARDAL_TRACEIN();
	(void) manager; // avoid warning
	on_release( NEARDAL_NDEFAGENT(object), NULL, user_data);
}

static void
on_name_acquired (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	(void) connection;	// avoid warning
	(void) name;		// avoid warning
	(void) user_data;	// avoid warning
	NEARDAL_TRACEIN();
	NEARDAL_TRACE_LOG(":%s\n", name);
}

static void
on_name_lost (GDBusConnection *connection,
                 const gchar     *name,
                 gpointer         user_data)
{
	(void) connection;	// avoid warning
	(void) name;		// avoid warning
	(void) user_data;	// avoid warning
	NEARDAL_TRACEIN();
	NEARDAL_TRACE_LOG(":%s\n", name);
}

/*****************************************************************************
 * neardal_ndefagent_prv_manage: create or release an agent and register or
 * unregister it with neardal object manager and Neard
 ****************************************************************************/
errorCode_t neardal_ndefagent_prv_manage(neardal_agent_t agentData)
{
	errorCode_t		err = NEARDAL_SUCCESS;
	neardalObjectSkeleton	*objSkel;
	neardalNDEFAgent	*ndefAgent;
	neardal_agent_t		*data;

	NEARDAL_TRACEIN();

	if (agentData.cb_agent != NULL) {
		data = g_try_malloc0(sizeof(neardal_agent_t));
		if (data == NULL)
			return NEARDAL_ERROR_NO_MEMORY;

		memcpy(data, &agentData, sizeof(neardal_agent_t));
		data->objPath = g_strdup(agentData.objPath);
		data->tagType = g_strdup(agentData.tagType);

		NEARDAL_TRACEF("Create agent '%s'\n", data->objPath);
		objSkel = neardal_object_skeleton_new (data->objPath);

		ndefAgent = neardal_ndefagent_skeleton_new();
		neardal_object_skeleton_set_ndefagent(objSkel, ndefAgent);

		/* Handle Poke() D-Bus method invocations on the .Animal interface */
		g_signal_connect ( ndefAgent, "handle-get-ndef"
				, G_CALLBACK (on_get_ndef)
				, data);

		g_signal_connect ( ndefAgent, "handle-release"
				, G_CALLBACK (on_release), data);

		g_signal_connect ( neardalMgr.agentMgr, "object-removed"
				, G_CALLBACK (on_object_removed), data);
		g_object_unref (ndefAgent);

		/* Export the object (@manager takes its own reference to @object) */
		g_dbus_object_manager_server_export (neardalMgr.agentMgr
					, G_DBUS_OBJECT_SKELETON (objSkel));
		g_object_unref (objSkel);
	} else {
		NEARDAL_TRACEF("Release agent '%s'\n", agentData.objPath);
		if (neardal_agent_prv_remove(agentData.objPath) == TRUE)
			err = NEARDAL_SUCCESS;
		else
			err = NEARDAL_ERROR_DBUS;
	}

	return err;
}

/*****************************************************************************
 * neardal_ndefagent_prv_release: unregister an agent from Neard and neardal
 * object manager
 ****************************************************************************/
errorCode_t neardal_ndefagent_prv_release(gchar *objPath)
{
	errorCode_t		err = NEARDAL_SUCCESS;

	if (neardal_agent_prv_remove(objPath) == TRUE)
		NEARDAL_TRACE("removed\n");
	else
		NEARDAL_TRACE("not removed!\n");
		
	return err;
}

/*****************************************************************************
 * neardal_agent_acquire_dbus_name: acquire dbus name for management of neard
 *  agent feature
 ****************************************************************************/
errorCode_t neardal_agent_acquire_dbus_name(void)
{
	errorCode_t			err = NEARDAL_SUCCESS;

	NEARDAL_TRACEIN();
	if (neardalMgr.conn == NULL)
		return NEARDAL_ERROR_DBUS;

	neardalMgr.OwnerId = g_bus_own_name_on_connection(neardalMgr.conn
				, NEARDAL_DBUS_WELLKNOWN_NAME
				, G_BUS_NAME_OWNER_FLAGS_ALLOW_REPLACEMENT |
				  G_BUS_NAME_OWNER_FLAGS_REPLACE
				, on_name_acquired	// on_name_acquired
				, on_name_lost		// on_name_lost
				, NULL			// user data
				, NULL);		// freeing user_data func
	
	if (neardalMgr.OwnerId == 0) {
		err = NEARDAL_ERROR_DBUS;
		goto exit;
	}
	
	/* Create a new org.freedesktop.DBus.ObjectManager rooted at /example/Animals */
	neardalMgr.agentMgr = g_dbus_object_manager_server_new (NEARDAL_AGENT_PREFIX);
	if (neardalMgr.agentMgr == NULL) {
		err = NEARDAL_ERROR_DBUS;
		goto exit;
	}
	
	/* Export all objects */
	g_dbus_object_manager_server_set_connection (neardalMgr.agentMgr, neardalMgr.conn);
	

exit:
	if (err != NEARDAL_SUCCESS)
		NEARDAL_TRACE_ERR("(%d:%s)\n", err
				  , neardal_error_get_text(err));
		
	return err;
}

/*****************************************************************************
 * neardal_agent_stop_owning_dbus_name: Stops owning a dbus name
 ****************************************************************************/
void neardal_agent_stop_owning_dbus_name(void)
{
	NEARDAL_TRACEIN();
	if (neardalMgr.OwnerId > 0)
		g_bus_unown_name (neardalMgr.OwnerId);
	neardalMgr.OwnerId = 0;
	
}
