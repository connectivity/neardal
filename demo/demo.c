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

static GMainLoop *main_loop;
static pthread_t tid;
static loop_state_t main_loop_state = wait;
static int g_argc, presence=0, keystroke_state=0;
static char **g_argv;
static char TagName[30];
static char DevName[30];
static char AdpName[30];
static neardal_record	rcd;
static int smartPoster;
static GOptionEntry options[] = {
	{ "act", 'a', 0, G_OPTION_ARG_STRING, &rcd.action, "Action", "save"},
	{ "encoding", 'e', 0, G_OPTION_ARG_STRING, &rcd.encoding, "Encoding", "UTF-8"},
	{ "lang", 'l', 0, G_OPTION_ARG_STRING	, &rcd.language, "Language", "en"},
	{ "mime", 'm', 0, G_OPTION_ARG_STRING	, &rcd.mime, "Mime-type", "audio/mp3"},
	{ "rep"	, 'r', 0, G_OPTION_ARG_STRING , &rcd.representation, "Representation", "sample text" },
	{ "smt"	, 's', 0, G_OPTION_ARG_INT , &smartPoster, "SmartPoster", "0 or <>0"},
	{ "type", 't', 0, G_OPTION_ARG_STRING, &rcd.type, "Record type (Text, URI...", "Text" },
	{ "uri", 'u', 0, G_OPTION_ARG_STRING, &rcd.uri , "URI", "http://www.nxp.com"},
	{ "carrier", 'c', 0, G_OPTION_ARG_STRING, &rcd.carrier, "Carrier", "bluetooth"},
	{ "ssid", 'd', 0, G_OPTION_ARG_STRING, &rcd.ssid, "SSID", "ssid"},
	{ "passphrase", 'p', 0, G_OPTION_ARG_STRING, &rcd.passphrase, "Passphrase", "psk"},
	{ "encryption", 'y', 0, G_OPTION_ARG_STRING , &rcd.encryption, "List separated by ','", "NONE,WEP,TKIP,AES"},
	{ "authentication", 'z', 0, G_OPTION_ARG_STRING	, &rcd.authentication, "List separated by ','",
		"OPEN,WPA-Personal,Shared,WPA-Enterprise," "WPA2-Enterprise,WPA2-Personal"},
	{ NULL, 0, 0, 0, NULL, NULL, NULL} /* End of List */
};

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
		printf("Unable to read tag properties. (error:%d)\n", ec);
	}
	main_loop_state = tag_found;
	presence = 1;
	
}

void cb_tag_lost (const char *tagName, void *user_data)
{
	printf("---- NFC Tag lost\n\n");
	main_loop_state = tag_lost;
	presence=0;
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
		printf("---- Reading done\n");
		printf("---- Waiting for tag removal\n");
		
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
		printf("Unable to read device properties (error:%d)\n", ec);
	}
	main_loop_state = device_found;
	presence=1;
}

static void cb_dev_lost(const char *devName, void *user_data)
{
	printf("---- NFC Device lost\n");
	main_loop_state = device_lost;
	presence=0;
}

static void cb_ndef_agent (unsigned char **rcdArray, unsigned int rcdLen,unsigned char *ndefArray, unsigned int ndefLen,void *user_data)
{
	printf("\n%d bytes of NDEF raw data Received :\n", ndefLen);
	trace_dump_mem(ndefArray, ndefLen);
	printf("\n");
}


static gboolean wait_lost (gpointer data)
{	

	if ((main_loop_state == tag_lost) || (main_loop_state == device_lost))
	{
		g_main_loop_quit( (GMainLoop*)data );
		if(keystroke_state)
			main_loop_state=keystroke;
		return FALSE;
	}
	else if(main_loop_state == keystroke)
	{
		if(presence==1)// check presence
		{
			if(!keystroke_state)
			printf("---- Remove tag/device \n");
			keystroke_state=1;//
			return TRUE;
		}
		g_main_loop_quit( (GMainLoop*)data );
		return FALSE;
	}

	return TRUE;
}

static gboolean wait_found(gpointer data)
{

	if ((main_loop_state == tag_found) || (main_loop_state == device_found) || (main_loop_state == keystroke) || (main_loop_state == tag_lost))
	{
		g_main_loop_quit( (GMainLoop*)data );
		return FALSE;

	}
	

	return TRUE;
}

static void help(void)
{
	printf("OPTIONS: \n");
	printf("\tpoll\tPolling mode \t e.g. <demo poll >\n");
	printf("\twrite\tWrite tag \t e.g. <demo write -t Text -l en -e UTF-8 -r \"Test\">\n");
	printf("\tpush\tPush to device \t e.g. <demo push -t URI -u http://www.nxp.com>\n");
	printf("\n");
}

