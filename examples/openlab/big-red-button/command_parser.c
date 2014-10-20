#include "contiki.h"
#include "dev/serial-line.h"
#include "dev/uart1.h"
#include "net/uip.h"
#include <stdio.h>

#define DEBUG DEBUG_PRINT
#include "net/uip-debug.h"

#include "state.h"
#include "web_service.h"
#include "uip_util.h"

/*---------------------------------------------------------------------------*/
extern void gpio_button_simulate_action();
extern void console_echo_init();
extern int  console_echo_toggle_echo();
/*---------------------------------------------------------------------------*/
static
char* get_button_state_str()
{
  static char buf[16+1];
  sprintf(buf, "button_state=%s", state.button_state ? "on" : "off");
  return buf;
}
/*---------------------------------------------------------------------------*/
static
void print_button_state()
{
  printf("%s\n", get_button_state_str());
}
/*---------------------------------------------------------------------------*/
void send_button_state()
{
  if (!state.dest_addr_set) {
	printf("destination address not set\n");
	return;
  }
  web_service_send_data(&state.dest_addr, get_button_state_str());
}
/*---------------------------------------------------------------------------*/
static
void set_destination(char *dest)
{
  if (!uip_util_text2addr(dest+1, &state.dest_addr, NULL)) {
	printf("invalid address\n");
	return;
  }
  printf("destination_address=");
  PRINT6ADDR(&state.dest_addr);
  printf("\n");
  state.dest_addr_set = 1;
}
/*---------------------------------------------------------------------------*/
static
void toggle_console_echo()
{
  int echo = console_echo_toggle_echo();
  printf("console_echo=%s\n", echo ? "on" : "off");
}
/*---------------------------------------------------------------------------*/
static char *HELP_TEXT = "\n\
available commands:\n\
	p: print button state\n\
	d: set destination address (argument: ipv6 address)\n\
	s: send button state to destination\n\
	l: print local ipv6 addresses\n\
	e: toggle console echo\n\
	!: simulate button action\n\
\n\
	h: this help\n\
";
/*---------------------------------------------------------------------------*/
void process_command(char *command)
{
  switch (command[0]) {
  case 'p':
	print_button_state();
	break;
  case 's':
	send_button_state();
	break;
  case 'd':
	set_destination(command+1);
	break;
  case 'e':
	toggle_console_echo();
	break;
  case 'l':
	uip_util_print_local_addresses();
	break;
  case '!':
	gpio_button_simulate_action();
	break;
  case 'h':
  case '?':
	printf("%s\n", HELP_TEXT);
	break;
  case '\0':
	break;
  default:
	printf("unknown command: %c\n", command[0]);
  }
  printf("> ");
}
/*---------------------------------------------------------------------------*/
void command_parser_init()
{
  console_echo_init();
  printf("Welcome to Big Red Button !\n type 'h' for help\n\n> ");
}
/*---------------------------------------------------------------------------*/
