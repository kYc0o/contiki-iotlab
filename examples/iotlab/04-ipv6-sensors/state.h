#include "contiki.h"

struct state {
  uip_ipaddr_t dest_addr;
  int dest_port;
  int dest_addr_set;
};

extern struct state state;
