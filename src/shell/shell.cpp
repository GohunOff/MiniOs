#include "shell.hpp"
#include "shell_c.h"
#include "kernel/event.h"

#include <cstdio>
#include <cstring>

#include "shell_command_event.h"

static Shell g_shell;

static void emitCommand(shell_command_id_t command,
    const char* args = "")
{
shell_command_event_data_t data = { command, args };

std::printf("\r\n[DEBUG] Emitting command ID = %d\r\n",
    (int)command);

event_emit(EVENT_COMMAND, &data);
}

static void handle_help(Shell& shell, const char*)
{
    shell.printHelp();
}

static void handle_clear(Shell&, const char*)
{
    std::printf("\033[2J\033[H");
}

static void handle_echo(Shell&, const char* args)
{
    std::printf("\r\n%s\r\n", args);
}

static void handle_info(Shell&, const char*)
{
    emitCommand(SHELL_CMD_INFO);
}

static void handle_version(Shell&, const char*)
{
    emitCommand(SHELL_CMD_VERSION);
}

static void handle_uptime(Shell&, const char*)
{
    emitCommand(SHELL_CMD_UPTIME);
}

static void handle_timer(Shell&, const char*)
{
    emitCommand(SHELL_CMD_TIMER);
}

static void handle_timer2(Shell&, const char*)
{
    emitCommand(SHELL_CMD_TIMER2);
}

static void handle_led(Shell&, const char* args)
{
    if (std::strcmp(args, "on") == 0 ||
        std::strcmp(args, "off") == 0)
    {
        emitCommand(SHELL_CMD_LED, args);
    }
    else
    {
        std::printf("\r\nUsage: led <on|off>\r\n");
    }
}

static void handle_history(Shell&, const char*)
{
    emitCommand(SHELL_CMD_HISTORY);
}

static void handle_morse(Shell&, const char* args)
{
    if (*args == '\0')
    {
        std::printf("\r\nUsage: morse <text>\r\n");
        return;
    }

    emitCommand(SHELL_CMD_MORSE, args);
}

static void handle_mem(Shell&, const char*)
{
    emitCommand(SHELL_CMD_MEMORY);
}

static void handle_diag(Shell&, const char*)
{
    emitCommand(SHELL_CMD_DIAG);
}

static void handle_taskTest(Shell&, const char*)
{
    emitCommand(SHELL_CMD_TASK_TEST);
}

static void handle_reboot(Shell&, const char*)
{
    emitCommand(SHELL_CMD_REBOOT);
    /*std::printf("\r\nRebooting...\r\n");
    sleep_ms(500);
    watchdog_reboot(0, 0, 0);

    while (true)
        tight_loop_contents();*/
}

static void handle_taskList(Shell&, const char*)
{
    emitCommand(SHELL_CMD_TASK_LIST);
}

static void handle_ls(Shell&, const char* args)
{
    emitCommand(SHELL_CMD_FS_LS, args);
}

static void handle_cat(Shell&, const char* args)
{
    emitCommand(SHELL_CMD_FS_CAT, args);
}

static void handle_write(Shell&, const char* args)
{
    emitCommand(SHELL_CMD_FS_WRITE, args);
}

static void handle_rm(Shell&, const char* args)
{
    emitCommand(SHELL_CMD_FS_RM, args);
}

static void handle_df(Shell&, const char* args)
{
    emitCommand(SHELL_CMD_FS_DF, args);
}

static void handle_run(Shell&, const char* args)
{
    emitCommand(SHELL_CMD_FS_RUN, args);
}


const Shell::Command Shell::commands[] = {
    {"help",    "Show this help",          handle_help},
    {"info",    "System information",      handle_info},
    {"version", "Show MiniOS version",     handle_version},
    {"uptime",  "Show system uptime",      handle_uptime},
    {"timer",   "Test kernel timer",       handle_timer},
    {"timer2",  "Test second timer",       handle_timer2},
    {"clear",   "Clear terminal",          handle_clear},
    {"led",     "Control LED",             handle_led},
    {"echo",    "Echo text",               handle_echo},
    {"history", "Show command history",    handle_history},
    {"morse",   "Send text using Morse",   handle_morse},
    {"mem",     "Show memory information", handle_mem},
    {"diag",    "System diagnostics",      handle_diag},
    {"ls",    "List files",             handle_ls},
    {"cat",   "Display file contents",  handle_cat},
    {"write", "Write text to file",     handle_write},
    {"rm",    "Remove file",            handle_rm},
    {"df",    "Show storage devices",   handle_df},
    {"run",   "Run MiniScript file",    handle_run},
    {"reboot",  "Reboot system",           handle_reboot},
    {"taskTest","Test Task",               handle_taskTest},
    {"taskList","List Tasks",               handle_taskList},
};

void Shell::execute(const char* input)
{
    if (input == nullptr)
        return;

    while (*input == ' ' || *input == '\t')
        ++input;

    if (*input == '\0')
        return;

    const std::size_t input_len = std::strlen(input);

    for (const auto& cmd : commands)
    {
        const std::size_t len = std::strlen(cmd.name);

        if (input_len >= len &&
            std::strncmp(input, cmd.name, len) == 0 &&
            (input_len == len ||
            input[len] == ' ' ||
            input[len] == '\t'))
        {
            const char* args = input + len;

            while (*args == ' ' || *args == '\t')
                ++args;

            cmd.handler(*this, args);
            return;
        }
    }

    std::printf("\r\nUnknown command: %s\r\n", input);
    std::printf("Type 'help' for available commands.\r\n");
}

void Shell::printHelp() const
{
    std::printf("\r\nAvailable commands:\r\n\r\n");

    for (const auto& cmd : commands)
        std::printf("  %-10s %s\r\n", cmd.name, cmd.description);

    std::printf("\r\n");
}

extern "C" void shell_init(void)
{
}

extern "C" void shell_execute(const char* command)
{
    g_shell.execute(command);
}