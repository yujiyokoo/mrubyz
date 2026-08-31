// T13 end-to-end PoC: receive Ruby SOURCE over USB serial, compile it ON the
// RP2350 with mruby-compiler2 (Prism) to RITE0300 bytecode, run through mrubyz,
// print the result. Type Ruby on the host, it compiles + runs on the chip.
//
// Protocol (T9 framing, payload is now source text): host waits for READY,
// then sends a 4-byte little-endian length + that many source bytes. Device
// replies "returned: <int>" (or an "error: ..." line) and loops.
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include "pico/cyw43_arch.h"

#include "mrubyz.h"
#include "mrc_ccontext.h"
#include "mrc_compile.h"
#include "mrc_irep.h"
#include "mrc_dump.h"

#define SRC_MAX 4096

// The read_bytecode builtin in mrubyz.c references this. Unused here.
uint8_t bytecode_at(uint8_t i) {
    (void)i;
    return 0;
}

static void led(bool on) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

static int read_byte(void) {
    for (;;) {
        int c = getchar_timeout_us(1000 * 1000);
        if (c != PICO_ERROR_TIMEOUT) {
            return c & 0xff;
        }
    }
}

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

static uint8_t src[SRC_MAX];

int main(void) {
    stdio_init_all();
    if (cyw43_arch_init()) {
        return -1;
    }

    // Wait for the serial host so READY and replies are never missed.
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    sleep_ms(200);

    printf("READY\n");

    for (;;) {
        uint32_t len = read_u32_le();
        if (len == 0 || len > SRC_MAX) {
            printf("error: bad length %lu (max %d)\n",
                   (unsigned long)len, SRC_MAX);
            continue;
        }
        read_exact(src, len);

        mrc_ccontext *cxt = mrc_ccontext_new(NULL);
        if (cxt == NULL) {
            printf("error: ccontext_new\n");
            continue;
        }

        const uint8_t *s = src;
        mrc_irep *irep = mrc_load_string_cxt(cxt, &s, len);
        if (irep == NULL) {
            printf("error: compile failed\n");
            mrc_ccontext_free(cxt);
            continue;
        }

        uint8_t *bin = NULL;
        size_t bin_size = 0;
        int rc = mrc_dump_irep(cxt, irep, 0, &bin, &bin_size);
        if (rc != MRC_DUMP_OK || bin == NULL) {
            printf("error: dump rc=%d\n", rc);
            mrc_ccontext_free(cxt);
            continue;
        }

        mrbz_vm vm;
        mrbz_val v;
        led(true);
        mrbz_vm_run(&vm, &v, bin);
        led(false);

        printf("returned: %d\n", v.u.intval);

        free(bin);
        mrc_ccontext_free(cxt);
    }
}
