#ifndef _BUTTON_H_
#define _BUTTON_H_

#include <dpl/dpl.h>
#include <hal/hal_gpio.h>

char serial_read();
void serial_write(char *chr, size_t len);
void serial_write_str(char *chr);

void serial_read_line(char *line, size_t len);
void serial_write_line(char *line);
#endif