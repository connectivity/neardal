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
 * @file neardal.h
 *
 * @brief Defines main NEARDAL apis and data types.
 * @author Frederic PAUT, Intel Corporation
 * @version 0.1
 *
 ******************************************************************************/

#ifndef __NEARDAL_H
#define __NEARDAL_H
#include "neardal_errors.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/*! @typedef neardal_t
 *  @brief NEARDAL Context
 */
typedef struct neardalCtx	*neardal_t;

/*!
 * @addtogroup NEARDAL_COMMON_TYPES Types
 * @ingroup NEARDAL_COMMON
 * @{
  */
/*!
 * @brief NEARDAL Adapter client properties.
 **/
typedef struct {
/*! \brief DBus interface adapter name (as identifier) */
	char	*name;
/*! \brief Neard adapter polling already active ? */
	short	polling;
/*! \brief Neard adapter powered ? */
	short	powered;
/*! \brief Number of supported protocols */
	int	nbProtocols;
/*! \brief Neard adapter supported protocols list (use  @link
 * neardal_free_array @endlink(& ) to free) */
	char	**protocols;
/*! \brief Number of targets managed by this adapter */
	int	nbTargets;
/*! \brief Neard adapter targets list (use @link neardal_free_array @endlink
 *(& ) to free) */
	char	**targets;
} neardal_adapter;

/*!
 * @brief NEARDAL Target client properties.
*/
typedef struct {
/*! @brief DBus interface target name (as identifier) */
	const char	*name;

/*! @brief Number of records in target */
	int		nbRecords;
/*! @brief target records list (use @link neardal_free_array @endlink (& ) to
 * free) */
	char		**records;
/*! @brief Number of supported 'types' in target */
	int		nbTagTypes;
/*! @brief types list (use @link neardal_free_array @endlink(& ) to free) */
	char		**tagType;
/*! @brief target type */
	const char	*type;
/*! @brief Read-Only flag (is target writable?) */
	short		readOnly;
} neardal_target;

/*!
 * @brief NEARDAL Record client properties.
*/
typedef struct {
/*! @brief DBus interface name (as identifier) */
	const char	*name;
/*! @brief character encoding */
	const char	*encoding;
/*! @brief connection established using other wireless communication
technologies */
	short		handOver;
/*! @brief internation language used */
	const char	*language;
/*! @brief is tag smartposter? */
	short		smartPoster;
/*! @brief 'Action' Save, Edit, Download... */
	const char	*action;
/*! @brief tag type: 'Text', 'Uri', 'MIME Type' */
	const char	*type;
/*! @brief Text datas */
	const char	*representation;
/*! @brief Uri datas */
	const char	*uri;
/*! @brief MIME Type datas */
	const char	*mime;
} neardal_record;

/* @}*/

/*! @brief NEARDAL Callbacks
 * @addtogroup NEARDAL_CALLBACK Callbacks
 * @{
*/

/**
 * @brief Callback prototype for 'NEARDAL adapter added/removed'
 *
 * @param adpName DBus interface adapter name (as identifier)
 * @param user_data Client user data
 **/
typedef void (*adapter_cb) (const char *adpName, void *user_data);
/**
 * @brief Callback prototype for 'NEARDAL adapter property changed'
 *
 * @param adpName DBus interface adapter name (as identifier)
 * @param propName Property name
 * @param value Property value
 * @param user_data Client user data
 **/
typedef void (*adapter_prop_cb) (char *adpName, char *propName, void * value,
				 void *user_data);

/** @brief NEARDAL Target Callbacks (TargetFound/Lost)
*/
/**
 * @brief Callback prototype for 'NEARDAL target found/lost'
 *
 * @param tgtName DBus interface target name (as identifier)
 * @param user_data Client user data
 **/
typedef void (*target_cb) (const char *tgtName, void *user_data);

/** @brief NEARDAL Record Callbacks ('RecordFound')
*/
/**
 * @brief Callback prototype for 'NEARDAL record found'
 *
 * @param rcdName DBus interface record name (as identifier)
 * @param user_data Client user data
 **/
typedef void (*record_cb) (const char *rcdName, void *user_data);

/* @}*/



