#include "contiki.h"
#include <stdio.h>

PROCESS(printf_test, "printf test");
AUTOSTART_PROCESSES(&printf_test);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(printf_test, ev, data)
{
  PROCESS_BEGIN();
  printf("printf:   %lu, %0.2f\n", (unsigned long)3, (float)2.1);
  printf("expected: 3, 2.10\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
