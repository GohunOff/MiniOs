#ifndef MINIOS_HISTORY_C_H
#define MINIOS_HISTORY_C_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void history_init(void);
void history_add(const char *command);
void history_print(void);

bool history_up(char *input_buffer,
                size_t buffer_size,
                int *input_pos);

bool history_down(char *input_buffer,
                  size_t buffer_size,
                  int *input_pos);

void history_reset_navigation(void);

#ifdef __cplusplus
}
#endif

#endif