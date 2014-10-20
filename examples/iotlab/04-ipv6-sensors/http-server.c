#include "contiki.h"

#include <stdio.h>
#include <string.h>

#include "httpd-simple.h"
#include "uip_util.h"
#include "state.h"

/*---------------------------------------------------------------------------*/
extern char* get_sensors_json();
/*---------------------------------------------------------------------------*/
static const char *TOP = "<html><head><title>Contiki Sensors</title></head><body>\n";
static const char *BOTTOM = "</body></html>\n";
static char buf[2048];
static int blen;
#define ADD(s, ...) do {                                                \
  blen += snprintf(&buf[blen], sizeof(buf) - blen, __VA_ARGS__);        \
  if (blen > sizeof(buf) - 128) { FLUSH(s); }                           \
} while(0)
#define FLUSH(s) { SEND_STRING(&s->sout, buf); blen = 0; }
#define ADD_ADDR(s, addr) { blen += uip_util_addr2text(addr, buf+blen); }
/*---------------------------------------------------------------------------*/
static char post_data[1024];
static
PT_THREAD(home_page(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  ADD(s, TOP);
  ADD(s, "<pre>\n%s\n</pre>\n", get_sensors_json());
  if(*post_data)
    ADD(s, "post data:\n<pre>\n%s\n</pre>\n", post_data);
  ADD(s, BOTTOM);
  FLUSH(s);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_data_in(struct httpd_state *s))
{
  strncpy(post_data, s->post_data, sizeof(post_data));
  PSOCK_BEGIN(&s->sout);
  //ADD(s, "OK.");
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(network_status(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  ADD(s, TOP);
  ADD(s, "<script src=\"script.js\"></script>\n");

  ADD(s, "Default Route\n<pre>\n<a>");
  ADD_ADDR(s, uip_ds6_defrt_choose());
  ADD(s, "</a>\n</pre>\n");

  ADD(s, "Routes\n<pre>\n");

  static uip_ds6_route_t *r;
  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {
    ADD(s, "<a>");
    ADD_ADDR(s, &r->ipaddr);
    ADD(s, "</a>");
    ADD(s, "/%u (via ", r->length);
    ADD(s, "<a>");
    ADD_ADDR(s, uip_ds6_route_nexthop(r));
    ADD(s, "</a>");
    if(1 || (r->state.lifetime < 600)) {
      ADD(s, ") %lus\n", r->state.lifetime);
    } else {
      ADD(s, ")\n");
    }
  }
  ADD(s, "</pre>\n");
  ADD(s, BOTTOM);
  FLUSH(s);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(set_destination(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);

  if (s->response_header != httpd_response_200) {
    ADD(s, "Incorrect IPV6 address.\n");
    goto end;
  }
  ADD(s, "dest_address=[");
  ADD_ADDR(s, &state.dest_addr);
  ADD(s, "]:%d\n", state.dest_port);

  end:
  FLUSH(s);

  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(stop_sending(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  ADD(s, "stopped sending to dest_address=[");
  ADD_ADDR(s, &state.dest_addr);
  ADD(s, "]:%d\n", state.dest_port);
  FLUSH(s);
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(generate_script(struct httpd_state *s))
{
  PSOCK_BEGIN(&s->sout);
  SEND_STRING(&s->sout, "\
  onload = function() {\n\
	p = location.host.replace(/::.*/, '::').substr(1);\n\
	a = document.getElementsByTagName('a');\n\
	for (i=0; i < a.length; i++) {\n\
		txt = a[i].innerHTML.replace(/^FE80::/, p);\n\
		a[i].href = 'http://['+txt+']';\n\
	}\n\
  }");
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
static struct httpd_query_map queries_map[] = {
  { "", home_page },
  { "network", network_status },
  { "data-in", handle_data_in },
  { "set_destination?", set_destination },
  { "stop_sending", stop_sending },
  { "script.js", generate_script },
  {}
};
/*---------------------------------------------------------------------------*/
static void validator(const char* params, struct httpd_state *s)
{
  if (s->generator == set_destination) {
    if (!uip_util_text2addr(params, &state.dest_addr, &state.dest_port)) {
      s->response_header = httpd_response_400;
      state.dest_addr_set = 0;
    }
    else {
      state.dest_addr_set = 1;
      if (!state.dest_port) state.dest_port = 80;
    }
  }
  else
  if (s->generator == stop_sending) {
      state.dest_addr_set = 0;
  }
}
/*---------------------------------------------------------------------------*/
void http_server_init()
{
  httpd_init(queries_map, validator);
}
/*---------------------------------------------------------------------------*/
