#ifndef MINIOS_EVENT_H
#define MINIOS_EVENT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    EVENT_NONE = 0,

    EVENT_BOOT,
    EVENT_SYSTEM_READY,

    EVENT_USB_CONNECTED,
    EVENT_USB_DISCONNECTED,

    EVENT_TERMINAL_CONNECTED,
    EVENT_TERMINAL_DISCONNECTED,

    EVENT_COMMAND,
    EVENT_REBOOT,
    
    EVENT_TIMER_EXPIRED,

    EVENT_MAX
} event_type_t;

typedef struct
{
    event_type_t type;
    void *data;
} event_t;

typedef void (*event_handler_t)(const event_t *event);

void event_init(void);

bool event_register(event_type_t type, event_handler_t handler);

void event_emit(event_type_t type, void *data);

#ifdef __cplusplus
}
#endif

#endif
