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
#include <glib-object.h>

#include "neard_manager_proxy.h"
#include "neard_adapter_proxy.h"

#include "neardal.h"
#include "neardal_prv.h"


#define g_marshal_value_peek_string(v)   (char *) g_value_get_string(v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed(v)

void
neardal_marshal_VOID__STRING_BOXED(GClosure *closure, GValue *return_value,
				    guint		n_param_values,
				    const GValue	*param_values,
				    gpointer		invocation_hint,
				    gpointer		marshal_data)
{
typedef void (*GMarshalFunc_VOID__STRING_BOXED)(gpointer data1, gpointer arg_1,
						gpointer arg_2, gpointer data2);

	register GMarshalFunc_VOID__STRING_BOXED callback;
	register gpointer			data1, data2;
	register GCClosure			*cc	= (GCClosure *)closure;
	GValue					*value	= NULL;

	g_return_if_fail(n_param_values == 3);
	(void) return_value; /* remove warning */
	(void) invocation_hint; /* remove warning */

	if (G_CCLOSURE_SWAP_DATA(closure)) {
		data1 = closure->data;
		data2 = g_value_peek_pointer(param_values + 0);
	} else {
		data1 = g_value_peek_pointer(param_values + 0);
		data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__STRING_BOXED) (marshal_data ?
	marshal_data : cc->callback);

	if (G_IS_VALUE(g_marshal_value_peek_boxed(param_values + 2)))
		value = g_marshal_value_peek_boxed(param_values + 2);

	callback(data1, g_marshal_value_peek_string(param_values + 1), value,
		 data2);
}

/******************************************************************************
 * neardal_tools_prv__g_ptr_array_copy: duplicate a 'GPtrArray' array
 *****************************************************************************/
static void neardal_tools_g_ptr_array_add(gpointer data, gpointer array)
{
	char *tmp;
	tmp = g_strdup(data);
	g_ptr_array_add(array, tmp);
}
void neardal_tools_prv_g_ptr_array_copy(GPtrArray **dest, GPtrArray *source)
{
	if (*dest == NULL)
		*dest = g_ptr_array_sized_new(source->len);
	g_ptr_array_foreach(source, neardal_tools_g_ptr_array_add, *dest);
}

/******************************************************************************
 * neardal_tools_prv_g_ptr_array_free: free a 'GPtrArray' array
 *****************************************************************************/
static void neardal_tools_g_ptr_array_free_node(gpointer data, gpointer array)
{
	(void) array; /* remove warning */
	g_free(data);
}
void neardal_tools_prv_g_ptr_array_free(GPtrArray *array)
{
	g_ptr_array_foreach(array, neardal_tools_g_ptr_array_free_node, NULL);
	g_ptr_array_free(array, TRUE);
}

/******************************************************************************
 * neardal_tools_prv_create_proxy: create dbus proxy to Neard daemon
 *****************************************************************************/
errorCode_t neardal_tools_prv_create_proxy(DBusGConnection *conn,
				       DBusGProxy **oProxy, const char *path,
				       const char *iface)
{
	DBusGProxy	*proxy	= NULL;
	GError		*gerror	= NULL;

	g_assert(conn != NULL);
	g_assert(oProxy != NULL);
	g_assert(path != NULL);
	g_assert(iface != NULL);

	proxy = dbus_g_proxy_new_for_name_owner(conn, NEARD_DBUS_SERVICE, path,
						iface, &gerror);
	if (proxy == NULL) {
		NEARDAL_TRACE_ERR(
		"Unable to connect to (path:'%s', interface:'%s') (err:'%s')\n",
				 path, iface, gerror->message);
		g_error_free(gerror);
		return NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY;
	}
	NEARDAL_TRACEF("Connection to (path:'%s', interface:'%s') OK!\n", path,
		      iface);
/*TODO g_signal_connect (G_OBJECT (proxy), "destroy",
 *			G_CALLBACK(destroy_signal), proxy); */

	*oProxy = proxy;

	return NEARDAL_SUCCESS;
}
/******************************************************************************
 * neardal_tools_prv_free_gerror: freeing gerror in neardal context
 *****************************************************************************/
void neardal_tools_prv_free_gerror(GError **gerror)
{
	if ((*gerror) != NULL)
		g_error_free((*gerror));
	(*gerror) = NULL;
}

/******************************************************************************
 * neardal_tools_prv_cmp_path: Compare dbus path.
 * return true (<>0) if path is same, 0 otherwise
 *****************************************************************************/
