#ifndef MINIOS_SHELL_C_H
#define MINIOS_SHELL_C_H

#ifdef __cplusplus
extern "C" {
#endif

void shell_init(void);
void shell_execute(const char *command);

#ifdef __cplusplus
}
#endif

#endif