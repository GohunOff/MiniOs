#ifndef MINIOS_TIMER_H
#define MINIOS_TIMER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint32_t start_ms;
    uint32_t interval_ms;
    bool active;
} kernel_timer_t;

void timer_init(void);

bool timer_start(kernel_timer_t *timer, uint32_t interval_ms);

bool timer_expired(kernel_timer_t *timer);

void timer_stop(kernel_timer_t *timer);

void timer_poll(void);

uint64_t timer_get_ms(void);

#endif