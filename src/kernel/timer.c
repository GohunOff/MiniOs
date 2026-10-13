#include <stddef.h>
#include "kernel/timer.h"
#include "kernel/event.h"
#include "pico/stdlib.h"

#define TIMER_MAX_COUNT 8

static kernel_timer_t *timers[TIMER_MAX_COUNT];

void timer_init(void)
{
    for (int i = 0; i < TIMER_MAX_COUNT; i++)
    {
        timers[i] = NULL;
    }
}

uint64_t timer_get_ms(void)
{
    return to_ms_since_boot(get_absolute_time());
}

bool timer_start(kernel_timer_t *timer, uint32_t interval_ms)
{
    if (timer == NULL)
    {
        return false;
    }

    int free_slot = -1;

    for (int i = 0; i < TIMER_MAX_COUNT; i++)
    {
        if (timers[i] == timer)
        {
            free_slot = i;
            break;
        }

        if (timers[i] == NULL && free_slot == -1)
        {
            free_slot = i;
        }
    }

    if (free_slot == -1)
    {
        timer->active = false;
        return false;
    }

    timers[free_slot] = timer;

    timer->start_ms = (uint32_t)timer_get_ms();
    timer->interval_ms = interval_ms;
    timer->active = true;

    return true;
}


bool timer_expired(kernel_timer_t *timer)
{
    if (timer == NULL || !timer->active)
    {
        return false;
    }

    uint32_t now = (uint32_t)timer_get_ms();

    if ((uint32_t)(now - timer->start_ms) >= timer->interval_ms)
    {
        timer->active = false;
        return true;
    }

    return false;
}

void timer_poll(void)
{
    for (int i = 0; i < TIMER_MAX_COUNT; i++)
    {
        kernel_timer_t *timer = timers[i];

        if (timer == NULL || !timer->active)
        {
            continue;
        }

        uint32_t now = (uint32_t)timer_get_ms();

        if ((uint32_t)(now - timer->start_ms) >= timer->interval_ms)
        {
            timer->active = false;

            event_emit(EVENT_TIMER_EXPIRED, timer);
        }
    }
}