/*
 *     NEARDAL (Neard Abstraction Library)
 *
 *     Copyright 2014      Marvell International Ltd.
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

#ifndef NEARDAL_RECORD_H
#define NEARDAL_RECORD_H

typedef struct {
	gchar		*name;	/* DBus interface name (as identifier) */
	void		*parent; /* parent (tag) */
	gboolean	notified; /* Already notified to client? */
} RcdProp;

void neardal_record_add(GVariant *record);
void neardal_record_remove(GVariant *record);
void neardal_record_free(neardal_record *record);

#endif /* NEARDAL_RECORD_H */
