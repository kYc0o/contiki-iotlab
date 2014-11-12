/*
 * Copyright (c) 2013, Matthias Kovatsch
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Erbium (Er) REST Engine example (with CoAP-specific code)
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "contiki-net.h"


/* Define which resources to include to meet memory constraints. */
#define REST_RES_HELLO 1
#define REST_RES_CHUNKS 0
#define REST_RES_SEPARATE 0
#define REST_RES_PUSHING 0
#define REST_RES_EVENT 0
#define REST_RES_SUB 0
#define REST_RES_LEDS 1
#define REST_RES_TOGGLE 0
#define REST_RES_LIGHT 0
#define REST_RES_BATTERY 0
#define REST_RES_RADIO 0
#define REST_RES_MIRROR 0 /* causes largest code size */



#include "erbium.h"


#if defined (PLATFORM_HAS_BUTTON)
#include "dev/button-sensor.h"
#endif
#if defined (PLATFORM_HAS_LEDS)
#include "dev/leds.h"
#endif
#if defined (PLATFORM_HAS_LIGHT)
#include "dev/light-sensor.h"
/*#include "dev/diverSE-sensors.h"*/
#endif
#if defined (PLATFORM_HAS_BATTERY)
#include "dev/battery-sensor.h"
/*#include "dev/diverSE-sensors.h"*/
#endif
#if defined (PLATFORM_HAS_SHT11)
#include "dev/sht11-sensor.h"
#endif
#if defined (PLATFORM_HAS_RADIO)
#include "dev/radio-sensor.h"
#endif


/* For CoAP-specific example: not required for normal RESTful Web service. */
#if WITH_COAP == 3
#include "er-coap-03.h"
#elif WITH_COAP == 7
#include "er-coap-07.h"
#elif WITH_COAP == 12
#include "er-coap-12.h"
#elif WITH_COAP == 13
#include "er-coap-13.h"
#else
#warning "Erbium example without CoAP-specifc functionality"
#endif /* CoAP-specific example */

/* Includes for stm32f4-diverSE */
/*#include "stepup.h"*/

#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF("[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x]", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF("[%02x:%02x:%02x:%02x:%02x:%02x]",(lladdr)->addr[0], (lladdr)->addr[1], (lladdr)->addr[2], (lladdr)->addr[3],(lladdr)->addr[4], (lladdr)->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#define PRINTLLADDR(addr)
#endif
/* Dynamic loading */
#include "loader/elfloader.h"
#include "cfs/cfs.h"
/* Defines by kYc0o  */
/*#include "er-coap-13-engine.h"*/
#define REST_RES_PUT 1
#define REST_RES_MODELS 0
#define COAP_CLIENT_ENABLED 0
#define MAX_KEVMOD_BODY 10240
#define MAX_KEVMODEL_SIZE 5*1024
#define CHUNKS_TOTAL 2097152

/*#define SERVER_NODE(ipaddr)   uip_ip6addr(ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0x1)
#define LOCAL_PORT      UIP_HTONS(COAP_DEFAULT_PORT+1)
#define REMOTE_PORT     UIP_HTONS(COAP_DEFAULT_PORT)*/

static int32_t large_update_size = 0;
static uint8_t large_update_store[MAX_KEVMOD_BODY] = {0};
static unsigned int large_update_ct = -1;
int32_t strAcc = 0;
int32_t length = 0;
int32_t length2 = 0;
uint16_t pref_size = 0;

static void
print_local_addresses(void)
{
	int i;
	uint8_t state;

	PRINTF("Server IPv6 addresses: \n");

	for(i = 0; i < UIP_DS6_ADDR_NB; i++)
	{
		state = uip_ds6_if.addr_list[i].state;

		if(uip_ds6_if.addr_list[i].isused && (state == ADDR_TENTATIVE || state == ADDR_PREFERRED))
		{
			PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
			PRINTF("\n");
		}
	}
}

/******************************************************************************/
#if REST_RES_MODELS
/*
 * Resource for GET and PUT models in .kev format (text or bytes)
 */

RESOURCE(models, METHOD_GET | METHOD_PUT, "models", "tile=\"?modelname=model_name.kev GET/PUT plain text\"; rt=\"Control & Data\"");

