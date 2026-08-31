cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(mrubyz_pico C CXX ASM)

set(PICO_BOARD pico2_w)

pico_sdk_init()

# Repo root holds the single-file VM and its headers.
set(MRUBYZ_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

# Bytecode now arrives over serial (T9/T10), not embedded, so no add.ruby.c.
add_executable(mrubyz_pico
    main.c
    ${MRUBYZ_ROOT}/mrubyz.c
)

# Integer-only VM base, no Master System / text builtins (T4).
target_compile_definitions(mrubyz_pico PRIVATE MRBZ_MINIMAL)

target_include_directories(mrubyz_pico PRIVATE ${MRUBYZ_ROOT})

target_link_libraries(mrubyz_pico
    pico_stdlib
    pico_cyw43_arch_none
)

# USB serial console, no UART (T2).
pico_enable_stdio_usb(mrubyz_pico 1)
pico_enable_stdio_uart(mrubyz_pico 0)

pico_add_extra_outputs(mrubyz_pico)
