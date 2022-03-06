#include "led.h"
#include "util.h"
#include <esp_event_loop.h>


#ifdef CONFIG_LED_NEOPIXEL
#include <NeoPixelBus.h>

const int brightness = 0xff;
NeoPixelBus<NeoGrbwFeature, Neo800KbpsMethod> led(1, 26);
volatile static uint32_t led_state_bits = 0;

void set_led(int r, int g, int b, int w = 0) {
    led.ClearTo(RgbwColor(r, g, b, w));
    led.Show();
}

void set_led_state(LedState state, bool toggle) {
    if (toggle) {
        led_state_bits |= 1<<state;
    } else {
        led_state_bits &= ~0L ^ (1<<state);
    }
}

void init_led_task() {
    led.Begin();
    set_led(0, 0, 0, 0);

    TaskHandle_t h = NULL;
    xTaskCreate([](void*) {
        while (true) {
            if (led_state_bits & 1<<WiFiConnecting) {
                set_led(0, 0, abs(sin(.001 * millis())) * brightness);
            } else if (led_state_bits & 1<<WiFiPortal) {
                set_led(
                    abs(sin(.001 * millis()) * 2) * brightness,
                    abs(cos(.001 * millis())) * brightness,
                    0
                );
            } else if (led_state_bits & 1<<Warning) {
                set_led(abs(sin(.001 * millis())) * brightness, 0, 0);
            } else if (led_state_bits & 1<<Alarm) {
                set_led(abs(sin(.005 * millis())) * brightness, 0, 0);
            } else {
                set_led(0, 0, 0);
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
    }, "led", 2048, NULL, tskIDLE_PRIORITY, &h);
    assert(h);
}

#else //fallback build in led
#define LED_BUILDIN GPIO_NUM_2

void set_led_state(LedState state, bool toggle) {
    if(state == WiFiConnecting){
        // turn on
        gpio_set_level(LED_BUILDIN, 1);
    } else {
        // turn off
        gpio_set_level(LED_BUILDIN, 0);
    }
}

void init_led_task() {
        // Configure pin
    gpio_config_t io_conf;
    io_conf.intr_type = (gpio_int_type_t)GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << LED_BUILDIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_set_level(LED_BUILDIN, 0);
}

#endif