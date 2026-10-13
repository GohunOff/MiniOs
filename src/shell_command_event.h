#ifndef MINIOS_SHELL_COMMAND_EVENT_H
#define MINIOS_SHELL_COMMAND_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SHELL_CMD_INFO,
    SHELL_CMD_VERSION,
    SHELL_CMD_UPTIME,
    SHELL_CMD_TIMER,
    SHELL_CMD_TIMER2,
    SHELL_CMD_LED,
    SHELL_CMD_HISTORY,
    SHELL_CMD_DIAG,
    SHELL_CMD_MEMORY,
    SHELL_CMD_MORSE,
    SHELL_CMD_REBOOT,

    SHELL_CMD_TASK_TEST,
    SHELL_CMD_TASK_LIST,
    SHELL_CMD_TASK_STOP,
    SHELL_CMD_TASK_START,
    SHELL_CMD_TASK_Remove
} shell_command_id_t;

typedef struct
{
    shell_command_id_t command;
    const char *args;
} shell_command_event_data_t;

void shell_command_handlers_init(void);

#ifdef __cplusplus
}
#endif

#endif