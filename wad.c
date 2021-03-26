#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wad.h"

void wad_load(wad_t * wad, const char * path) {
    FILE * file;
    int loc;
    int i;

    wad->file_name = malloc(strlen(path) + 1);
    strcpy(wad->file_name, path);
    file = fopen(path, "rb");
    fseek(file, 4, SEEK_SET);
    fread(&wad->num_entries, 4, 1, file);
    fread(&loc, 4, 1, file);
    fseek(file, loc, SEEK_SET);
    wad->entries = malloc(wad->num_entries * sizeof(wad_entry_t));
    for (i = 0; i < wad->num_entries; i++)
        fread(wad->entries + i, sizeof(wad_entry_t), i, file);
    fclose(file);
}

void wad_free(wad_t * wad) {
    free(wad->file_name);
    free(wad->entries);
}

wad_entry_t * wad_find(wad_t * wad, const char * name) {
    int i;

    for (i = 0; i < wad->num_entries; i++)
        if (strncmp(wad->entries[i].name, name, NAME_SIZE) == 0)
            return wad->entries + i;
    return NULL;
}

unsigned char * wad_read(wad_t * wad, const char * name, int * size) {
    unsigned char * buffer;
    wad_entry_t * entry;
    FILE * file;

    if ((entry = wad_find(wad, name)) != NULL) {
        file = fopen(name, "rb");
        * size = entry->size;
        buffer = malloc(entry->size);
        fseek(file, entry->file_pos, SEEK_SET);
        fread(buffer, 1, entry->size, file);
        fclose(file);
        return buffer;
    }
    * size = 0;
    return NULL;
}