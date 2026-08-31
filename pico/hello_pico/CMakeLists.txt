cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(hello_pico C CXX ASM)

set(PICO_BOARD pico2_w)

pico_sdk_init()

add_executable(hello_pico main.c)

target_link_libraries(hello_pico
    pico_stdlib
    pico_cyw43_arch_none
)

# USB serial console, no UART (see T2).
pico_enable_stdio_usb(hello_pico 1)
pico_enable_stdio_uart(hello_pico 0)

# Produce .uf2 / .bin / .hex alongside the .elf.
pico_add_extra_outputs(hello_pico)