void
models_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	PRINTF("Entering models_handler \n");
	PRINTF("offset: %ld\n", *offset);

	if (*offset >= CHUNKS_TOTAL)
	{
		REST.set_response_status(response, REST.status.BAD_OPTION);
		/* A block error message should not exceed the minimum block size (16).*/
		const char *error_msg = "BlockOutOfScope";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		PRINTF("ERROR: Block out of scope.\n");
		return;
	}

	size_t len = 0;
	int32_t strpos = 0;
	const char *filename = NULL;
	const char modelname[32] = {0};
	coap_packet_t *const coap_req = (coap_packet_t *) request;
	uint8_t method = REST.get_method_type(request);
	char buf[preferred_size];
	const char *modelLengthStr = NULL;
	char modelLengthChar[10];
	int modelLength = 0;
	int fd_read;
	int32_t n = 0;

	if ((len=(REST.get_query_variable(request, "modelname", &filename))))
	{
		memcpy(modelname, filename, len);

		if (method & METHOD_GET)
		{
			PRINTF("Method GET\nmodel name (size): %i\nmodel name: %s\n", len, modelname);

			fd_read = cfs_open(modelname, CFS_READ);

			if (fd_read != -1)
			{
				PRINTF("Sending %s model\n", modelname);
				/*PRINTF("*offset = %ld\npreferred_size = %d\nstrAcc = %ld\n", *offset, preferred_size, strAcc);*/


				/* strAcc is 0 when the request is made for the first time. We read the file and calculate size */
				if (strAcc == 0)
				{
					PRINTF("First read!\n");

					/*if (fd_read != -1)
                    {*/
					length = cfs_seek(fd_read, 0 , CFS_SEEK_END);
					cfs_seek(fd_read, 0, CFS_SEEK_SET);

					PRINTF("Model length: %ld \n", length);
					length2 = length;
					/*}
                    else
                    {
                        REST.set_response_status(response, REST.status.BAD_OPTION);*/
					/* A block error message should not exceed the minimum block size (16). */
					/*const char *error_msg = "FileCouldNotRead";*/
					/*REST.set_response_payload(response, error_msg, strlen(error_msg));
                        PRINTF("ERROR: could not read from memory.\n");
                        return;
                    }*/
				}

				/*fd_read = cfs_open(modelname, CFS_READ);*/

				/* Send data until reaching file lentgh.*/
				if (strpos < preferred_size && fd_read != -1)
				{
					if (length2 - strAcc >= preferred_size)
					{
						/* strAcc is 0 when the request is made for the first time, we must read the first "prefered_size" bytes */
						if (strAcc == 0)
						{
							n = cfs_read(fd_read, buf, sizeof(buf));
							/* Stock the pointer wih the last "preferred_size" */
							pref_size = preferred_size;
							/*PRINTF("FIRST TIME of reading\n");*/
						}
						else
						{
							/* For the second and next requests we seek for the right data in the file, then cumulate the pointer */
							cfs_seek(fd_read, pref_size, CFS_SEEK_SET);
							n = cfs_read(fd_read, buf, sizeof(buf));
							pref_size += preferred_size;
							/*PRINTF("data SEEKED and READED\n");*/
						}
					}
					else
					{
						/* When the last bytes are less than a complete "preferred_size" block, we read only these last bytes */
						cfs_seek(fd_read, pref_size, CFS_SEEK_SET);
						n = cfs_read(fd_read, buf, (length2 - strAcc));
						PRINTF("last read! \n");
						/*strAcc = 0;*/
					}
					/*PRINTF("bytes readed %ld\n", n);*/
					cfs_close(fd_read);
					strpos += snprintf((char *)buffer, preferred_size - strpos + 1, buf);
					length -= strpos;
					/*PRINTF("length = %ld\n", length);
                    PRINTF("strpos = %ld \n", strpos);*/
				}
				else
				{
					REST.set_response_status(response, REST.status.BAD_OPTION);
					/* A block error message should not exceed the minimum block size (16). */
					const char *error_msg = "FileCouldNotRead";
					REST.set_response_payload(response, error_msg, strlen(error_msg));
					PRINTF("ERROR: could not read from memory.\n");
					return;
				}

				/* snprintf() does not adjust return value if truncated by size.*/
				if (strpos > preferred_size)
				{
					strpos = preferred_size;
					/*PRINTF("strpos = prefered_size, strpos : %ld \n", strpos);*/
				}

				/* Truncate if above CHUNKS_TOTAL bytes. */
				if (/* *offset*/ strAcc + (int32_t)strpos > length2)
				{
					strpos = length2 - strAcc;/* *offset; */
					/*PRINTF("strpos = length2 - *offset : %ld \n", strpos);*/
				}

				/* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
				REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
				REST.set_header_etag(response, (uint8_t *) &strpos, 1);
				REST.set_response_payload(response, buffer, strpos);

				/* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
				*offset += strpos;
				strAcc += strpos;
				PRINTF("offset: %ld \nstrAcc = %ld\n", *offset, strAcc);

				PRINTF("Length = %ld\n", length2);
				/* Signal end of resource representation. */
				if (/* *offset*/ strAcc >= length2)
				{
					*offset = -1;
					strAcc = 0;
					length = 0;
					/*PRINTF("*offset >= length, offset : %ld \n", *offset);*/
				}
			}
			else
			{
				REST.set_response_status(response, REST.status.BAD_OPTION);
				/* A block error message should not exceed the minimum block size (16). */
				const char *error_msg = "ModelUnavailable";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				PRINTF("ERROR: Model unavailable.\n");
				return;
			}
		}
		else if (method & METHOD_PUT)
		{
			PRINTF("Method PUT\nmodel name(size): %i\nmodel name: %s\n", len, modelname);

			int fd_write = 0;
			int fd_read = 0;
			int n = 0;
			uint8_t *incoming = NULL;

			if ((len = (REST.get_query_variable(request, "length", &modelLengthStr))))
			{
				memcpy(modelLengthChar, modelLengthStr, len);
				printf("String model legth = %s\n", modelLengthChar);
				modelLength = atoi(modelLengthChar);
				printf("Integer model length = %d\n", modelLength);
			}
			else
				printf("Cannot get second query variable\n");

			/*unsigned int ct = REST.get_header_content_type(request);

            if (ct==-1)
            {
                REST.set_response_status(response, REST.status.BAD_REQUEST);
                const char *error_msg = "NoContentType";
                REST.set_response_payload(response, error_msg, strlen(error_msg));
                PRINTF("ERROR: No content type.\n");
                return;
            }*/
			if (pref_size != 0 && coap_req->block1_num == 0)
			{
				fd_read = cfs_open(modelname, CFS_READ);

				if (cfs_seek(fd_read, 0, CFS_SEEK_SET) == -1)
				{
					PRINTF("New file can be written\n");
				}
				else
				{
					cfs_remove(modelname);
					fd_read = cfs_open(modelname, CFS_READ);
					if (fd_read == -1)
					{
						PRINTF("Same model name has been found, overwritting...\n");
					}
					else
					{
						PRINTF("ERROR: could read from memory, file exists.\n");
						cfs_close(fd_read);
					}
				}
			}

			if ((len = REST.get_request_payload(request, (const uint8_t **) &incoming)))
			{
				/*if ((len2 = REST.get_query_variable(request, "modelname", &filename)))
                {
                    PRINTF("File name %.*s\n", len2, filename);
                }*/

				if (coap_req->block1_num*coap_req->block1_size+len <= sizeof(large_update_store))
				{
					if (coap_req->block1_num == 0)
					{
						fd_write = cfs_open(modelname, CFS_WRITE);
						PRINTF("Writing data...\n");
					}
					else
					{
						fd_write = cfs_open(modelname, CFS_WRITE | CFS_APPEND);
						PRINTF("Appending data...\n");
					}
					if(fd_write != -1)
					{
						n = cfs_write(fd_write, incoming, len);
						cfs_close(fd_write);
						length += n;
						PRINTF("Successfully appended data to cfs. wrote %i bytes, Acc=%ld\n", n, length);
					}
					else
					{
						REST.set_response_status(response, REST.status.BAD_OPTION);
						/* A block error message should not exceed the minimum block size (16). */
						const char *error_msg = "CannotWriteCFS";
						REST.set_response_payload(response, error_msg, strlen(error_msg));
						PRINTF("ERROR: could not write to memory \n");
						return;
					}
					large_update_size = coap_req->block1_num*coap_req->block1_size+len;
					large_update_ct = REST.get_header_content_type(request);

					REST.set_response_status(response, REST.status.CHANGED);
					coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);

					PRINTF("Chunk num. : %ld Size: %d \n", coap_req->block1_num, coap_req->block1_size);
				}
				else
				{
					REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
					REST.set_response_payload(response, buffer, snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%uB max.", sizeof(large_update_store)));
					return;
				}

				if(modelLength == length)
				{
					PRINTF("File transferred successfully\n");
					strAcc = length = length2 = 0;
					*offset = -1;
				}

			}
			else
			{
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				const char *error_msg = "NoPayload";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				return;
			}
		}

	}
	else
	{
		REST.set_response_status(response, REST.status.BAD_OPTION);
		/* A block error message should not exceed the minimum block size (16). */
		const char *error_msg = "BadQueryVariable";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		PRINTF("ERROR: Bad query variable.\n");
		return;
	}
}
#endif

