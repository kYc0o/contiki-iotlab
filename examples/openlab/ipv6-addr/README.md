print-ipv6-addr
===============

Prints the default link-local ipv6 address, every second.

Usage:
------

See ``iot-lab/qualif/get_ipv6.sh``

Notes:
------

We do
```C
#define DEBUG 1
#include "net/uip-debug.h"
```
to get ``PRINT6ADDR``, and then use ``uip_ds6_set_addr_iid()``
to convert ``uip_lladdr`` to a proper ``uip_ipaddt_t``.  This is
not exactly straightforward, but this is what border-routers do.

An alternative is to use ``uip_lladdr`` directly and print it.
This requires exposing the internals of ``uip_lladdr``, making sure
byte-order is as expected, and finally taking a carefull look at
file ``net/uip-ds6.c`` to see what lies in ``uip_ds6_set_addr_iid()``.

The resulting code would look like:
```C
  uint8_t *l = uip_lladdr.addr;
  printf("%x%02x:%x%02x:%x%02x:%x%02x\n",
         l[0]^2,l[1],l[2],l[3],l[4],l[5],l[6],l[7]);
  /* see net/uip-ds6.c uip_ds6_set_addr_iid() */
```
