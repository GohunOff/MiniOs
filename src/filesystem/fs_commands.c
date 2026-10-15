#include "filesystem/fs_commands.h"

#include "filesystem/vfs.h"
#include "storage/internal_flash.h"

#include <stdio.h>
#include <string.h>

#define FS_CAT_BUFFER_SIZE (MINIFS_MAX_FILE_SIZE + 1u)

static vfs_status_t g_vfs_status = VFS_STATUS_NOT_INITIALIZED;

static const char *vfs_status_name(vfs_status_t status)
{
    switch (status) {
    case VFS_STATUS_OK:
        return "OK";
    case VFS_STATUS_NOT_INITIALIZED:
        return "NOT INITIALIZED";
    case VFS_STATUS_NOT_AVAILABLE:
        return "NOT AVAILABLE";
    case VFS_STATUS_NOT_FOUND:
        return "NOT FOUND";
    case VFS_STATUS_INVALID_ARGUMENT:
        return "INVALID ARGUMENT";
    case VFS_STATUS_BUFFER_TOO_SMALL:
        return "BUFFER TOO SMALL";
    case VFS_STATUS_IO_ERROR:
        return "I/O ERROR";
    case VFS_STATUS_CORRUPT:
        return "CORRUPT";
    default:
        return "UNKNOWN ERROR";
    }
}

void fs_commands_init(void)
{
    g_vfs_status = vfs_init();

    printf("\r\nFilesystem: %s\r\n",
           vfs_status_name(g_vfs_status));
}

static bool filesystem_ready(void)
{
    if (g_vfs_status != VFS_STATUS_OK) {
        printf("\r\nFilesystem unavailable: %s\r\n",
               vfs_status_name(g_vfs_status));
        return false;
    }

    return true;
}

void fs_command_ls(const char *args)
{
    /* MiniFS v0.1 obsługuje wyłącznie katalog główny. */
    if (args != NULL && args[0] != '\0' &&
        strcmp(args, "/") != 0) {
        printf("\r\nUsage: ls [/] \r\n");
        return;
    }

    if (!filesystem_ready()) {
        return;
    }

    size_t count = 0;
    vfs_status_t status = vfs_list_count(&count);

    if (status != VFS_STATUS_OK) {
        printf("\r\nls: %s\r\n", vfs_status_name(status));
        return;
    }

    printf("\r\nFiles in /\r\n");
    printf("%-32s %10s\r\n", "NAME", "SIZE");
    printf("---------------------------------------------\r\n");

    size_t listed = 0;

    for (size_t i = 0; i < count; ++i) {
        minifs_entry_t entry;

        status = vfs_list_entry(i, &entry);

        if (status != VFS_STATUS_OK) {
            printf("ls: read error: %s\r\n",
                   vfs_status_name(status));
            return;
        }

        printf("%-32s %10lu B\r\n",
               entry.name,
               (unsigned long)entry.size);

        ++listed;
    }

    printf("\r\nFiles: %lu\r\n", (unsigned long)listed);
}

void fs_command_cat(const char *args)
{
    if (args == NULL || args[0] == '\0') {
        printf("\r\nUsage: cat <filename>\r\n");
        return;
    }

    /* Obsługujemy ścieżkę /nazwa, ale bez podkatalogów. */
    const char *name = args;

    if (name[0] == '/') {
        ++name;
    }

    if (name[0] == '\0' || strchr(name, '/') != NULL ||
        strchr(name, '\\') != NULL) {
        printf("\r\ncat: invalid filename\r\n");
        return;
    }

    if (!filesystem_ready()) {
        return;
    }

    /*
     * Największy plik MiniFS ma 3840 bajtów.
     * Dodatkowy bajt pozwala zakończyć tekst znakiem NUL.
     */
    static char buffer[FS_CAT_BUFFER_SIZE];
    size_t bytes_read = 0;

    vfs_status_t status = vfs_read(
        name,
        buffer,
        sizeof(buffer) - 1u,
        &bytes_read
    );

    if (status != VFS_STATUS_OK) {
        printf("\r\ncat: %s: %s\r\n",
               name, vfs_status_name(status));
        return;
    }

    buffer[bytes_read] = '\0';

    printf("\r\n");

    /*
     * MiniFS przechowuje pliki tekstowe. Nie używamy
     * printf("%s"), ponieważ plik może zawierać znaki '%'.
     */
    if (bytes_read != 0u) {
        fwrite(buffer, 1u, bytes_read, stdout);
    }

    printf("\r\n");
}

void fs_command_df(const char *args)
{
    if (args != NULL && args[0] != '\0' &&
        strcmp(args, "internal") != 0) {
        printf("\r\nUsage: df [internal]\r\n");
        return;
    }

    internal_flash_info_t flash;

    if (!internal_flash_get_info(&flash)) {
        printf("\r\nInternal Flash: configuration error\r\n");
        return;
    }

    printf("\r\nMiniOS storage status\r\n\r\n");
    printf("%-12s %-18s %s\r\n",
           "DEVICE", "STATUS", "CAPACITY");
    printf("-----------------------------------------------\r\n");

    if (!flash.configured) {
        printf("%-12s %-18s -\r\n",
               "internal", "not configured");
        printf("\r\nPartition size: not confirmed\r\n");
        return;
    }

    vfs_info_t info;
    vfs_status_t status = vfs_get_info(&info);

    const bool available =
        status == VFS_STATUS_OK && info.available;

    printf("%-12s %-18s %lu KiB\r\n",
           "internal",
           available ? "available" : "not mounted",
           (unsigned long)(flash.size / 1024u));

    if (!available) {
        printf("\r\nFilesystem: %s\r\n",
               vfs_status_name(status));
        return;
    }

    printf("Files: %lu\r\n", (unsigned long)info.file_count);
    printf("File data: %lu bytes\r\n",
           (unsigned long)info.total_bytes);

    /*
     * MiniFS nie ma jeszcze mechanizmu alokacji ani
     * licznika wolnej przestrzeni. Nie podajemy jej
     * jako rzekomo dokładnej wartości.
     */
    printf("Free space: not reported by MiniFS\r\n");
}