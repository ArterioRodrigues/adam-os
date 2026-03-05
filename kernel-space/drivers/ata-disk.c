#include "../pch.h"

void ata_await() {
    uint8_t status = inb(ATA_STATUS_PORT);
    while (status & ATA_STATUS_BSY)
        status = inb(ATA_STATUS_PORT);
}

static void ata_select(uint32_t lba) {
    outb(ATA_SECTOR_COUNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_DRIVE_PORT, ATA_DRIVE_MASTER | ((lba >> 24) & 0x0F));
}

bool ata_read_sector(uint32_t lba, uint8_t *buf) {
    ata_await();
    ata_select(lba);
    outb(ATA_COMMAND_PORT, ATA_CMD_READ);

    uint8_t status = inb(ATA_STATUS_PORT);
    while (!(status & ATA_STATUS_DRQ))
        status = inb(ATA_STATUS_PORT);

    uint16_t *ptr = (uint16_t *)buf;
    for (int i = 0; i < 256; i++) {
        ptr[i] = inw(ATA_DATA_PORT);
    }

    return true;
}

bool ata_write_sector(uint32_t lba, uint8_t *buf) {
    ata_await();
    ata_select(lba);
    outb(ATA_COMMAND_PORT, ATA_CMD_WRITE);

    uint8_t status = inb(ATA_STATUS_PORT);
    while (!(status & ATA_STATUS_DRQ))
        status = inb(ATA_STATUS_PORT);

    uint16_t *ptr = (uint16_t *)buf;
    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA_PORT, ptr[i]);
    }

    outb(ATA_COMMAND_PORT, ATA_CMD_FLUSH);
    ata_await();

    return true;
}
