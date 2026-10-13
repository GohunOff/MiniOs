cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(MiniOS C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

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
)

target_include_directories(MiniOS PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)

target_link_libraries(MiniOS
    pico_stdlib
)

pico_enable_stdio_usb(MiniOS 1)
pico_enable_stdio_uart(MiniOS 0)

target_link_options(MiniOS PRIVATE
    "-Wl,--defsym=HEAP_LIMIT=0x2000292c"
)

pico_add_extra_outputs(MiniOS)
