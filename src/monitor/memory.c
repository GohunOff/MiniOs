#include <stdint.h>
#include <stdio.h>

#include "monitor/memory.h"

/* Symbols provided by the linker script. */
extern uint8_t __data_start__;
extern uint8_t __data_end__;
extern uint8_t __bss_start__;
extern uint8_t __bss_end__;

#define SRAM_TOTAL (264u * 1024u)

uint32_t memory_get_total_sram(void)
{
    return SRAM_TOTAL;
}

static uint32_t section_size(uintptr_t start, uintptr_t end)
{
    if (end < start)
        return 0;

    return (uint32_t)(end - start);
}

void memory_print(void)
{
    const uintptr_t data_start = (uintptr_t)&__data_start__;
    const uintptr_t data_end   = (uintptr_t)&__data_end__;
    const uintptr_t bss_start  = (uintptr_t)&__bss_start__;
    const uintptr_t bss_end    = (uintptr_t)&__bss_end__;

    const uint32_t data_used =
        section_size(data_start, data_end);

    const uint32_t bss_used =
        section_size(bss_start, bss_end);

    const uint32_t static_used = data_used + bss_used;

    printf("\r\n=== MiniOS Memory Monitor ===\r\n");
    printf("MCU:             RP2040\r\n");
    printf("Total SRAM:      %u KB\r\n",
           (unsigned)(SRAM_TOTAL / 1024u));

    printf("\r\n--- Static RAM ---\r\n");
    printf(".data:           %lu B\r\n",
           (unsigned long)data_used);
    printf(".bss:            %lu B\r\n",
           (unsigned long)bss_used);
    printf("Static total:    %lu B\r\n",
           (unsigned long)static_used);

    printf("\r\n--- Dynamic memory ---\r\n");
    printf("Heap used:       not measured\r\n");
    printf("Heap free:       not measured\r\n");

    printf("\r\n--- Stack ---\r\n");
    printf("Stack used:      not measured\r\n");
    printf("Stack peak:      not measured\r\n");

    printf("\r\nRAM available:   not calculated\r\n");
}