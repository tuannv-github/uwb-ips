#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <dpl/dpl.h>
#include <hal/hal_gpio.h>

typedef void (*button_handler_t)(int pin, void *arg);

int
button_init(int pin, button_handler_t handler, void *arg,
                  hal_gpio_irq_trig_t trig, hal_gpio_pull_t pull);
#endif