#if REST_RES_PUT
/*
 * PUT in flash new application modules
 */

RESOURCE(putData, METHOD_PUT | METHOD_GET, "data", "tile=\"ELF MODULE: ?filename='filename.ce', PUT APPLICATION/OCTET_STREAM\"; rt=\"Control\"");

void
putData_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	PRINTF("Entering putData_handler \n");

	coap_packet_t *const coap_req = (coap_packet_t *) request;
	uint8_t method = REST.get_method_type(request);
	int fd_write = 0;
	int fd_read = 0;
	int n = 0;
	int32_t strpos = 0;
	const char *binLengthStr;
	const char *filename = NULL;
	char binName[32] = {0};
	char binLengthChar[10];
	char buf[preferred_size];
	uint8_t *incoming = NULL;
	size_t len = 0;
	int binLength = 0;

	/*unsigned int ct = REST.get_header_content_type(request);
    if (ct==-1)
    {
        REST.set_response_status(response, REST.status.BAD_REQUEST);
        const char *error_msg = "NoContentType";
        REST.set_response_payload(response, error_msg, strlen(error_msg));
        return;
    }*/
	if (*offset >= CHUNKS_TOTAL)
	{
		REST.set_response_status(response, REST.status.BAD_OPTION);
		/* A block error message should not exceed the minimum block size (16).*/
		const char *error_msg = "BlockOutOfScope";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		PRINTF("ERROR: Block out of scope.\n");
		return;
	}

	if ((len=(REST.get_query_variable(request, "filename", &filename))))
	{
		memcpy(binName, filename, len);

		if (method & METHOD_GET)
		{
			PRINTF("Method GET\nELF module name (size): %i\nELF module name: %s\n", len, binName);

			fd_read = cfs_open(binName, CFS_READ);

			if (fd_read != -1)
			{
				PRINTF("Sending %s ELF module\n", binName);
				PRINTF("*offset = %ld\npreferred_size = %d\nstrAcc = %ld\n", *offset, preferred_size, strAcc);


				/* strAcc is 0 when the request is made for the first time. We read the file and calculate size */
				if (strAcc == 0)
				{
					if (fd_read != -1)
					{
						length = cfs_seek(fd_read, 0 , CFS_SEEK_END);
						cfs_seek(fd_read, 0, CFS_SEEK_SET);

						PRINTF("ELF Module length: %ld \n", length);
						length2 = length;
					}
					else
					{
						*offset = -1;
						strAcc = 0;
						length = 0;
						REST.set_response_status(response, REST.status.BAD_OPTION);
						/* A block error message should not exceed the minimum block size (16). */
						const char *error_msg = "FileCouldNotRead";
						REST.set_response_payload(response, error_msg, strlen(error_msg));
						PRINTF("ERROR: could not read from memory.\n");
						return;
					}
				}

				/* Send data until reaching file lentgh.*/
				if (strpos < preferred_size && fd_read != -1)
				{
					if (length2 - strAcc >= preferred_size)
					{
						/* strAcc is 0 when the request is made for the first time, we must read the first "prefered_size" bytes */
						if (strAcc == 0)
						{
							n = cfs_read(fd_read, buf, sizeof(buf));
							/* Stock the pointer wih the last "preferred_size" */
							pref_size = preferred_size;
							PRINTF("FIRST TIME of reading\n");
						}
						else
						{
							/* For the second and next requests we seek for the right data in the file, then cumulate the pointer */
							cfs_seek(fd_read, pref_size, CFS_SEEK_SET);
							n = cfs_read(fd_read, buf, sizeof(buf));
							pref_size += preferred_size;
							/*PRINTF("data SEEKED and READED\n");*/
						}
					}
					else
					{
						/* When the last bytes are less than a complete "preferred_size" block, we read only these last bytes */
						cfs_seek(fd_read, pref_size, CFS_SEEK_SET);
						n = cfs_read(fd_read, buf, (length2 - strAcc));
						PRINTF("last read! \n");
						/*strAcc = 0;*/
					}
					/*PRINTF("bytes readed %ld\n", n);*/
					cfs_close(fd_read);
					strpos += snprintf((char *)buffer, preferred_size - strpos + 1, buf);
					length -= strpos;
					PRINTF("length = %ld\n", length);
					PRINTF("strpos = %ld \n", strpos);
				}
				else
				{
					*offset = -1;
					strAcc = 0;
					length = 0;
					REST.set_response_status(response, REST.status.BAD_OPTION);
					/* A block error message should not exceed the minimum block size (16). */
					const char *error_msg = "FileCouldNotRead";
					REST.set_response_payload(response, error_msg, strlen(error_msg));
					PRINTF("ERROR: could not read from memory.\n");
					return;
				}

				/* snprintf() does not adjust return value if truncated by size.*/
				if (strpos > preferred_size)
				{
					strpos = preferred_size;
					/*PRINTF("strpos = prefered_size, strpos : %ld \n", strpos);*/
				}

				/* Truncate if above CHUNKS_TOTAL bytes. */
				if (/* *offset*/ strAcc + (int32_t)strpos > length2)
				{
					strpos = length2 - strAcc;/* *offset; */
					PRINTF("strpos = length2 - *offset : %ld \n", strpos);
				}

				/* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
				REST.set_header_content_type(response, REST.type.APPLICATION_OCTET_STREAM); /* text/plain is the default, hence this option could be omitted. */
				REST.set_header_etag(response, (uint8_t *) &strpos, 1);
				REST.set_response_payload(response, buffer, strpos);

				/* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
				*offset += strpos;
				strAcc += strpos;
				PRINTF("offset: %ld \nstrAcc = %ld\n", *offset, strAcc);

				PRINTF("Length = %ld\n", length2);
				/* Signal end of resource representation. */
				if (/* *offset*/ strAcc >= length2)
				{
					*offset = -1;
					strAcc = 0;
					length = 0;
					/*PRINTF("*offset >= length, offset : %ld \n", *offset);*/
				}
			}
			else
			{
				*offset = -1;
				strAcc = 0;
				length = 0;
				REST.set_response_status(response, REST.status.BAD_OPTION);
				/* A block error message should not exceed the minimum block size (16). */
				const char *error_msg = "ELFUnavailable";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				PRINTF("ERROR: ELF Module unavailable.\n");
				return;
			}
		}
		else if (method & METHOD_PUT)
		{
			if ((len = (REST.get_query_variable(request, "length", &binLengthStr))))
			{
				memcpy(binLengthChar, binLengthStr, len);
				printf("String ELF module legth = %s\n", binLengthChar);
				binLength = atoi(binLengthChar);
				printf("Integer ELF module length = %d\n", binLength);
			}
			else
				printf("Cannot get second query variable\n");

			/*unsigned int ct = REST.get_header_content_type(request);

            if (ct==-1)
            {
                REST.set_response_status(response, REST.status.BAD_REQUEST);
                const char *error_msg = "NoContentType";
                REST.set_response_payload(response, error_msg, strlen(error_msg));
                PRINTF("ERROR: No content type.\n");
                return;
            }*/
			if (pref_size != 0 && coap_req->block1_num == 0)
			{
				fd_read = cfs_open(binName, CFS_READ);

				if (cfs_seek(fd_read, 0, CFS_SEEK_SET) == -1)
				{
					PRINTF("New file can be written\n");
				}
				else
				{
					cfs_remove(binName);
					fd_read = cfs_open(binName, CFS_READ);

					if (fd_read == -1)
					{
						PRINTF("Same ELF module name has been found, overwritting...\n");
					}
					else
					{
						PRINTF("ERROR: could read from memory, file exists.\n");
						cfs_close(fd_read);
					}
				}
			}

			if ((len = REST.get_request_payload(request, (const uint8_t **) &incoming)))
			{
				/*if ((len2 = REST.get_query_variable(request, "modelname", &filename)))
                {
                    PRINTF("File name %.*s\n", len2, filename);
                }*/

				if (coap_req->block1_num*coap_req->block1_size+len <= sizeof(large_update_store))
				{
					if (coap_req->block1_num == 0)
					{
						fd_write = cfs_open(binName, CFS_WRITE);
						PRINTF("Writing data...\n");
					}
					else
					{
						fd_write = cfs_open(binName, CFS_WRITE | CFS_APPEND);
						PRINTF("Appending data...\n");
					}
					if(fd_write != -1)
					{
						n = cfs_write(fd_write, incoming, len);
						cfs_close(fd_write);
						length += n;
						PRINTF("Successfully appended data to cfs. wrote %i bytes, Acc=%ld\n", n, length);
					}
					else
					{
						REST.set_response_status(response, REST.status.BAD_OPTION);
						/* A block error message should not exceed the minimum block size (16). */
						const char *error_msg = "CannotWriteCFS";
						REST.set_response_payload(response, error_msg, strlen(error_msg));
						PRINTF("ERROR: could not write to memory \n");
						return;
					}
					large_update_size = coap_req->block1_num*coap_req->block1_size+len;
					large_update_ct = REST.get_header_content_type(request);

					REST.set_response_status(response, REST.status.CHANGED);
					coap_set_header_block1(response, coap_req->block1_num, 0, coap_req->block1_size);

					PRINTF("Chunk num. : %ld Size: %d \n", coap_req->block1_num, coap_req->block1_size);
				}
				else
				{
					REST.set_response_status(response, REST.status.REQUEST_ENTITY_TOO_LARGE);
					REST.set_response_payload(response, buffer, snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%uB max.", sizeof(large_update_store)));
					return;
				}

				if(binLength == length)
				{
					printf("Binary transfer ended, loading started. \n");
					printf("ELF module lenght = %ld\n", length);

					static int fd;
					static int ret;

					fd = cfs_open(binName, CFS_READ);

					if (fd != -1)
					{
						/*int i;
						char bin[1270];

						cfs_read(fd, bin, 1260);

						for(i = 0; i < 1260; i++)
						{
							printf("%2x", bin[i]);
						}*/

						/*elfloader_init();*/
						ret = elfloader_load(fd);

						if(ret == ELFLOADER_OK)
						{
							printf("Ok, starting new program.\n");
							/* Start processes. */
							autostart_start(elfloader_autostart_processes);
						}
						else
						{
							printf("Error %d %s\n", ret, elfloader_unknown);
						}

						strAcc = length = length2 = 0;
						*offset = -1;
						large_update_size = 0;
						large_update_ct = -1;
						pref_size = 0;
					}
					else
					{
						printf("Cannot load module\n");
						strAcc = length = length2 = 0;
						*offset = -1;
						large_update_size = 0;
						large_update_ct = -1;
						pref_size = 0;
					}

					/*fd_read = cfs_open(binName, CFS_READ);
					length = cfs_seek(fd_read, 0 , CFS_SEEK_END);
					cfs_seek(fd_read, 0, CFS_SEEK_SET);
					PRINTF("ELF Module length: %ld \n", length);

					strAcc = length = length2 = 0;
					*offset = -1;
					large_update_size = 0;
					large_update_ct = -1;
					pref_size = 0;*/
				}

			}
			else
			{
				REST.set_response_status(response, REST.status.BAD_REQUEST);
				const char *error_msg = "NoPayload";
				REST.set_response_payload(response, error_msg, strlen(error_msg));
				return;
			}
		}
	}


}
#endif

