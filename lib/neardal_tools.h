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

#ifndef NEARDAL_TOOLS_H
#define NEARDAL_TOOLS_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/*****************************************************************************
 * Debugging macro to manage assertion.
 *****************************************************************************/
#define NEARDAL_ASSERT(cond) do { \
	if (!(cond)) { \
		neardal_trace(NULL, stderr, "\nASSERT!! %s -> %s():l%d: cond=(%s)\n\n" \
			, __FILE__, __func__, __LINE__, #cond); \
		return; \
	} \
} while (0);

#define NEARDAL_ASSERT_RET(cond, val) do { \
	if (!(cond)) { \
		neardal_trace(NULL, stderr, "\nASSERT!! %s -> %s():l%d: cond=(%s)\n\n" \
			, __FILE__, __func__, __LINE__, #cond); \
		return val; \
	} \
} while (0);

/*****************************************************************************
 * neardal_tools_prv_free_gerror: freeing gerror in neardal context
 *****************************************************************************/
void neardal_tools_prv_free_gerror(GError **gerror);

/*****************************************************************************
 * neardal_tools_prv_cmp_path: Compare dbus path.
 * return true (<>0) if path is same, 0 otherwise
 *****************************************************************************/
int neardal_tools_prv_cmp_path(const char *neardalPath, const char *reqPath);

/******************************************************************************
 * neardal_tools_prv_create_dict: Create a GHashTable for dict_entries.
 *****************************************************************************/
GHashTable *neardal_tools_prv_create_dict(void);

/******************************************************************************
 * neardal_tools_prv_add_dict_entry: add an entry in a dictionnary
 *****************************************************************************/
errorCode_t neardal_tools_prv_add_dict_entry(GVariantBuilder *builder
					     , gchar *key, void *value
					     , gsize valueSize
					     , int gVariantType);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* NEARDAL_TOOLS_H */
