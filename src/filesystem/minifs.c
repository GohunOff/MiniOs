#include "minifs.h"

#include <string.h>

#include "storage/internal_flash.h"

#define MINIFS_MAGIC_0       'M'
#define MINIFS_MAGIC_1       'F'
#define MINIFS_MAGIC_2       'S'
#define MINIFS_MAGIC_3       '1'

#define MINIFS_FORMAT_VERSION 1u
#define MINIFS_VALID_FLAGS    0xA55Au

#define OFF_MAGIC       0u
#define OFF_VERSION     4u
#define OFF_FLAGS       6u
#define OFF_NAME        8u
#define OFF_FILE_SIZE   40u
#define OFF_DATA_CRC    44u
#define OFF_HEADER_CRC  48u

#define MINIFS_CRC_HEADER_LENGTH OFF_HEADER_CRC

static minifs_entry_t g_entries[MINIFS_SLOT_COUNT];
static size_t g_entry_count;
static bool g_mounted;

static uint16_t read_u16_le(const uint8_t *p)
{
    return (uint16_t)p[0] |
           ((uint16_t)p[1] << 8);
}

static uint32_t read_u32_le(const uint8_t *p)
{
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

static uint32_t crc32_update(
    uint32_t crc,
    const uint8_t *data,
    size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];

        for (unsigned bit = 0; bit < 8u; ++bit) {
            if ((crc & 1u) != 0u) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

static uint32_t crc32_buffer(
    const uint8_t *data,
    size_t length)
{
    return crc32_update(0xFFFFFFFFu, data, length) ^
           0xFFFFFFFFu;
}

static bool valid_name(const char *name)
{
    if (name == NULL || name[0] == '\0') {
        return false;
    }

    if (strcmp(name, ".") == 0 ||
        strcmp(name, "..") == 0) {
        return false;
    }

    size_t length = 0;

    while (name[length] != '\0') {
        unsigned char c = (unsigned char)name[length];

        if (length >= MINIFS_NAME_MAX) {
            return false;
        }

        /* Płaski katalog: bez separatorów ścieżki. */
        if (c < 0x21u || c > 0x7Eu ||
            c == '/' || c == '\\' || c == ':') {
            return false;
        }

        ++length;
    }

    return true;
}

static bool header_is_erased(const uint8_t *header)
{
    for (size_t i = 0; i < MINIFS_HEADER_SIZE; ++i) {
        if (header[i] != 0xFFu) {
            return false;
        }
    }

    return true;
}

static bool parse_header(
    const uint8_t *header,
    minifs_entry_t *entry,
    uint32_t *data_crc)
{
    if (header[OFF_MAGIC] != MINIFS_MAGIC_0 ||
        header[OFF_MAGIC + 1u] != MINIFS_MAGIC_1 ||
        header[OFF_MAGIC + 2u] != MINIFS_MAGIC_2 ||
        header[OFF_MAGIC + 3u] != MINIFS_MAGIC_3) {
        return false;
    }

    if (read_u16_le(&header[OFF_VERSION]) !=
            MINIFS_FORMAT_VERSION ||
        read_u16_le(&header[OFF_FLAGS]) !=
            MINIFS_VALID_FLAGS) {
        return false;
    }

    if (read_u32_le(&header[OFF_HEADER_CRC]) !=
            crc32_buffer(header, MINIFS_CRC_HEADER_LENGTH)) {
        return false;
    }

    /*
     * Nazwa musi kończyć się w obrębie pola o 32 bajtach.
     */
    const uint8_t *name = &header[OFF_NAME];
    size_t name_length = 0;

    while (name_length <= MINIFS_NAME_MAX &&
           name[name_length] != '\0') {
        ++name_length;
    }

    if (name_length == 0 ||
        name_length > MINIFS_NAME_MAX) {
        return false;
    }

    for (size_t i = name_length + 1u;
         i < MINIFS_NAME_MAX + 1u;
         ++i) {
        if (name[i] != '\0') {
            return false;
        }
    }

    char local_name[MINIFS_NAME_MAX + 1u];
    memcpy(local_name, name, name_length);
    local_name[name_length] = '\0';

    if (!valid_name(local_name)) {
        return false;
    }

    uint32_t file_size = read_u32_le(&header[OFF_FILE_SIZE]);

    if (file_size > MINIFS_MAX_FILE_SIZE) {
        return false;
    }

    memcpy(entry->name, local_name, name_length + 1u);
    entry->size = file_size;
    entry->slot = 0u;

    *data_crc = read_u32_le(&header[OFF_DATA_CRC]);

    return true;
}

minifs_status_t minifs_init(void)
{
    internal_flash_info_t info;

    g_mounted = false;
    g_entry_count = 0u;
    memset(g_entries, 0, sizeof(g_entries));

    if (!internal_flash_get_info(&info)) {
        return MINIFS_STATUS_IO_ERROR;
    }

    if (!info.configured) {
        return MINIFS_STATUS_NOT_CONFIGURED;
    }

    if (info.size < MINIFS_SLOT_COUNT * MINIFS_SLOT_SIZE) {
        return MINIFS_STATUS_CORRUPT;
    }

    uint8_t header[MINIFS_HEADER_SIZE];
    uint32_t data_crcs[MINIFS_SLOT_COUNT];
    bool duplicate[MINIFS_SLOT_COUNT] = { false };

    for (size_t slot = 0; slot < MINIFS_SLOT_COUNT; ++slot) {
        size_t offset = slot * MINIFS_SLOT_SIZE;

        if (!internal_flash_read(offset, header, sizeof(header))) {
            g_entry_count = 0u;
            return MINIFS_STATUS_IO_ERROR;
        }

        if (header_is_erased(header)) {
            continue;
        }

        minifs_entry_t entry;
        uint32_t data_crc;

        if (!parse_header(header, &entry, &data_crc)) {
            /*
             * Nieprawidłowy slot nie jest udostępniany jako plik.
             */
            continue;
        }

        entry.slot = (uint8_t)slot;
        g_entries[g_entry_count] = entry;
        data_crcs[g_entry_count] = data_crc;
        ++g_entry_count;
    }

    /*
     * Duplikaty nazw są niejednoznaczne.
     * Ukrywamy wszystkie wpisy o powtarzającej się nazwie.
     */
    for (size_t i = 0; i < g_entry_count; ++i) {
        for (size_t j = i + 1u; j < g_entry_count; ++j) {
            if (strcmp(g_entries[i].name, g_entries[j].name) == 0) {
                duplicate[i] = true;
                duplicate[j] = true;
            }
        }
    }

    size_t out = 0u;

    for (size_t i = 0; i < g_entry_count; ++i) {
        if (!duplicate[i]) {
            if (out != i) {
                g_entries[out] = g_entries[i];
                data_crcs[out] = data_crcs[i];
            }
            ++out;
        }
    }

    g_entry_count = out;

    /*
     * Zapamiętane CRC danych jest weryfikowane ponownie
     * podczas odczytu pliku.
     */
    (void)data_crcs;

    g_mounted = true;
    return MINIFS_STATUS_OK;
}

bool minifs_is_mounted(void)
{
    return g_mounted;
}

size_t minifs_file_count(void)
{
    return g_mounted ? g_entry_count : 0u;
}

minifs_status_t minifs_get_entry(
    size_t index,
    minifs_entry_t *entry)
{
    if (entry == NULL) {
        return MINIFS_STATUS_INVALID_ARGUMENT;
    }

    if (!g_mounted) {
        return MINIFS_STATUS_NOT_CONFIGURED;
    }

    if (index >= g_entry_count) {
        return MINIFS_STATUS_NOT_FOUND;
    }

    *entry = g_entries[index];
    return MINIFS_STATUS_OK;
}

minifs_status_t minifs_read_file(
    const char *name,
    void *buffer,
    size_t capacity,
    size_t *bytes_read)
{
    if (bytes_read != NULL) {
        *bytes_read = 0u;
    }

    if (!valid_name(name) || (buffer == NULL && capacity != 0u)) {
        return MINIFS_STATUS_INVALID_ARGUMENT;
    }

    if (!g_mounted) {
        return MINIFS_STATUS_NOT_CONFIGURED;
    }

    const minifs_entry_t *entry = NULL;

    for (size_t i = 0; i < g_entry_count; ++i) {
        if (strcmp(g_entries[i].name, name) == 0) {
            entry = &g_entries[i];
            break;
        }
    }

    if (entry == NULL) {
        return MINIFS_STATUS_NOT_FOUND;
    }

    if (capacity < entry->size) {
        return MINIFS_STATUS_BUFFER_TOO_SMALL;
    }

    size_t data_offset =
        (size_t)entry->slot * MINIFS_SLOT_SIZE +
        MINIFS_HEADER_SIZE;

    if (entry->size != 0u &&
        !internal_flash_read(data_offset, buffer, entry->size)) {
        return MINIFS_STATUS_IO_ERROR;
    }

    /*
     * Ponownie odczytujemy nagłówek, aby zweryfikować CRC danych
     * zapisane w formacie MiniFS.
     */
    uint8_t header[MINIFS_HEADER_SIZE];

    if (!internal_flash_read(
            (size_t)entry->slot * MINIFS_SLOT_SIZE,
            header,
            sizeof(header))) {
        return MINIFS_STATUS_IO_ERROR;
    }

    uint32_t expected_crc = read_u32_le(&header[OFF_DATA_CRC]);
    uint32_t actual_crc = crc32_buffer(
        (const uint8_t *)buffer,
        entry->size);

    if (actual_crc != expected_crc) {
        if (buffer != NULL && entry->size != 0u) {
            memset(buffer, 0, entry->size);
        }

        return MINIFS_STATUS_CORRUPT;
    }

    if (bytes_read != NULL) {
        *bytes_read = entry->size;
    }

    return MINIFS_STATUS_OK;
}