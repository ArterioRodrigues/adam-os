#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H
#include "config.h"
#include "types.h"

typedef uint32_t page_entry_t;

typedef struct {
    page_entry_t entries[1024];
} page_table_t;

typedef struct {
    page_entry_t entries[1024];
} page_directory_t;


extern page_directory_t* page_directory;

page_directory_t* create_page_directory(); 
page_table_t* create_page_table();

void map_page(page_directory_t* page_directory, uint32_t virtual_address, uint32_t physical_address, uint32_t flag);

void load_page_directory(page_directory_t* pd);
void enable_paging();
#endif
