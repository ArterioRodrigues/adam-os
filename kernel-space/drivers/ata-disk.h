#ifndef DISK_H
#define DISK_H
#include "../kernel/types.h" 

void ata_await();
bool ata_read_sector(uint32_t lba, uint8_t *buf);
bool ata_write_sector(uint32_t lba, uint8_t *buf);

#endif
