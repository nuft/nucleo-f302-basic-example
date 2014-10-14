
#ifndef FLASH_WRITER_H_
#define FLASH_WRITER_H_

#include <stdint.h>
#include <stdlib.h>

uint32_t flash_writer_num_pages(void);
bool flash_writer_page_info(uint32_t page, void *addr, size_t *size);
size_t flash_writer_page_size(uint32_t page);
uint32_t flash_writer_page_address(uint32_t page);
int flash_program_half_word(uint32_t address, uint16_t data);
void flash_writer_page_erase(uint32_t page);
int flash_writer_write_word(uint32_t address, uint32_t word);
void flash_writer_page_write(uint32_t page, uint8_t *data, size_t len);
void flash_writer_start(void);
void flash_writer_finish(void);

#endif /* FLASH_WRITER_H_ */
