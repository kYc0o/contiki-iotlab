#include "gpio_button.h"
#include "clock.h"


process_event_t gpio_button_changed_event;

static clock_time_t last_event;

/*---------------------------------------------------------------------------*/
PROCESS(gpio_button_process, "GPIO Button driver");
/*---------------------------------------------------------------------------*/
extern void fox_gpio_button_init(void (*irq_handler)(void));
/*---------------------------------------------------------------------------*/
static
void irq_handler()
{
  if (clock_time() - last_event > CLOCK_SECOND/10) {
	process_poll(&gpio_button_process);
	last_event = clock_time();
  }
}
/*---------------------------------------------------------------------------*/
static
void poll_handler()
{
  process_post(PROCESS_BROADCAST, gpio_button_changed_event, NULL);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(gpio_button_process, ev, data)
{
  PROCESS_POLLHANDLER(poll_handler());
  PROCESS_BEGIN();
  while (1) PROCESS_YIELD();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void gpio_button_init()
{
  gpio_button_changed_event = process_alloc_event();
  process_start(&gpio_button_process, NULL);
#if AGILE_FOX
  fox_gpio_button_init(irq_handler);
#endif
}
/*---------------------------------------------------------------------------*/
void gpio_button_simulate_action()
{
  irq_handler();
}
/*---------------------------------------------------------------------------*/
