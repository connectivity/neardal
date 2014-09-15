/*
 *     NEARDAL (Neard Abstraction Library)
 *
 *     Copyright 2014 Marvell International Ltd.
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
/*!
 * @file neardal.h
 *
 * @brief Defines main NEARDAL apis and data types.
 * @author Frederic PAUT, Intel Corporation
 * @version 0.1
 *
 ******************************************************************************/

#ifndef NEARDAL_H
#define NEARDAL_H
#include "neardal_errors.h"

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/*!
 * @addtogroup NEARDAL_COMMON_TYPES Types
 * @ingroup NEARDAL_COMMON
 * @{
  */
/*!
 * @brief NEARDAL Adapter client properties
 * release with (@link neardal_free_adapter @endlink)
 **/
typedef struct {
/*! \brief DBus interface adapter name (as identifier) */
	char	*name;
/*! \brief Neard adapter NFC radio mode */
	char	*mode;
/*! \brief Neard adapter polling already active ? */
	short	polling;
/*! \brief Neard adapter powered ? */
	short	powered;
/*! \brief Number of supported protocols */
	int	nbProtocols;
/*! \brief Neard adapter supported protocols list */
	char	**protocols;
/*! \brief Number of tags managed by this adapter */
	int	nbTags;
/*! \brief Neard adapter tags list */
	char	**tags;
/*! \brief Number of devices managed by this adapter */
	int	nbDevs;
/*! \brief Neard adapter devices list */
	char	**devs;
} neardal_adapter;

/*!
 * @brief NEARDAL Tag client properties.
 * release with (@link neardal_free_tag @endlink)
*/
typedef struct {
/*! @brief DBus interface tag name (as identifier) */
	const char	*name;

/*! @brief Number of records in tag */
	int		nbRecords;
/*! @brief tag records list */
	char		**records;
/*! @brief Number of supported 'types' in tag */
	int		nbTagTypes;
/*! @brief types list */
	char		**tagType;
/*! @brief tag type */
	const char	*type;
/*! @brief Read-Only flag (is tag writable?) */
	short		readOnly;
} neardal_tag;

/*!
 * @brief NEARDAL Device client properties.
 * release with (@link neardal_free_device @endlink)
*/
typedef struct {
/*! @brief DBus interface tag name (as identifier) */
	const char	*name;

/*! @brief Number of records in device */
	int		nbRecords;
/*! @brief device records list */
	char		**records;
} neardal_dev;

/**
 * NFC record. Release with neardal_free_record().
 */
typedef struct {
	char *action;		/**< Action. Save, Edit, Download. */
	char *carrier;		/**< Handover carrier. Bluetooth. */
	char *encoding;		/**< Encoding. */
	char *language;		/**< Language. ISO/IANA: en, jp, etc. */
	char *mime;		/**< MIME type. */
	char *name;		/**< Identifier. DBus path. */
	char *representation;	/**< Human readable representation. */
	char *type;		/**< NDEF record type.
					Text,
					URI,
					SmartPoster,
					HandoverCarrier,
					HandoverRequest,
					HandoverSelect. */
	/* WiFi handover parameters. */
	char *ssid;		/**< WiFi SSID. */
	char *passphrase;	/**< WiFi Passphrase. */
	char *encryption;	/**< WiFi Encryption. */
	char *authentication;	/**< WiFi Authentication. */

	char *uri;		/**< URI including scheme and resource. */

	/* Keep uriObjSize first after contiguous array of pointers. */

	unsigned int uriObjSize;/**< URI object size. */
} neardal_record;

/* @}*/

/*! @brief NEARDAL Callbacks
 * @addtogroup NEARDAL_CALLBACK CALLBACKS
 * @{
*/

/**
 * @brief Callback prototype for 'NEARDAL adapter added/removed'
 *
 * @param adpName DBus interface adapter name (as identifier=dbus object path)
 * @param user_data Client user data
 **/
typedef void (*adapter_cb) (const char *adpName, void *user_data);
/**
 * @brief Callback prototype for 'NEARDAL adapter property changed'
 *
 * @param adpName DBus interface adapter name (as identifier=dbus object path)
 * @param propName Property name
 * @param value Property value
 * @param user_data Client user data
 **/
