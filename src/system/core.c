#include <stdio.h>
#include "services/uptime.h"
#include "pico/stdlib.h"
#include "system/core.h"
#include "monitor/memory.h"

#include "kernel/timer.h"
#include "services/uptime.h"
#include "kernel/event.h"

static kernel_timer_t test_timer;
static kernel_timer_t test_timer_b;

#include "kernel/taskshadow.h"

static taskshadow_id_t diagnostic_task_id = TASKSHADOW_INVALID_ID;
static unsigned int diagnostic_counter = 0;

static bool diagnostic_task(void *context)
{
    (void)context;

    ++diagnostic_counter;

    printf("[TaskShadow] Step %u\r\n", diagnostic_counter);

    return diagnostic_counter >= 5;
}

void diagnostic_task_start(void)
{
    /* Utwórz zadanie tylko wtedy, gdy jeszcze nie istnieje. */
    if (diagnostic_task_id == TASKSHADOW_INVALID_ID)
    {
        diagnostic_counter = 0;

        diagnostic_task_id = taskshadow_create(
            "diagnostic",
            diagnostic_task,
            NULL,
            1000
        );

        if (diagnostic_task_id == TASKSHADOW_INVALID_ID)
        {
            printf("Failed to create diagnostic task.\r\n");
            return;
        }
    }

    taskshadow_state_t state;

    if (!taskshadow_get_state(diagnostic_task_id, &state))
    {
        printf("Failed to read diagnostic task state.\r\n");
        diagnostic_task_id = TASKSHADOW_INVALID_ID;
        return;
    }

    if (state == TASKSHADOW_READY)
    {
        printf("Diagnostic task is already running.\r\n");
        return;
    }

    /* Zadanie zakończone można przygotować do ponownego startu. */
    if (state == TASKSHADOW_COMPLETED ||
        state == TASKSHADOW_STOPPED)
    {
        diagnostic_counter = 0;
    }

    if (!taskshadow_start(diagnostic_task_id))
    {
        printf("Failed to start diagnostic task.\r\n");
        return;
    }

    printf("Diagnostic task started. ID: %u\r\n",
           (unsigned int)diagnostic_task_id);
}

void diagnostic_task_stop(void)
{
    if (diagnostic_task_id == TASKSHADOW_INVALID_ID)
    {
        printf("Diagnostic task does not exist.\r\n");
        return;
    }

    if (taskshadow_stop(diagnostic_task_id))
    {
        printf("Diagnostic task stopped.\r\n");
    }
    else
    {
        printf("Cannot stop diagnostic task.\r\n");
    }
}

void diagnostic_task_remove(void)
{
    if (diagnostic_task_id == TASKSHADOW_INVALID_ID)
    {
        printf("Diagnostic task does not exist.\r\n");
        return;
    }

    if (taskshadow_remove(diagnostic_task_id))
    {
        diagnostic_task_id = TASKSHADOW_INVALID_ID;
        diagnostic_counter = 0;

        printf("Diagnostic task removed.\r\n");
    }
    else
    {
        printf("Failed to remove diagnostic task.\r\n");
    }
}

void print_banner(void)
{
    printf("\033[2J\033[H");

    printf("MiniOS v0.1\r\n");
    printf("-------------------------\r\n");
    printf("RP2040 detected\r\n");
    printf("CPU: 133 MHz\r\n");
    printf("RAM: 264 KB\r\n");
    printf("Flash: 2048 KB\r\n");
    printf("\r\n");

    printf("System READY.\r\n");
    printf("\r\n");

    printf("Type 'help' for commands.\r\n");
    printf("\r\n");

    printf("mini> ");
}

 void boot_sequence(void)
{
    printf("\033[2J\033[H");

    printf("MiniOS v0.1\r\n");
    printf("=========================\r\n");
    printf("Booting...\r\n\r\n");

    sleep_ms(300);

    printf("[ OK ] Initializing RP2040\r\n");
    sleep_ms(150);

    printf("[ OK ] CPU: 133 MHz\r\n");
    sleep_ms(150);

    printf("[ OK ] Memory: 264 KB RAM\r\n");
    sleep_ms(150);

    printf("[ OK ] Flash: 2048 KB\r\n");
    sleep_ms(150);

    printf("[ OK ] USB serial\r\n");
    sleep_ms(150);

    printf("[ OK ] LED GPIO\r\n");
    sleep_ms(150);

    printf("\r\n");
    printf("System READY.\r\n");
    sleep_ms(300);

    printf("\r\n");
}