/*! @brief NEARDAL APIs
 *  @note NEARDAL lib exported functions
 * @addtogroup NEARDAL_APIS APIs
 * @{
*/

/*! \fn neardal_t neardal_construct(errorCode_t *ec)
*  \brief create NEARDAL object instance, Neard Dbus connection,
* register Neard's events
*  \param ec : optional, pointer to store error code
*  \return the NEARDAL context
*/
neardal_t neardal_construct(errorCode_t *ec);


/*! \fn void neardal_destroy(neardal_t neardalMgr)
*  \brief destroy NEARDAL object instance, disconnect Neard Dbus connection,
* unregister Neard's events
*  \param neardalMgr : NEARDAL context to destroy
*/
void neardal_destroy(neardal_t neardalMgr);

/*! \fn void neardal_start_poll(neardal_t neardalMgr, char *adpName,
 * errorCode_t *ec)
*  \brief Request Neard to start polling on specific NEARDAL adapter
*  \param neardalMgr : NEARDAL context
*  \param adpName : DBus interface adapter name (as identifier)
*  \param ec : optional, pointer to store error code
*/
void neardal_start_poll(neardal_t neardalMgr, char *adpName,
			 errorCode_t *ec);

/*! \fn void neardal_stop_poll(neardal_t neardalMgr, char *adpName,
 * errorCode_t *ec)
*  \brief Request Neard to stop polling on specific NEARDAL adapter
*  \param neardalMgr : NEARDAL context
*  \param adpName : DBus interface adapter name (as identifier)
*  \param ec : optional, pointer to store error code
*/
void neardal_stop_poll(neardal_t neardalMgr, char *adpName, errorCode_t *ec);

/*! \fn errorCode_t neardal_get_adapters(neardal_t neardalMgr, char ***array,
 * int *len)
 * @brief get an array of NEARDAL adapters present
 *
 * @param neardalMgr NEARDAL context
 * @param array array of DBus interface adapter name (as identifier), use @link
 * neardal_free_array @endlink(& ) to free
 * @param len (optional), number of adapters
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_adapters(neardal_t neardalMgr, char ***array,
				  int *len);

/*! \fn errorCode_t neardal_get_targets(neardal_t neardalMgr, char *adpName,
 *				     char ***array, int *len)
 * @brief get an array of NEARDAL targets present
 *
 * @param neardalMgr NEARDAL context
 * @param adpName adapter name (identifier) on which targets list must be
 * retrieve
 * @param array array of DBus interface target name (as identifier), use @link
 * neardal_free_array @endlink(& ) to free
 * @param len (optional), number of targets
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_targets(neardal_t neardalMgr, char *adpName,
				 char ***array, int *len);

/*! \fn errorCode_t neardal_get_records(neardal_t neardalMgr, char *tgtName,
 *				     char ***array, int *len)
 * @brief get an array of NEARDAL records present
 *
 * @param neardalMgr NEARDAL context
 * @param tgtName target name (identifier) on which records list must be
 * retrieve
 * @param array array of DBus interface record name (as identifier), use @link
 * neardal_free_array @endlink(& ) to free
 * @param len (optional), number of records
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_records(neardal_t neardalMgr, char *tgtName,
				 char ***array, int *len);

/*! @fn errorCode_t neardal_free_array(char ***array)
 *
 * @brief free memory used for adapters array, targets array or records array
 *
 * @param array adapters array, targets array or records array
 * @return errorCode_t error code
 *
 **/
errorCode_t neardal_free_array(char ***array);

