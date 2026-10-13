#ifndef UPTIMEH
#define UPTIMEH

#include <stdint.h>

void uptime_init(void); 
uint64_t uptime_getms(void); 
void uptime_print(void);

#endif