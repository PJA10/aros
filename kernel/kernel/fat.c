#include <kernel/fat.h>
#include <kernel/heap.h>

#include <driver/ata.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* local functions */
static int fat16_get_next_cluster(int active_cluster);
static int fat16_set_next_cluster(int active_cluster);
static fat_file_t *fat_parse_dir(fat_file_t *dir, char *search_name);
static void fat_read_cluster(int cluster_num, unsigned char *buf);
static void fat_write_cluster(int cluster_num, unsigned char *buf);
static void fat_parse_name(unsigned char *entry, char *file_name);
static void free_fat_file(fat_file_t *f);


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
static int first_root_dir_cluster;

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
    int first_root_dir_sector = first_data_sector - root_dir_sectors;
    first_root_dir_cluster = (first_root_dir_sector - first_data_sector)/fat_boot->sectors_per_cluster + 2;
    
    // printf("fat_init: \ntotal_sectors: %d\nfat_size: %d\nsectors_per_cluster: %d\nroot_dir_sectors: %d\nfirst_data_sector: %d\nfirst_fat_sector: %d\ndata_sectors: %d\ntotal_clusters: %d\n",
    //  total_sectors, fat_size, fat_boot->sectors_per_cluster, root_dir_sectors ,first_data_sector ,first_fat_sector, data_sectors, total_clusters);

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
    printf("first_root_dir_sector: %d\n", first_root_dir_sector);
    printf("first_data_sector: %d\n", first_data_sector);
    printf("root_dir_sectors: %d\n", root_dir_sectors);
    printf("first_root_dir_cluster: %d\n", first_root_dir_cluster);
    for (size_t i = 0; i < 40; i++)
    {
        printf("fat16_get_next_cluster(%d) = %d - %d      ", i, fat16_get_next_cluster(i), ((i - 2) * fat_boot->sectors_per_cluster) + first_data_sector);
        if (i % 2 == 0) {
            printf("\n");
        }
    }
    
}

int fat_read(fat_file_t *file, char *buf, int size) {
    int left_to_read = size;
    while (left_to_read) {
        int cluster_num = file->curr_cluster_num;
        int next_cluster = fat16_get_next_cluster(cluster_num);
        //printf("file: %s at curr_cluster_num: %d, pos_in_cluster: %d left_to_read: %d\n", file->name, file->curr_cluster_num, file->pos_in_cluster, left_to_read);
        unsigned char cluster[fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster];
        fat_read_cluster(cluster_num, cluster);
        int min = fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster - file->pos_in_cluster;
        if (left_to_read < fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster - file->pos_in_cluster) {
            min = left_to_read;
        }
        if (next_cluster == -1 && file->size % (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster) - file->pos_in_cluster < min) {
            min = file->size % (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster) - file->pos_in_cluster;
        }
        memcpy(buf, cluster + file->pos_in_cluster, min);
        left_to_read -= min;
        buf += min;
        //printf("file->pos_in_cluster: %d, min: %d\n", file->pos_in_cluster, min);
        file->pos_in_cluster = (file->pos_in_cluster + min) % (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster);

        if (next_cluster == -1 && file->pos_in_cluster == file->size % (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster)) {
            file->eof = 1;
            break;
        }
        if (file->pos_in_cluster == 0) {
            //printf("(file->pos_in_cluster == 0)\n");
            file->curr_cluster_num = next_cluster;
        }
    }
    return size - left_to_read;
}

int fat_write(fat_file_t *file, char *buf, int size) {
    int left_to_write = size;
    while (left_to_write) {
        int cluster_num = file->curr_cluster_num;
        //printf("file: %s at curr_cluster_num: %d, pos_in_cluster: %d left_to_write: %d\n", file->name, file->curr_cluster_num, file->pos_in_cluster, left_to_write);
        if (file->pos_in_cluster == 0 && left_to_write >= fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster) {
            fat_write_cluster(cluster_num, (unsigned char *)buf);
            buf += fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster;
            left_to_write -= fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster;
        } else {
            unsigned char cluster[fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster];
            fat_read_cluster(cluster_num, cluster);
            int min = fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster - file->pos_in_cluster;
            if (left_to_write < fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster - file->pos_in_cluster) {
                min = left_to_write;
            }
            memcpy(cluster + file->pos_in_cluster, buf, min);
            //printf("writing: %s\n", cluster);
            fat_write_cluster(cluster_num, cluster);
            //printf("here\n");

            left_to_write -= min;
            buf += min;
            file->pos_in_cluster = (file->pos_in_cluster + min) % (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster);
        }

        int next_cluster = fat16_get_next_cluster(cluster_num);
        //printf("next_cluster: %d, file->pos_in_cluster: %d\n", next_cluster, file->pos_in_cluster);
        if (next_cluster == -1) { // last allocted claster
            // then we changed the size of the file
            int was_written = file->size % (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster);
            int curr_written = file->pos_in_cluster ? file->pos_in_cluster : (fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster);
            file->size += curr_written - was_written;
        }
        if (file->pos_in_cluster == 0) {
            if (next_cluster == -1) {
                next_cluster = fat16_set_next_cluster(file->curr_cluster_num); // get another cluster
            }
            file->curr_cluster_num = next_cluster;
        }
    }
    return size - left_to_write;
}


