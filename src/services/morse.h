#ifndef MORSE_H
#define MORSE_H

#include <stdbool.h>

void morse_init(unsigned int led_pin);
void morse_send(const char *text);

#endif