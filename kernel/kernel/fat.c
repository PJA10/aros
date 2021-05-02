#include <driver/ata.h>

#include <kernel/heap.h>
#include <kernel/fat.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

static fat_BS_t *fat_boot;
static fat_extBS_16_t *fat_boot_ext_16;
static fat_extBS_32_t *fat_boot_ext_32;
static int total_sectors;
static int fat_size;
static int root_dir_sectors;
static int first_data_sector;
static int first_fat_sector;
static int data_sectors;
static int total_clusters;
static int fat_type;

static int drive_num = 1;

void fat_init() {
    ata_init(drive_num);
    uint16_t buff[256];
    ata_read(0, drive_num, buff, 1);
    fat_boot = kmalloc(sizeof(fat_BS_t));
    memcpy(fat_boot, buff, sizeof(fat_BS_t));
    fat_boot_ext_16 = (fat_extBS_16_t *) &(fat_boot->extended_section);
    fat_boot_ext_32 = (fat_extBS_32_t *) &(fat_boot->extended_section);

    total_sectors = (fat_boot->total_sectors_16 == 0)? fat_boot->total_sectors_32 : fat_boot->total_sectors_16;
    fat_size = (fat_boot->table_size_16 == 0)? fat_boot_ext_32->table_size_32 : fat_boot->table_size_16;
    root_dir_sectors = ((fat_boot->root_entry_count * 32) + (fat_boot->bytes_per_sector - 1)) / fat_boot->bytes_per_sector;
    first_data_sector = fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors;
    first_fat_sector = fat_boot->reserved_sector_count;
    data_sectors = total_sectors - (fat_boot->reserved_sector_count + (fat_boot->table_count * fat_size) + root_dir_sectors);
    total_clusters = data_sectors / fat_boot->sectors_per_cluster;

    printf("fat_init: \ntotal_sectors: %d\nfat_size: %d\nsectors_per_cluster: %d\nroot_dir_sectors: %d\nfirst_data_sector: %d\nfirst_fat_sector: %d\ndata_sectors: %d\ntotal_clusters: %d\n",
    total_sectors, fat_size, fat_boot->sectors_per_cluster, root_dir_sectors ,first_data_sector ,first_fat_sector, data_sectors, total_clusters);

    if(total_clusters < 4085)  {
        fat_type = FAT12;
        printf("fat_type: FAT12- unsupported!!!!!!!!\n");
    } 
    else if(total_clusters < 65525)  {
        fat_type = FAT16;
        printf("fat_type: FAT16\n");
    } 
    else if (total_clusters < 268435445) {
        fat_type = FAT32;
        printf("fat_type: FAT32 - unsupported!!!!!!!!\n");
    }
    else { 
        fat_type = ExFAT;
        printf("fat_type: ExFAT - unsupported!!!!!!!!\n");
    }

    printf("table_count: %d\n", fat_boot->table_count);
    printf("fat_boot->bytes_per_sector: %d\n", fat_boot->bytes_per_sector);

    for (int i = 0; i < 20; i++) {        
        printf("fat16_get_next_cluster(%d): %d     ", i, fat16_get_next_cluster(i));
        if (i % 2 == 0) {
            printf("\n");
        }
    }
}

int fat16_get_next_cluster(int active_cluster) {
    unsigned char FAT_table[fat_boot->bytes_per_sector];
    unsigned int fat_offset = active_cluster * 2;
    unsigned int fat_sector = first_fat_sector + (fat_offset / fat_boot->bytes_per_sector);
    unsigned int ent_offset = fat_offset % fat_boot->bytes_per_sector;
    
    //read from sector "fat_sector" on the disk into "FAT_table".
    ata_read(fat_sector, drive_num, &FAT_table, 1);
    
    unsigned short table_value = *(unsigned short*)&FAT_table[ent_offset];
    //the variable "table_value" now has the information you need about the next cluster in the chain.

    if (table_value >= 0xFFF8) {
        // there are no more clusters in the chain
        return -1;
    } else if (table_value == 0xFFF7) {
        // this clister is "bad"
        printf("fat16_get_next_cluster: bad cluster!\n");
        return -1;
    }
    return (int) table_value;
}