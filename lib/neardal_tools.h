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

#define BUG(...)						\
do {								\
	fprintf(stderr, "%s:%d BUG:",	__FILE__, __LINE__);	\
	fprintf(stderr, " " __VA_ARGS__);			\
	abort();						\
} while (0)

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
					     , const gchar *key, void *value
					     , gsize valueSize
					     , int gVariantType);

void neardal_g_strfreev(void **array, void *end);
void neardal_g_variant_add_parsed(GVariant **v, const char *format, ...);
void *neardal_g_variant_get(GVariant *data, const char *key, const char *fmt);
void *neardal_data_search(const char *name);
GVariant *neardal_data_insert(const char *name, const char *type, GVariant *in);
void neardal_data_remove(GVariant *data);
guint neardal_data_to_arrayv(void ***array);
void **neardal_arrayv_append(void **array, void *data);
char *neardal_dirname(const char *path);

static inline gpointer neardal_g_callback(GCallback gc)
{
	union {
		gpointer gp;
		GCallback gc;
	} p;
	p.gc = gc;
	return p.gp;
}

#define NEARDAL_G_CALLBACK(_cb) neardal_g_callback(G_CALLBACK((_cb)))

#define NEARDAL_G_VARIANT_IN(_builder, _format, _data)			\
do {									\
	if ((_data))							\
		g_variant_builder_add_parsed((_builder),		\
						(_format), (_data));	\
} while (0)

#define NEARDAL_G_VARIANT_OUT(_dictionary, _key, _format, _data)	\
do {									\
	if (g_variant_lookup((_dictionary), (_key), (_format),		\
			(_data)) == TRUE) {				\
		NEARDAL_TRACEF("%s = '%" _format "'\n",			\
				(_key), *(_data));			\
	}								\
} while (0)

#endif /* NEARDAL_TOOLS_H */