typedef void (*adapter_prop_cb) (char *adpName, char *propName, void *value,
				 void *user_data);

/** @brief NEARDAL Tag Callbacks (TagFound/Lost)
*/
/**
 * @brief Callback prototype for 'NEARDAL tag found/lost'
 *
 * @param tagName DBus interface tag name (as identifier=dbus object path)
 * @param user_data Client user data
 **/
typedef void (*tag_cb) (const char *tagName, void *user_data);

/** @brief NEARDAL Device Callbacks (Device Found/Lost)
*/
/**
 * @brief Callback prototype for 'NEARDAL device found/lost'
 *
 * @param devName DBus interface dev name (as identifier=dbus object path)
 * @param user_data Client user data
 **/
typedef void (*dev_cb) (const char *devName, void *user_data);

/** @brief NEARDAL Record Callbacks ('RecordFound')
*/
/**
 * @brief Callback prototype for 'NEARDAL record found'
 *
 * @param rcdName DBus interface record name (as identifier=dbus object path)
 * @param user_data Client user data
 **/
typedef void (*record_cb) (const char *rcdName, void *user_data);

/**
 * @brief Callback prototype for a registered tag type
 *
 * @param rcdArray array of records path (as identifier=dbus object path)
 * @param rcdLen number of records path in rcdArray
 * @param ndefArray array of raw NDEF data
 * @param ndefLen number of bytes in ndefArray
 * @param user_data Client user data
 **/
typedef void (*ndef_agent_cb) (unsigned char **rcdArray, unsigned int rcdLen,
			       unsigned char *ndefArray, unsigned int ndefLen,
			       void *user_data);

/**
 * @brief Callback prototype to cleanup agent user data. Gets called when
 * Neard unregisters the agent.
 *
 * @param user_data Client user data
 **/
typedef void (*ndef_agent_free_cb) (void *user_data);

/**
 * @brief Pointer to function used to release oob data generated by callback
 * @link oob_req_agent_cb @endlink
 **/
typedef void (*freeFunc) (void *ptr);

/**
 * @brief Callback prototype to get Out Of Band data from the handover agent
 *
 * @param blobEIR  EIR blob as described in Bluetooth Core
 * Specification 4.0 (Vol 3, Part C, chapter 8.1.6). Used by SSP capable
 * devices
 *
 * @param blobSize EIR blob size (in bytes)
 * @param oobData Out Of Band data returned (as an array of bytes) used to
 * build a Handover Request or Select message (oobData* will be never null)
 * @param oobDataSize Out Of Band data size returned (oobDataSize* will be
 * never null)
 * @param freeFunc Free function to release oobData
 * @param user_data Client user data
 **/
typedef void (*oob_req_agent_cb) (unsigned char *blobEIR,
				  unsigned int blobSize,
				  unsigned char **oobData,
				  unsigned int *oobDataSize, freeFunc *freeF,
				  void *user_data);

/**
 * @brief Callback prototype to to pass remote Out Of Band data to agent to
 * start handover
 *
 * @param blobEIR  EIR blob as described in Bluetooth Core
 * Specification 4.0 (Vol 3, Part C, chapter 8.1.6). Used by SSP capable
 * devices
 *
 * @param blobSize EIR blob size (in bytes)
 * @param user_data Client user data
 **/
typedef void (*oob_push_agent_cb) (unsigned char *blobEIR,
				   unsigned int blobSize, void *user_data);


/**
 * @brief Callback prototype to cleanup agent user data. Gets called when
 * Neard unregisters the agent.
 *
 * @param user_data Client user data
 **/
typedef void (*oob_agent_free_cb) (void *user_data);

/* @}*/



/*! @brief NEARDAL APIs
 *  @note NEARDAL lib exported functions
 * @addtogroup NEARDAL_APIS APIs
 * @{
*/

/*! \fn void neardal_destroy()
*  \brief destroy NEARDAL object instance, disconnect Neard Dbus connection,
* unregister Neard's events
*/
void neardal_destroy();

/*! @brief NEARDAL Properties identifiers
 * @addtogroup NEARDAL_POLLING_MODE
 * @{ */
