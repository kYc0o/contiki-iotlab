#include "contiki.h"

PROCESS(print_ipv6_addr, "print_ipv6_addr");
AUTOSTART_PROCESSES(&print_ipv6_addr);

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/uip-debug.h"
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(print_ipv6_addr, ev, data)
{
  static struct etimer timer;
  static uip_ipaddr_t addr;
  
  PROCESS_BEGIN();
  uip_ds6_set_addr_iid(&addr, &uip_lladdr);
  while (1) {
    PRINT6ADDR(&addr);    
    printf("\n");
    etimer_set(&timer, CLOCK_SECOND);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
