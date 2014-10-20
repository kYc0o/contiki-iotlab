#include "contiki.h"

#include <string.h>
#include <stdio.h>

#include "net/rpl/rpl.h"
#include "dev/slip.h"

/*---------------------------------------------------------------------------*/
PROCESS(border_router_process, "Border Router");
/*---------------------------------------------------------------------------*/
static struct {
  char prefix[8]; // tunslip6 sends 8 bytes
  int  prefix_set;
} slip;
/*---------------------------------------------------------------------------*/
static void
request_prefix(void)
{
  memcpy(uip_buf, "?P", 2);
  uip_len = 2;
  slip_send();
  uip_len = 0;
}
/*---------------------------------------------------------------------------*/
static int
check_receive_prefix()
{
  if(memcmp(uip_buf, "!P", 2))
    return 0;
  memcpy(slip.prefix, uip_buf+2, sizeof(slip.prefix));
  slip.prefix_set = 1;
  return 1;
}
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
static uip_ipaddr_t last_sender;
/*---------------------------------------------------------------------------*/
static void
slip_input_callback(void)
{
  if(check_receive_prefix())
    return;
  uip_ipaddr_copy(&last_sender, &UIP_IP_BUF->srcipaddr);
}
/*---------------------------------------------------------------------------*/
static void
slip_output_callback(void)
{
  /* Do not bounce packets back over SLIP if received over SLIP */
  if(uip_ipaddr_cmp(&last_sender, &UIP_IP_BUF->srcipaddr))
    return;
  slip_send();
}
/*---------------------------------------------------------------------------*/
static void
slip_init(void)
{
  process_start(&slip_process, NULL);
  slip_set_input_callback(slip_input_callback);
}
/*---------------------------------------------------------------------------*/
/* bind the slip fallback interface that is declared in project-conf.h       */
#ifndef UIP_FALLBACK_INTERFACE
#error  UIP_FALLBACK_INTERFACE not defined
#endif
const struct uip_fallback_interface UIP_FALLBACK_INTERFACE = {
  slip_init, slip_output_callback
};
/*---------------------------------------------------------------------------*/
static void
make_host_ipv6_addr(uip_ipaddr_t *ipaddr, const char prefix[], int size)
{
  memcpy(ipaddr, prefix, size);
  uip_ds6_set_addr_iid(ipaddr, &uip_lladdr);
  uip_ds6_addr_add(ipaddr, 0, ADDR_AUTOCONF);
}
/*---------------------------------------------------------------------------*/
static rpl_dag_t *
create_rpl_dag_root(uip_ipaddr_t *ipaddr)
{
  rpl_dag_t *dag;

  dag = rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
  if(dag != NULL)
    rpl_set_prefix(dag, ipaddr, 64);
  return dag;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(border_router_process, ev, data)
{
  static int *start_mode;
  static struct etimer et;
  static uip_ipaddr_t global_ipaddr;

  PROCESS_BEGIN();

  /* save 'status out' reference passed via process_start(), if any */
  start_mode = (int*)(data ? data : &start_mode);

  NETSTACK_MAC.off(0); /* turn MAC off to avoid joining other nodes */

  static int nb_attempts = 5;
  while(!slip.prefix_set && nb_attempts--) {
    etimer_set(&et, CLOCK_SECOND);
    request_prefix();
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  if (slip.prefix_set) {
    make_host_ipv6_addr(&global_ipaddr, slip.prefix, sizeof(slip.prefix));
    create_rpl_dag_root(&global_ipaddr);
    NETSTACK_MAC.off(1); /* radio on, MAC off */
    *start_mode = 1;
  }
  else {
    NETSTACK_MAC.on();
    *start_mode = 0;
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
