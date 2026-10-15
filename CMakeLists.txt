cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(MiniOS C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w)
pico_sdk_init()

add_executable(MiniOS
    src/main.c
    src/shell_command_handlers.c

    src/kernel/event.c
    src/kernel/timer.c
    src/kernel/taskshadow.c

    src/services/morse.c
    src/services/uptime.c
    
    src/monitor/memory.c

    src/shell/shell.cpp
    src/shell/history.cpp
    src/shell/history_c.cpp

    src/system/core.c
    src/filesystem/minifs.c
    src/storage/internal_flash.c
    src/filesystem/vfs.c
    src/filesystem/fs_commands.c
)

# Pozostaw końcowe 64 KiB poza regionem linkera.
pico_override_flash_size(MiniOS 0x1F0000)

target_include_directories(MiniOS PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_compile_definitions(MiniOS PRIVATE
    MINIOS_INTERNAL_FLASH_OFFSET=0x1F0000
    MINIOS_INTERNAL_FLASH_SIZE=0x10000
    MINIOS_INTERNAL_FLASH_PHYSICAL_SIZE=0x200000
)

target_link_libraries(MiniOS
    pico_stdlib
    hardware_flash
    pico_flash
)

pico_enable_stdio_usb(MiniOS 1)
pico_enable_stdio_uart(MiniOS 0)

target_link_options(MiniOS PRIVATE
    "-Wl,--defsym=HEAP_LIMIT=0x2000292c"
)

target_link_options(MiniOS PRIVATE
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/MiniOS.map"
)

pico_add_extra_outputs(MiniOS)
