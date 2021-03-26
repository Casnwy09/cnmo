#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"
#include "wad.h"
#include "room.h"
#include "blocks.h"
#include "thing.h"
#include "roomconv.h"

typedef enum tiledprop_type {
    TILEDPROP_NONE,
    TILEDPROP_STRING,
    TILEDPROP_INT,
    TILEDPROP_FLOAT,
    TILEDPROP_BOOL,
    TILEDPROP_COLOR,
    TILEDPROP_FILE,
    TILEDPROP_MAX
} tiledprop_type_t;

typedef struct tiledprop {
    const char * name;
    int type;
    union {
        int valueint;
        unsigned int valuecolor;
        double valuedouble;
        const char * valuestring;
    } value;
} tiledprop_t;

const cJSON * roomconv_tiled_json_find_layer(const cJSON * map, const char * name);

void roomconv_tiled_json_get_prop(tiledprop_t * prop, const cJSON * props, const char * prop_name);

unsigned char * roomconv_tiled_json(int * size, const char * json_file) {
    unsigned char * bin, * base;
    char * json_source;
    cJSON * json;
    tiledprop_t prop;

    char sprite_name[NAME_SIZE + 2];
    char title[ROOM_TITLE_SIZE + 2];
    int outside_block;
    int width;
    int height;
    int num_spawns;
    int num_player_spawns;

    {
        long file_len;
        FILE * file;
        file = fopen(json_file, "rb");
        if (file == NULL) {
            printf("Cannot convert file tiled json file %s to binary format.\n", json_file);
            return NULL;
        }
        fseek(file, 0, SEEK_END);
        file_len = ftell(file);
        fseek(file, 0, SEEK_SET);
        json_source = malloc(file_len);
        fread(json_source, 1, file_len, file);

        json = cJSON_ParseWithLength(json_source, (size_t)file_len);
        if (json == NULL) {
            printf("File %s givin is not a json formatted file!\n", json_file);
            return NULL;
        }
    }

    {
        // Get the sprite name from the tiled map properties
        const cJSON * props;
        props = cJSON_GetObjectItemCaseSensitive(json_file, "properties");
        roomconv_tiled_json_get_prop(&prop, props, "sprite_name");
        sprite_name[NAME_SIZE + 1] = '\0';
        if (prop.type == TILEDPROP_STRING)
            strncpy(sprite_name, prop.value.valuestring, NAME_SIZE + 1);
        else
            printf("Did not find a valid map sprite of name!\n");

        // Get the map title from the tiled map properties
        roomconv_tiled_json_get_prop(&prop, props, "title");
        strcpy(title, "super city");
        title[NAME_SIZE + 1] = '\0';
        if (prop.type == TILEDPROP_STRING)
            strncpy(title, prop.value.valuestring, ROOM_TITLE_SIZE + 1);
        else
            printf("No valid map title, using default.\n");
        
        // Get the maps outside block
        roomconv_tiled_json_get_prop(&prop, props, "outside_block");
        outside_block = 0;
        if (prop.type == TILEDPROP_INT)
            outside_block = prop.value.valueint & (unsigned char)-1;
        else
            printf("No outside block for map, using BLOCK_AIR.\n");
    }

    {
        // Get misc map fields
        const cJSON * map_field;
        map_field = cJSON_GetObjectItemCaseSensitive(json_file, "width");
        width = 0;
        if (map_field != NULL && cJSON_IsNumber(map_field))
            width = map_field->valueint;
        else
            printf("No map width in map fields!\n");
        map_field = cJSON_GetObjectItemCaseSensitive(json_file, "height");
        height = 0;
        if (map_field != NULL && cJSON_IsNumber(map_field))
            width = map_field->valueint;
        else
            printf("No map height in map fields!\n");
    }

    {
        // Get the number of normal spawns and player spawns
        const cJSON * layer, * objects;
        layer = roomconv_tiled_json_find_layer(json_file, "normal_spawns");
        num_spawns = 0;
        if (layer != NULL) {
            objects = cJSON_GetObjectItemCaseSensitive(layer, "objects");
            if (objects != NULL && cJSON_IsArray(objects))
                num_spawns = cJSON_GetArraySize(objects);
            else
                printf("Map has no normal spawns!\n");
        }
        else
            printf("Map has no normal spawns!\n");
        layer = roomconv_tiled_json_find_layer(json_file, "player_spawns");
        num_player_spawns = 0;
        if (layer != NULL) {
            objects = cJSON_GetObjectItemCaseSensitive(layer, "objects");
            if (objects != NULL && cJSON_IsArray(objects))
                num_player_spawns = cJSON_GetArraySize(objects);
            else
                printf("Map has no player spawns!\n");
        }
        else
            printf("Map has no player spawns!\n");
    }

    // Initialize the new buffer for the data
    bin = malloc(NAME_SIZE + ROOM_TITLE_SIZE + 20 + width * height + ROOM_SPAWN_SIZE * num_spawns + ROOM_PLAYER_SIZE * num_player_spawns);
    memcpy(bin, sprite_name, NAME_SIZE);
    memcpy(bin + NAME_SIZE, title, ROOM_TITLE_SIZE);
    base = bin + NAME_SIZE + ROOM_TITLE_SIZE;
    memcpy(base, outside_block, 4);
    memcpy(base + 4, width, 4);
    memcpy(base + 8, height, 4);
    memcpy(base + 12, num_spawns, 4);
    memcpy(base + 16, num_player_spawns, 4);
    base += 20;

    {
        // Get the room tile data
        int i;
        const cJSON * layer, * data, * block;
        layer = roomconv_tiled_json_find_layer(json, "blocks");
        memset(base, BLOCK_AIR, width * height);
        if (layer != NULL) {
            data = cJSON_GetObjectItemCaseSensitive(layer, "data");
            if (data != NULL && cJSON_IsArray(data) && cJSON_GetArraySize(data) == width * height) {
                for (i = 0; i < width * height; i++) {
                    block = cJSON_GetArrayItem(data, i);
                    if (block != NULL && cJSON_IsNumber(block))
                        base[i] = block->valueint & (unsigned char)-1;
                }
            }
            else
                printf("Map has no blocks! Using BLOCK_AIR as default.\n");
        }
        else
            printf("Map has no blocks! Using BLOCK_AIR as default.\n");
    }

    base += width * height;

    {
        // Get the room normal spawns data
        int i;
        const cJSON * layer, * objects, * spawn;
        const cJSON * spawn_props, * spawn_pos;
        tiledprop_t spawn_prop;
        layer = roomconv_tiled_json_find_layer(json, "normal_spawns");
        memset(base, 0, num_spawns * ROOM_SPAWN_SIZE);
        if (layer != NULL) {
            objects = cJSON_GetObjectItemCaseSensitive(layer, "objects");
            if (objects != NULL && cJSON_IsArray(objects) && cJSON_GetArraySize(objects) == num_spawns) {
                for (i = 0; i < num_spawns; i++) {
                    spawn = cJSON_GetArrayItem(spawn, i);
                    if (spawn != NULL) {
                        spawn_props = cJSON_GetObjectItemCaseSensitive(spawn, "properties");
                        spawn_pos = cJSON_GetObjectItemCaseSensitive(spawn, "x");
                        if (spawn_pos != NULL && cJSON_IsNumber(spawn_pos))
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 0)) = (float)spawn_pos->valuedouble;
                        else
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 0)) = 0.0f;
                        spawn_pos = cJSON_GetObjectItemCaseSensitive(spawn, "y");
                        if (spawn_pos != NULL && cJSON_IsNumber(spawn_pos))
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 4)) = (float)spawn_pos->valuedouble;
                        else
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 4)) = 0.0f;
                        spawn_pos = cJSON_GetObjectItemCaseSensitive(spawn, "type");
                        if (spawn_pos != NULL && cJSON_IsString(spawn_pos))
                            *(int * )(base + (i * ROOM_SPAWN_SIZE + 8)) = thing_name_id(spawn_pos->valuestring);
                        else
                            *(int * )(base + (i * ROOM_SPAWN_SIZE + 8)) = 0;
                        roomconv_tiled_json_get_prop(&spawn_prop, spawn_props, "wait_time");
                        if (spawn_prop.type == TILEDPROP_INT)
                            *(int * )(base + (i * ROOM_SPAWN_SIZE + 12)) = spawn_prop.value.valueint;
                        else
                            *(int * )(base + (i * ROOM_SPAWN_SIZE + 12)) = 0;
                    }
                }
            }
            else
                printf("Map has no normal spawns! Using nothing as default.\n");
        }
        else
            printf("Map has no normal spawns! Using nothing as default.\n");
    }

    base += num_spawns * ROOM_SPAWN_SIZE;

    {
        // Get the room player spawns data
        int i;
        const cJSON * layer, * objects, * spawn;
        const cJSON * spawn_props, * spawn_pos;
        layer = roomconv_tiled_json_find_layer(json, "player_spawns");
        memset(base, 0, num_player_spawns * ROOM_PLAYER_SIZE);
        if (layer != NULL) {
            objects = cJSON_GetObjectItemCaseSensitive(layer, "objects");
            if (objects != NULL && cJSON_IsArray(objects) && cJSON_GetArraySize(objects) == num_player_spawns) {
                for (i = 0; i < num_player_spawns; i++) {
                    spawn = cJSON_GetArrayItem(spawn, i);
                    if (spawn != NULL) {
                        spawn_pos = cJSON_GetObjectItemCaseSensitive(spawn, "x");
                        if (spawn_pos != NULL && cJSON_IsNumber(spawn_pos))
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 0)) = (float)spawn_pos->valuedouble;
                        else
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 0)) = 0.0f;
                        spawn_pos = cJSON_GetObjectItemCaseSensitive(spawn, "y");
                        if (spawn_pos != NULL && cJSON_IsNumber(spawn_pos))
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 4)) = (float)spawn_pos->valuedouble;
                        else
                            *(float * )(base + (i * ROOM_SPAWN_SIZE + 4)) = 0.0f;
                    }
                }
            }
            else
                printf("Map has no player spawns! Using (0, 0) as default.\n");
        }
        else
            printf("Map has no player spawns! Using (0, 0) as default.\n");
    }

    cJSON_Delete(json);
    free(json_source);
    return bin;
}

