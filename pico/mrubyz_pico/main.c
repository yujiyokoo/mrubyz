// mrubyz minimal VM on Pico 2 W (RP2350) with serial bytecode upload.
// Protocol (T9): host prints nothing until it sees READY, then sends a 4-byte
// little-endian length followed by that many .mrb bytes. The device runs the
// program and prints "returned: <int>". Repeats forever. See ticket T10.
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "mrubyz.h"

#define BUF_MAX 4096

// The read_bytecode builtin in mrubyz.c references this. Unused here.
uint8_t bytecode_at(uint8_t i) {
    (void)i;
    return 0;
}

static uint8_t prog[BUF_MAX];

// Block until one byte is available on stdin (USB serial).
static int read_byte(void) {
    for (;;) {
        int c = getchar_timeout_us(1000 * 1000);
        if (c != PICO_ERROR_TIMEOUT) {
            return c & 0xff;
        }
    }
}

// Read exactly n bytes into dst.
static void read_exact(uint8_t *dst, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        dst[i] = (uint8_t)read_byte();
    }
}

static uint32_t read_u32_le(void) {
    uint32_t b0 = read_byte();
    uint32_t b1 = read_byte();
    uint32_t b2 = read_byte();
    uint32_t b3 = read_byte();
    return b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
}

static void led(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

int main(void) {
    stdio_init_all();
    if (cyw43_arch_init()) {
        return -1;
    }

    mrbz_vm vm;
    mrbz_val v;

    printf("READY\n");

    for (;;) {
        uint32_t len = read_u32_le();
        if (len == 0 || len > BUF_MAX) {
            // Cannot resync a stream cleanly; report and wait for the next
            // framed program. Bad framing is a host bug during bring-up.
            printf("error: bad length %lu (max %d)\n",
                   (unsigned long)len, BUF_MAX);
            continue;
        }

        read_exact(prog, len);

        led(true);
        mrbz_vm_run(&vm, &v, prog);
        led(false);

        printf("returned: %d\n", v.u.intval);
    }
}