fat_file_t *fat_open(char *path) {
    char *curr_file_name;
    char *p = path;
    int finished = 0;
    if (path[0] != '/') {
        printf("fat_open: bad path name\n");
        return NULL;
    }
    fat_file_t *curr_dir = kmalloc(sizeof(fat_file_t));
    curr_dir->path = kmalloc(2);
    strcpy(curr_dir->path, "/");
    curr_dir->name = kmalloc(1);
    strcpy(curr_dir->name, "");
    curr_dir->first_cluster_num = first_root_dir_cluster;
    curr_dir->curr_cluster_num = first_root_dir_cluster;
    curr_dir->is_folder = 1;

    while (!finished) {
        p++;
        curr_file_name = p;
        while (*p != '/') {
            if (*p == 0) {
                finished = 1;
                break;;
            }
            p++;
        }
        *p = 0;
        // debug
        //print_fat_file_metadata(curr_dir);
        //printf("fat_open: curr_file_name=%s\n\n", curr_file_name);
        fat_file_t *next_dir = fat_parse_dir(curr_dir, curr_file_name);
        free_fat_file(curr_dir);
        if (next_dir == NULL) {
            return NULL;
        }
        curr_dir = next_dir;
    }
    curr_dir->pos_in_cluster = 0;
    curr_dir->eof= 0;
    return(curr_dir);
}

void print_fat_file_metadata(fat_file_t *file) {
    printf("file: %s at %s - size: %d, first_cluster_num: %d, is_folder: %d\ncurr_cluster_num: %d, pos_in_cluster: %d\n\n",
    file->name, file->path, file->size, file->first_cluster_num, file->is_folder, file->curr_cluster_num, file->pos_in_cluster);
}

void seek_start(fat_file_t *file) {
	file->curr_cluster_num = file->first_cluster_num;
	file->pos_in_cluster = 0;
	file->eof = 0;
}

static fat_file_t *fat_parse_dir(fat_file_t *dir, char *search_name) {
    unsigned char cluster[fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster];
    fat_file_t *file = NULL;
    int done = 0;
    while (!done && !file) {
        fat_read_cluster(dir->curr_cluster_num, cluster);
        unsigned char *entry = cluster;
        while (!file) {
            if (entry >= cluster + fat_boot->bytes_per_sector * fat_boot->sectors_per_cluster || entry[0] == 0) {
                break;
            }
            if (entry[0] == 0xe5) {
                goto endloop;
            }
            if (entry[11] == 0x0f) {
                // long name entry
                // here we need to read the portion of the long filename into a temporary buffer
                // unsupported right now 
                goto endloop;
            }

            char file_name[11] = {'\0'};
            fat_parse_name(entry, file_name);
            if (strcmp(file_name, search_name) == 0) {
                int file_first_cluster_number = (entry[27] << 8) | entry[26];
                int file_size = (entry[31] << 24) | (entry[30] << 16) | (entry[29] << 8) | entry[28];
                file = kmalloc(sizeof(fat_file_t));
                file->name = kmalloc(strlen(file_name) + 1);
                strcpy(file->name, file_name);
                file->path = kmalloc(strlen(file_name) + strlen(dir->path) + 2);
                strcpy(file->path, dir->path);
                file->path[strlen(dir->path)] = '/';
                strcpy(file->path+strlen(dir->path)+1, file->name);
                if (strcmp(dir->path, "/") == 0) {
                    strcpy(file->path+strlen(dir->path), file->name);
                    file->path[strlen(dir->path) + strlen(file->name) + 1] = '\0';
                }
                file->size = file_size;
                file->first_cluster_num = file_first_cluster_number;
                file->curr_cluster_num = file->first_cluster_num;
                file->is_folder = (entry[11] == 0x10);
            }
            //printf("file name: %s     first cluster number: %d    file_size: %d\n", file_name, file_first_cluster_number, file_size);
endloop:
            entry += 32;
            continue;
        }
        if (strcmp(dir->path,"/") == 0) { // root dir
            dir->curr_cluster_num = dir->curr_cluster_num + 1;
            if (dir->curr_cluster_num == dir->first_cluster_num + (root_dir_sectors/fat_boot->sectors_per_cluster)) {
                done = 1;
            }
        }
        else {
            dir->curr_cluster_num = fat16_get_next_cluster(dir->curr_cluster_num);
            if (dir->curr_cluster_num == -1) {
                done = 1;
            }
        }
    }
    dir->curr_cluster_num = dir->first_cluster_num;
    return file;
}