static errorCode_t cmd_prv_parseOptions(int *argc, char **argv[], GOptionEntry *options)
{
	GOptionContext	*context;
	GError		*error		= NULL;
	errorCode_t	ec		= NEARDAL_SUCCESS;

	context = g_option_context_new(NULL);

	g_option_context_add_main_entries(context, options, NULL);

	if (!g_option_context_parse(context, argc, argv, &error))
	{
		if(error != NULL)
		{
			printf("%s\n", error->message);
			g_error_free(error);
		}
		else
		{
			ec = NEARDAL_ERROR_INVALID_PARAMETER;
		}
	}

	g_option_context_free(context);
	return ec;
}

static void free_record(neardal_record record)
{
	if (record.action != NULL) g_free((gchar *) record.action);
	if (record.encoding != NULL) g_free((gchar *) record.encoding);
	if (record.language != NULL) g_free((gchar *) record.language);
	if (record.mime != NULL) g_free((gchar *) record.mime);
	if (record.representation != NULL) g_free((gchar *) record.representation);
	if (record.type != NULL) g_free((gchar *) record.type);
	if (record.uri != NULL) g_free((gchar *) record.uri);
	if (record.ssid != NULL) g_free((gchar *) record.ssid);
	if (record.passphrase != NULL) g_free((gchar *) record.passphrase);
	if (record.authentication != NULL) g_free((gchar *) record.authentication);
	if (record.encryption != NULL) g_free((gchar *) record.encryption);
}

static errorCode_t create_loop (GMainLoop **loop)
{
	*loop = g_main_loop_new(NULL, FALSE);
	if (loop == NULL)
	{
		printf("Error creating context\n");
		return NEARDAL_ERROR_NO_MEMORY;
	}
	return NEARDAL_SUCCESS;
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
		printf("---- Error starting discovery loop %d\n", ec);
		return ec;
	}
	return NEARDAL_SUCCESS;
}

