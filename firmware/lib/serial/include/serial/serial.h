#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <dpl/dpl.h>
#include <hal/hal_gpio.h>

void serial_init();

char serial_read();
void serial_write(char *chr, size_t len);

#endif