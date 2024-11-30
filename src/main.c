#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/hid.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>

#define USB_EP 0x81
#define LED_PIN GPIO13
#define USB_PIN GPIO12
#define STARTUP_DELAY 10000

static usbd_device *usb_dev;
static bool key_sent;
static uint32_t ticks;
static uint8_t ctrl_buf[128];

static const uint8_t hid_report[] = {
    0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x05, 0x07, 
    0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, 0x25, 0x01,
    0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 
    0x75, 0x08, 0x81, 0x03, 0x95, 0x06, 0x75, 0x08,
    0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, 0x00,
    0x29, 0x65, 0x81, 0x00, 0xc0
};

#include "usb_descriptors.h"

void sys_tick_handler(void) {
    ticks++;
    
    // Simple LED blink at 1Hz
    if (ticks % 1000 < 500) {
        gpio_clear(GPIOC, LED_PIN);
    } else {
        gpio_set(GPIOC, LED_PIN);
    }
    
    if(!key_sent && ticks >= STARTUP_DELAY) {
        uint8_t buf[8] = {0};
		buf[2] = 0x2C;  // Press space
        usbd_ep_write_packet(usb_dev, USB_EP, buf, 8);
        for(volatile int i = 0; i < 100000; i++);
        
        buf[2] = 0;     // Release
        usbd_ep_write_packet(usb_dev, USB_EP, buf, 8);
        key_sent = true;
        systick_counter_disable();
    }
}

static void hid_set_config(usbd_device *d, uint16_t w) {
    (void)w;
    usbd_ep_setup(d, USB_EP, USB_ENDPOINT_ATTR_INTERRUPT, 8, NULL);
    usbd_register_control_callback(d,
        USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        hid_control_request);

    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
    systick_set_reload(5999);  // 1kHz @ 48MHz/8
    systick_interrupt_enable();
    systick_counter_enable();
}

int main(void) {
    rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_HSI_48MHZ]);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, LED_PIN);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, USB_PIN);
    
    gpio_clear(GPIOA, USB_PIN);
    for (int i = 0; i < 800000; i++) __asm__("nop");

    usb_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, &config,
                    usb_strings, 3, ctrl_buf, sizeof(ctrl_buf));
    usbd_register_set_config_callback(usb_dev, hid_set_config);

    while (1) usbd_poll(usb_dev);
    return 0;
}