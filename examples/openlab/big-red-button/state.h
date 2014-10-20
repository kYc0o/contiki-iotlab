#include "net/uip.h"

extern struct red_button_state {
  uip_ipaddr_t dest_addr;
  int dest_addr_set;
  int button_state;
} state;
