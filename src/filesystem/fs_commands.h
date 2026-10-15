#ifndef MINIOS_FS_COMMANDS_H
#define MINIOS_FS_COMMANDS_H

void fs_commands_init(void);

void fs_command_ls(const char *args);
void fs_command_cat(const char *args);
void fs_command_df(const char *args);

#endif