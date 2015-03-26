#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>

#include <glib.h>
#include <neardal.h>
#include <pthread.h>

typedef enum
{
	wait,
	tag_found,
	device_found,
	tag_lost,
	device_lost,
	keystroke
} loop_state_t;

static pthread_t tid;
static loop_state_t main_loop_state = wait;
static char TagName[30];
static char DevName[30];
static char AdpName[30];

#define NB_COLUMN		16

static void trace_prv_dump_data_as_binary_format (unsigned char *bufToReadP,
						  int remainingSize,
						  GString *bufDestP,
						  int nbColumn)
{
	int offset = 0;

	while (offset < nbColumn && offset < remainingSize) 
	{
		g_string_append_printf(bufDestP, "%02hhX ", bufToReadP[offset]);
		offset++;
	}
	/* Adding space to align ascii format */
	if (offset < nbColumn) 
	{
		/* 3 space because each byte in binary format as 2 digit and 1 space */
		remainingSize = (nbColumn - offset) * 3;
		offset = 0;
		while ((offset++) < remainingSize)
		{
			g_string_append_c(bufDestP, ' ');
		}
	}
}

static void trace_prv_dump_data_as_ascii_format (unsigned char *bufToReadP,
					         int remainingSize,
					         GString *bufDestP, int nbColumn)
{
	int offset = 0;

	while (offset < nbColumn && offset < remainingSize) 
	{
		if (g_ascii_isprint(((unsigned char) bufToReadP[offset])))
			g_string_append_c(bufDestP, ((unsigned char) bufToReadP[offset]));
		else
			g_string_append_c(bufDestP, '.');
		offset++;
	}
	/* Adding space to finish ascii column */
 	if (offset < nbColumn) 
	{
		remainingSize = nbColumn - offset;
		offset = 0;
		while ((offset++) < remainingSize)
			g_string_append_c(bufDestP, '.');
	}
}

static void trace_dump_mem (unsigned char *bufToReadP, unsigned int size)
{
	unsigned char	*memP = bufToReadP;
	int		len = size;
	int		offset = 0;
	GString 	*bufTrace;

	if (!memP || size <= 0)	return;

	offset	= 0;

	bufTrace = g_string_new(NULL);
	while (len > 0)
	{
		g_string_append_printf(bufTrace, "\t");
		trace_prv_dump_data_as_binary_format(&bufToReadP[offset], len, bufTrace, NB_COLUMN);
		trace_prv_dump_data_as_ascii_format(&bufToReadP[offset], len, bufTrace, NB_COLUMN);
		printf("%s\n", bufTrace->str);
		len -= NB_COLUMN;
		offset += NB_COLUMN;
		g_string_truncate(bufTrace, 0);
	}
	g_string_free(bufTrace, TRUE);
}

static void dump_tag (neardal_tag *tag)
{
	char **records;
	char **tagTypes;

	printf("---- Type:\t\t'%s'\n", tag->type);
	tagTypes = tag->tagType;
	if (tag->nbTagTypes > 0)
	{
		while ((*tagTypes) != NULL)
		{
			printf("'%s', ", *tagTypes);
			tagTypes++;
		}
		printf("\n");
	}
	records = tag->records;
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
	}
	printf("---- ReadOnly:\t\t%s\n", tag->readOnly ? "TRUE" : "FALSE");
}

static void dump_record (neardal_record *record)
{
	GVariantIter iter;
	char *s = NULL;
	GVariant *v = NULL;
	GVariant *data;
	data = (neardal_record_to_g_variant(record));
	g_variant_iter_init(&iter, data);
	while (g_variant_iter_loop(&iter, "{sv}", &s, &v))
		printf("\t---- %s = %s\n", s, g_variant_print(v, 0));
}

static void open_uri (neardal_record *record)
{
	/* If URI, then open web browser */
	if((strcmp(record->type, "URI") == 0) || (strcmp(record->type,"SmartPoster") == 0))
	{
		if(strncmp(record->uri, "http://", 7) == 0)
		{
			char *temp = g_malloc(strlen("xdg-open ") + strlen(record->uri) + 1);
			if (temp != NULL)
			{
				strcpy(temp, "xdg-open ");
				strcat(temp, record->uri);
				strcat(temp, "&");
				printf("\t\t- Opening URI in web browser ...\n");
				int ans = system(temp);
				g_free(temp);
			}
		}
	}
}

void cb_tag_found (const char *tagName, void *user_data)
{
	neardal_tag	*tag;
	errorCode_t	ec;

	printf("---- NFC Tag found\n");
	strcpy(TagName,tagName);

	ec = neardal_get_tag_properties(tagName, &tag);
	if (ec == NEARDAL_SUCCESS)
	{
		dump_tag(tag);
		neardal_free_tag(tag);
	}
	else
	{
		printf("Unable to read tag properties (error:%d='%s'). exit...\n", ec, neardal_error_get_text(ec));
	}
	main_loop_state = tag_found;
}

void cb_tag_lost (const char *tagName, void *user_data)
{
	printf("---- NFC Tag lost\n\n");
	main_loop_state = tag_lost;
}

void cb_record_found (const char *rcdName, void *user_data)
{
	errorCode_t	ec;
	neardal_record	*record;

	printf("\t---- Record found \n");

	ec = neardal_get_record_properties(rcdName, &record);
	if (ec == NEARDAL_SUCCESS)
	{
		dump_record(record);
		open_uri(record);
		neardal_free_record(record);
	} else
	{
		printf("Read record error (error:%d='%s').\n",ec,neardal_error_get_text(ec));
	}
}

