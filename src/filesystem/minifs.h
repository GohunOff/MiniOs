#ifndef MINIOS_MINIFS_H
#define MINIOS_MINIFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MINIFS_SLOT_COUNT       16u
#define MINIFS_SLOT_SIZE        4096u
#define MINIFS_HEADER_SIZE      256u
#define MINIFS_MAX_FILE_SIZE    (MINIFS_SLOT_SIZE - MINIFS_HEADER_SIZE)
#define MINIFS_NAME_MAX         31u

typedef struct {
    char name[MINIFS_NAME_MAX + 1u];
    uint32_t size;
    uint8_t slot;
} minifs_entry_t;

typedef enum {
    MINIFS_STATUS_OK = 0,
    MINIFS_STATUS_NOT_CONFIGURED,
    MINIFS_STATUS_IO_ERROR,
    MINIFS_STATUS_CORRUPT,
    MINIFS_STATUS_NOT_FOUND,
    MINIFS_STATUS_BUFFER_TOO_SMALL,
    MINIFS_STATUS_INVALID_ARGUMENT
} minifs_status_t;

/*
 * Skanuje nagłówki slotów.
 * Nie formatuje i nie zapisuje Flash.
 */
minifs_status_t minifs_init(void);

bool minifs_is_mounted(void);

size_t minifs_file_count(void);

/* index oznacza indeks na liście poprawnych plików. */
minifs_status_t minifs_get_entry(
    size_t index,
    minifs_entry_t *entry
);

/*
 * Odczytuje cały plik do bufora użytkownika.
 * capacity musi pomieścić całą zawartość.
 * bytes_read może być NULL.
 */
minifs_status_t minifs_read_file(
    const char *name,
    void *buffer,
    size_t capacity,
    size_t *bytes_read
);

#endif