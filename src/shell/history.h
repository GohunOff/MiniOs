#ifndef MINIOS_HISTORY_H
#define MINIOS_HISTORY_H

#include <stdint.h>
#include <stddef.h>

class History
{
public:
    History();

    void add(const char *command);
    void print();

    bool up(char *input_buffer, size_t buffer_size, int &input_pos);
    bool down(char *input_buffer, size_t buffer_size, int &input_pos);

    void reset_navigation();

private:
    enum
    {
        HISTORY_SIZE = 10,
        HISTORY_CMD_LEN = 64
    };

    char command_history[HISTORY_SIZE][HISTORY_CMD_LEN];

    uint8_t history_count;
    uint8_t history_next;

    int history_pos;
    char history_current[HISTORY_CMD_LEN];
};

#endif