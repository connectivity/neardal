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
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "neardal.h"
#include "neardal_prv.h"

void neardal_record_free(neardal_record *r)
{
	g_return_if_fail(r);
	neardal_g_strfreev((void **) r, &r->uriObjSize);
	memset(r, 0, sizeof(*r));
}

void neardal_free_record(neardal_record *record) \
	__attribute__ ((alias("neardal_record_free")));

GVariant *neardal_record_to_g_variant(neardal_record *in)
{
	GVariantBuilder b;
	GVariant *out;

	g_variant_builder_init(&b, G_VARIANT_TYPE_ARRAY);

	NEARDAL_G_VARIANT_IN(&b, "{'Action', <%s>}", in->action);
	NEARDAL_G_VARIANT_IN(&b, "{'Carrier', <%s>}", in->carrier);
	NEARDAL_G_VARIANT_IN(&b, "{'Encoding', <%s>}", in->encoding);
	NEARDAL_G_VARIANT_IN(&b, "{'Language', <%s>}", in->language);
	NEARDAL_G_VARIANT_IN(&b, "{'MIME', <%s>}", in->mime);
	NEARDAL_G_VARIANT_IN(&b, "{'Name', <%s>}", in->name);
	NEARDAL_G_VARIANT_IN(&b, "{'Representation', <%s>}",
				in->representation);
	NEARDAL_G_VARIANT_IN(&b, "{'Size', <%u>}", in->uriObjSize);
	NEARDAL_G_VARIANT_IN(&b, "{'Type', <%s>}", in->type);

	NEARDAL_G_VARIANT_IN(&b, "{'SSID', <%s>}", in->ssid);
	NEARDAL_G_VARIANT_IN(&b, "{'Passphrase', <%s>}", in->passphrase);
	NEARDAL_G_VARIANT_IN(&b, "{'Authentication', <%s>}",
			     in->authentication);
	NEARDAL_G_VARIANT_IN(&b, "{'Encryption', <%s>}", in->encryption);

	NEARDAL_G_VARIANT_IN(&b, "{'URI', <%s>}", in->uri);

	out = g_variant_builder_end(&b);

	return out;
}

neardal_record *neardal_g_variant_to_record(GVariant *in)
{
	neardal_record *out = g_new0(neardal_record, 1);

	NEARDAL_G_VARIANT_OUT(in, "Action", "s", &out->action);
	NEARDAL_G_VARIANT_OUT(in, "Carrier", "s", &out->carrier);
	NEARDAL_G_VARIANT_OUT(in, "Encoding", "s", &out->encoding);
	NEARDAL_G_VARIANT_OUT(in, "Language", "s", &out->language);
	NEARDAL_G_VARIANT_OUT(in, "MIME", "s", &out->mime);
	NEARDAL_G_VARIANT_OUT(in, "Name", "s", &out->name);
	NEARDAL_G_VARIANT_OUT(in, "Representation", "s", &out->representation);
	NEARDAL_G_VARIANT_OUT(in, "Size", "u", &out->uriObjSize);
	NEARDAL_G_VARIANT_OUT(in, "Type", "s", &out->type);

	NEARDAL_G_VARIANT_OUT(in, "SSID", "s", &out->ssid);
	NEARDAL_G_VARIANT_OUT(in, "Passphrase", "s", &out->passphrase);
	NEARDAL_G_VARIANT_OUT(in, "Authentication", "s", &out->authentication);
	NEARDAL_G_VARIANT_OUT(in, "Encryption", "s", &out->encryption);

	NEARDAL_G_VARIANT_OUT(in, "URI", "s", &out->uri);

	return out;
}

void neardal_record_add(GVariant *record)
{
	NEARDAL_TRACEIN();

	neardal_g_variant_dump(record);

	if (neardalMgr.cb.rcd_found != NULL)
		neardalMgr.cb.rcd_found(neardal_g_variant_get(record, "Name", "&s"),
					neardalMgr.cb.rcd_found_ud);
}

void neardal_record_remove(GVariant *record)
{
	NEARDAL_TRACEIN();

	neardal_g_variant_dump(record);
}
