#pragma once

#include <stdlib.h>
#include <string.h>

#include "cpbase.h"
#include "cpmath.h"

typedef struct {
    u64 capacity;
    u64 pos;
} mem_arena;

#define ALIGN_POW2(n, p) (((u64)(n) + ((u64)(p) - 1)) & (~((u64)(p) - 1)))
#define ARENA_BASE_POS (sizeof(mem_arena))
#define ARENA_ALIGN (sizeof(void *))

mem_arena *mem_arena_create(u64 capacity) {
    mem_arena *arena = malloc(capacity);
    arena->capacity = capacity;
    arena->pos = ARENA_BASE_POS;

    return arena;
}

void mem_arena_destroy(mem_arena *arena) { free(arena); }

void *mem_arena_push(mem_arena *arena, u64 size, b8 nz) {
    u64 pos_aligned = ALIGN_POW2(arena->pos, ARENA_ALIGN);
    u64 new_pos = pos_aligned + size;
    if (new_pos > arena->capacity) {
        return NULL;
    }
    arena->pos = new_pos;
    u8 *out = (u8 *)arena + pos_aligned;

    if (!nz) {
        memset(out, 0, size);
    }

    return out;
}

void mem_arena_pop(mem_arena *arena, u64 size) {
    size = CPM_MIN(size, arena->pos - ARENA_BASE_POS);
    arena->pos -= size;
}

void mem_arena_pop_to(mem_arena *arena, u64 pos) {
    u64 size = pos < arena->pos ? arena->pos - pos : 0;
    mem_arena_pop(arena, size);
}

void mem_arena_clear(mem_arena *arena) {
    mem_arena_pop_to(arena, ARENA_BASE_POS);
}
