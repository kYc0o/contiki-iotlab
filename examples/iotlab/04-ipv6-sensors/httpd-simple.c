/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         A simple web server forwarding page generation to a protothread
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Olivier Fambon <olivier.fambon@inria.fr>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "contiki-net.h"

#include "httpd-simple.h"
#define webserver_log_file(...)
#define webserver_log(...)

#define STATE_WAITING 0
#define STATE_OUTPUT  1

MEMB(conns, struct httpd_state, UIP_CONNS);

#define ISO_nl      0x0a
#define ISO_space   0x20
#define ISO_period  0x2e
#define ISO_slash   0x2f

/*---------------------------------------------------------------------------*/
const char http_content_type_html[] = "Content-type: text/html\r\n\r\n";
/*---------------------------------------------------------------------------*/
#define SERVER_STRING "\r\nServer: Contiki/2.7\r\n"
#define HTTP_VERSION  "HTTP/1.0 "
#define DEFN_HTTP_RESPONSE(code, text) \
	DECL_HTTP_RESPONSE(code) = HTTP_VERSION #code " " text SERVER_STRING;
DEFN_HTTP_RESPONSE(200, "OK");
DEFN_HTTP_RESPONSE(400, "Invalid request");
DEFN_HTTP_RESPONSE(404, "Not found");
/*---------------------------------------------------------------------------*/
static struct {
  struct httpd_query_map *map;
  httpd_validator_t validator;
} resources;
/*---------------------------------------------------------------------------*/
#define LAST(x) x[strlen(x)-1]
#define MATCH(x, y) LAST(x) == '?' ? !strncmp(x, y, strlen(x)) : !strcmp(x, y) 
static
const char *handle_request(const char *req, struct httpd_state *s)
{
  int i;
  for (i=0; resources.map[i].generator; i++) {
	if (MATCH(resources.map[i].query, req)) {
		s->generator = resources.map[i].generator;
		s->response_header = httpd_response_200;
		return resources.map[i].query;
	}
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_headers(struct httpd_state *s, const char *statushdr))
{
  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, statushdr);
  SEND_STRING(&s->sout, http_content_type_html);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_text(struct httpd_state *s, const char *text))
{
  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, text);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_output(struct httpd_state *s))
{
  PT_BEGIN(&s->outputpt);

  s->generator = NULL;
  s->response_header = NULL;
  const char *req = s->filename + 1;
  const char *path = handle_request(req, s);
  const char *params = req + strlen(path);
  resources.validator(params, s);
  if(s->generator == NULL) {
	PT_WAIT_THREAD(&s->outputpt, send_headers(s, httpd_response_404));
	PT_WAIT_THREAD(&s->outputpt, send_text(s, httpd_response_404));
  }
  else {
	PT_WAIT_THREAD(&s->outputpt, send_headers(s, s->response_header));
	PT_WAIT_THREAD(&s->outputpt, s->generator(s));
  }
  PSOCK_CLOSE(&s->sout);
  PT_END(&s->outputpt);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_input(struct httpd_state *s))
{
  static char post_data_buf[1024];
  static int  post_data_len;

  PSOCK_BEGIN(&s->sin);

  PSOCK_READTO(&s->sin, ISO_space);
  if(strncmp(s->inputbuf, "GET ", 4) && strncmp(s->inputbuf, "POST ", 5)) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  s->request_type = s->inputbuf[0];

  PSOCK_READTO(&s->sin, ISO_space);
  if(s->inputbuf[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sin);
  }
  if(s->inputbuf[1] == ISO_space) {
    strcpy(s->filename, "/");
  } else {
    s->inputbuf[PSOCK_DATALEN(&s->sin) - 1] = 0;
    strncpy(s->filename, s->inputbuf, sizeof(s->filename));
    s->filename[sizeof(s->filename) -1] = 0;
  }

  webserver_log_file(&uip_conn->ripaddr, s->filename);

  if(s->request_type == 'P') {
    do { /* search for content-length */
      PSOCK_READTO(&s->sin, ISO_nl);
    } while (strncmp("Content-Length:", s->inputbuf, 15));
    post_data_len = atoi(s->inputbuf + 15);
    do { /* skip everything until blank line */
      PSOCK_READTO(&s->sin, ISO_nl);
    } while(PSOCK_DATALEN(&s->sin) != 1);
    s->post_data = post_data_buf;
    do { /* copy post data up to content-length bytes */
      PSOCK_READTO(&s->sin, ISO_nl);
      memcpy(s->post_data, s->inputbuf, PSOCK_DATALEN(&s->sin));
      s->post_data += PSOCK_DATALEN(&s->sin);
    } while(s->post_data < post_data_buf + post_data_len);
    *s->post_data = 0;
    s->post_data = post_data_buf;
  }

  s->state = STATE_OUTPUT;

  while(1) {
    PSOCK_READTO(&s->sin, ISO_nl);
  }

  PSOCK_END(&s->sin);
}
/*---------------------------------------------------------------------------*/
static void
handle_connection(struct httpd_state *s)
{
  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
  }
}

/*---------------------------------------------------------------------------*/
static void
httpd_appcall(void *state)
{
  struct httpd_state *s = (struct httpd_state *)state;

  if(uip_closed() || uip_aborted() || uip_timedout()) {
    if(s != NULL) {
      s->generator = NULL;
      memb_free(&conns, s);
    }
  } else if(uip_connected()) {
    s = (struct httpd_state *)memb_alloc(&conns);
    if(s == NULL) {
      uip_abort();
      webserver_log_file(&uip_conn->ripaddr, "reset (no memory block)");
      return;
    }
    tcp_markconn(uip_conn, s);
    PSOCK_INIT(&s->sin, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PSOCK_INIT(&s->sout, (uint8_t *)s->inputbuf, sizeof(s->inputbuf) - 1);
    PT_INIT(&s->outputpt);
    s->generator = NULL;
    s->state = STATE_WAITING;
    timer_set(&s->timer, CLOCK_SECOND * 10);
    handle_connection(s);
  } else if(s != NULL) {
    if(uip_poll()) {
      if(timer_expired(&s->timer)) {
        uip_abort();
        s->generator = NULL;
        memb_free(&conns, s);
        webserver_log_file(&uip_conn->ripaddr, "reset (timeout)");
      }
    } else {
      timer_restart(&s->timer);
    }
    handle_connection(s);
  } else {
    uip_abort();
  }
}

/*---------------------------------------------------------------------------*/
PROCESS(webserver_process, "httpd");
PROCESS_THREAD(webserver_process, ev, data)
{
  PROCESS_BEGIN();
  tcp_listen(UIP_HTONS(80));
  memb_init(&conns);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    httpd_appcall(data);
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void
httpd_init(struct httpd_query_map *query_map, httpd_validator_t validator)
{
  resources.map = query_map;
  resources.validator = validator;
  process_start(&webserver_process, NULL);
}
/*---------------------------------------------------------------------------*/