/******************************************************************************/
#if REST_RES_HELLO
/*
 * Resources are defined by the RESOURCE macro.
 * Signature: resource name, the RESTful methods it handles, and its URI path (omitting the leading slash).
 */
RESOURCE(helloworld, METHOD_GET, "hello", "title=\"Hello world: ?len=0..\";rt=\"Text\"");

/*
 * A handler function named [resource name]_handler must be implemented for each RESOURCE.
 * A buffer for the response payload is provided through the buffer pointer. Simple resources can ignore
 * preferred_size and offset, but must respect the REST_MAX_CHUNK_SIZE limit for the buffer.
 * If a smaller block size is requested for CoAP, the REST framework automatically splits the data.
 */
void
helloworld_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	const char *len = NULL;
	/* Some data that has the length up to REST_MAX_CHUNK_SIZE. For more, see the chunk resource. */
	char const * const message = "Hello World! ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxy";
	int length = 12; /*           |<-------->| */

	/* The query string can be retrieved by rest_get_query() or parsed for its key-value pairs. */
	if (REST.get_query_variable(request, "len", &len)) {
		length = atoi(len);
		if (length<0) length = 0;
		if (length>REST_MAX_CHUNK_SIZE) length = REST_MAX_CHUNK_SIZE;
		memcpy(buffer, message, length);
	} else {
		memcpy(buffer, message, length);
	}

	REST.set_header_content_type(response, REST.type.TEXT_PLAIN); /* text/plain is the default, hence this option could be omitted. */
	REST.set_header_etag(response, (uint8_t *) &length, 1);
	REST.set_response_payload(response, buffer, length);
}
#endif

