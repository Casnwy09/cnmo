#ifndef _blocks_h
#define _blocks_h

#define BLOCK_SIZE 32

typedef enum blocks {
    BLOCK_AIR = 0x000,
    BLOCK_STONE,
    BLOCK_IRON,
    BLOCK_BRICKS,
    BLOCK_BACKGROUND_BRICKS,
    BLOCK_GRASS,
    BLOCK_LEAVES,
    BLOCK_WOOL,
    BLOCK_WATER,
    BLOCK_WOOD,
    BLOCK_GLASS,
    BLOCK_OOB,
    BLOCK_MAX = 0x100
} blocks_t;

int is_block_solid(int id);

#endif