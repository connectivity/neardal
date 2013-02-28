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
/*!
 * @file neardal_errors.h
 *
 * @brief Defines NEARDAL error code and api helper
 *
 ******************************************************************************/

#ifndef NEARDAL_ERRORS_H
#define NEARDAL_ERRORS_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/*!
 * @addtogroup NEARDAL_ERROR_CODES NEARDAL Error codes
 * @ingroup NEARDAL_COMMON
 * @{
  */
/**
 * @brief Type used for NEARDAL lib error codes
 **/
typedef		int	errorCode_t;

/*! @brief Success! (It's not an error) */
#define NEARDAL_SUCCESS					((errorCode_t) 1)

/*! @brief General error */
#define NEARDAL_ERROR_GENERAL_ERROR			((errorCode_t) -1)
/*! @brief Invalid parameter */
#define NEARDAL_ERROR_INVALID_PARAMETER		((errorCode_t) -2)
/*! @brief Memory allocation error */
#define NEARDAL_ERROR_NO_MEMORY				((errorCode_t) -3)
/*! @brief DBUS general error */
#define NEARDAL_ERROR_DBUS				((errorCode_t) -4)
/*! @brief DBUS error (Can not create a proxy to dbus interface) */
#define NEARDAL_ERROR_DBUS_CANNOT_CREATE_PROXY		((errorCode_t) -5)
/*! @brief DBUS error (Can not invoke a method) */
#define NEARDAL_ERROR_DBUS_CANNOT_INVOKE_METHOD	((errorCode_t) -6)

/*! @brief Neard service, no NEARDAL adapter present */
#define NEARDAL_ERROR_NO_ADAPTER			((errorCode_t) -7)
/*! @brief Neard service, polling already active */
#define NEARDAL_ERROR_POLLING_ALREADY_ACTIVE		((errorCode_t) -8)
/*! @brief Neard service, no NEARDAL tag present */
#define NEARDAL_ERROR_NO_TAG				((errorCode_t) -9)
/*! @brief Neard service, no NEARDAL record present */
#define NEARDAL_ERROR_NO_RECORD				((errorCode_t) -10)
/*! @brief Invalid record format */
#define NEARDAL_ERROR_INVALID_RECORD			((errorCode_t) -11)
/*! @brief Neard service, no NEARDAL device present */
#define NEARDAL_ERROR_NO_DEV				((errorCode_t) -12)

/*! @brief Neard service, Error while invoking error */
#define NEARDAL_ERROR_DBUS_INVOKE_METHOD_ERROR		((errorCode_t) -13)

/* @}*/


/*!
 * @addtogroup NEARDAL_APIS APIs
 * @{
*/
/**
 * @brief map NEARDAL error value to NEARDAL error message string
 *
 * @param ec error code
 * @return NEARDAL error message string
 **/
char *neardal_error_get_text(errorCode_t ec);
/* @}*/

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* NEARDAL_ERRORS_H */