/******************************************************************************/
#if REST_RES_MIRROR
/* This resource mirrors the incoming request. It shows how to access the options and how to set them for the response. */
RESOURCE(mirror, METHOD_GET | METHOD_POST | METHOD_PUT | METHOD_DELETE, "debug/mirror", "title=\"Returns your decoded message\";rt=\"Debug\"");

void
mirror_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	/* The ETag and Token is copied to the header. */
	uint8_t opaque[] = {0x0A, 0xBC, 0xDE};

	/* Strings are not copied, so use static string buffers or strings in .text memory (char *str = "string in .text";). */
	static char location[] = {'/','f','/','a','?','k','&','e', 0};

	/* Getter for the header option Content-Type. If the option is not set, text/plain is returned by default. */
	unsigned int content_type = REST.get_header_content_type(request);

	/* The other getters copy the value (or string/array pointer) to the given pointers and return 1 for success or the length of strings/arrays. */
	uint32_t max_age_and_size = 0;
	const char *str = NULL;
	uint32_t observe = 0;
	const uint8_t *bytes = NULL;
	uint32_t block_num = 0;
	uint8_t block_more = 0;
	uint16_t block_size = 0;
	const char *query = "";
	int len = 0;

	/* Mirror the received header options in the response payload. Unsupported getters (e.g., rest_get_header_observe() with HTTP) will return 0. */

	int strpos = 0;
	/* snprintf() counts the terminating '\0' to the size parameter.
	 * The additional byte is taken care of by allocating REST_MAX_CHUNK_SIZE+1 bytes in the REST framework.
	 * Add +1 to fill the complete buffer, as the payload does not need a terminating '\0'. */
	if (content_type!=-1)
	{
		strpos += snprintf((char *)buffer, REST_MAX_CHUNK_SIZE+1, "CT %u\n", content_type);
	}

	/* Some getters such as for ETag or Location are omitted, as these options should not appear in a request.
	 * Max-Age might appear in HTTP requests or used for special purposes in CoAP. */
	if (strpos<=REST_MAX_CHUNK_SIZE && REST.get_header_max_age(request, &max_age_and_size))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "MA %lu\n", max_age_and_size);
	}
	/* For HTTP this is the Length option, for CoAP it is the Size option. */
	if (strpos<=REST_MAX_CHUNK_SIZE && REST.get_header_length(request, &max_age_and_size))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "SZ %lu\n", max_age_and_size);
	}

	if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_header_host(request, &str)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "UH %.*s\n", len, str);
	}

	/* CoAP-specific example: actions not required for normal RESTful Web service. */
#if WITH_COAP > 1
	if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_observe(request, &observe))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Ob %lu\n", observe);
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_token(request, &bytes)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "To 0x");
		int index = 0;
		for (index = 0; index<len; ++index) {
			strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%02X", bytes[index]);
		}
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n");
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_etag(request, &bytes)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "ET 0x");
		int index = 0;
		for (index = 0; index<len; ++index) {
			strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%02X", bytes[index]);
		}
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "\n");
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_uri_path(request, &str)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "UP ");
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%.*s\n", len, str);
	}
#if WITH_COAP == 3
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_location(request, &str)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Lo %.*s\n", len, str);
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_block(request, &block_num, &block_more, &block_size, NULL)) /* This getter allows NULL pointers to get only a subset of the block parameters. */
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Bl %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
	}
#else
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_location_path(request, &str)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "LP %.*s\n", len, str);
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = coap_get_header_location_query(request, &str)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "LQ %.*s\n", len, str);
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_block2(request, &block_num, &block_more, &block_size, NULL)) /* This getter allows NULL pointers to get only a subset of the block parameters. */
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "B2 %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
	}
	/*
	 * Critical Block1 option is currently rejected by engine.
	 *
  if (strpos<=REST_MAX_CHUNK_SIZE && coap_get_header_block1(request, &block_num, &block_more, &block_size, NULL))
  {
    strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "B1 %lu%s (%u)\n", block_num, block_more ? "+" : "", block_size);
  }
	 */
#endif /* CoAP > 03 */
#endif /* CoAP-specific example */

	if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_query(request, &query)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "Qu %.*s\n", len, query);
	}
	if (strpos<=REST_MAX_CHUNK_SIZE && (len = REST.get_request_payload(request, &bytes)))
	{
		strpos += snprintf((char *)buffer+strpos, REST_MAX_CHUNK_SIZE-strpos+1, "%.*s", len, bytes);
	}

	if (strpos >= REST_MAX_CHUNK_SIZE)
	{
		buffer[REST_MAX_CHUNK_SIZE-1] = 0xBB; /* '»' to indicate truncation */
	}

	REST.set_response_payload(response, buffer, strpos);

	PRINTF("/mirror options received: %s\n", buffer);

	/* Set dummy header options for response. Like getters, some setters are not implemented for HTTP and have no effect. */
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	REST.set_header_max_age(response, 17); /* For HTTP, browsers will not re-request the page for 17 seconds. */
	REST.set_header_etag(response, opaque, 2);
	REST.set_header_location(response, location); /* Initial slash is omitted by framework */
	REST.set_header_length(response, strpos); /* For HTTP, browsers will not re-request the page for 10 seconds. CoAP action depends on the client. */

	/* CoAP-specific example: actions not required for normal RESTful Web service. */
#if WITH_COAP > 1
	coap_set_header_uri_host(response, "tiki");
	coap_set_header_observe(response, 10);
#if WITH_COAP == 3
	coap_set_header_block(response, 42, 0, 64); /* The block option might be overwritten by the framework when blockwise transfer is requested. */
#else
	coap_set_header_proxy_uri(response, "ftp://x");
	coap_set_header_block2(response, 42, 0, 64); /* The block option might be overwritten by the framework when blockwise transfer is requested. */
	coap_set_header_block1(response, 23, 0, 16);
	coap_set_header_accept(response, TEXT_PLAIN);
	coap_set_header_if_none_match(response);
#endif /* CoAP > 03 */
#endif /* CoAP-specific example */
}
#endif /* REST_RES_MIRROR */

/******************************************************************************/
#if REST_RES_CHUNKS
/*
 * For data larger than REST_MAX_CHUNK_SIZE (e.g., stored in flash) resources must be aware of the buffer limitation
 * and split their responses by themselves. To transfer the complete resource through a TCP stream or CoAP's blockwise transfer,
 * the byte offset where to continue is provided to the handler as int32_t pointer.
 * These chunk-wise resources must set the offset value to its new position or -1 of the end is reached.
 * (The offset for CoAP's blockwise transfer can go up to 2'147'481'600 = ~2047 M for block size 2048 (reduced to 1024 in observe-03.)
 */
