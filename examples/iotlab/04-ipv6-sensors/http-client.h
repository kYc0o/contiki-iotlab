#include "contiki-net.h"

struct http_request {
  uip_ipaddr_t addr;
  int   port;
  char *path;
  char *data;
};

PROCESS_NAME(http_client_process);
#ifndef HTTP_CLIENT_POST
#define HTTP_CLIENT_POST(req) \
  process_start(&http_client_process, (const char*)req); \
  PROCESS_WAIT_EVENT_UNTIL(!process_is_running(&http_client_process))
#endif
