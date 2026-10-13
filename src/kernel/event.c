#include <stddef.h>
#include "event.h"

#define MAX_EVENT_HANDLERS 8

static event_handler_t handlers[EVENT_MAX][MAX_EVENT_HANDLERS];

void event_init(void)
{
    for (int i = 0; i < EVENT_MAX; i++)
    {
        for (int j = 0; j < MAX_EVENT_HANDLERS; j++)
        {
            handlers[i][j] = NULL;
        }
    }
}

bool event_register(event_type_t type, event_handler_t handler)
{
    if (type <= EVENT_NONE || type >= EVENT_MAX)
        return false;

    if (handler == NULL)
        return false;

    for (int i = 0; i < MAX_EVENT_HANDLERS; i++)
    {
        if (handlers[type][i] == NULL)
        {
            handlers[type][i] = handler;
            return true;
        }
    }

    return false;
}

void event_emit(event_type_t type, void *data)
{
    if (type <= EVENT_NONE || type >= EVENT_MAX)
        return;

    event_t event = {
        .type = type,
        .data = data
    };

    for (int i = 0; i < MAX_EVENT_HANDLERS; i++)
    {
        if (handlers[type][i] != NULL)
        {
            handlers[type][i](&event);
        }
    }
}