RESOURCE(chunks, METHOD_GET, "test/chunks", "title=\"Blockwise demo\";rt=\"Data\"");

#define CHUNKS_TOTAL    3000

void
chunks_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	int32_t strpos = 0;

	/* Check the offset for boundaries of the resource data. */
	if (*offset>=CHUNKS_TOTAL)
	{
		REST.set_response_status(response, REST.status.BAD_OPTION);
		/* A block error message should not exceed the minimum block size (16). */

		const char *error_msg = "BlockOutOfScope";
		REST.set_response_payload(response, error_msg, strlen(error_msg));
		return;
	}

	/* Generate data until reaching CHUNKS_TOTAL. */
	while (strpos<preferred_size)
	{
		strpos += snprintf((char *)buffer+strpos, preferred_size-strpos+1, "|%ld|", *offset);
	}

	/* snprintf() does not adjust return value if truncated by size. */
	if (strpos > preferred_size)
	{
		strpos = preferred_size;
	}

	/* Truncate if above CHUNKS_TOTAL bytes. */
	if (*offset+(int32_t)strpos > CHUNKS_TOTAL)
	{
		strpos = CHUNKS_TOTAL - *offset;
	}

	REST.set_response_payload(response, buffer, strpos);

	/* IMPORTANT for chunk-wise resources: Signal chunk awareness to REST engine. */
	*offset += strpos;

	/* Signal end of resource representation. */
	if (*offset>=CHUNKS_TOTAL)
	{
		*offset = -1;
	}
}
#endif

/******************************************************************************/
#if REST_RES_SEPARATE && defined (PLATFORM_HAS_BUTTON) && WITH_COAP > 3
/* Required to manually (=not by the engine) handle the response transaction. */
#if WITH_COAP == 7
#include "er-coap-07-separate.h"
#include "er-coap-07-transactions.h"
#elif WITH_COAP == 12
#include "er-coap-12-separate.h"
#include "er-coap-12-transactions.h"
#elif WITH_COAP == 13
#include "er-coap-13-separate.h"
#include "er-coap-13-transactions.h"
#endif
/*
 * CoAP-specific example for separate responses.
 * Note the call "rest_set_pre_handler(&resource_separate, coap_separate_handler);" in the main process.
 * The pre-handler takes care of the empty ACK and updates the MID and message type for CON requests.
 * The resource handler must store all information that required to finalize the response later.
 */
RESOURCE(separate, METHOD_GET, "test/separate", "title=\"Separate demo\"");

/* A structure to store the required information */
typedef struct application_separate_store {
	/* Provided by Erbium to store generic request information such as remote address and token. */
	coap_separate_t request_metadata;
	/* Add fields for addition information to be stored for finalizing, e.g.: */
	char buffer[16];
} application_separate_store_t;

static uint8_t separate_active = 0;
static application_separate_store_t separate_store[1];

void
separate_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	/*
	 * Example allows only one open separate response.
	 * For multiple, the application must manage the list of stores.
	 */
	if (separate_active)
	{
		coap_separate_reject();
	}
	else
	{
		separate_active = 1;

		/* Take over and skip response by engine. */
		coap_separate_accept(request, &separate_store->request_metadata);
		/* Be aware to respect the Block2 option, which is also stored in the coap_separate_t. */

		/*
		 * At the moment, only the minimal information is stored in the store (client address, port, token, MID, type, and Block2).
		 * Extend the store, if the application requires additional information from this handler.
		 * buffer is an example field for custom information.
		 */
		snprintf(separate_store->buffer, sizeof(separate_store->buffer), "StoredInfo");
	}
}

void
separate_finalize_handler()
{
	if (separate_active)
	{
		coap_transaction_t *transaction = NULL;
		if ( (transaction = coap_new_transaction(separate_store->request_metadata.mid, &separate_store->request_metadata.addr, separate_store->request_metadata.port)) )
		{
			coap_packet_t response[1]; /* This way the packet can be treated as pointer as usual. */

			/* Restore the request information for the response. */
			coap_separate_resume(response, &separate_store->request_metadata, REST.status.OK);

			coap_set_payload(response, separate_store->buffer, strlen(separate_store->buffer));

			/*
			 * Be aware to respect the Block2 option, which is also stored in the coap_separate_t.
			 * As it is a critical option, this example resource pretends to handle it for compliance.
			 */
			coap_set_header_block2(response, separate_store->request_metadata.block2_num, 0, separate_store->request_metadata.block2_size);

			/* Warning: No check for serialization error. */
			transaction->packet_len = coap_serialize_message(response, transaction->packet);
			coap_send_transaction(transaction);
			/* The engine will clear the transaction (right after send for NON, after acked for CON). */

			separate_active = 0;
		}
		else
		{
			/*
			 * Set timer for retry, send error message, ...
			 * The example simply waits for another button press.
			 */
		}
	} /* if (separate_active) */
}
#endif

/******************************************************************************/
#if REST_RES_PUSHING
/*
 * Example for a periodic resource.
 * It takes an additional period parameter, which defines the interval to call [name]_periodic_handler().
 * A default post_handler takes care of subscriptions by managing a list of subscribers to notify.
 */
PERIODIC_RESOURCE(pushing, METHOD_GET, "test/push", "title=\"Periodic demo\";obs", 5*CLOCK_SECOND);

void
pushing_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);

	/* Usually, a CoAP server would response with the resource representation matching the periodic_handler. */
	const char *msg = "It's periodic!";
	REST.set_response_payload(response, msg, strlen(msg));

	/* A post_handler that handles subscriptions will be called for periodic resources by the REST framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the REST manager process with the defined period.
 */
void
pushing_periodic_handler(resource_t *r)
{
	static uint16_t obs_counter = 0;
	static char content[11];

	++obs_counter;

	PRINTF("TICK %u for /%s\n", obs_counter, r->url);

	/* Build notification. */
	coap_packet_t notification[1]; /* This way the packet can be treated as pointer as usual. */
	coap_init_message(notification, COAP_TYPE_NON, REST.status.OK, 0 );
	coap_set_payload(notification, content, snprintf(content, sizeof(content), "TICK %u", obs_counter));

	/* Notify the registered observers with the given message type, observe option, and payload. */
	REST.notify_subscribers(r, obs_counter, notification);
}
#endif

/******************************************************************************/
#if REST_RES_EVENT && defined (PLATFORM_HAS_BUTTON)
/*
 * Example for an event resource.
 * Additionally takes a period parameter that defines the interval to call [name]_periodic_handler().
 * A default post_handler takes care of subscriptions and manages a list of subscribers to notify.
 */
