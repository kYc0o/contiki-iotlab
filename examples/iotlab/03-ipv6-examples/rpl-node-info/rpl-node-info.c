#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/uip.h"
#include "net/uip-ds6.h"
#include "net/uip-udp-packet.h"
#include "sys/ctimer.h"
#include <stdio.h>
#include <string.h>
#include "rpl.h"

#include "../ipv6_common.h"

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#ifndef PERIOD
#define PERIOD 60
#endif

#define START_INTERVAL		(15 * CLOCK_SECOND)
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
#define MAX_PAYLOAD_LEN		30

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "node process");
AUTOSTART_PROCESSES(&node_process);
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  static struct etimer et;
  PROCESS_BEGIN();

  PRINTF("Node process started\n");

  etimer_set(&et, 5 * CLOCK_SECOND);
  while (1) {
    PROCESS_YIELD();
    /* TODO regularly print routes and neighboors or all */
    print_local_addresses();
    print_routes();
    print_default_route();

    etimer_restart(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
