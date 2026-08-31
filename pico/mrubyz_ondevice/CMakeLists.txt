cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(mrubyz_ondevice C CXX ASM)

set(PICO_BOARD pico2_w)

pico_sdk_init()

# Repo root holds the single-file VM and its headers.
set(MRUBYZ_ROOT ${CMAKE_CURRENT_LIST_DIR}/../..)

# mruby-compiler2 checkout (tag 3.4.1) with Prism codegen already generated.
set(MRC_ROOT $ENV{HOME}/Repos/mruby-compiler2)

file(GLOB MRC_SRCS   ${MRC_ROOT}/src/*.c)
file(GLOB PRISM_SRCS ${MRC_ROOT}/lib/prism/src/*.c)
file(GLOB PRISM_UTIL ${MRC_ROOT}/lib/prism/src/util/*.c)

# mrc and prism both provide a diagnostic.c -> diagnostic.o. Put them in
# separate OBJECT libraries so CMake never collides the object file names.
add_library(mrc_obj OBJECT ${MRC_SRCS})
target_include_directories(mrc_obj PRIVATE
    ${MRC_ROOT}/include
    ${MRC_ROOT}/lib/prism/include
)
target_compile_definitions(mrc_obj PRIVATE PRISM_XALLOCATOR PRISM_BUILD_MINIMAL)

add_library(prism_obj OBJECT ${PRISM_SRCS} ${PRISM_UTIL})
target_include_directories(prism_obj PRIVATE
    ${MRC_ROOT}/include
    ${MRC_ROOT}/lib/prism/include
)
target_compile_definitions(prism_obj PRIVATE PRISM_XALLOCATOR PRISM_BUILD_MINIMAL)

add_executable(mrubyz_ondevice
    main.c
    ${MRUBYZ_ROOT}/mrubyz.c
    $<TARGET_OBJECTS:mrc_obj>
    $<TARGET_OBJECTS:prism_obj>
)

# Integer-only VM base; compiler+prism include dirs for main.c and mrubyz.c.
target_compile_definitions(mrubyz_ondevice PRIVATE
    MRBZ_MINIMAL
    PRISM_XALLOCATOR
    PRISM_BUILD_MINIMAL
)
target_include_directories(mrubyz_ondevice PRIVATE
    ${MRUBYZ_ROOT}
    ${MRC_ROOT}/include
    ${MRC_ROOT}/lib/prism/include
)

target_link_libraries(mrubyz_ondevice
    pico_stdlib
    pico_cyw43_arch_none
)

# USB serial console, no UART.
pico_enable_stdio_usb(mrubyz_ondevice 1)
pico_enable_stdio_uart(mrubyz_ondevice 0)

pico_add_extra_outputs(mrubyz_ondevice)
