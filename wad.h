#ifndef _wad_h
#define _wad_h

#define NAME_SIZE 8

typedef struct wad_entry {
    int file_pos;
    int size;
    char name[NAME_SIZE];
} wad_entry_t;

typedef struct wad {
    char * file_name;
    int num_entries;
    wad_entry_t * entries;
} wad_t;

void wad_load(wad_t * wad, const char * path);

void wad_free(wad_t * wad);

wad_entry_t * wad_find(wad_t * wad, const char * name);

unsigned char * wad_read(wad_t * wad, const char * name, int * size);

#endif