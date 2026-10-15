#ifndef MINIOS_INTERNAL_FLASH_H
#define MINIOS_INTERNAL_FLASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MINIOS_STORAGE_SIZE        (64u * 1024u)
#define MINIOS_STORAGE_SECTOR_SIZE  4096u
#define MINIOS_STORAGE_PAGE_SIZE    256u

typedef struct {
    uint32_t offset;
    size_t size;
    bool configured;
} internal_flash_info_t;

/*
 * Zwraca informacje o partycji.
 * configured == false oznacza, że partycja
 * nie została jawnie skonfigurowana.
 */
bool internal_flash_get_info(internal_flash_info_t *info);

/*
 * Odczytuje dane z partycji.
 * offset jest względny wobec początku partycji.
 * Nie zapisuje ani nie kasuje pamięci Flash.
 */
bool internal_flash_read(
    size_t offset,
    void *destination,
    size_t length
);

#endif