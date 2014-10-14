
#include <libopencm3/stm32/flash.h>
#include "flash_writer.h"

/* Flash memory layout, adjust for specific MCU */
/* STM32F302R8 */
#define STM32F3_FLASH_PAGE_SIZE 0x800 /* 2048 bytes */
#define STM32F3_FLASH_LAST_PAGE 31
#define FLASH_BASE

void flash_writer_unlock(void)
{
    flash_unlock();
}

void flash_writer_lock(void)
{
    flash_lock();
}

uint16_t flash_writer_num_pages(void)
{
    return STM32F3_FLASH_LAST_PAGE + 1;
}

bool flash_writer_page_info(uint16_t page, void **addr, size_t *size)
{
    if (page > STM32F3_FLASH_LAST_PAGE) {
        return false;
    }
    *addr = (void *)(0x08000000 + page * STM32F3_FLASH_PAGE_SIZE);
    *size = STM32F3_FLASH_PAGE_SIZE;
    return true;
}

size_t flash_writer_page_size(uint16_t page)
{
    if (page > STM32F3_FLASH_LAST_PAGE) {
        return 0;
    }
    return STM32F3_FLASH_PAGE_SIZE;
}

void *flash_writer_page_address(uint16_t page)
{
    return (void *)(0x08000000 + page * STM32F3_FLASH_PAGE_SIZE);
}


/* implement missing libopencm3 flash erase function */
static void flash_erase_page(void *page_address)
{
    flash_wait_for_last_operation();

    FLASH_CR |= FLASH_CR_PER;
    FLASH_AR = (uint32_t) page_address;
    FLASH_CR |= FLASH_CR_STRT;

    flash_wait_for_last_operation();

    FLASH_CR &= ~FLASH_CR_PER;
}

void flash_writer_page_erase(uint16_t page)
{
    void *addr;

    if (page > STM32F3_FLASH_LAST_PAGE) {
        return;
    }


    addr = flash_writer_page_address(page);

    bool erased = true;

    size_t size = flash_writer_page_size(page);

    uint32_t *p = (uint32_t *) addr;
    while (p < p + size) {
        if (*p != 0xffffffff) {
            erased = false;
            break;
        }
        p++;
    }

    /* only erase page, if necessary as a security mechanism */
    if (!erased) {
        flash_erase_page(addr);
    }
}

/* implement missing libopencm3 flash program function */
/* returns 0 if succeeded */
static int flash_program_half_word(void *address, uint16_t data)
{
    flash_wait_for_last_operation();

    FLASH_CR |= FLASH_CR_PG;

    /* perform half-word write */
    *(uint16_t *)address = data;

    flash_wait_for_last_operation();

    FLASH_CR &= ~FLASH_CR_PG;

    int ret = 1;
    if (FLASH_SR & FLASH_SR_EOP && (FLASH_SR & FLASH_SR_PGPERR) == 0) {
        /* programming succeded */
        FLASH_SR |= FLASH_SR_EOP; /* reset flag by writing 1 */
        ret = 0;
    }
    return ret;
}

int flash_writer_word_write(void *address, uint32_t word)
{
    int ret;

    ret = flash_program_half_word(address, (uint16_t) (word & 0xffff));
    if (ret != 0) {
        return ret;
    }

    address = (void *)((uint16_t *)address + 1);

    ret = flash_program_half_word(address, (uint16_t) (word >> 16));

    return ret;
}

void flash_writer_page_write(uint16_t page, uint8_t *data)
{
    if (page > STM32F3_FLASH_LAST_PAGE) {
        return;
    }

    uint16_t *wdata = (uint16_t *) data;
    uint16_t *flash = (uint16_t *) flash_writer_page_address(page);

    size_t size = flash_writer_page_size(page);

    uint16_t *page_end = (uint16_t *)((uint8_t *)flash + size);

    flash_wait_for_last_operation();

    while (flash < page_end) {
        FLASH_CR |= FLASH_CR_PG;

        /* perform half-word write */
        *flash++ = *wdata++;

        flash_wait_for_last_operation();
    }

    /* reset flags */
    FLASH_CR &= ~FLASH_CR_PG;
    FLASH_SR |= FLASH_SR_EOP;
}
