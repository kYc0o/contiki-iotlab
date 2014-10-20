#define DEBUG 1
#include "net/uip-debug.h"
#include "net/uip-ds6.h"

#include "ipv6_common.h"

void
print_local_addresses(void)
{
  int i;
  uint8_t state;

  PRINTF("Node IPv6 addresses: \n");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused) {
      PRINTF("Address:  ");
      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      /*
      // hack to make address "final"
      // ?why? not required ?
      if(state == ADDR_TENTATIVE) {
        uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
      }
      */
      switch (state) {
        case ADDR_TENTATIVE: PRINTF(" ADDR_TENTATIVE"); break;
        case ADDR_PREFERRED: PRINTF(" ADDR_PREFERRED"); break;
        case ADDR_DEPRECATED: PRINTF(" ADDR_DEPRECATED"); break;
      }
      PRINTF("\n");
    }
  }
}

void print_routes(void)
{
  static uip_ds6_route_t *r;

  PRINTF("Routes:\n");
  for(r = uip_ds6_route_head(); r != NULL; r = uip_ds6_route_next(r)) {

    PRINTF("Route:  ");
    PRINT6ADDR(&r->ipaddr);

    PRINTF("/%u (via ", r->length);
    PRINT6ADDR(uip_ds6_route_nexthop(r));
    if(r->state.lifetime < 600) {
      PRINTF(") %us\n", (unsigned int)r->state.lifetime);
    } else {
      PRINTF(")\n");
    }
  }

}

void print_default_route(void)
{
  PRINTF("Default Route:\n");
  PRINTF("Default_route:  ");
  PRINT6ADDR(uip_ds6_defrt_choose());
  PRINTF("\n");
}


void print_neighbors(void)
{
  static uip_ds6_nbr_t *nbr;

  PRINTF("Neighbors:\n");
  for(nbr = nbr_table_head(ds6_neighbors);
      nbr != NULL;
      nbr = nbr_table_next(ds6_neighbors, nbr)) {

    PRINTF("Neighbor  ");
    PRINT6ADDR(&nbr->ipaddr);

    switch (nbr->state) {
      case NBR_INCOMPLETE: PRINTF(" INCOMPLETE");break;
      case NBR_REACHABLE: PRINTF(" REACHABLE");break;
      case NBR_STALE: PRINTF(" STALE");break;
      case NBR_DELAY: PRINTF(" DELAY");break;
      case NBR_PROBE: PRINTF(" NBR_PROBE");break;
    }
    PRINTF("\n");
  }
}