#define NEARD_ADP_MODE_INITIATOR		0
#define NEARD_ADP_MODE_TARGET			1
#define NEARD_ADP_MODE_DUAL			2
/* @}*/


/*! \fn errorCode_t neardal_start_poll_loop(char *adpName, int mode)
*  \brief Request Neard to start polling on specific NEARDAL adapter with
*  specific mode
*  \param adpName : DBus interface adapter name (as identifier=dbus object
*		     path)
*  \param mode : Polling mode (see @link NEARDAL_POLLING_MODE @endlink ...)
*  @return errorCode_t error code
*/
errorCode_t neardal_start_poll_loop(char *adpName, int mode);

/*! \fn errorCode_t neardal_start_poll(char *adpName)
*  \brief Request Neard to start polling on specific NEARDAL adapter in
* Initiator mode
*  \param adpName : DBus interface adapter name (as identifier=dbus object path)
*  @return errorCode_t error code
*/
#define neardal_start_poll(adpName)	 neardal_start_poll_loop(adpName, \
						NEARD_ADP_MODE_INITIATOR);

/*! \fn errorCode_t neardal_stop_poll(char *adpName)
*  \brief Request Neard to stop polling on specific NEARDAL adapter
*  \param adpName : DBus interface adapter name (as identifier=dbus object path)
*  @return errorCode_t error code
*/
errorCode_t neardal_stop_poll(char *adpName);

/*! \fn errorCode_t neardal_get_adapters(char ***array, int *len)
 * @brief get an array of NEARDAL adapters present
 *
 * @param array array of DBus interface adapter name (as identifier=dbus
 * object path).  release with @link neardal_free_adapter @endlink(& )
 * @param len (optional), number of adapters
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_adapters(char ***array, int *len);

/*! \fn errorCode_t neardal_get_devices(char *adpName, char ***array, int *len)
 * @brief get an array of NEARDAL devices present
 *
 * @param adpName adapter name (identifier) on which devices list must be
 * retrieve
 * @param array array of DBus interface device name (as identifier=dbus object
 * path), release with @link neardal_free_device @endlink(& )
 * @param len (optional), number of devs
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_devices(char *adpName, char ***array, int *len);


/*! \fn errorCode_t neardal_get_tags(char *adpName, char ***array, int *len)
 * @brief get an array of NEARDAL tags present
 *
 * @param adpName adapter name (identifier) on which tags list must be
 * retrieve
 * @param array array of DBus interface tag name (as identifier=dbus object
 * path), release with @link neardal_free_tag @endlink(& )
 * @param len (optional), number of tags
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_tags(char *adpName, char ***array, int *len);

/*! \fn errorCode_t neardal_get_records(char *tagName, char ***array, int *len)
 * @brief get an array of NEARDAL records present
 *
 * @param tagName tag name (identifier) on which records list must be
 * retrieve
 * @param array array of DBus interface record name (as identifier=dbus object
 * path), release with @link neardal_free_record @endlink(& )
 * @param len (optional), number of records
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_records(char *tagName, char ***array, int *len);

/*! @brief NEARDAL Properties identifiers
 * @addtogroup NEARDAL_CALLBACK Defines
 * @{ */
#define NEARD_ADP_PROP_POWERED			0


/* @}*/


/*! \fn errorCode_t neardal_get_adapter_properties(const char* adpName,
 * neardal_adapter **adapter)
 * @brief Get properties of a specific NEARDAL adapter
 *
 * @param adpName DBus interface adapter name (as identifier=dbus object path)
 * @param adapter Pointer on pointer of client adapter struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_adapter_properties(const char *adpName,
					   neardal_adapter **adapter);

/*! \fn void neardal_free_adapter(neardal_adapter * adapter)
 * @brief Release memory allocated for properties of an adapter
 *
 * @param adapter Pointer on client adapter struct where datas are stored
 * @return nothing
 **/
void neardal_free_adapter(neardal_adapter *adapter);

