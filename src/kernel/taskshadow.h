#ifndef MINIOS_TASKSHADOW_H
#define MINIOS_TASKSHADOW_H

#include <stdbool.h>
#include <stdint.h>

#define TASKSHADOW_MAX_TASKS 8
#define TASKSHADOW_INVALID_ID 0

typedef uint16_t taskshadow_id_t;

/*
 * Zwraca true, gdy zadanie zakończyło pracę.
 * Zwraca false, gdy należy zaplanować kolejny krok.
 */
typedef bool (*taskshadow_fn_t)(void *context);

typedef enum
{
    TASKSHADOW_UNUSED = 0,
    TASKSHADOW_STOPPED,
    TASKSHADOW_READY,
    TASKSHADOW_COMPLETED
} taskshadow_state_t;

#ifdef __cplusplus
extern "C" {
#endif

void taskshadow_init(void);
void taskshadow_poll(void);

taskshadow_id_t taskshadow_create(
    const char *name,
    taskshadow_fn_t function,
    void *context,
    uint32_t interval_ms
);

typedef struct
{
    taskshadow_id_t id;
    const char *name;
    taskshadow_state_t state;
    uint32_t interval_ms;
} taskshadow_info_t;

bool taskshadow_start(taskshadow_id_t id);
bool taskshadow_stop(taskshadow_id_t id);
bool taskshadow_remove(taskshadow_id_t id);

bool taskshadow_get_state(
    taskshadow_id_t id,
    taskshadow_state_t *state
);

uint8_t taskshadow_get_count(void);

bool taskshadow_get_info(uint8_t slot, taskshadow_info_t *info);
#ifdef __cplusplus
}
#endif

#endif /* MINIOS_TASKSHADOW_H */