void command_diag(void)
{
    printf("\r\n");
    printf("MiniOS System Diagnostics\r\n");
    printf("=========================\r\n");

    printf("System:          MiniOS v0.1\r\n");
    printf("MCU:             RP2040\r\n");
    printf("USB Serial:      Connected\r\n");
    printf("Event System:    Initialized\r\n");
    printf("Morse Service:   Initialized\r\n");
    printf("Uptime Service:  Initialized\r\n");
    printf("Memory Service:  Initialized\r\n");

    printf("\r\n");
    printf("--- Uptime ---\r\n");
    uptime_print();

    printf("\r\n");
    printf("--- Memory ---\r\n");
    memory_print();

    printf("\r\n");
    printf("Diagnostics completed.\r\n");
}

void command_info(void)
{
    printf("\r\n");
    printf("MiniOS v0.1\r\n");
    printf("\r\n");

    printf("Hardware:\r\n");
    printf("  CPU: RP2040\r\n");
    printf("  Clock: 133 MHz\r\n");
    printf("  RAM: 264 KB\r\n");
    printf("  Flash: 2048 KB\r\n");
    printf("\r\n");
}

void command_version(void)
{
    printf("\r\n");
    printf("MiniOS v0.1\r\n");
    printf("\r\n");
}

void command_timer2(void)
{
    if (test_timer_b.active)
    {
        printf("\r\nTimer B is already running.\r\n");
        return;
    }

    if (!timer_start(&test_timer_b, 3500))
    {
        printf("\r\nTimer B registration failed.\r\n");
        return;
    }

    printf("\r\nTimer B started: 3.5 seconds\r\n");
}

void command_timer(void)
{
    if (test_timer.active)
    {
        printf("\r\nTimer is already running.\r\n");
        return;
    }

    if (!timer_start(&test_timer, 2000))
    {
        printf("\r\nTimer registration failed.\r\n");
        return;
    }

    printf("\r\nTimer started: 2 seconds\r\n");
}

void command_uptime(void)
{
    uptime_print();
}


static void on_timer_expired(const event_t *event)
{
    if (event == NULL || event->data == NULL)
    {
        return;
    }

    kernel_timer_t *timer = (kernel_timer_t *)event->data;

    if (timer == &test_timer)
    {
        printf("\r\nTimer A expired!\r\nmini> ");
    }
    else if (timer == &test_timer_b)
    {
        printf("\r\nTimer B expired!\r\nmini> ");
    }
}

static void on_boot(const event_t *event)
{
    printf("[EVENT] System boot\r\n");
}

static void on_system_ready(const event_t *event)
{
    printf("[EVENT] System ready\r\n");
}

static void on_terminal_connected(const event_t *event)
{
    printf("\r\n");
    printf("[EVENT] Terminal connected\r\n");
    printf("Welcome to MiniOS!\r\n");
    printf("mini> ");
}

void core_init(void)
{
    uptime_init();
    timer_init();
}


void core_init_event(void)
{
    event_register(EVENT_BOOT, on_boot);
    event_register(EVENT_SYSTEM_READY, on_system_ready);
    event_register(EVENT_TERMINAL_CONNECTED, on_terminal_connected);
    event_register(EVENT_TIMER_EXPIRED, on_timer_expired);

    event_emit(EVENT_BOOT, NULL);
}