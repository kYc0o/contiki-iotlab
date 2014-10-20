/*
 * Sample contiki application exposing a gpio-based button state.
 *
 * The program polls the state of a gpio-based button
 * and posts the value (on|off) of the button to specified IPv6 host.
 *
 * Destination host may be configured via serial link or via http
 * using the following query:
 * http://[<node ipv6 address>]/set_destination?<destination ipv6 address>
 */

#include "contiki.h"
#include "serial-line.h"
#include <stdio.h>

#include "gpio_button.h"
#include "state.h"
struct red_button_state state;

/*---------------------------------------------------------------------------*/
PROCESS(node_process, "Red Button");
AUTOSTART_PROCESSES(&node_process);
/*---------------------------------------------------------------------------*/
extern void command_parser_init();
extern void process_command(char*);
extern void send_button_state();
extern void http_server_init();
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(node_process, ev, data)
{
  PROCESS_BEGIN();

  command_parser_init();
  gpio_button_init();
  http_server_init();

  while(1) {
    PROCESS_YIELD();
    if (ev == serial_line_event_message) {
      process_command((char*)data);
    }
    else
    if (ev == gpio_button_changed_event) {
      state.button_state = ! state.button_state;
      if (state.dest_addr_set)
        send_button_state();
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
