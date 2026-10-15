#include "internal_flash.h"

#include <string.h>

#include "hardware/regs/addressmap.h"

/*
 * Nie definiujemy domyślnie adresu partycji.
 *
 * Włączenie wymaga jawnego podania:
 * MINIOS_INTERNAL_FLASH_OFFSET
 * MINIOS_INTERNAL_FLASH_SIZE
 * MINIOS_INTERNAL_FLASH_PHYSICAL_SIZE
 *
 * Wartości muszą być potwierdzone dla konkretnej
 * konfiguracji sprzętu i linkera.
 */

#if defined(MINIOS_INTERNAL_FLASH_OFFSET) && \
    defined(MINIOS_INTERNAL_FLASH_SIZE) && \
    defined(MINIOS_INTERNAL_FLASH_PHYSICAL_SIZE)

#define MINIOS_IFLASH_CONFIGURED 1

#define IFLASH_OFFSET \
    ((uint32_t)MINIOS_INTERNAL_FLASH_OFFSET)

#define IFLASH_SIZE \
    ((size_t)MINIOS_INTERNAL_FLASH_SIZE)

#define IFLASH_PHYSICAL_SIZE \
    ((size_t)MINIOS_INTERNAL_FLASH_PHYSICAL_SIZE)

#if (MINIOS_INTERNAL_FLASH_SIZE == 0)
#error "MiniOS internal Flash partition cannot be empty"
#endif

#if (MINIOS_INTERNAL_FLASH_OFFSET < 0)
#error "MiniOS internal Flash offset cannot be negative"
#endif

#if (MINIOS_INTERNAL_FLASH_PHYSICAL_SIZE == 0)
#error "MiniOS physical Flash size cannot be zero"
#endif

#else

#define MINIOS_IFLASH_CONFIGURED 0

#endif

bool internal_flash_get_info(internal_flash_info_t *info)
{
    if (info == NULL) {
        return false;
    }

    info->offset = 0;
    info->size = 0;
    info->configured = false;

#if MINIOS_IFLASH_CONFIGURED

    /*
     * Kontrola zakresu bez dodawania offsetu i rozmiaru,
     * co zapobiega przepełnieniu arytmetycznemu.
     */
    if ((size_t)IFLASH_OFFSET > IFLASH_PHYSICAL_SIZE ||
        IFLASH_SIZE > IFLASH_PHYSICAL_SIZE - (size_t)IFLASH_OFFSET) {
        return false;
    }

    if (IFLASH_SIZE != MINIOS_STORAGE_SIZE) {
        return false;
    }

    info->offset = IFLASH_OFFSET;
    info->size = IFLASH_SIZE;
    info->configured = true;

    return true;

#else

    return true;

#endif
}

bool internal_flash_read(
    size_t offset,
    void *destination,
    size_t length)
{
    internal_flash_info_t info;

    if (destination == NULL && length != 0) {
        return false;
    }

    if (!internal_flash_get_info(&info)) {
        return false;
    }

    if (!info.configured) {
        return false;
    }

    if (offset > info.size || length > info.size - offset) {
        return false;
    }

    if (length == 0) {
        return true;
    }

    /*
     * Odczyt przez mapowanie XIP.
     * Ten moduł nie wykonuje programowania ani kasowania.
     */
    const uintptr_t address =
        (uintptr_t)XIP_BASE + (uintptr_t)info.offset + offset;

    memcpy(destination, (const void *)address, length);

    return true;
}