const cJSON * roomconv_tiled_json_find_layer(const cJSON * map, const char * name) {
    const cJSON * current_layer, * layers, * layer_name;

    layers = cJSON_GetObjectItemCaseSensitive(map, "layers");
    if (layers == NULL) {
        printf("Map has no layers!\n");
        return NULL;
    }
    for (current_layer = layers->child; current_layer != NULL; current_layer = current_layer->next) {
        layer_name = cJSON_GetObjectItemCaseSensitive(current_layer, "name");
        if (layer_name != NULL && cJSON_IsString(layer_name) && strcmp(layer_name->valuestring, name) == 0)
            return current_layer;
    }
    return NULL;
}

void roomconv_tiled_json_get_prop(tiledprop_t * prop, const cJSON * props, const char * prop_name) {
    const cJSON * json_prop, * name, * type, * value;
    char hex_buf[8];

    if (props == NULL) {
        memset(prop, 0, sizeof(tiledprop_t));
        printf("Cannot find prop within NULL tiled json props.\n", prop_name);
        return;
    }

    for (json_prop = props->child; json_prop = json_prop->next; json_prop != NULL) {
        name = cJSON_GetObjectItemCaseSensitive(props, "name");
        type = cJSON_GetObjectItemCaseSensitive(props, "type");
        value = cJSON_GetObjectItemCaseSensitive(props, "value");
        if (name != NULL && cJSON_IsString(name) && strcmp(name->valuestring, prop_name) == 0) {
            prop->name = name->valuestring;
            prop->type = TILEDPROP_NONE;
            if (type != NULL && cJSON_IsString(type)) {
                if (strcmp(type->valuestring, "string") == 0)
                    prop->type = TILEDPROP_STRING;
                else if (strcmp(type->valuestring, "int") == 0)
                    prop->type = TILEDPROP_INT;
                else if (strcmp(type->valuestring, "float") == 0)
                    prop->type = TILEDPROP_FLOAT;
                else if (strcmp(type->valuestring, "bool") == 0)
                    prop->type = TILEDPROP_BOOL;
                else if (strcmp(type->valuestring, "color") == 0)
                    prop->type = TILEDPROP_COLOR;
                else if (strcmp(type->valuestring, "file") == 0)
                    prop->type = TILEDPROP_FILE;
            }
            memset(&prop->type, 0, sizeof(prop->type));
            if (value != NULL) {
                switch(prop->type) {
                    case TILEDPROP_STRING:
                    case TILEDPROP_FILE:
                    prop->value.valuestring = value->valuestring; break;
                    case TILEDPROP_INT:
                    case TILEDPROP_BOOL:
                    prop->value.valuestring = value->valuestring; break;
                    case TILEDPROP_FLOAT:
                    prop->value.valuedouble = value->valuedouble; break;
                    case TILEDPROP_COLOR:
                    memcpy(hex_buf, value->valuestring + 1, strlen(value->valuestring) - 1);
                    sscanf(hex_buf, "%x", &prop->value.valuecolor);
                    break;
                }
            }
            return;
        }
    }
    memset(prop, 0, sizeof(tiledprop_t));
    printf("Couldn't find tiled json property %s!\n", prop_name);
}