/*! \fn errorCode_t neardal_set_adapter_properties(const char* adpName,
 * int adpPropId, void * value)
 * @brief Set a property on a specific NEARDAL adapter
 *
 * @param adpName DBus interface adapter name (as identifier=dbus object path)
 * @param adpPropId Adapter Property Identifier (see NEARD_ADP_PROP_ ...)
 * @param value Value
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_adapter_property(const char *adpName,
					  int adpPropId, void *value);

/*! \fn errorCode_t neardal_set_cb_adapter_added( adapter_cb cb_adp_added,
 *					     void * user_data)
 * @brief setup a client callback for 'NEARDAL adapter added'. cb_adp_added = NULL
 * to remove actual callback
 *
 * @param cb_adp_added Client callback 'adapter added'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_adapter_added(adapter_cb cb_adp_added,
					 void *user_data);

/*! \fn errorCode_t neardal_set_cb_adapter_removed(adapter_cb cb_adp_removed,
 *					       void * user_data)
 * @brief setup a client callback for 'NEARDAL adapter removed'.
 * cb_adp_removed = NULL to remove actual callback
 *
 * @param cb_adp_removed Client callback 'adapter removed'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_adapter_removed(adapter_cb cb_adp_removed,
					    void *user_data);

/*! \fn errorCode_t neardal_set_cb_adapter_property_changed(
 * adapter_prop_cb cb_adp_property_changed,
 * void *user_data)
 * @brief setup a client callback for 'NEARDAL Adapter Property Changed'.
 * cb_adp_property_changed = NULL to remove actual callback.
 *
 * @param cb_adp_property_changed Client callback 'Adapter Property Changed'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_adapter_property_changed(
					adapter_prop_cb cb_adp_property_changed,
						void *user_data);

/*! \fn errorCode_t neardal_get_tag_properties(const char* tagName,
 * neardal_tag **tag)
 * @brief Get properties of a specific NEARDAL tag
 *
 * @param tagName tag name (identifier) on which properties must be retrieve
 * @param tag Pointer on pointer of client tag struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_tag_properties(const char *tagName,
				       neardal_tag **tag);

/*! \fn errorCode_t neardal_tag_write(neardal_record *record)
 * @brief Write NDEF record to an NFC tag
 *
 * @param record Pointer on client record used to create NDEF record
 * @return errorCode_t error code
 **/
errorCode_t neardal_tag_write(neardal_record *record);

/*! \fn void neardal_free_tag(neardal_tag *tag)
 * @brief Release memory allocated for properties of a tag
 *
 * @param tag Pointer on client tag struct where datas are stored
 * @return nothing
 **/
void neardal_free_tag(neardal_tag *tag);

/*! \fn errorCode_t neardal_set_cb_tag_found(tag_cb cb_tag_found,
 * void * user_data)
 * @brief setup a client callback for 'NEARDAL tag found'.
 * cb_tag_found = NULL to remove actual callback.
 *
 * @param cb_tag_found Client callback 'tag found'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_tag_found(tag_cb cb_tag_found,
				     void *user_data);

/*! \fn errorCode_t neardal_set_cb_tag_lost(tag_cb cb_tag_lost,
 * void * user_data)
 * @brief setup a client callback for 'NEARDAL tag lost'.
 * cb_tag_lost = NULL to remove actual callback.
 *
 * @param cb_tag_lost Client callback 'tag lost'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_tag_lost(tag_cb cb_tag_lost,
				    void *user_data);

/*! \fn errorCode_t neardal_get_dev_properties(const char* devName,
 * neardal_dev **dev)
 * @brief Get properties of a specific NEARDAL dev
 *
 * @param devName dev name (identifier) on which properties must be retrieve
 * @param dev Pointer on pointer of client dev struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_dev_properties(const char *devName,
				       neardal_dev **dev);

/*! \fn errorCode_t neardal_dev_push(neardal_record *record)
 * @brief Create and push NDEF record to an NFC device
 *
 * @param record Pointer on client record used to create NDEF record
 * @return errorCode_t error code
 **/
errorCode_t neardal_dev_push(neardal_record *record);

/*! \fn void neardal_free_device(neardal_dev *dev)
 * @brief Release memory allocated for properties of a dev
 *
 * @param dev Pointer on client dev struct where datas are stored
 * @return nothing
 **/