EVENT_RESOURCE(event, METHOD_GET, "sensors/button", "title=\"Event demo\";obs");

void
event_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
	/* Usually, a CoAP server would response with the current resource representation. */
	const char *msg = "It's eventful!";
	REST.set_response_payload(response, (uint8_t *)msg, strlen(msg));

	/* A post_handler that handles subscriptions/observing will be called for periodic resources by the framework. */
}

/* Additionally, a handler function named [resource name]_event_handler must be implemented for each PERIODIC_RESOURCE defined.
 * It will be called by the REST manager process with the defined period. */
void
event_event_handler(resource_t *r)
{
	static uint16_t event_counter = 0;
	static char content[12];

	++event_counter;

	PRINTF("TICK %u for /%s\n", event_counter, r->url);

	/* Build notification. */
	coap_packet_t notification[1]; /* This way the packet can be treated as pointer as usual. */
	coap_init_message(notification, COAP_TYPE_CON, REST.status.OK, 0 );
	coap_set_payload(notification, content, snprintf(content, sizeof(content), "EVENT %u", event_counter));

	/* Notify the registered observers with the given message type, observe option, and payload. */
	REST.notify_subscribers(r, event_counter, notification);
}
#endif /* PLATFORM_HAS_BUTTON */

/******************************************************************************/
#if REST_RES_SUB
/*
 * Example for a resource that also handles all its sub-resources.
 * Use REST.get_url() to multiplex the handling of the request depending on the Uri-Path.
 */
RESOURCE(sub, METHOD_GET | HAS_SUB_RESOURCES, "test/path", "title=\"Sub-resource demo\"");

void
sub_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	REST.set_header_content_type(response, REST.type.TEXT_PLAIN);

	const char *uri_path = NULL;
	int len = REST.get_url(request, &uri_path);
	int base_len = strlen(resource_sub.url);

	if (len==base_len)
	{
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "Request any sub-resource of /%s", resource_sub.url);
	}
	else
	{
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, ".%.*s", len-base_len, uri_path+base_len);
	}

	REST.set_response_payload(response, buffer, strlen((char *)buffer));
}
#endif

/******************************************************************************/
#if defined (PLATFORM_HAS_LEDS)
/******************************************************************************/
#if REST_RES_LEDS
/*A simple actuator example, depending on the color query parameter and post variable mode, corresponding led is activated or deactivated*/
RESOURCE(leds, METHOD_POST | METHOD_PUT , "actuators/leds", "title=\"LEDs: ?color=r|g|y, POST/PUT mode=on|off\";rt=\"Control\"");

void
leds_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	size_t len = 0;
	const char *color = NULL;
	const char *mode = NULL;
	uint8_t led = 0;
	int success = 1;

	if ((len=REST.get_query_variable(request, "color", &color))) {
		PRINTF("color %.*s\n", len, color);

		if (strncmp(color, "r", len)==0) {
			led = LEDS_RED;
		} else if(strncmp(color,"g", len)==0) {
			led = LEDS_GREEN;
		} /*else if (strncmp(color,"b", len)==0) {
      led = LEDS_BLUE;*/
		else if (strncmp(color,"y", len)==0) {
			led = LEDS_YELLOW;
		} else {
			success = 0;
		}
	} else {
		success = 0;
	}

	if (success && (len=REST.get_post_variable(request, "mode", &mode))) {
		PRINTF("mode %s\n", mode);

		if (strncmp(mode, "on", len)==0) {
			leds_on(led);
		} else if (strncmp(mode, "off", len)==0) {
			leds_off(led);
		} else {
			success = 0;
		}
	} else {
		success = 0;
	}

	if (!success) {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
	}
}
#endif

/******************************************************************************/
#if REST_RES_TOGGLE
/* A simple actuator example. Toggles the red led */
RESOURCE(toggle, METHOD_POST, "actuators/toggle", "title=\"Red LED\";rt=\"Control\"");
void
toggle_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	leds_toggle(LEDS_RED);
}
#endif
#endif /* PLATFORM_HAS_LEDS */

/******************************************************************************/
#if REST_RES_LIGHT && defined (PLATFORM_HAS_LIGHT)
/* A simple getter example. Returns the reading from light sensor with a simple etag */
RESOURCE(light, METHOD_GET, "sensors/light", "title=\"Photosynthetic and solar light (supports JSON)\";rt=\"LightSensor\"");
void
light_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	uint16_t light_photosynthetic = light_sensor.value(LIGHT_SENSOR_PHOTOSYNTHETIC);
	uint16_t light_solar = light_sensor.value(LIGHT_SENSOR_TOTAL_SOLAR);

	const uint16_t *accept = NULL;
	int num = REST.get_header_accept(request, &accept);

	if ((num==0) || (num && accept[0]==REST.type.TEXT_PLAIN))
	{
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%u;%u", light_photosynthetic, light_solar);

		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	}
	else if (num && (accept[0]==REST.type.APPLICATION_XML))
	{
		REST.set_header_content_type(response, REST.type.APPLICATION_XML);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "<light photosynthetic=\"%u\" solar=\"%u\"/>", light_photosynthetic, light_solar);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	}
	else if (num && (accept[0]==REST.type.APPLICATION_JSON))
	{
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'light':{'photosynthetic':%u,'solar':%u}}", light_photosynthetic, light_solar);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	}
	else
	{
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		const char *msg = "Supporting content-types text/plain, application/xml, and application/json";
		REST.set_response_payload(response, msg, strlen(msg));
	}
}
#endif /* PLATFORM_HAS_LIGHT */

/******************************************************************************/
#if REST_RES_BATTERY && defined (PLATFORM_HAS_BATTERY)
/* A simple getter example. Returns the reading from light sensor with a simple etag */
RESOURCE(battery, METHOD_GET, "sensors/battery", "title=\"Battery status\";rt=\"Battery\"");
void
battery_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	/*int battery = battery_sensor.value(0);*/
	float battery = ((diverse_sensors.value(STM32_BATTERY))* 3.3) / 4095;
	const uint16_t *accept = NULL;
	int num = REST.get_header_accept(request, &accept);

	if ((num==0) || (num && accept[0]==REST.type.TEXT_PLAIN))
	{
		REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%f", battery);

		REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
	}
	else if (num && (accept[0]==REST.type.APPLICATION_JSON))
	{
		REST.set_header_content_type(response, REST.type.APPLICATION_JSON);
		snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'battery':%f}", battery);

		REST.set_response_payload(response, buffer, strlen((char *)buffer));
	}
	else
	{
		REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
		const char *msg = "Supporting content-types text/plain and application/json";
		REST.set_response_payload(response, msg, strlen(msg));
	}
}
#endif /* PLATFORM_HAS_BATTERY */


