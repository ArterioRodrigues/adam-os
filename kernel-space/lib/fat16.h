#ifndef FAT16_H
#define FAT16_H
#include "../kernel/types.h"

typedef struct fat16_bpb {
    uint8_t padding[11];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t num_fats;
    uint16_t root_entry_count;
    uint8_t padding2[3];
    uint16_t sectors_per_fat;
    uint8_t padding3[30];
    uint8_t fat_type[8];
} __attribute__((packed)) fat16_bpb_t;

typedef struct fat16_geometry {
    uint32_t fat_start_lba;
    uint32_t root_dir_start_lba;
    uint32_t root_dir_sectors;
    uint32_t data_start_lba;
} fat16_geometry_t;

typedef struct fat16_entry {
    uint8_t name[8];
    uint8_t extension[3];
    uint8_t attributes;
    uint8_t reserved[14];
    uint16_t start_cluster;
    uint32_t file_size;
} __attribute__((packed)) fat16_entry_t;

typedef struct {
    uint8_t *data;
    uint32_t size;
    uint8_t attributes;
} fat16_fd_t;

extern fat16_bpb_t *bpb;
extern fat16_geometry_t *geometry;

void dump_entry(fat16_entry_t *entry);
void init_fat16();
fat16_entry_t *fat16_find_file(char *input);
uint32_t fat16_read_folder(fat16_entry_t *folder_entry, uint8_t *buf); 
uint32_t fat16_read_file(fat16_entry_t *entry, uint8_t *buf);
uint16_t fat16_alloc_cluster();
void fat16_create_dir_entry(char *input, uint16_t cluster, uint32_t size);
bool fat16_write_file(char *name, uint8_t *data, uint32_t size);
#endif
