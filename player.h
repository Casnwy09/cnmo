#ifndef _player_h
#define _player_h

#define MAX_PLAYERS 64

typedef struct thing thing_t;

typedef struct player_stats {
    int strength;
    int hp;
    int money;
    float jump_power;
    float speed;
} player_stats_t;

typedef struct player {
    thing_t * body;
    player_stats_t stats;
} player_t;

player_t * get_player(int id);

void update_player(int id);

void load_player(const char * player_file, int id);

void save_player(const char * player_file, int id);

#endif