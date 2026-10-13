#include <stdio.h>
//#include "hardware/watchdog.h"
//#include <string.h>
//#include <stdint.h>

#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "kernel/timer.h"
#include "kernel/taskshadow.h"
#include "kernel/event.h"

#include "shell/history_c.h"
#include "shell/shell_c.h"
#include "shell_command_event.h"

//#include "services/uptime.h"

#include "system/core.h"

#define INPUT_BUFFER_SIZE 64

static int escape_state = 0;

static char input_buffer[INPUT_BUFFER_SIZE];
static int input_pos = 0;

static void input_redraw(void)
{
    printf("\rmini> %s\033[K", input_buffer);
}

int main(void)
{
    stdio_init_all();

    event_init();
    core_init();
    taskshadow_init();
    history_init();

    core_init_event();

    shell_command_handlers_init();
    shell_init();

    sleep_ms(2000);

    event_emit(EVENT_SYSTEM_READY, NULL);

    bool terminal_connected = false;

    boot_sequence();
    print_banner();

    while (true)
    {
        bool connected = stdio_usb_connected();

        if (connected && !terminal_connected)
        {
            terminal_connected = true;

            event_emit(EVENT_TERMINAL_CONNECTED, NULL);
        }

        if (!connected)
        {
            terminal_connected = false;

            event_emit(EVENT_TERMINAL_DISCONNECTED, NULL);
        }

        tight_loop_contents();

        timer_poll();
        taskshadow_poll();

        int c = getchar_timeout_us(10000);

        if (c == PICO_ERROR_TIMEOUT)
        {
            continue;
        }

        /* ANSI escape sequences: arrows from PuTTY */
        if (escape_state == 1)
        {
            if (c == '[')
            {
                escape_state = 2;
            }
            else
            {
                escape_state = 0;
            }

            continue;
        }

        if (escape_state == 2)
        {
            escape_state = 0;

            if (c == 'A')
            {
                if (history_up(input_buffer, sizeof(input_buffer), &input_pos))
                {
                    input_redraw();
                }
            }
            else if (c == 'B')
            {
                if (history_down(input_buffer, sizeof(input_buffer), &input_pos))
                {
                    input_redraw();
                }
            }

            continue;
        }

        static bool skip_lf = false;

        /* Przed zwykłą obsługą znaków wejściowych */
        if (skip_lf)
        {
            skip_lf = false;

            if (c == '\n')
            {
                continue;
            }
        }

        if (c == 27)
        {
            escape_state = 1;
        }
        else if (c == '\r' || c == '\n')
        {
            if (c == '\r')
            {
                skip_lf = true;
            }

            input_buffer[input_pos] = '\0';

            bool has_command = false;

            for (int i = 0; input_buffer[i] != '\0'; ++i)
            {
                if (input_buffer[i] != ' ' && input_buffer[i] != '\t')
                {
                    has_command = true;
                    break;
                }
            }

            if (has_command)
            {
                history_add(input_buffer);
                shell_execute(input_buffer);
            }

            input_pos = 0;
            input_buffer[0] = '\0';
            history_reset_navigation();

            printf("\r\nmini> ");
        }
        else if (c == '\b' || c == 127)
        {
            if (input_pos > 0)
            {
                input_pos--;
                input_buffer[input_pos] = '\0';

                printf("\b \b");
            }

            /* Editing a recalled command starts a new input line */
            history_reset_navigation();
        }
        else if (c >= 32 && c <= 126)
        {
            if (input_pos < INPUT_BUFFER_SIZE - 1)
            {
                input_buffer[input_pos++] = (char)c;
                input_buffer[input_pos] = '\0';

                putchar(c);
                history_reset_navigation();
            }
        }
    }

    return 0;
}

