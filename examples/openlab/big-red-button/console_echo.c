#include "contiki.h"
#include "dev/serial-line.h"
#include "dev/uart1.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
static int console_echo = 1;
/*---------------------------------------------------------------------------*/
static struct {
  unsigned char buf[16];
  int pos;
  int last;
} echo;
/*---------------------------------------------------------------------------*/
int console_echo_toggle_echo()
{
  return console_echo = console_echo ? 0 : 1;
}
/*---------------------------------------------------------------------------*/
static void poll_handler()
{
  do {
	printf("%c", echo.buf[echo.last]);
	echo.last = (echo.last+1) % sizeof(echo.buf);
  }
  while (echo.last != echo.pos);
}
/*---------------------------------------------------------------------------*/
PROCESS(console_echo_process, "Console echo");
PROCESS_THREAD(console_echo_process, ev, data)
{
  PROCESS_POLLHANDLER(poll_handler());
  PROCESS_BEGIN();
  while (1) PROCESS_YIELD();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static
int input_bytes_handler(unsigned char c)
{
  if (console_echo) {
	echo.buf[echo.pos] = c;
	echo.pos = (echo.pos + 1) % sizeof(echo.buf);
	process_poll(&console_echo_process);
  }
  return serial_line_input_byte(c); // default contiki behaviour: line-buffer
}
/*---------------------------------------------------------------------------*/
void console_echo_init()
{
  process_start(&console_echo_process, NULL);
  uart1_set_input(input_bytes_handler);
  serial_line_init();
}
/*---------------------------------------------------------------------------*/
