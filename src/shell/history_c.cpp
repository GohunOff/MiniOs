#include "history_c.h"
#include "history.h"

namespace
{
    History history;
}

extern "C"
{

void history_init(void)
{
    history = History();
}

void history_add(const char *command)
{
    history.add(command);
}

void history_print(void)
{
    history.print();
}

bool history_up(char *input_buffer,
                size_t buffer_size,
                int *input_pos)
{
    if (input_pos == nullptr)
        return false;

    return history.up(input_buffer, buffer_size, *input_pos);
}

bool history_down(char *input_buffer,
                  size_t buffer_size,
                  int *input_pos)
{
    if (input_pos == nullptr)
        return false;

    return history.down(input_buffer, buffer_size, *input_pos);
}

void history_reset_navigation(void)
{
    history.reset_navigation();
}

}