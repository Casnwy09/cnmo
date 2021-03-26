#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "blocks.h"
#include "room.h"

void room_new_empty(room_t * room, int width, int height, int max_spawns, int max_player_spawns) {
    room->width = width;
    room->height = height;
    memset(room->title, 0, sizeof(room->title));
    room->blocks = malloc(width * height);
    memset(room->blocks, BLOCK_AIR, width * height);
    room->num_spawns = max_spawns;
    room->num_player_spawns = max_player_spawns;
    room->spawns = malloc(max_spawns * sizeof(room_spawn_t));
    room->player_spawns = malloc(max_player_spawns * sizeof(room_spawn_t));
    memset(room->spawns, 0, sizeof(room_spawn_t) * max_spawns);
    memset(room->player_spawns, 0, sizeof(room_spawn_t) * max_player_spawns);
    room->outside_block = BLOCK_AIR;
}

void room_load(room_t * room, const unsigned char * bin) {
    const unsigned char * base;
    int i;

    memcpy(room->sprite_name, bin, NAME_SIZE);
    memcpy(room->title, bin + NAME_SIZE, ROOM_TITLE_SIZE);
    base = bin + NAME_SIZE + ROOM_TITLE_SIZE;
    room->outside_block = base;
    room->width = *(int * )(base + 4);
    room->height = *(int * )(base + 8);
    room->num_spawns = *(int * )(base + 12);
    room->num_player_spawns = *(int * )(base + 16);
    room_new_empty(room, room->width, room->height, room->num_spawns, room->num_player_spawns);
    memcpy(room->blocks, base + 20, room->width * room->height);
    base += 20 + room->width * room->height;
    for (i = 0; i < room->num_spawns; i++) {
        room->spawns[i].x = *(float * )(base + i * ROOM_SPAWN_SIZE);
        room->spawns[i].y = *(float * )(base + i * ROOM_SPAWN_SIZE + 4);
        room->spawns[i].obj_type = * (int * )(base + i * ROOM_SPAWN_SIZE + 8);
        room->spawns[i].wait_time = * (int * )(base + i * ROOM_SPAWN_SIZE + 12);
    }
    base += ROOM_SPAWN_SIZE *  room->num_spawns;
    for (i = 0; i < room->num_player_spawns; i++) {
        room->player_spawns[i].x = *(float * )(base + i * ROOM_PLAYER_SIZE);
        room->player_spawns[i].y = *(float * )(base + i * ROOM_PLAYER_SIZE + 4);
    }
}

void room_free(room_t * room) {
    free(room->blocks);
    free(room->spawns);
    free(room->player_spawns);
}

unsigned char room_get_block(room_t * room, int x, int y) {
    if (x < 0 || y < 0 || x >= room->width || y >= room->height)
        return room->outside_block;
    else
        return room->blocks[y * room->width + x];
}

int room_is_colliding(room_t * room, float x, float y, float w, float h) {
    float aabbx, aabby;
    int cx, cy;
    for (cx = (int)x / BLOCK_SIZE; cx < (int)(x + w) / BLOCK_SIZE; cx++) {
        for (cy = (int)y / BLOCK_SIZE; cy < (int)(y + h) / BLOCK_SIZE; cy++) {
            aabbx = (float)(cx * BLOCK_SIZE);
            aabby = (float)(cy * BLOCK_SIZE);
            if (aabbx + (float)BLOCK_SIZE > x &&
                aabby + (float)BLOCK_SIZE > y &&
                x + w > aabbx &&
                y + h > aabby)
                return 1;
        }
    }
    return 0;
}