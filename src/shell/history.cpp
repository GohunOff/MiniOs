#include "history.h"

#include <stdio.h>
#include <string.h>

History::History()
    : history_count(0),
      history_next(0),
      history_pos(-1)
{
    history_current[0] = '\0';
}

void History::add(const char *command)
{
    if (command == NULL || command[0] == '\0')
    {
        return;
    }

    strncpy(command_history[history_next],
            command,
            HISTORY_CMD_LEN - 1);

    command_history[history_next][HISTORY_CMD_LEN - 1] = '\0';

    history_next = (history_next + 1) % HISTORY_SIZE;

    if (history_count < HISTORY_SIZE)
    {
        history_count++;
    }
}

void History::print()
{
    printf("\r\nCommand History\r\n");
    printf("----------------\r\n");

    if (history_count == 0)
    {
        printf("History is empty.\r\n");
        return;
    }

    uint8_t oldest =
        (history_next + HISTORY_SIZE - history_count) % HISTORY_SIZE;

    for (uint8_t i = 0; i < history_count; i++)
    {
        uint8_t index = (oldest + i) % HISTORY_SIZE;

        printf("%u  %s\r\n",
               static_cast<unsigned>(i + 1),
               command_history[index]);
    }
}

bool History::up(char *input_buffer,
                 size_t buffer_size,
                 int &input_pos)
{
    if (input_buffer == NULL || buffer_size == 0 || history_count == 0)
    {
        return false;
    }

    if (history_pos == -1)
    {
        strncpy(history_current, input_buffer, sizeof(history_current) - 1);
        history_current[sizeof(history_current) - 1] = '\0';

        history_pos = 0;
    }
    else if (history_pos < history_count - 1)
    {
        history_pos++;
    }

    uint8_t index =
        (history_next + HISTORY_SIZE - 1 - history_pos) % HISTORY_SIZE;

    strncpy(input_buffer, command_history[index], buffer_size - 1);
    input_buffer[buffer_size - 1] = '\0';

    input_pos = static_cast<int>(strlen(input_buffer));

    return true;
}

bool History::down(char *input_buffer,
                   size_t buffer_size,
                   int &input_pos)
{
    if (input_buffer == NULL || buffer_size == 0 || history_pos == -1)
    {
        return false;
    }

    if (history_pos > 0)
    {
        history_pos--;

        uint8_t index =
            (history_next + HISTORY_SIZE - 1 - history_pos) % HISTORY_SIZE;

        strncpy(input_buffer, command_history[index], buffer_size - 1);
        input_buffer[buffer_size - 1] = '\0';
    }
    else
    {
        history_pos = -1;

        strncpy(input_buffer, history_current, buffer_size - 1);
        input_buffer[buffer_size - 1] = '\0';
    }

    input_pos = static_cast<int>(strlen(input_buffer));

    return true;
}

void History::reset_navigation()
{
    history_pos = -1;
    history_current[0] = '\0';
}