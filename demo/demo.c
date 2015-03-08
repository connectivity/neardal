#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include "neardal.h"

GMainLoop	*gMain_loop = NULL;

static void cb_adapter_added(const char *adpName, void *user_data)
{
	(void) user_data; /* Remove warning */

	printf("NFC Adapter added '%s'\n", adpName);
}

static void cb_adapter_removed(const char *adpName, void *user_data)
{
	(void) user_data; /* remove warning */

	printf("NFC Adapter removed '%s'\n", adpName);
}

static void cb_adapter_prop_changed(char *adpName, char *propName,
					    void *value, void *user_data)
{
	int		polling;

	(void) user_data; /* remove warning */

	if (!strcmp(propName, "Polling")) {
		polling = *(int*)&value;
		printf("Polling=%d\n", polling);
	} else
		printf("Adapter '%s' -> Property=%s=0x%X\n", adpName,
				propName, GPOINTER_TO_UINT(value));

	printf("\n");
}

/*****************************************************************************
 * Get readable string of bytes
 ****************************************************************************/
static gchar* bytes_to_str(GBytes* bytes)
{
	gchar* str = g_malloc0( 2*g_bytes_get_size(bytes) + 1 );
	const guint8* data = g_bytes_get_data(bytes, NULL);
	for(int i = 0 ; i < g_bytes_get_size(bytes); i++)
	{
		sprintf(&str[2*i], "%02X", data[i]);
	}
	return str;
}

