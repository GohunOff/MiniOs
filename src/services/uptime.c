#include "pico/stdlib.h" 
#include <stdio.h>

static uint64_t boottime_ms = 0;

void uptime_init(void) 
{ 
    boottime_ms = to_ms_since_boot(get_absolute_time()); 
}

uint64_t uptime_getms(void) 
{ 
    uint64_t now = to_ms_since_boot(get_absolute_time());

    return now - boottime_ms; 
}

void uptime_print(void) 
{ 
    uint64_t totalseconds = uptime_getms() / 1000;

    uint32_t seconds = totalseconds % 60; 
    uint32_t minutes = (totalseconds / 60) % 60; 
    uint32_t hours = (totalseconds / 3600);

    printf("\r\n"); 
    printf("Uptime: %02lu:%02lu:%02lu\r\n", (unsigned long)hours, (unsigned long)minutes, (unsigned long)seconds); 
} 