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
// static void neardal_tools_g_ptr_array_add(gpointer data, gpointer array)
// {
// 	char *tmp;
// 	tmp = g_strdup(data);
// 	g_ptr_array_add(array, tmp);
// }
// void neardal_tools_prv_array_copy(gchar ***target, gchar **source)
// {
// 	if (*target == NULL)
// 		*target = g_ptr_array_sized_new(source->len);
// 	g_ptr_array_foreach(source, neardal_tools_g_ptr_array_add, *target);
// }

/******************************************************************************
 * neardal_tools_prv_g_ptr_array_free: free a 'GPtrArray' array
 *****************************************************************************/
// static void neardal_tools_g_ptr_array_free_node(gpointer data, gpointer array)
// {
// 	(void) array; /* remove warning */
// 	g_free(data);
// }
// void neardal_tools_prv_g_ptr_array_free(GPtrArray *array)
// {
// 	g_ptr_array_foreach(array, neardal_tools_g_ptr_array_free_node, NULL);
// 	g_ptr_array_free(array, TRUE);
// }

/******************************************************************************
 * neardal_tools_prv_free_gerror: freeing gerror in neardal context
 *****************************************************************************/
void neardal_tools_prv_free_gerror(neardal_t neardalMgr)
{
	g_assert(neardalMgr != NULL);

	if (neardalMgr->gerror != NULL)
		g_error_free(neardalMgr->gerror);
	neardalMgr->gerror = NULL;
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

