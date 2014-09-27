
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdint.h>
#include <stdio.h>

#include <platform-abstraction/threading.h>
#include <platform-abstraction/semaphore.h>

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

void led_toggle(void)
{
    gpio_toggle(GPIOB, GPIO13);
}


#define FPCCR (*((volatile uint32_t *)0xE000EF34))
#define CPACR (*((volatile uint32_t *)0xE000ED88))

void fpu_config(void)
{
    // Enable the Floating-point coprocessor (CP10 and CP11 to full access)
    CPACR |= (0x03<<(2*10)|(0x03<<(2*11)));

    __asm__ volatile (
        "dsb \n\t"  /* wait for store to complete */
        "isb \n\t"  /* reset pipeline, FPU is now enabled */
        :::);

    uint32_t fpccr = 0;
    // Disable automatic state preservation of FP state
    fpccr &= ~(1<<31);
    // Enable Lazy context save of FP state
    // -> whole fpu context is saved by ucos
    fpccr |= (1<<30);

    FPCCR = fpccr;
}

semaphore_t mysem;

os_thread_t signal_thread;
THREAD_STACK signal_stack[512];

void signal_main(void *context)
{
    (void) context;

    printf("Signal Thread\n");

    while (1) {
        // poll user-button state
        while (gpio_get(GPIOC, GPIO13) != 0) {
            os_thread_sleep_us(10000);
        }
        while (gpio_get(GPIOC, GPIO13) == 0) {
            os_thread_sleep_us(10000);
        }

        // signal mythread to toggle user-LED
        os_semaphore_signal(&mysem);
    }
}

os_thread_t mythread;
THREAD_STACK mystack[1024];

void mythread_main(void *context)
{
    (void) context;

    printf("My Thread\n");

    os_semaphore_init(&mysem, 0);

    printf("Create Signal Thread\n");

    os_thread_create(&signal_thread, signal_main, signal_stack, sizeof(signal_stack), "Signal Thread", 1, NULL);

    while (1) {
        os_semaphore_wait(&mysem);

        // toggle user-LED
        gpio_toggle(GPIOB, GPIO13);

        printf("Sieg!\n");
    }
}

int main(void)
{
    rcc_clock_setup_hsi(&hsi_8mhz[CLOCK_64MHZ]);

    fpu_config();

    uart2_init();

    // User LED
    rcc_periph_clock_enable(RCC_GPIOB);
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    // User button
    rcc_periph_clock_enable(RCC_GPIOC);
    gpio_mode_setup(GPIOC, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO13);

    os_init();

    // printf can only be used after os_run()
    uart_write("Create My Thread\n", 17);

    os_thread_create(&mythread, mythread_main, mystack, sizeof(mystack), "My Thread", 0, NULL);

    os_run();

    while (1);
}
