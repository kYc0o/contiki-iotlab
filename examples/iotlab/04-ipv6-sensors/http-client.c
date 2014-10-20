#include "http-client.h"
#include <string.h> // strlen
#include <stdio.h>

static struct {
  struct psock ps;
  char buffer[1024];
} glob;
/*---------------------------------------------------------------------------*/
#define log_trace(...) // printf(__VA_ARGS__)
#define log_error(...) printf(__VA_ARGS__)
/*---------------------------------------------------------------------------*/
PROCESS(http_client_process, "http client");
/*---------------------------------------------------------------------------*/
#define ADD(len, ...) (len += sprintf((char*)uip_appdata+len, __VA_ARGS__))
/*---------------------------------------------------------------------------*/
static unsigned short generator(void *arg)
{
  struct http_request *req = (struct http_request *)arg;
  unsigned short len = 0;

  log_trace("generating data...");
  ADD(len, "POST %s HTTP/1.0\r\n", req->path);
  ADD(len, "User-Agent: Contiki/2.7 protosocket client\r\n");
  ADD(len, "Content-Type: text/plain\r\n");
  ADD(len, "Content-Length: ");
  ADD(len, "%d\r\n", strlen(req->data));
  ADD(len, "\r\n");
  ADD(len, "%s", req->data);
  ADD(len, "\r\n");

  log_trace("done generating data:\n%s\n(len=%d)\n",
            (char*)uip_appdata, strlen(uip_appdata));
  return len;
}
/*---------------------------------------------------------------------------*/
static int
handle_connection(struct psock *p, struct http_request *req)
{
  PSOCK_BEGIN(p);

  PSOCK_GENERATOR_SEND(p, generator, req);
#if 0
  while(1) {
    PSOCK_READTO(p, '\n');
    printf("Got: %s", glob.buffer);
  }
#endif
  PSOCK_CLOSE(p);
  PSOCK_END(p);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(http_client_process, ev, data)
{
  static struct http_request *req;

  PROCESS_BEGIN();
  if (req) {
    log_error("concurrent http-client access\n");
    goto end;
  }
  req = (struct http_request *)data;

  tcp_connect(&req->addr, UIP_HTONS(req->port), NULL);
  log_trace("Connecting...\n");
  PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
  if(uip_aborted() || uip_timedout() || uip_closed()) {
    log_error("Could not establish connection\n");
  } else if(uip_connected()) {
    log_trace("Connected\n");
    PSOCK_INIT(&glob.ps, (unsigned char*)glob.buffer, sizeof(glob.buffer));
    do {
      handle_connection(&glob.ps, req);
      PROCESS_WAIT_EVENT_UNTIL(ev == tcpip_event);
    } while(!(uip_closed() || uip_aborted() || uip_timedout()));
    log_trace("Connection closed.\n");
  }
  req = NULL;
  end: do ; while (0);
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