static void fat_parse_name(unsigned char *entry, char *file_name) {
    int j = 0;
        for (int i = 0; i < 11; i++) {
            if (entry[i] != 0x20) {
                if (i == 8) {
                    file_name[j] = '.';
                    j++;
                }
                file_name[j] = entry[i];
                j++;
            }
        }
}

static int fat16_get_next_cluster(int active_cluster) {
    unsigned char FAT_table[fat_boot->bytes_per_sector];
    unsigned int fat_offset = active_cluster * 2;
    unsigned int fat_sector = first_fat_sector + (fat_offset / fat_boot->bytes_per_sector);
    unsigned int ent_offset = fat_offset % fat_boot->bytes_per_sector;
    
    //read from sector "fat_sector" on the disk into "FAT_table".
    ata_read(fat_sector, drive_num, FAT_table, 1);
    unsigned short table_value = *(unsigned short*)&FAT_table[ent_offset];

    //the variable "table_value" now has the information you need about the next cluster in the chain.
    if (table_value >= 0xFFF8) {
        // there are no more clusters in the chain
        return -1;
    } else if (table_value == 0xFFF7) {
        // this clister is "bad"
        printf("fat16_get_next_cluster: bad cluster!\n");
        return -2;
    }
    return (int) table_value;
}

static int fat16_set_next_cluster(int active_cluster) {
    unsigned char FAT_table[fat_boot->bytes_per_sector];
    unsigned int fat_offset = active_cluster * 2;
    unsigned int fat_sector = first_fat_sector + (fat_offset / fat_boot->bytes_per_sector);
    unsigned int ent_offset = fat_offset % fat_boot->bytes_per_sector;
    unsigned int next_cluster = 0;

    //read from sector "fat_sector" on the disk into "FAT_table".
    for (int curr_fat_sector = first_fat_sector; curr_fat_sector < first_fat_sector + fat_size; curr_fat_sector++)
    {
        ata_read(curr_fat_sector, drive_num, FAT_table, 1);
        for (int j = 0; j < fat_boot->bytes_per_sector / 2; j++)
        {
            int curr_cluster = (curr_fat_sector - first_fat_sector) * (fat_boot->bytes_per_sector/2) + j;
            int curr_sector = ((curr_cluster - 2) * fat_boot->sectors_per_cluster) + first_data_sector;
            if (curr_sector > first_data_sector) {
                unsigned short curr_table_value = *(unsigned short*)&FAT_table[j * 2];
                if (curr_table_value == 0)
                {
                    next_cluster = curr_cluster;
                    *(unsigned short*)&FAT_table[j * 2] = -1;
                    ata_write(curr_fat_sector, drive_num, FAT_table, 1);
                    break;
                }
            }
        }
        if (next_cluster)
        {
            break;
        }
    }
    if (!next_cluster) {
        printf("error in fat16_set_next_cluster, disk is full?");
        return -1;
    }
    ata_read(fat_sector, drive_num, FAT_table, 1);
    *(unsigned short*)&FAT_table[ent_offset] = next_cluster;
    ata_write(fat_sector, drive_num, FAT_table, 1);
    return next_cluster;
}

static void fat_read_cluster(int cluster_num, unsigned char *buf) {
    int first_sector_of_cluster = ((cluster_num - 2) * fat_boot->sectors_per_cluster) + first_data_sector;
    ata_read(first_sector_of_cluster, drive_num, buf, fat_boot->sectors_per_cluster);
}

static void fat_write_cluster(int cluster_num, unsigned char *buf) {
    int first_sector_of_cluster = ((cluster_num - 2) * fat_boot->sectors_per_cluster) + first_data_sector;
    ata_write(first_sector_of_cluster, drive_num, buf, fat_boot->sectors_per_cluster);
}

static void free_fat_file(fat_file_t *f) {
    kfree(f->path);
    kfree(f->name);
    kfree(f);
}

void fat_close(fat_file_t *file) {
    free_fat_file(file);
}