static void dump_dev (neardal_dev *dev)
{
	char **records;

	records = dev->records;
	if (records != NULL) 
	{
		while ((*records) != NULL) 
		{
			printf("'%s', ", *records);
			records++;
		}
	}
}

void cb_dev_found (const char *devName, void *user_data)
{
	neardal_dev	*dev;
	errorCode_t	ec;

	printf("---- NFC Device found\n");
	strcpy(DevName,devName);

	ec = neardal_get_dev_properties(devName, &dev);
	if (ec == NEARDAL_SUCCESS)
	{
		dump_dev(dev);
		neardal_free_device(dev);
	} 
	else
	{
		printf("Unable to read device properties (error:%d='%s'). exit...\n", ec, neardal_error_get_text(ec));
	}
	main_loop_state = device_found;
}

static void cb_dev_lost(const char *devName, void *user_data)
{
	printf("---- NFC Device lost\n");
	main_loop_state = device_lost;
}

static void cb_ndef_agent (unsigned char **rcdArray, 
		 	   unsigned int rcdLen,
			   unsigned char *ndefArray,
			   unsigned int ndefLen,
			   void *user_data)
{
	printf("\n%d bytes of NDEF raw data Received :\n", ndefLen);
	trace_dump_mem(ndefArray, ndefLen);
	printf("\n");
}

static gboolean wait_lost (gpointer data)
{
	if ((main_loop_state == tag_lost) || (main_loop_state == device_lost) || (main_loop_state == keystroke))
	{
		g_main_loop_quit( (GMainLoop*)data );
		return FALSE;
	}

	return TRUE;
}

static errorCode_t start_discovery (int mode)
{
	errorCode_t	ec;

	ec = neardal_start_poll_loop(AdpName, mode);
	if (ec == NEARDAL_ERROR_POLLING_ALREADY_ACTIVE)
	{
	}
	else if (ec != NEARDAL_SUCCESS)
	{
		printf("---- Error starting discovery loop (error:%d='%s'). exit...", ec, neardal_error_get_text(ec));
		return ec;
	}
	return NEARDAL_SUCCESS;
}

void* wait_for_keystroke(void *arg)
{
	printf(" ... press enter to quit\n\n");
	getchar ();

	main_loop_state = keystroke;

	return NULL;
}

static int power_adapter(void)
{
	errorCode_t	ec;
	char		**adpArray = NULL;
	int		adpLen;
	int		power = 1;
	neardal_adapter	*adapter;

	/* Look for available adapter */
	ec = neardal_get_adapters(&adpArray, &adpLen);
	if (ec == NEARDAL_SUCCESS)
	{
		memcpy(AdpName, adpArray[0], sizeof(AdpName));
		neardal_free_array(&adpArray);
	} else
	{
		printf("---- No adapter found\n");
		return 1;
	}

	/* Power on first adapter found */
	ec = neardal_get_adapter_properties(AdpName,&adapter);	
	if (ec == NEARDAL_SUCCESS)
	{
		power=adapter->powered;
		neardal_free_adapter(adapter);			
		if (!power)
		{
			power = 1;
			ec = neardal_set_adapter_property(AdpName, NEARD_ADP_PROP_POWERED, GINT_TO_POINTER(power));
			if (ec != NEARDAL_SUCCESS) {
				printf("---- Error setting adapter properties\n");
				return 1;
			}
		}
	} else
	{
		printf("---- Error getting adapter properties\n");
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	errorCode_t	ec;
	static GMainLoop *main_loop;

	printf("#####################################\n");
	printf("##     NFC demo through Neardal    ##\n");
	printf("#####################################\n\n");

	if (power_adapter() != 0) return 1;

	/* Register callbacks */
	neardal_set_cb_tag_found(cb_tag_found, NULL);
	neardal_set_cb_tag_lost(cb_tag_lost, NULL);
	neardal_set_cb_dev_found(cb_dev_found, NULL);
	neardal_set_cb_dev_lost(cb_dev_lost, NULL);
	neardal_agent_set_NDEF_cb("urn:nfc:wkt:U", cb_ndef_agent, NULL, NULL);
	neardal_set_cb_record_found(cb_record_found, NULL);

	main_loop = g_main_loop_new(NULL, FALSE);
	if (main_loop == NULL)
	{
		printf("Error creating context\n");
		return 1;
	}

	do
	{
		main_loop_state = wait;

		/* Start discovery Loop */
		ec = neardal_start_poll_loop(AdpName, NEARD_ADP_MODE_DUAL);
		if (ec == NEARDAL_ERROR_POLLING_ALREADY_ACTIVE)
		{
		}
		else if (ec != NEARDAL_SUCCESS)
		{
			printf("---- Error starting discovery loop (error:%d='%s'). exit...\n", ec, neardal_error_get_text(ec));
			return 1;
		}

		/* Start a thread to get keystroke */
		if (0 != pthread_create(&tid, NULL, &wait_for_keystroke, NULL))
		{
	 		printf("can't create thread\n");
			return 1;	
		}

		printf("---- Waiting for a Tag/Device");

		/* Wait for tag/device lost to restart the discovery or for keystroke to leave */
		g_timeout_add (100 , wait_lost , main_loop);
		g_main_loop_run(main_loop);

	/* loop until keystroke */
	} while (main_loop_state != keystroke);

	g_main_loop_unref(main_loop);

	printf("Leaving ...\n");
	
	return 0;
}