static void dump_tag(neardal_tag *tag)
{
	char **records;
	char **tagTypes;

	printf("--- Tag:\n");
	printf("---- Name:\t\t'%s'\n", tag->name);
	printf("---- Type:\t\t'%s'\n", tag->type);
	printf("---- Number of 'Tag Type':%d\n", tag->nbTagTypes);
	tagTypes = tag->tagType;
	if (tag->nbTagTypes > 0)
	{
		printf("---- Tags type[]:\t\t");
		while ((*tagTypes) != NULL)
		{
			printf("'%s', ", *tagTypes);
			tagTypes++;
		}
		printf("\n");
	}
	records = tag->records;
	printf("---- Number of records:\t%d\n", tag->nbRecords);
	printf("---- Records[]:\t\t");
	if (records != NULL)
	{
		while ((*records) != NULL)
		{
			printf("'%s', ", *records);
			records++;
		}
	} 
	else
	{
		printf("No records!");
	}
	printf("\n");
	printf("---- ReadOnly:\t\t%s\n", tag->readOnly ? "TRUE" : "FALSE");
	if(tag->iso14443aAtqa != NULL)
	{
		gchar *str = bytes_to_str(tag->iso14443aAtqa);
		printf("---- ISO14443A ATQA:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->iso14443aSak != NULL)
	{
		gchar *str = bytes_to_str(tag->iso14443aSak);
		printf("---- ISO14443A SAK:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->iso14443aUid != NULL)
	{
		gchar *str = bytes_to_str(tag->iso14443aUid);
		printf("---- ISO14443A UID:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaManufacturer != NULL)
	{
		gchar *str = bytes_to_str(tag->felicaManufacturer);
		printf("---- Felica Manufacturer:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaCid != NULL)
	{
		gchar *str = bytes_to_str(tag->felicaCid);
		printf("---- Felica CID:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaIc != NULL)
	{
		gchar *str = bytes_to_str(tag->felicaIc);
		printf("---- Felica IC Code:\t\t'%s'\n", str);
		g_free(str);
	}
	if(tag->felicaMaxRespTimes != NULL)
	{
		gchar *str = bytes_to_str(tag->felicaMaxRespTimes);
		printf("---- Felica Maximum Response times:\t\t'%s'\n", str);
		g_free(str);
	}
}

static void dump_record(neardal_record *record)
{
	printf(" ---- Action         :%s\n"
		, record->action);
	printf(" ---- Carrier        :%s\n"
		, record->carrier);
	printf(" ---- Encoding       :%s\n"
		, record->encoding);
	printf(" ---- Language       :%s\n"
		, record->language);
	printf(" ---- MIME           :%s\n"
		, record->mime);
	printf(" ---- Name           :%s\n"
		, record->name);
	printf(" ---- Representation :%s\n"
		, record->representation);
	printf(" ---- Size           :%u\n"
		, record->uriObjSize);
	printf(" ---- Type           :%s\n"
		, record->type);
	printf(" ---- SSID           :%s\n"
		, record->ssid);
	printf(" ---- Passphrase     :%s\n"
		, record->passphrase);
	printf(" ---- Authentication :%s\n"
		, record->authentication);
	printf(" ---- Encryption     :%s\n"
		, record->encryption);
	printf(" ---- URI            :%s\n"
		, record->uri);
}


static void cb_tag_found(const char *tagName, void *user_data)
{
	neardal_tag	*tag;
	errorCode_t	ec;
	
	(void) user_data; /* remove warning */
	
	printf(" --- NFC Tag found (%s)\n", tagName);
	
	ec = neardal_get_tag_properties(tagName, &tag);
	if (ec == NEARDAL_SUCCESS)
	{
		dump_tag(tag);
		neardal_free_tag(tag);
	}
	else
	{
		printf("Unable to read tag properties. (error:%d). exit...\n", ec);
	}	
}

static void cb_tag_lost(const char *tagName, void *user_data)
{
	neardal_tag	*tag;
	errorCode_t	ec;
	
	(void) user_data; /* remove warning */
	
	printf(" --- NFC Tag lost (%s)\n", tagName);
	
 	g_main_loop_quit(gMain_loop);	
}

static void cb_record_found(const char *rcdName, void *user_data)
{
	errorCode_t	ec;
	neardal_record	*record;
	
	(void) user_data; /* remove warning */
	
	printf("\n --- NFC Record found (%s)\n", rcdName);
	ec = neardal_get_record_properties(rcdName, &record);
	if (ec == NEARDAL_SUCCESS) {
 		dump_record(record);
		neardal_free_record(record);
	} else
		printf("Read record error. (error:%d='%s').\n", ec,
			       neardal_error_get_text(ec));
		
		return;
}

int main(int argc, char *argv[])
{
	errorCode_t	ec;
	char		**adpArray = NULL;
	int		adpLen;
	char 		adpName[30];
	neardal_adapter	*adapter;
	static int	power = 1;

	(void) argc; /* Remove warning */
	(void) argv; /* Remove warning */

	/* Look for available adapter */
	ec = neardal_get_adapters(&adpArray, &adpLen);
	if (ec == NEARDAL_SUCCESS)
	{
		printf(".. Adapter found at '%s'\n", adpArray[0]);
		memcpy(adpName, adpArray[0], sizeof(adpName));
		neardal_free_array(&adpArray);
	} else
	{
		printf("No adapter found (%s)\n", neardal_error_get_text(ec));
		return 1;
	}

	/* Power on first adapter found */
	ec = neardal_get_adapter_properties(adpName,&adapter);	
	if (ec == NEARDAL_SUCCESS)
	{
		power=adapter->powered;
		neardal_free_adapter(adapter);			
		if (!power)
		{
			power = 1;
			ec = neardal_set_adapter_property(adpName, NEARD_ADP_PROP_POWERED, GINT_TO_POINTER(power));
			if (ec != NEARDAL_SUCCESS) {
				printf("Error setting adapter properties\n");
				return 1;
			}
		}
	} else
	{
		printf("Error getting adapter properties\n");
		return 1;
	}

	/* Register callbacks */
	neardal_set_cb_adapter_added(cb_adapter_added, NULL);
	neardal_set_cb_adapter_removed(cb_adapter_removed, NULL);
	neardal_set_cb_adapter_property_changed(cb_adapter_prop_changed, NULL);
	ec = neardal_set_cb_tag_found(cb_tag_found, NULL);
	if (ec != NEARDAL_SUCCESS)
	{
		printf("Error registering Tag found callback\n");
		return 1;
	}
	neardal_set_cb_tag_lost(cb_tag_lost, NULL);
	neardal_set_cb_record_found(cb_record_found, NULL);
	
	/* Start Discovery Loop*/
	ec = neardal_start_poll(adpName);
	if (ec != NEARDAL_SUCCESS && ec != NEARDAL_ERROR_POLLING_ALREADY_ACTIVE)
	{
		printf("Error starting discovery loop\n");
		return 1;
	}

	gMain_loop = g_main_loop_new(NULL, FALSE);
	if (gMain_loop) {
		g_main_loop_run(gMain_loop);
		g_main_loop_unref(gMain_loop);
	} else
		return 1;

	return 0;
}
