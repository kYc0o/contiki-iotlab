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
 *         A simple webserver
 * \author
 *         Adam Dunkels <adam@sics.se>
 *         Niclas Finne <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Olivier Fambon <olivier.fambon.at.inria.fr>
 */

#ifndef HTTPD_SIMPLE_H_
#define HTTPD_SIMPLE_H_

#include "contiki-net.h"

#define HTTPD_PATHLEN 64

struct httpd_state;
typedef char (* httpd_generator_t)(struct httpd_state *s);
typedef void (* httpd_validator_t)(const char *params, struct httpd_state *s);

typedef struct httpd_query_map {
  const char *query;
  httpd_generator_t generator;
} httpd_query_map_t;

void httpd_init(httpd_query_map_t query_map[], httpd_validator_t validator);

struct httpd_state {
  struct timer timer;
  struct psock sin, sout;
  struct pt outputpt;
  char inputbuf[HTTPD_PATHLEN + 24];
  char filename[HTTPD_PATHLEN];
  httpd_generator_t generator;
  const char *response_header;
  char request_type;
  char *post_data;
  char state;
};

#define SEND_STRING(s, str) PSOCK_SEND(s, (uint8_t *)str, strlen(str))

#define DECL_HTTP_RESPONSE(code) const char httpd_response_ ## code []
extern DECL_HTTP_RESPONSE(200);
extern DECL_HTTP_RESPONSE(400);
extern DECL_HTTP_RESPONSE(404);

#endif /* HTTPD_SIMPLE_H_ */
