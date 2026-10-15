#ifndef MINIOS_VFS_H
#define MINIOS_VFS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "minifs.h"

typedef enum {
    VFS_STATUS_OK = 0,
    VFS_STATUS_NOT_INITIALIZED,
    VFS_STATUS_NOT_AVAILABLE,
    VFS_STATUS_NOT_FOUND,
    VFS_STATUS_INVALID_ARGUMENT,
    VFS_STATUS_BUFFER_TOO_SMALL,
    VFS_STATUS_IO_ERROR,
    VFS_STATUS_CORRUPT
} vfs_status_t;

typedef struct {
    bool initialized;
    bool available;
    size_t file_count;
    size_t total_bytes;
} vfs_info_t;

/* Inicjalizuje MiniFS bez formatowania ani zapisu Flash. */
vfs_status_t vfs_init(void);

/* Zwraca informacje o stanie VFS. */
vfs_status_t vfs_get_info(vfs_info_t *info);

/* Liczba poprawnych plików w płaskim katalogu. */
vfs_status_t vfs_list_count(size_t *count);

/* Pobiera wpis pliku według indeksu. */
vfs_status_t vfs_list_entry(
    size_t index,
    minifs_entry_t *entry
);

/* Odczytuje cały plik do bufora użytkownika. */
vfs_status_t vfs_read(
    const char *name,
    void *buffer,
    size_t capacity,
    size_t *bytes_read
);

#endif