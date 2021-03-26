#include <stdio.h>
#include "player.h"

player_t players[MAX_PLAYERS];

player_t * get_player(int id) {
    return players + id;
}

void load_player(const char * player_file, int id) {
    FILE *file;
    file = fopen(player_file, "rb");
    if (file != NULL) {
        if (fseek(file, id * sizeof(player_stats_t), SEEK_SET) != 0)
            goto failure;
        if (fread(get_player(id), sizeof(player_t), 1, file) != 1)
            goto failure;
        return;
    }
    else {
        printf("The save file %s doesnt exist!\n", player_file);
        return;
    }

failure:
    printf("Couldn't find player %d data from %s!\n", id, player_file);
}

void save_player(const char * player_file, int id) {
    FILE *file;
    file = fopen(player_file, "wb");
    if (file != NULL) {
        if (fseek(file, id * sizeof(player_stats_t), SEEK_SET) != 0)
            goto failure;
        if (fwrite(get_player(id), sizeof(player_t), 1, file) != 1)
            goto failure;
        return;
    }
    else {
        printf("Can't save player data to file %s!\n", player_file);
        return;
    }

failure:
    printf("Couldn't save player %d data to %s file!\n", id, player_file);
}