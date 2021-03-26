#include "blocks.h"

int is_block_solid(int id) {
    switch (id) {
        case BLOCK_AIR:
        case BLOCK_BACKGROUND_BRICKS:
        case BLOCK_WATER:
        return 0;
    }
    return 1;
}