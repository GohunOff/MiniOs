#ifndef SYSTEM_CORE_H
#define SYSTEM_CORE_H

#include <stdio.h>

void print_banner(void);
void boot_sequence(void);

void command_info(void);
void command_version(void);
void command_diag(void);

void command_timer(void);
void command_timer2(void);

void command_uptime(void);

void core_init(void);
void core_init_event(void);

void diagnostic_task_start(void);
#endif