int neardal_tools_prv_cmp_path(const char *neardalPath, const char *reqPath)
{
	const char	*shortest;
	const char	*longest;
	int		len, lenNfcPath, lenReqPath;
	int		ret = FALSE;

	g_assert(neardalPath != NULL);
	g_assert(reqPath != NULL);

	lenNfcPath = strlen(neardalPath);
	lenReqPath = strlen(reqPath);
	if (lenNfcPath  > lenReqPath) {
		shortest	= reqPath;
		longest		= neardalPath;
		len		= lenReqPath;
	} else {
		shortest	= neardalPath;
		longest		= reqPath;
		len		= lenNfcPath;
	}

	if (!strncmp(shortest, longest, len)) {
		if (longest[len] == '/' || longest[len] == '\0')
			ret = TRUE;
		else
			ret = FALSE;
	}
	return ret;
}


/******************************************************************************
 * neardal_tools_prv_hashtable_get: Parse a hashtable and get value of GType
 * 'type' with a specific key
 *****************************************************************************/
errorCode_t neardal_tools_prv_hashtable_get(GHashTable *hashTable,
					gconstpointer key, GType gtype,
					void *value)
{
	gpointer	valueGp	= NULL;
	errorCode_t	err	= NEARDAL_ERROR_GENERAL_ERROR;
	gboolean	found;

	NEARDAL_TRACE("%s? ", key);
	found = g_hash_table_lookup_extended(hashTable, key, NULL, &valueGp);
	if (found == FALSE) {
		NEARDAL_TRACE("NOT FOUND!!!\n", key);
		return err;
	}

	if (valueGp == NULL)
		goto exit;

	if (G_VALUE_HOLDS(valueGp, gtype) != TRUE) {
		NEARDAL_TRACE_ERR(
		"Unknown GType (waiting 0x%X='%s' instead of 0x%X='%s')",
				 gtype, g_type_name(gtype),
				G_VALUE_TYPE(valueGp),
				 g_type_name(G_VALUE_TYPE(valueGp)));
		goto exit;
	}

	if (value == NULL)
		goto exit;

	switch (gtype) {
	case G_TYPE_BOOLEAN:
		*((gboolean *)value) = g_value_get_boolean(valueGp);
		NEARDAL_TRACE("(boolean) %d", *(gboolean *)value);
		err = NEARDAL_SUCCESS;
		break;
	case G_TYPE_STRING:
		*((const gchar * *)value) = g_value_get_string(valueGp);
		NEARDAL_TRACE("(string) %s", *(const gchar * *)value);
		err = NEARDAL_SUCCESS;
		break;
	default:
		if (gtype == G_TYPE_STRV) {
			void *tmp;

			tmp = g_value_get_boxed(valueGp);
			*((gchar ***)value) = tmp;
			NEARDAL_TRACE("(array of strings, first string =) %s",
					((gchar **) tmp)[0]);
			err = NEARDAL_SUCCESS;
			break;
		}
		if (gtype == DBUS_TYPE_G_ARRAY_OF_OBJECT_PATH) {
			GPtrArray	*tmp = NULL;
			tmp = g_value_get_boxed(valueGp);

			err = NEARDAL_ERROR_GENERAL_ERROR;
			if (tmp == NULL)
				break;

			NEARDAL_TRACE("(array of DBusGObjectPath: ");
			if (tmp->len > 0)
				NEARDAL_TRACE(
					"%d elements, first path=%s)",
					tmp->len,
					g_ptr_array_index(tmp, 0));
			else
				NEARDAL_TRACE(" (Empty!)");
			*((GPtrArray **)value) = tmp;
			err = NEARDAL_SUCCESS;
			break;
		}

		NEARDAL_TRACE_ERR(
		"Unknown GType (waiting 0x%X='%s' instead of 0x%X='%s')",
					gtype, g_type_name(gtype),
					G_VALUE_TYPE(valueGp),
					g_type_name(G_VALUE_TYPE(valueGp)));
		break;
	}

	NEARDAL_TRACE("\n");

exit:
	return err;
}

/******************************************************************************
 * neardal_tools_create_dict: Create a GHashTable for dict_entries.
 *****************************************************************************/
GHashTable *neardal_tools_create_dict(void)
{
	return g_hash_table_new(g_str_hash, g_str_equal);
}

/******************************************************************************
 * neardal_tools_add_dict_entry: add an entry in a dictionnary
 *****************************************************************************/
errorCode_t neardal_tools_add_dict_entry(GHashTable *hash, gchar *key,
					  gchar *value)
{
	errorCode_t	err = NEARDAL_ERROR_NO_MEMORY;
	GValue		*gvalue;

	g_assert(hash != NULL);

	gvalue = g_try_malloc0(sizeof(GValue));
	if (gvalue == NULL)
		goto error;
	
	g_value_init (gvalue, G_TYPE_STRING);
	g_value_set_string (gvalue, value);
	g_hash_table_insert (hash, (char *) key, gvalue);

	err = NEARDAL_SUCCESS;

error:
	return err;
}

