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

#include <glib/gtypes.h>
#include <neardal_errors.h>

char *neardal_error_get_text(errorCode_t ec)
{
	switch (ec) {
	case NEARDAL_SUCCESS:
		return "Success";

	case NEARDAL_ERROR_GENERAL_ERROR:
		return "General error";

	case NEARDAL_ERROR_INVALID_PARAMETER:
		return "Invalid parameter";

	case NEARDAL_ERROR_NO_MEMORY:
		return "Memory allocation error";

	case NEARDAL_ERROR_DBUS:
		return "DBUS general error";

	case NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY:
		return "Can not create a DBUS proxy";

	case NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD:
		return "Can not invoke a DBUS method";

	case NEARDAL_ERROR_NO_ADAPTER:
		return "No NFC adapter found...";

	case NEARDAL_ERROR_NO_TARGET:
		return "No NFC target found...";

	case NEARDAL_ERROR_NO_RECORD:
		return "No target record found...";

	case NEARDAL_ERROR_POLLING_ALREADY_ACTIVE:
		return "Polling already active";

	case NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR:
		return "Error while invoking method";
	}

	return "UNKNOWN ERROR !!!";
}