void neardal_free_device(neardal_dev *dev);

/*! \fn errorCode_t neardal_set_cb_dev_found(dev_cb cb_dev_found,
 * void * user_data)
 * @brief setup a client callback for 'NEARDAL dev found'.
 * cb_dev_found = NULL to remove actual callback.
 *
 * @param cb_dev_found Client callback 'dev found'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_dev_found(dev_cb cb_dev_found,
					 void *user_data);

/*! \fn errorCode_t neardal_set_cb_dev_lost(dev_cb cb_dev_lost,
 * void * user_data)
 * @brief setup a client callback for 'NEARDAL dev lost'.
 * cb_dev_lost = NULL to remove actual callback.
 *
 * @param cb_dev_lost Client callback 'dev lost'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_dev_lost(dev_cb cb_dev_lost,
					void *user_data);


/*! \fn errorCode_t neardal_get_record_properties(const char *recordName,
 *					      neardal_record **record)
 * @brief Get properties of a specific NEARDAL tag record
 *
 * @param recordName DBus interface record name (as identifier=dbus object path)
 * @param record Pointer on pointer of client record struct to store datas
 * @return errorCode_t error code
 **/
errorCode_t neardal_get_record_properties(const char *recordName,
					  neardal_record **record);

/*! \fn void neardal_free_record(neardal_record *record)
 * @brief Release memory allocated for properties of a record
 *
 * @param record Pointer on client tag struct where datas are stored
 * @return nothing
 **/
void neardal_free_record(neardal_record *record);


/*! \fn errorCode_t neardal_set_cb_record_found( record_cb cb_rcd_found,
 * void * user_data)
 * @brief Setup a client callback for 'NEARDAL tag record found'.
 * cb_rcd_found = NULL to remove actual callback
 *
 * @param cb_rcd_found Client callback 'record found'
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_set_cb_record_found(record_cb cb_rcd_found,
					 void *user_data);

/*! \fn errorCode_t neardal_agent_set_NDEF_cb(char *tagType, agent_cb cb_agent,
 * void *user_data)
 * @brief register or unregister a callback to handle a record macthing
 * a registered tag type. This callback will received the whole NDEF as a raw
 * byte stream and the records object paths.
 * If the callback is null, the agent is unregistered.
 * @param tagType tag type to register
 * @param cb_ndef_agent Client callback for the registered tag type
 * @param cb_ndef_release_agent Client callback to cleanup agent user data
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_agent_set_NDEF_cb(char *tagType
				      , ndef_agent_cb cb_ndef_agent
				     , ndef_agent_free_cb cb_ndef_release_agent
				      , void *user_data);


/*! \fn errorCode_t neardal_agent_set_handover_cb(
 * 					  oob_push_agent_cb cb_oob_push_agent
 * 					, oob_req_agent_cb  cb_oob_req_agent
					, void *user_data)
 * @brief register or unregister two callbacks to handle handover connection.
 * If one of this callback is null, the agent is unregistered.
 * @param cb_oob_push_agent used to pass remote Out Of Band data
 * @param cb_oob_req_agent used to get Out Of Band data
 * @param cb_oob_release_agent used to cleanup agent user data
 * @param user_data Client user data
 * @return errorCode_t error code
 **/
errorCode_t neardal_agent_set_handover_cb(oob_push_agent_cb cb_oob_push_agent
					  , oob_req_agent_cb  cb_oob_req_agent
				, oob_agent_free_cb cb_oob_release_agent
					  , void *user_data);

/*! @fn errorCode_t neardal_free_array(char ***array)
 *
 * @brief free memory used by array of adapters/tags/device or records
 *
 * @param array array (of adapters/tags/devices or records)
 * @return errorCode_t error code
 *
 **/
errorCode_t neardal_free_array(char ***array);

/**
 * Dump GVariant in a human readable format.
 */
void neardal_g_variant_dump(GVariant *data);

/**
 * Convert neardal_record to GVariant.
 */
GVariant *neardal_record_to_g_variant(neardal_record *in);

/**
 * Convert GVariant to neardal_record.
 */
neardal_record *neardal_g_variant_to_record(GVariant *in);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* NEARDAL_H */
