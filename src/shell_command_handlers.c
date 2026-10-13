#include "shell_command_event.h"
#include "kernel/event.h"

#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "shell/history_c.h"
#include "kernel/taskshadow.h"

#include "services/morse.h"
#include "monitor/memory.h"

#include "system/core.h"

#define LED_PIN 25

static void command_led(bool state)
{
    gpio_put(LED_PIN, state);

    printf("\r\n");
    printf("LED: %s\r\n", state ? "ON" : "OFF");
}

static const char *task_state_name(taskshadow_state_t state)
{
    switch (state)
    {
        case TASKSHADOW_STOPPED:
            return "STOPPED";

        case TASKSHADOW_READY:
            return "READY";

        case TASKSHADOW_COMPLETED:
            return "COMPLETED";

        case TASKSHADOW_UNUSED:
        default:
            return "UNUSED";
    }
}

static void command_task_list(void)
{
    printf("\r\nTaskShadow tasks:\r\n");
    printf("%-5s %-16s %-12s %s\r\n",
           "ID", "NAME", "STATE", "INTERVAL");

    uint8_t found = 0;

    for (uint8_t slot = 0; slot < TASKSHADOW_MAX_TASKS; ++slot)
    {
        taskshadow_info_t info;

        if (!taskshadow_get_info(slot, &info))
        {
            continue;
        }

        printf("%-5u %-16s %-12s %lu ms\r\n",
               (unsigned int)info.id,
               info.name,
               task_state_name(info.state),
               (unsigned long)info.interval_ms);

        ++found;
    }

    printf("\r\nTasks: %u/%u\r\n",
           (unsigned int)found,
           (unsigned int)TASKSHADOW_MAX_TASKS);
}

static void on_shell_command(const event_t *event)
{
    if (event == NULL || event->data == NULL)
        return;

    const shell_command_event_data_t *data =
        (const shell_command_event_data_t *)event->data;

    switch (data->command)
    {
        case SHELL_CMD_INFO:
            command_info();
            break;

        case SHELL_CMD_VERSION:
            command_version();
            break;

        case SHELL_CMD_UPTIME:
            command_uptime();
            break;

        case SHELL_CMD_TIMER:
            command_timer();
            break;

        case SHELL_CMD_TIMER2:
            command_timer2();
            break;

        case SHELL_CMD_TASK_TEST:
            diagnostic_task_start();
            break;
            
        case SHELL_CMD_TASK_LIST:
            command_task_list();
            break;

        case SHELL_CMD_LED:
            if (data->args != NULL && strcmp(data->args, "on") == 0)
                command_led(true);
            else if (data->args != NULL &&
                     strcmp(data->args, "off") == 0)
                command_led(false);
            else
                printf("\r\nUsage: led <on|off>\r\n");
            break;

        case SHELL_CMD_HISTORY:
            history_print();
            break;

        case SHELL_CMD_DIAG:
            command_diag();
            break;

        case SHELL_CMD_MEMORY:
            memory_print();
            break;

        case SHELL_CMD_MORSE:
            if (data->args != NULL && data->args[0] != '\0')
            {
                printf("\r\nMorse: %s\r\n", data->args);
                morse_send(data->args);
                printf("Morse transmission complete.\r\n");
            }
            else
            {
                printf("\r\nUsage: morse <text>\r\n");
            }
            break;

        case SHELL_CMD_REBOOT:
            printf("\r\nRebooting...\r\n");
            sleep_ms(500);
            watchdog_reboot(0, 0, 0);

            while (true)
                tight_loop_contents();

        default:
            printf("\r\nInternal error: unknown command ID.\r\n");
            break;
    }
}

void shell_command_handlers_init(void)
{
    //gpio_init(LED_PIN);
    //gpio_set_dir(LED_PIN, GPIO_OUT);
    //gpio_put(LED_PIN, false);

    event_register(EVENT_COMMAND, on_shell_command);
    morse_init(LED_PIN);
}