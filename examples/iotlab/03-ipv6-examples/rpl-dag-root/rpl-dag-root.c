/*
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
 *
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include <string.h>
#include "dev/serial-line.h"

#define DEBUG 1
#include "net/uip-debug.h"
#include "net/rpl/rpl.h"
#include "dev/watchdog.h"
#include "dev/slip.h"
#include "dev/leds.h"

#include "../ipv6_common.h"


/*---------------------------------------------------------------------------*/
PROCESS(border_router_process, "Border Router process");
AUTOSTART_PROCESSES(&border_router_process);

/*---------------------------------------------------------------------------*/
/* Manually set a prefix */
void
set_prefix_64(uint16_t addr0, uint16_t addr1, uint16_t addr2, uint16_t addr3)
{
  rpl_dag_t *dag;
  uip_ipaddr_t ipaddr;
  uip_ip6addr(&ipaddr, addr0, addr1, addr2, addr3, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  /* Become root of a new DODAG with ID our global v6 address */
  dag = rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
  if(dag != NULL) {
    rpl_set_prefix(dag, &ipaddr, 64);
    PRINTF("Created a new RPL dag with ID: ");
    PRINT6ADDR(&dag->dag_id);
    PRINTF("\n");
  }
}




/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static struct etimer et;

  PROCESS_BEGIN();
  PRINTF("RPL DAG started\n");

  set_prefix_64(0xabcd, 0, 0, 0);

  /* We have created a new DODAG when we reach here */
  print_local_addresses();


  etimer_set(&et, 5 * CLOCK_SECOND);
  while (1) {
      PROCESS_WAIT_EVENT();
      if (ev == serial_line_event_message)
          print_local_addresses();


      print_neighbors();
      print_routes();

      /* TODO regularly print routes and neighboors or all */
      etimer_restart(&et);
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
