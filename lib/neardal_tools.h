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

#ifndef __NEARDAL_TOOLS_H
#define __NEARDAL_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/******************************************************************************
 * neardal_tools_prv_free_gerror: freeing gerror in neardal context
 *****************************************************************************/
void neardal_tools_prv_free_gerror(GError **gerror);

/******************************************************************************
 * neardal_tools_prv_cmp_path: Compare dbus path.
 * return true (<>0) if path is same, 0 otherwise
 *****************************************************************************/
int neardal_tools_prv_cmp_path(const char *neardalPath, const char *reqPath);

/******************************************************************************
 * neardal_tools_prv_hashtable_get: Parse a hashtable and get value of GType
 * 'type' with a specific key
 *****************************************************************************/
errorCode_t neardal_tools_prv_hashtable_get(GHashTable *hashTable,
					gconstpointer key, GType gtype,
					void *value);

/******************************************************************************
 * neardal_tools_prv_create_proxy: create dbus proxy to Neard daemon
 *****************************************************************************/
errorCode_t neardal_tools_prv_create_proxy(DBusGConnection *conn,
				       DBusGProxy **oProxy, const char *path,
				       const char *iface);

/******************************************************************************
 * neardal_marshal_VOID__STRING_BOXED: marshaller function for signal
 * invocations
 *****************************************************************************/
void neardal_marshal_VOID__STRING_BOXED(GClosure	*closure,
					 GValue		*return_value,
					 guint		n_param_values,
					 const GValue	*param_values,
					 gpointer	invocation_hint,
					 gpointer	marshal_data);

/******************************************************************************
 * neardal_tools_prv__g_ptr_array_copy: duplicate a 'GPtrArray' array
 *****************************************************************************/
void neardal_tools_prv_g_ptr_array_copy(GPtrArray **dest, GPtrArray *source);

/******************************************************************************
 * neardal_tools_prv_g_ptr_array_free: free a 'GPtrArray' array
 *****************************************************************************/
void neardal_tools_prv_g_ptr_array_free(GPtrArray *array);

/******************************************************************************
 * neardal_tools_create_dict: Create a GHashTable for dict_entries.
 *****************************************************************************/
GHashTable *neardal_tools_create_dict(void);

/******************************************************************************
 * neardal_tools_add_dict_entry: add an entry in a dictionnary
 *****************************************************************************/
errorCode_t neardal_tools_add_dict_entry(GHashTable *hash, gchar *key,
					  gchar *value);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_TOOLS_H */
