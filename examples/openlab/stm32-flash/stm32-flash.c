#include "contiki.h"
#include "drivers/stm32f1xx/flash.h"

PROCESS(stm32_flash, "stm32-flash");
AUTOSTART_PROCESSES(&stm32_flash);

/*---------------------------------------------------------------------------*/
static void lookup_str(int start_addr, int end_addr, char *str, char *section)
{
  printf("searching in section %-8s [%08x - %08x]\n",
         section, start_addr, end_addr);
  end_addr = end_addr - strlen(str) - 1;
  int i;
  for (i = start_addr; i < end_addr; i++) {
    if (! strncmp(i, str, strlen(str)))
      printf("found at: %08x = %s\n", i, i);
  }
}
/*---------------------------------------------------------------------------*/
#define check_err(expr, on_error) \
  { int e=expr; if (e) { printf("%s: error=%d\n", #expr, e); on_error; } }
/*---------------------------------------------------------------------------*/
static void increment_value_at(uint32_t page)
{
  uint16_t value;

  value = *(uint16_t*)page;
  printf("incrementing on-chip flash value: addr=%08x, value=%d, ",
         page, value);
  value += 1;
  check_err(flash_erase_memory_page(page), return);
  check_err(flash_write_memory_half_word(page, value), return);
  value = *(uint16_t*)page;
  printf("updated=%d\n", value);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(stm32_flash, ev, data)
{
  static char data_buf[] = "iotlab stm32 contiki firmware flash";
  static char bss_buf[8];
  
  PROCESS_BEGIN();

  printf("data: addr=%08x, data=%s\n", data_buf, data_buf);
  printf("bss : addr=%08x, data=%s\n", bss_buf, bss_buf);

  printf("looking for data_buf in flash...\n");
  lookup_str(0x00000000, 0x00080000, data_buf, "aliased");
  lookup_str(0x08000000, 0x08080000, data_buf, "flash");
  lookup_str(0x1FFFF000, 0x1FFFF800, data_buf, "system");
  lookup_str(0x1FFFF800, 0x1FFFF810, data_buf, "options");
  printf("completed looking for data_buf in flash.\n");

  increment_value_at(0x0807E000); // last page (one page = 2KB)
  increment_value_at(0x0007E000); // last page (aliased address)

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
