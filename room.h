#ifndef _room_h
#define _room_h
#include "wad.h"

#define ROOM_TITLE_SIZE 32
#define ROOM_SPAWN_SIZE 16
#define ROOM_PLAYER_SIZE 8

typedef struct room_spawn {
    float x;
    float y;
    int obj_type;
    int wait_time;
    int timer;
} room_spawn_t;

typedef struct room {
    int width;
    int height;
    int outside_block;
    char sprite_name[NAME_SIZE];
    char title[ROOM_TITLE_SIZE];
    unsigned char * blocks;
    int num_spawns;
    int num_player_spawns;
    room_spawn_t * spawns;
    room_spawn_t * player_spawns;
} room_t;

void room_new_empty(room_t * room, int width, int height, int max_spawns, int max_player_spawns);

void room_load(room_t * room, const unsigned char * bin);

void room_free(room_t * room);

unsigned char room_get_block(room_t * room, int x, int y);

int room_is_colliding(room_t * room, float x, float y, float w, float h);

#endif