/*! \fn errorCode_t neardal_get_adapter_properties(neardal_t neardalMgr,
 * const char* adpName, neardal_adapter *adapter)
 * @brief Get properties of a specific NEARDAL adapter
 *
 * @param neardalMgr NEARDAL context
 * @param adpName DBus interface adapter name (as identifier)
 * @param adapter Pointer on client adapter struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_adapter_properties(neardal_t neardalMgr,
					    const char *adpName,
					    neardal_adapter *adapter);

/*! \fn errorCode_t neardal_set_cb_adapter_added(neardal_t neardalMgr,
 *					     adapter_cb cb_adp_added,
 *					     void * user_data)
 * @brief setup a client callback for 'NEARDAL adapter added'. cb_adp_added = NULL
 * to remove actual callback
 *
 * @param neardalMgr NEARDAL context
 * @param cb_adp_added Client callback 'adapter added'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_adapter_added(neardal_t neardalMgr,
					  adapter_cb cb_adp_added,
					  void *user_data);

/*! \fn errorCode_t neardal_set_cb_adapter_removed(neardal_t neardalMgr,
 *					       adapter_cb cb_adp_removed,
 *					       void * user_data)
 * @brief setup a client callback for 'NEARDAL adapter removed'.
 * cb_adp_removed = NULL to remove actual callback
 *
 * @param neardalMgr NEARDAL context
 * @param cb_adp_removed Client callback 'adapter removed'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_adapter_removed(neardal_t neardalMgr,
					    adapter_cb cb_adp_removed,
					    void *user_data);

/*! \fn errorCode_t neardal_set_cb_adapter_property_changed(
 * neardal_t neardalMgr, adapter_prop_cb cb_adp_property_changed,
 * void *user_data)
 * @brief setup a client callback for 'NEARDAL Adapter Property Changed'.
 * cb_adp_property_changed = NULL to remove actual callback.
 *
 * @param neardalMgr NEARDAL context
 * @param cb_adp_property_changed Client callback 'Adapter Property Changed'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_adapter_property_changed(neardal_t neardalMgr,
					adapter_prop_cb cb_adp_property_changed,
						void *user_data);

/*! \fn errorCode_t neardal_get_target_properties(neardal_t neardalMgr,
 * const char* tgtName, neardal_target *target)
 * @brief Get properties of a specific NEARDAL target
 *
 * @param neardalMgr NEARDAL context
 * @param tgtName target name (identifier) on which properties must be retrieve
 * @param target Pointer on client target struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_target_properties(neardal_t neardalMgr,
					   const char *tgtName,
					   neardal_target *target);

/*! \fn errorCode_t neardal_set_cb_target_found(neardal_t neardalMgr,
 * target_cb cb_tgt_found, void * user_data)
 * @brief setup a client callback for 'NEARDAL target found'.
 * cb_tgt_found = NULL to remove actual callback.
 *
 * @param neardalMgr NEARDAL context
 * @param cb_tgt_found Client callback 'target found'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_target_found(neardal_t neardalMgr,
					 target_cb cb_tgt_found,
					 void *user_data);

/*! \fn errorCode_t neardal_set_cb_target_lost(neardal_t neardalMgr,
 * target_cb cb_tgt_lost, void * user_data)
 * @brief setup a client callback for 'NEARDAL target lost'.
 * cb_tgt_lost = NULL to remove actual callback.
 *
 * @param neardalMgr NEARDAL context
 * @param cb_tgt_lost Client callback 'target lost'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_target_lost(neardal_t neardalMgr,
					target_cb cb_tgt_lost,
					void *user_data);


/*! \fn errorCode_t neardal_get_record_properties(neardal_t neardalMgr,
 *					      const char *recordName,
 *					      neardal_record *record)
 * @brief Get properties of a specific NEARDAL target record
 *
 * @param neardalMgr NEARDAL context
 * @param recordName DBus interface record name (as identifier)
 * @param record Pointer on client record struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_record_properties(neardal_t neardalMgr,
					   const char *recordName,
					   neardal_record *record);

/*! \fn errorCode_t neardal_publish(neardal_t neardalMgr,
 * neardal_record *record)
 * @brief Write NDEF record to an NFC tag
 *
 * @param neardalMgr NEARDAL context
 * @param record Pointer on client record used to create NDEF record
 * @return errorCode_t error code
 **/
errorCode_t neardal_publish(neardal_t neardalMgr, neardal_record *record);


/*! \fn errorCode_t neardal_set_cb_record_found(neardal_t neardalMgr,
 * record_cb cb_rcd_found, void * user_data)
 * @brief Setup a client callback for 'NEARDAL target record found'.
 * cb_rcd_found = NULL to remove actual callback
 *
 * @param neardalMgr NEARDAL context
 * @param cb_rcd_found Client callback 'record found'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_record_found(neardal_t neardalMgr,
					 record_cb cb_rcd_found,
					 void *user_data);

/* @}*/


#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __NEARDAL_H */
