/*
 *     NEARDAL (Neard Abstraction Library)
 *
 *     Copyright 2012-2014 Intel Corporation. All rights reserved.
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

void neardal_g_strfreev(void **array, void *end)
{
	void **p = array;
	for (; (void *) p < end; p++)
		g_free(*p);
	g_free(array);
}

void neardal_g_variant_add_parsed(GVariant **v, const char *format, ...)
{
	GVariantBuilder b;
	GVariantIter iter;
	GVariant *i = NULL;
	va_list ap;

	g_variant_builder_init(&b, G_VARIANT_TYPE_ARRAY);

	va_start(ap, format);
	g_variant_builder_add_value(&b, g_variant_new_parsed_va(format, &ap));
	va_end(ap);

	g_variant_iter_init(&iter, *v);
	while (g_variant_iter_loop(&iter, "*", &i))
		g_variant_builder_add_value(&b, i);

	*v = g_variant_builder_end(&b);
}

void neardal_g_variant_dump(GVariant *data)
{
	GVariantIter iter;
	char *s = NULL;
	GVariant *v = NULL;
	g_variant_iter_init(&iter, data);
	while (g_variant_iter_loop(&iter, "{sv}", &s, &v))
		NEARDAL_TRACEF(".. %s = %s\n", s, g_variant_print(v, 0));
}

void *neardal_g_variant_get(GVariant *data, const char *key, const char *fmt)
{
	char *out = NULL;
	g_variant_lookup(data, key, fmt, &out);
	return out;
}

void *neardal_data_search(const char *name)
{
	GData **l = &(neardalMgr.dbus_data);
	return g_datalist_get_data(l, name);
}

void **neardal_arrayv_append(void **array, void *data)
{
	guint len = array ? g_strv_length((gchar **) array) : 0;
	void **out = g_realloc(array, sizeof(void *) * (len + 2));
	out[len] = data;
	out[len + 1] = NULL;
	return out;
}

static void neardal_data_cb(GQuark id, gpointer data, gpointer user_data)
{
	void ***array = user_data;
	*array = neardal_arrayv_append(*array, data);
}

guint neardal_data_to_arrayv(void ***array)
{
	GData **l = &(neardalMgr.dbus_data);
	g_datalist_foreach(l, neardal_data_cb, array);
	return *array ? g_strv_length((gchar **) *array) : 0;
}

GVariant *neardal_data_insert(const char *name, const char *type, GVariant *in)
{
	GVariant *out = in;
	GData **l = &(neardalMgr.dbus_data);
	neardal_g_variant_add_parsed(&out, "{'NeardalType', <%s>}", type);
	neardal_g_variant_add_parsed(&out, "{'Name', <%s>}", name);
	g_datalist_set_data_full(l, name, g_variant_ref(out),
					(GDestroyNotify) g_variant_unref);
	return out;
}

void neardal_data_remove(GVariant *data)
{
	GData **l = &(neardalMgr.dbus_data);
	g_datalist_remove_data(l, neardal_g_variant_get(data, "Name", "&s"));
}

char *neardal_dirname(const char *path)
{
	char *tmp = strrchr(path, '/');

	return tmp ? g_strndup(path, tmp - path) : NULL;
}

/*****************************************************************************
 * neardal_tools_prv_free_gerror: freeing gerror in neardal context
 ****************************************************************************/
void neardal_tools_prv_free_gerror(GError **gerror)
{
	NEARDAL_ASSERT(gerror != NULL);

	if (*gerror != NULL)
		g_error_free(*gerror);
	*gerror = NULL;
}

/*****************************************************************************
 * neardal_tools_prv_cmp_path: Compare dbus path.
 * return true (<>0) if path is same, 0 otherwise
 ****************************************************************************/
int neardal_tools_prv_cmp_path(const char *neardalPath, const char *reqPath)
{
	const char	*shortest;
	const char	*longest;
	int		len, lenNfcPath, lenReqPath;
	int		ret = FALSE;

	NEARDAL_ASSERT_RET((neardalPath != NULL) && (reqPath != NULL)
			  , FALSE);

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

/*****************************************************************************
 * neardal_tools_prv_create_dict: Create a GHashTable for dict_entries.
 ****************************************************************************/
GHashTable *neardal_tools_prv_create_dict(void)
{
	return g_hash_table_new(g_str_hash, g_str_equal);
}

/*****************************************************************************
 * neardal_tools_prv_add_dict_entry: add an entry in a dictionnary
 ****************************************************************************/
errorCode_t neardal_tools_prv_add_dict_entry(GVariantBuilder *builder
					     , const gchar *key, void *value
					     , gsize valueSize
					     , int gVariantType)
{
	GVariant	*tmp			= NULL;
	GVariantBuilder	*bytesArrayBuilder	= NULL;
	unsigned char	*bytePtr;
	gsize		count;
	errorCode_t	err			= NEARDAL_SUCCESS;

	NEARDAL_ASSERT_RET(builder != NULL, NEARDAL_ERROR_INVALID_PARAMETER);

	switch (gVariantType) {
	case G_TYPE_STRING:
		tmp = g_variant_new_string(value);
		break;
	case G_TYPE_UINT:
		tmp = g_variant_new_uint32(GPOINTER_TO_UINT(value));
		break;
	default:
		/* if valueSize > 0, consider value as byte array
		 processing like g_variant_new_bytestring() but based on array
		 size and not a null terminated string  (allowing byte = 0
		 in array) */
		if (valueSize > 0) {
			err = NEARDAL_ERROR_GENERAL_ERROR;
			bytePtr = (unsigned char*) value;
			bytesArrayBuilder = g_variant_builder_new(
						G_VARIANT_TYPE_BYTESTRING);
			if (bytesArrayBuilder == NULL)
				break;

			for (count = 0; count < valueSize; count++)
				g_variant_builder_add(bytesArrayBuilder, "y"
						      , bytePtr[count]);

			tmp = g_variant_builder_end(bytesArrayBuilder);
			g_variant_builder_unref(bytesArrayBuilder);
			err = NEARDAL_SUCCESS;
		}
		break;
	}
	g_variant_builder_add(builder, "{sv}", key, tmp);

	return err;
}

