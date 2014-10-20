#include "uip.h"

extern int  uip_util_text2addr(const char *, uip_ipaddr_t *, int *port);
extern int  uip_util_addr2text(uip_ipaddr_t *, char *);
extern void uip_util_print_local_addresses();
