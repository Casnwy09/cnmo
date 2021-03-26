#ifndef _thing_h
#define _thing_h
#include <stddef.h>

#define THING_SIZE offsetof(thing_t, next)

typedef struct thing thing_t;

typedef enum thing_types {
    THINGTYPE_NULL,
    THINGTYPE_SLIME,
    THINGTYPE_MAX
} thing_types_t;

typedef enum thing_flags {
    THINGFLAG_HOSTILE = 1 << 0,
    THINGFLAG_SOLID = 1 << 1
} thing_flags_t;

typedef struct thing {
    float x;
    float y;
    float vx;
    float vy;
    float w;
    float h;
    int type;
    int timer;
    int state;
    int hp;
    unsigned short money;
    short reward;
    int flags;
    char sprite[8];
    char hurt_snd[8];

    thing_t * next;
    thing_t ** last;
} thing_t;

typedef void(*thing_start_t)(thing_t *thing);

typedef void(*thing_end_t)(thing_t *thing);

typedef void(*thing_update_t)(thing_t *thing);

int thing_name_id(const char * name);

void free_things(void);

thing_t ** get_thing_head(void);

thing_t * thing_add(void);

void thing_remove(thing_t * thing);

thing_update_t thing_update_func(int type);

thing_start_t thing_start_func(int type);

thing_end_t thing_end_func(int type);

#endif