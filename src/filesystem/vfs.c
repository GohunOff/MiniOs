#include "vfs.h"

#include "storage/internal_flash.h"

static bool g_initialized;
static bool g_available;

static vfs_status_t map_minifs_status(minifs_status_t status)
{
    switch (status) {
    case MINIFS_STATUS_OK:
        return VFS_STATUS_OK;

    case MINIFS_STATUS_NOT_CONFIGURED:
        return VFS_STATUS_NOT_AVAILABLE;

    case MINIFS_STATUS_NOT_FOUND:
        return VFS_STATUS_NOT_FOUND;

    case MINIFS_STATUS_BUFFER_TOO_SMALL:
        return VFS_STATUS_BUFFER_TOO_SMALL;

    case MINIFS_STATUS_INVALID_ARGUMENT:
        return VFS_STATUS_INVALID_ARGUMENT;

    case MINIFS_STATUS_IO_ERROR:
        return VFS_STATUS_IO_ERROR;

    case MINIFS_STATUS_CORRUPT:
        return VFS_STATUS_CORRUPT;

    default:
        return VFS_STATUS_IO_ERROR;
    }
}

vfs_status_t vfs_init(void)
{
    g_initialized = false;
    g_available = false;

    internal_flash_info_t flash_info;

    if (!internal_flash_get_info(&flash_info)) {
        return VFS_STATUS_IO_ERROR;
    }

    if (!flash_info.configured) {
        return VFS_STATUS_NOT_AVAILABLE;
    }

    minifs_status_t status = minifs_init();

    if (status != MINIFS_STATUS_OK) {
        return map_minifs_status(status);
    }

    g_initialized = true;
    g_available = minifs_is_mounted();

    return g_available ? VFS_STATUS_OK : VFS_STATUS_NOT_AVAILABLE;
}

vfs_status_t vfs_get_info(vfs_info_t *info)
{
    if (info == NULL) {
        return VFS_STATUS_INVALID_ARGUMENT;
    }

    info->initialized = g_initialized;
    info->available = g_available;
    info->file_count = 0u;
    info->total_bytes = 0u;

    if (!g_initialized) {
        return VFS_STATUS_NOT_INITIALIZED;
    }

    if (!g_available) {
        return VFS_STATUS_NOT_AVAILABLE;
    }

    size_t count = minifs_file_count();

    for (size_t i = 0; i < count; ++i) {
        minifs_entry_t entry;

        if (minifs_get_entry(i, &entry) != MINIFS_STATUS_OK) {
            return VFS_STATUS_IO_ERROR;
        }

        if (entry.size > SIZE_MAX - info->total_bytes) {
            return VFS_STATUS_CORRUPT;
        }

        info->total_bytes += entry.size;
    }

    info->file_count = count;

    return VFS_STATUS_OK;
}

vfs_status_t vfs_list_count(size_t *count)
{
    if (count == NULL) {
        return VFS_STATUS_INVALID_ARGUMENT;
    }

    *count = 0u;

    if (!g_initialized) {
        return VFS_STATUS_NOT_INITIALIZED;
    }

    if (!g_available) {
        return VFS_STATUS_NOT_AVAILABLE;
    }

    *count = minifs_file_count();

    return VFS_STATUS_OK;
}

vfs_status_t vfs_list_entry(
    size_t index,
    minifs_entry_t *entry)
{
    if (entry == NULL) {
        return VFS_STATUS_INVALID_ARGUMENT;
    }

    if (!g_initialized) {
        return VFS_STATUS_NOT_INITIALIZED;
    }

    if (!g_available) {
        return VFS_STATUS_NOT_AVAILABLE;
    }

    return map_minifs_status(minifs_get_entry(index, entry));
}

vfs_status_t vfs_read(
    const char *name,
    void *buffer,
    size_t capacity,
    size_t *bytes_read)
{
    if (bytes_read != NULL) {
        *bytes_read = 0u;
    }

    if (name == NULL || (buffer == NULL && capacity != 0u)) {
        return VFS_STATUS_INVALID_ARGUMENT;
    }

    if (!g_initialized) {
        return VFS_STATUS_NOT_INITIALIZED;
    }

    if (!g_available) {
        return VFS_STATUS_NOT_AVAILABLE;
    }

    return map_minifs_status(
        minifs_read_file(name, buffer, capacity, bytes_read)
    );
}