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


/*****************************************************************************
 * neardal_tools_prv_free_gerror: freeing gerror in neardal context
 ****************************************************************************/
void neardal_tools_prv_free_gerror(GError **gerror)
{
	g_assert(gerror != NULL);

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
errorCode_t neardal_tools_prv_add_dict_entry(GVariantBuilder *builder, gchar *key,
					  void *value, int gVariantType)
{
	GVariant *tmp;
	g_assert(builder != NULL);

	switch (gVariantType) {
	case G_TYPE_STRING:
		tmp = g_variant_new_string(value);
		break;
	case G_TYPE_UINT:
		tmp = g_variant_new_uint32((guint32) value);
		break;
	}
	g_variant_builder_add(builder, "{sv}", key, tmp);

	return NEARDAL_SUCCESS;
}