static errorCode_t start_thread (void *routine)
{
	if (0 != pthread_create(&tid, NULL, routine, NULL))
	{
 		printf("can't create thread\n");
		return NEARDAL_ERROR_NO_MEMORY;	
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

static void cmd_poll(void)
{
	printf("#####################################\n");
	printf("##     NFC demo through Neardal    ##\n");
	printf("#####################################\n");

	if (create_loop(&main_loop) != NEARDAL_SUCCESS) return;

	do
	{
		main_loop_state = wait;

		/* Start discovery Loop */
		if (start_discovery(NEARD_ADP_MODE_DUAL) != NEARDAL_SUCCESS) return;	

		printf("---- Waiting for a Tag/Device");
		/* Start a thread to get keystroke */
		if (start_thread(&wait_for_keystroke) != NEARDAL_SUCCESS) return;


		/* Wait for tag/device lost to restart the discovery or for keystroke to leave */
		g_timeout_add (100 , wait_lost , main_loop);
		g_main_loop_run(main_loop);

	/* loop until keystroke */
	} while (main_loop_state != keystroke);


	g_main_loop_unref(main_loop);
	neardal_stop_poll(AdpName);
	printf("Leaving ...\n");
}

static void cmd_write(int argc, char *argv[])
{
	errorCode_t ec = NEARDAL_SUCCESS;

	memset(&rcd, 0, sizeof(neardal_record));

	if (argc > 2) {
		/* Parse options */
		ec = cmd_prv_parseOptions(&argc, &argv, options);
	} else
		ec = NEARDAL_ERROR_INVALID_PARAMETER;

	if (ec != NEARDAL_SUCCESS)
	{
		printf("\nexample of use: demo write -t Text -l en -e UTF-8 -r \"Test\"");
		printf("\n<demo write -h> for more help\n");
	}
	else
	{
		if (create_loop(&main_loop) != NEARDAL_SUCCESS) return;

		printf("---- Waiting for a Tag to write");

		/* Start a thread to get keystroke */
		if (start_thread(&wait_for_keystroke) != NEARDAL_SUCCESS) return;

		while (1)
		{
			/* Start discovery Loop */
			if (start_discovery(NEARD_ADP_MODE_INITIATOR) != NEARDAL_SUCCESS) return;	

			/* Wait for tag/device found or for keystroke */
			main_loop_state = wait;
			g_timeout_add (100, wait_found, main_loop);
			g_main_loop_run(main_loop);
			

			if (main_loop_state == tag_found)
			{
				/* Tag found, write data */
				rcd.name=TagName;

				sleep(1);//workaround crash neard

				if (neardal_tag_write(&rcd) != NEARDAL_SUCCESS)
					printf("\t---- Failed to write !!!\n");
				else
					printf("\t---- Write sucessful !\n");
				break;
			}
			else if (main_loop_state == device_found)
			{
				/* Device found, wait for Device lost before restarting */
				g_timeout_add (100 , wait_lost , main_loop);
				g_main_loop_run(main_loop);

				if (main_loop_state == keystroke) 
				{
					printf("Leaving ...\n");
					break;
				}
			}
			else if (main_loop_state == tag_lost)
			{
				printf("\t---- Failed to write\n");
				break;
			
			}
			else
			{
				/* Stop Discovery Loop*/
				neardal_stop_poll(AdpName);
				break;
			}
		}

		main_loop_state = keystroke;
		g_timeout_add (100, wait_lost, main_loop);
		g_main_loop_run(main_loop);
		g_main_loop_unref(main_loop);
	}

	free_record(rcd);
	printf("Leaving ...\n");
}

static void cmd_push(int argc, char *argv[])
{
	errorCode_t		ec = NEARDAL_SUCCESS;

	memset(&rcd, 0, sizeof(neardal_record));

	if (argc > 2) {
		/* Parse options */
		ec = cmd_prv_parseOptions(&argc, &argv, options);
	} else
		ec = NEARDAL_ERROR_INVALID_PARAMETER;

	if (ec != NEARDAL_SUCCESS)
	{
		printf("\nexample of use: demo push -t URI -u http://www.nxp.com");
		printf("\n<demo push -h> for more help\n");
	}
	else
	{
		if (create_loop(&main_loop) != NEARDAL_SUCCESS) return;

		main_loop_state = wait;

			
		printf("---- Waiting for a Device to push");
		/* Start a thread to get keystroke */
		if (start_thread(&wait_for_keystroke) != NEARDAL_SUCCESS) return;

		while (1)
		{
			/* Start discovery Loop */
			if (start_discovery(NEARD_ADP_MODE_DUAL) != NEARDAL_SUCCESS) return;	



			/* Wait for tag/device found or for keystroke */
			main_loop_state = wait;
			g_timeout_add (100, wait_found, main_loop);
			g_main_loop_run(main_loop);

			if (main_loop_state == device_found)
			{
				/* Device found, push data */
				rcd.name=DevName;
				if (neardal_dev_push(&rcd) != NEARDAL_SUCCESS)
					printf("\t---- Failed to push !!!\n");
				else
					printf("\t---- Push sucessful !\n");
				break;
			}
			else if (main_loop_state == tag_found)
			{
				/* Tag found, wait for tag lost before restarting */
				g_timeout_add (100 , wait_lost , main_loop);
				g_main_loop_run(main_loop);

				if (main_loop_state == keystroke) 
				{
					printf("Leaving ...\n");
					break;
				}
			}
			else
			{
				/* Stop Discovery Loop*/
				neardal_stop_poll(AdpName);
				break;
			}
		}
		main_loop_state = keystroke;
		g_timeout_add (100, wait_lost, main_loop);
		g_main_loop_run(main_loop);
		g_main_loop_unref(main_loop);
	}

	free_record(rcd);
	printf("Leaving ...\n");
}

int main(int argc, char *argv[])
{
	errorCode_t	ec;
	char		**adpArray = NULL;
	int		adpLen;
	neardal_adapter	*adapter;
	static int	power = 1;

	g_argc=argc;
	g_argv=argv;

	printf("\n");

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

	/* Register callbacks */
	neardal_set_cb_tag_found(cb_tag_found, NULL);
	neardal_set_cb_tag_lost(cb_tag_lost, NULL);
	neardal_set_cb_dev_found(cb_dev_found, NULL);
	neardal_set_cb_dev_lost(cb_dev_lost, NULL);

	if (argc<2)
	{
		printf("Missing argument\n");
		help();
	}
	else if (strcmp(argv[1],"poll") == 0)
	{
		neardal_agent_set_NDEF_cb("urn:nfc:wkt:U", cb_ndef_agent, NULL, NULL);
		neardal_set_cb_record_found(cb_record_found, NULL);
		cmd_poll();
	}
	else if(strcmp(argv[1],"write") == 0)
	{
		cmd_write(g_argc,g_argv);
	}
	else if(strcmp(argv[1],"push") == 0)
	{
		cmd_push(g_argc,g_argv);
	}
	else
	{
		help();
	}

	printf("\n");
	return 0;
}
