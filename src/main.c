
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdint.h>
#include <stdio.h>

void uart2_init(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);
    // uart tx pin
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
    // uart rx pin
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO3);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_OD, GPIO_OSPEED_25MHZ, GPIO3);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO3);

    rcc_periph_clock_enable(RCC_USART2);
    usart_set_baudrate(USART2, 19200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX_RX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_enable(USART2);
}

void uart_putc(char c)
{
    usart_send_blocking(USART2, c);
}

char uart_getc(void)
{
    return usart_recv_blocking(USART2);
}

int uart_write(const char *p, int len)
{
    int i;
    for(i = 0; i < len; i++) {
        uart_putc(p[i]);
    }
    return len;
}

int uart_read(char *p, int len)
{
    int i;
    for(i = 0; i < len; i++) {
        p[i] = uart_getc();
    }
    return len;
}

void delay(unsigned int n)
{
    while (n-- > 0) {
        __asm__ volatile ("nop":::);
    }
}

int main(void)
{
    rcc_clock_setup_hsi(&hsi_8mhz[CLOCK_64MHZ]);

    uart2_init();

    // User LED
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    // User button
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO13);

    while (1) {
        // toggle user-LED
        gpio_toggle(GPIOB, GPIO13);

        printf("Sieg!\n");

        // wait for user-button to be pressed
        while (gpio_get(GPIOC, GPIO13) != 0);
        delay(1000);
        while (gpio_get(GPIOC, GPIO13) == 0);
    }
}
