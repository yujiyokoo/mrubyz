#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Hello-world for Pico 2 W: blink the onboard LED and print over USB serial.
// The onboard LED on the W is driven through the CYW43 wireless chip, so
// cyw43_arch must be initialised even though no WiFi is used.
int main(void) {
    stdio_init_all();

    if (cyw43_arch_init()) {
        return -1;
    }

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        printf("Hello from Pico 2 W -- mrubyz port\n");
        sleep_ms(500);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(500);
    }
}
