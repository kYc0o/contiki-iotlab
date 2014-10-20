#include "drivers/gpio.h"
#include "drivers/exti.h"
#include "drivers/stm32f1xx/stm32f1xx.h"
#include "drivers/stm32f1xx/afio.h"
#include "drivers/stm32f1xx/nvic_.h"

void fox_gpio_button_init(void (*irq_handler)(void))
{
  // set GPIO 0 to input on the Fox daughter board
  gpio_set_input(GPIO_A, GPIO_PIN_7);
  gpio_enable(GPIO_A);

  // link GPIO to IRQ line
  afio_select_exti_pin(EXTI_LINE_Px7, AFIO_PORT_A);
  nvic_enable_interrupt_line(NVIC_IRQ_LINE_EXTI9_5);

  // set IRQ handler and configure to RISING signal
  exti_set_handler(EXTI_LINE_Px7, (handler_t)irq_handler, NULL);
  exti_enable_interrupt_line(EXTI_LINE_Px7, EXTI_TRIGGER_RISING);
}
