#include "kernel/taskshadow.h"
#include "kernel/timer.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
    taskshadow_id_t id;
    const char *name;
    taskshadow_fn_t function;
    void *context;

    taskshadow_state_t state;

    uint32_t interval_ms;
    uint64_t next_run_ms;
} taskshadow_task_t;

static taskshadow_task_t tasks[TASKSHADOW_MAX_TASKS];

static taskshadow_id_t next_id = 1;
static uint8_t task_count = 0;

static taskshadow_task_t *find_task(taskshadow_id_t id)
{
    if (id == TASKSHADOW_INVALID_ID)
    {
        return NULL;
    }

    for (size_t i = 0; i < TASKSHADOW_MAX_TASKS; ++i)
    {
        if (tasks[i].state != TASKSHADOW_UNUSED &&
            tasks[i].id == id)
        {
            return &tasks[i];
        }
    }

    return NULL;
}

void taskshadow_init(void)
{
    memset(tasks, 0, sizeof(tasks));

    next_id = 1;
    task_count = 0;
}

taskshadow_id_t taskshadow_create(
    const char *name,
    taskshadow_fn_t function,
    void *context,
    uint32_t interval_ms)
{
    if (name == NULL || function == NULL || interval_ms == 0)
    {
        return TASKSHADOW_INVALID_ID;
    }

    for (size_t i = 0; i < TASKSHADOW_MAX_TASKS; ++i)
    {
        if (tasks[i].state != TASKSHADOW_UNUSED)
        {
            continue;
        }

        taskshadow_id_t id = next_id++;

        /* Zero jest zarezerwowane jako nieprawidłowe ID. */
        if (next_id == TASKSHADOW_INVALID_ID)
        {
            next_id = 1;
        }

        /* Unikamy kolizji z nadal zarejestrowanymi zadaniami. */
        while (find_task(id) != NULL)
        {
            id = next_id++;

            if (next_id == TASKSHADOW_INVALID_ID)
            {
                next_id = 1;
            }
        }

        tasks[i].id = id;
        tasks[i].name = name;
        tasks[i].function = function;
        tasks[i].context = context;
        tasks[i].state = TASKSHADOW_STOPPED;
        tasks[i].interval_ms = interval_ms;
        tasks[i].next_run_ms = 0;

        ++task_count;

        return id;
    }

    return TASKSHADOW_INVALID_ID;
}


bool taskshadow_get_state(
    taskshadow_id_t id,
    taskshadow_state_t *state)
{
    if (state == NULL)
    {
        return false;
    }

    taskshadow_task_t *task = find_task(id);

    if (task == NULL)
    {
        return false;
    }

    *state = task->state;

    return true;
}

uint8_t taskshadow_get_count(void)
{
    return task_count;
}

void taskshadow_poll(void)
{
    const uint64_t now = timer_get_ms();

    for (size_t i = 0; i < TASKSHADOW_MAX_TASKS; ++i)
    {
        taskshadow_task_t *task = &tasks[i];

        if (task->state != TASKSHADOW_READY)
        {
            continue;
        }

        if (now < task->next_run_ms)
        {
            continue;
        }

        /*
         * Funkcja wykonuje tylko jeden krok zadania.
         * Nie należy używać w niej długich sleep_ms().
         */
        taskshadow_id_t id = task->id;
        taskshadow_fn_t function = task->function;
        void *context = task->context;

        bool completed = function(context);

        /*
         * Ponowne wyszukanie chroni przed zapisem do slotu,
         * który callback mógł usunąć lub ponownie wykorzystać.
         */
        task = find_task(id);

        if (task == NULL ||
            task->state != TASKSHADOW_READY)
        {
            continue;
        }

        if (completed)
        {
            task->state = TASKSHADOW_COMPLETED;
        }
        else
        {
            /*
             * Harmonogram liczony od zakończenia kroku.
             * Zapobiega nadrabianiu zaległych wykonań seriami.
             */
            task->next_run_ms =
                timer_get_ms() + task->interval_ms;
        }
    }
}

bool taskshadow_start(taskshadow_id_t id)
{
    taskshadow_task_t *task = find_task(id);

    if (task == NULL)
    {
        return false;
    }

      /* Uruchamiamy wyłącznie zadania zatrzymane. */
    if (task->state != TASKSHADOW_STOPPED)
    {
        return false;
    }

    task->state = TASKSHADOW_READY;
    task->next_run_ms = timer_get_ms();

    return true;
}

bool taskshadow_stop(taskshadow_id_t id)
{
    taskshadow_task_t *task = find_task(id);

    if (task == NULL)
    {
        return false;
    }

    /* Zatrzymać można tylko aktywne zadanie. */
    if (task->state != TASKSHADOW_READY)
    {
        return false;
    }

    task->state = TASKSHADOW_STOPPED;

    return true;
}

bool taskshadow_remove(taskshadow_id_t id)
{
    taskshadow_task_t *task = find_task(id);

    if (task == NULL)
    {
        return false;
    }

    /*
     * Usunięcie zwalnia slot i zmniejsza liczbę
     * zarejestrowanych zadań.
     */
    memset(task, 0, sizeof(*task));

    if (task_count > 0)
    {
        --task_count;
    }

    return true;
}

bool taskshadow_get_info(uint8_t slot, taskshadow_info_t *info)
{
    if (slot >= TASKSHADOW_MAX_TASKS || info == NULL)
    {
        return false;
    }

    const taskshadow_task_t *task = &tasks[slot];

    if (task->state == TASKSHADOW_UNUSED)
    {
        return false;
    }

    info->id = task->id;
    info->name = task->name;
    info->state = task->state;
    info->interval_ms = task->interval_ms;

    return true;
}