
#include <stdlib.h>
#include <libopencm3/stm32/flash.h>
#include "flash_writer.h"

/* Flash memory layout, adjust for specific MCU */
/* STM32F302R8 */
#define STM32F3_FLASH_PAGE_SIZE 0x800 /* 2048 bytes */
#define STM32F3_FLASH_LAST_PAGE 31
#define FLASH_BASE


/** get number of available pages */
uint32_t flash_writer_num_pages(void)
{
    return STM32F3_FLASH_LAST_PAGE + 1;
}

/** get info about start address and size of page */
bool flash_writer_page_info(uint32_t page, void *addr, size_t *size)
{
    return true;
}

/** get flash page size, returns 0 if page does not exist */
size_t flash_writer_page_size(uint32_t page)
{
    if (page > STM32F3_FLASH_LAST_PAGE) {
        return 0;
    }
    return STM32F3_FLASH_PAGE_SIZE;
}

uint32_t flash_writer_page_address(uint32_t page)
{
    return 0x08000000 + page * STM32F3_FLASH_PAGE_SIZE;
}


/* implement missing libopencm3 flash function */
static inline void flash_erase_page(uint32_t page_address)
{
    flash_wait_for_last_operation();

    FLASH_CR |= FLASH_CR_PER;
    FLASH_AR = page_address;
    FLASH_CR |= FLASH_CR_STRT;

    flash_wait_for_last_operation();

    FLASH_CR &= ~FLASH_CR_PER;
}

/* implement missing libopencm3 flash function */
/* returns 0 if succeeded */
int flash_program_half_word(uint32_t address, uint16_t data)
{
    flash_wait_for_last_operation();

    FLASH_CR |= FLASH_CR_PG;

    /* perform half-word writ */
    *(uint16_t *)address = data;

    flash_wait_for_last_operation();

    FLASH_CR &= ~FLASH_CR_PG;

    int ret = 1;
    if (FLASH_SR & FLASH_SR_EOP) {
        /* programming succeded */
        FLASH_SR |= FLASH_SR_EOP; /* reset flag by writing 1 */
        ret = 0;
    }
    return ret;
}

void flash_writer_page_erase(uint32_t page)
{
    uint32_t addr;

    if (page > STM32F3_FLASH_LAST_PAGE) {
        return;
    }

    // check if page is already erased? (all set to 0xffffffff)

    addr = flash_writer_page_address(page);

    flash_erase_page(addr);
}

int flash_writer_write_word(uint32_t address, uint32_t word)
{
    int ret;

    ret = flash_program_half_word(address, (uint16_t) (word & 0xffff));
    if (ret != 0) {
        return ret;
    }

    ret = flash_program_half_word(address + 2, (uint16_t) (word >> 16));

    return ret;
}

void flash_writer_page_write(uint32_t page, uint8_t *data, size_t len)
{
    if (page > STM32F3_FLASH_LAST_PAGE) {
        return;
    }

    // uint16_t *flash = (uint16_t *)flash_writer_page_address(page);

    // todo
}

void flash_writer_start(void)
{
    flash_unlock();
}

void flash_writer_finish(void)
{
    flash_lock();
}
