#include <string.h>
#include <stdlib.h>
#include "thing.h"

extern thing_update_t thing_updates[THINGTYPE_MAX - 1];

extern thing_start_t thing_starts[THINGTYPE_MAX - 1];

extern thing_end_t thing_ends[THINGTYPE_MAX - 1];

thing_t * thing_head = NULL;

int thing_name_id(const char * name) {
    return 0;
}

void free_things(void) {
    thing_t * thing;
    for (thing = thing_head; thing != NULL; thing = thing->footer.next)
        free(thing);
}

thing_t ** get_thing_head(void) {
    return &thing_head;
}

thing_t * thing_add(void) {
    thing_t * thing;
    thing = malloc(sizeof(thing_t));
    thing->last = &thing_head;
    thing->next = thing_head != NULL ? thing_head : NULL;
    if (thing_head != NULL)
        thing_head->last = &thing->next;
    thing_head = thing;
    return thing;
}

void thing_remove(thing_t * thing) {
    if (thing != NULL) {
        if (thing_end_func(thing->type) != NULL)
            thing_end_func(thing->type)(thing);
        *thing->last = thing->next;
        if (thing->next != NULL)
            thing->next->last = &thing->next;
    }
}

thing_update_t thing_update_func(int type) {
    return type ? thing_updates[type - 1] : NULL;
}

thing_start_t thing_start_func(int type) {
    return type ? thing_starts[type - 1] : NULL;
}

thing_end_t thing_end_func(int type) {
    return type ? thing_ends[type - 1] : NULL;
}