#if defined (PLATFORM_HAS_RADIO) && REST_RES_RADIO
/* A simple getter example. Returns the reading of the rssi/lqi from radio sensor */
RESOURCE(radio, METHOD_GET, "sensor/radio", "title=\"RADIO: ?p=lqi|rssi\";rt=\"RadioSensor\"");

void
radio_handler(void* request, void* response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	size_t len = 0;
	const char *p = NULL;
	uint8_t param = 0;
	int success = 1;

	const uint16_t *accept = NULL;
	int num = REST.get_header_accept(request, &accept);

	if ((len=REST.get_query_variable(request, "p", &p))) {
		PRINTF("p %.*s\n", len, p);
		if (strncmp(p, "lqi", len)==0) {
			param = RADIO_SENSOR_LAST_VALUE;
		} else if(strncmp(p,"rssi", len)==0) {
			param = RADIO_SENSOR_LAST_PACKET;
		} else {
			success = 0;
		}
	} else {
		success = 0;
	}

	if (success) {
		if ((num==0) || (num && accept[0]==REST.type.TEXT_PLAIN))
		{
			REST.set_header_content_type(response, REST.type.TEXT_PLAIN);
			snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "%d", radio_sensor.value(param));

			REST.set_response_payload(response, (uint8_t *)buffer, strlen((char *)buffer));
		}
		else if (num && (accept[0]==REST.type.APPLICATION_JSON))
		{
			REST.set_header_content_type(response, REST.type.APPLICATION_JSON);

			if (param == RADIO_SENSOR_LAST_VALUE) {
				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'lqi':%d}", radio_sensor.value(param));
			} else if (param == RADIO_SENSOR_LAST_PACKET) {
				snprintf((char *)buffer, REST_MAX_CHUNK_SIZE, "{'rssi':%d}", radio_sensor.value(param));
			}

			REST.set_response_payload(response, buffer, strlen((char *)buffer));
		}
		else
		{
			REST.set_response_status(response, REST.status.NOT_ACCEPTABLE);
			const char *msg = "Supporting content-types text/plain and application/json";
			REST.set_response_payload(response, msg, strlen(msg));
		}
	} else {
		REST.set_response_status(response, REST.status.BAD_REQUEST);
	}
}
#endif



PROCESS(rest_server_example, "Erbium Example Server");
AUTOSTART_PROCESSES(&rest_server_example);

PROCESS_THREAD(rest_server_example, ev, data)
{
	PROCESS_BEGIN();

	/*cfs_coffee_format();*/

	print_local_addresses();

	/*SENSORS_ACTIVATE(button_sensor);*/
	/*SENSORS_ACTIVATE(diverse_sensors);*/

	/*stepup_init();
  stepup_off();*/

	PRINTF("Starting Erbium Example Server\n");

#ifdef RF_CHANNEL
	PRINTF("RF channel: %u\n", RF_CHANNEL);
#endif
#ifdef IEEE802154_PANID
	PRINTF("PAN ID: 0x%04X\n", IEEE802154_PANID);
#endif

	PRINTF("uIP buffer: %u\n", UIP_BUFSIZE);
	PRINTF("LL header: %u\n", UIP_LLH_LEN);
	PRINTF("IP+UDP header: %u\n", UIP_IPUDPH_LEN);
	PRINTF("REST max chunk: %u\n", REST_MAX_CHUNK_SIZE);
	PRINTF("ELF Loader data size: %u\n", ELFLOADER_DATAMEMORY_SIZE);
	PRINTF("ELF Loader text size: %u\n", ELFLOADER_TEXTMEMORY_SIZE);

	/* Initialize the REST engine. */
	rest_init_engine();

	/* Activate the application-specific resources. */
#if REST_RES_HELLO
	rest_activate_resource(&resource_helloworld);
#endif
#if REST_RES_MIRROR
	rest_activate_resource(&resource_mirror);
#endif
#if REST_RES_CHUNKS
	rest_activate_resource(&resource_chunks);
#endif
#if REST_RES_PUSHING
	rest_activate_periodic_resource(&periodic_resource_pushing);
#endif
#if defined (PLATFORM_HAS_BUTTON) && REST_RES_EVENT
	rest_activate_event_resource(&resource_event);
#endif
#if defined (PLATFORM_HAS_BUTTON) && REST_RES_SEPARATE && WITH_COAP > 3
	/* No pre-handler anymore, user coap_separate_accept() and coap_separate_reject(). */
	rest_activate_resource(&resource_separate);
#endif
#if defined (PLATFORM_HAS_BUTTON) && (REST_RES_EVENT || (REST_RES_SEPARATE && WITH_COAP > 3))
	SENSORS_ACTIVATE(button_sensor);
#endif
#if REST_RES_SUB
	rest_activate_resource(&resource_sub);
#endif
#if defined (PLATFORM_HAS_LEDS)
#if REST_RES_LEDS
	rest_activate_resource(&resource_leds);
#endif
#if REST_RES_TOGGLE
	rest_activate_resource(&resource_toggle);
#endif
#endif /* PLATFORM_HAS_LEDS */
#if defined (PLATFORM_HAS_LIGHT) && REST_RES_LIGHT
	SENSORS_ACTIVATE(light_sensor);
	rest_activate_resource(&resource_light);
#endif
#if defined (PLATFORM_HAS_BATTERY) && REST_RES_BATTERY
	/*SENSORS_ACTIVATE(battery_sensor);*/
	SENSORS_ACTIVATE(diverse_sensors);
	rest_activate_resource(&resource_battery);
#endif
#if defined (PLATFORM_HAS_RADIO) && REST_RES_RADIO
	SENSORS_ACTIVATE(radio_sensor);
	rest_activate_resource(&resource_radio);
#endif
#if REST_RES_MODELS
	rest_activate_resource(&resource_models);
#endif
#if REST_RES_PUT
	rest_activate_resource(&resource_putData);
#endif
	/* Define application-specific events here. */
	while(1) {
		PROCESS_WAIT_EVENT();
#if defined (PLATFORM_HAS_BUTTON)
		if (ev == sensors_event && data == &button_sensor) {
			PRINTF("BUTTON\n");
#if REST_RES_EVENT
			/* Call the event_handler for this application-specific event. */
			event_event_handler(&resource_event);
#endif
#if REST_RES_SEPARATE && WITH_COAP>3
			/* Also call the separate response example handler. */
			separate_finalize_handler();
#endif
		}
#endif /* PLATFORM_HAS_BUTTON */
	} /* while (1) */

	PROCESS_END();
}
