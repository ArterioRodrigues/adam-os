#include "../pch.h"

fat16_bpb_t *bpb;
fat16_geometry_t *geometry;

void dump_geometry() {
    char buffer[100];
    print("FAT START: ");
    print(itos(buffer, geometry->fat_start_lba));
    print("\nROOT DIR START: ");
    print(itos(buffer, geometry->root_dir_start_lba));
    print("\nROOT DIR SECTORS: ");
    print(itos(buffer, geometry->root_dir_sectors));
    print("\nDATA START: ");
    print(itos(buffer, geometry->data_start_lba));
    print("\nSectors per cluster: ");
    print(itos(buffer, bpb->sectors_per_cluster));
}

void init_fat16_bpb() {
    uint8_t *sector = kmalloc(SECTOR_SIZE);
    ata_read_sector(FAT16_START_LBA, sector);
    bpb = (fat16_bpb_t *)sector;

    if (!strncmp((char *)bpb->fat_type, "FAT16   ", 8)) {
        print("ERROR: FAT16 Not found halting ");
        while (1)
            ;
    }
}

void init_fat16_geometry() {
    geometry = kmalloc(sizeof(fat16_geometry_t));
    geometry->fat_start_lba = FAT16_START_LBA + bpb->reserved_sectors;
    geometry->root_dir_start_lba = geometry->fat_start_lba + (bpb->num_fats * bpb->sectors_per_fat);
    geometry->root_dir_sectors = ceil(bpb->root_entry_count * 32, bpb->bytes_per_sector);
    geometry->data_start_lba = geometry->root_dir_start_lba + geometry->root_dir_sectors;
}

void fat16_format_name(char *input, char *name_out, char *ext_out) {
    char name[8];
    strtok(name, input, '.');
    char ext[3];
    strtok(ext, NULL, '.');

    for (int i = 0; i < 8; i++) {
        if (name[i])
            name_out[i] = name[i];
        else
            name_out[i] = ' ';

        if (name[i] >= 'a' && name[i] <= 'z')
            name_out[i] -= 32;
    }

    for (int i = 0; i < 3; i++) {
        if (ext[i])
            ext_out[i] = ext[i];
        else
            ext_out[i] = ' ';

        if (ext[i] >= 'a' && ext[i] <= 'z')
            ext_out[i] -= 32;
    }
}

fat16_entry_t *fat16_find_file(char *input) {
    uint8_t *sector = kmalloc(SECTOR_SIZE);
    fat16_entry_t *entry = kmalloc(sizeof(fat16_entry_t));
    fat16_entry_t *buf;
    char name[8];
    char ext[3];
    fat16_format_name(input, name, ext);

    for (int i = 0; i < geometry->root_dir_sectors; i++) {
        ata_read_sector(geometry->root_dir_start_lba + i, sector);
        for (int j = 0; j < SECTOR_SIZE; j += sizeof(fat16_entry_t)) {
            buf = (fat16_entry_t *)(sector + j);

            if (buf->name[0] == 0x00) {
                kfree(sector);
                kfree(entry);
                return NULL;
            }
            if (buf->name[0] == 0xE5)
                continue;

            if (strncmp(name, (char *)buf->name, 8) && strncmp(ext, (char *)buf->extension, 3)) {
                kfree(sector);
                memcpy(entry, buf, sizeof(fat16_entry_t));
                return entry;
            }
        }
    }

    kfree(sector);
    return NULL;
}

uint32_t fat16_read_file(fat16_entry_t *entry, uint8_t *buf) {
    uint32_t cluster_lba = geometry->data_start_lba + (entry->start_cluster - 2) * bpb->sectors_per_cluster;
    uint32_t fat_sector = geometry->fat_start_lba + (entry->start_cluster / 256);
    uint32_t fat_offset = (entry->start_cluster % 256) * 2;
    uint32_t size = 0;

    for (int i = 0; i < bpb->sectors_per_cluster; i++) {
        ata_read_sector(cluster_lba + i, buf + size);
        size += SECTOR_SIZE;
    }

    uint8_t *fat_buffer = kmalloc(bpb->sectors_per_cluster * SECTOR_SIZE);
    ata_read_sector(fat_sector, fat_buffer);
    uint16_t next_cluster = *(uint16_t *)(fat_buffer + fat_offset);
    while (next_cluster < 0xFFF8) {
        cluster_lba = geometry->data_start_lba + (next_cluster - 2) * bpb->sectors_per_cluster;
        fat_sector = geometry->fat_start_lba + (next_cluster / 256);
        fat_offset = (next_cluster % 256) * 2;

        ata_read_sector(fat_sector, fat_buffer);
        for (int i = 0; i < bpb->sectors_per_cluster; i++) {
            ata_read_sector(cluster_lba + i, buf + size);
            size += SECTOR_SIZE;
        }

        next_cluster = *(uint16_t *)(fat_buffer + fat_offset);
    }

    kfree(fat_buffer);
    return size;
}

void init_fat16() {
    init_fat16_bpb();
    init_fat16_geometry();
}
