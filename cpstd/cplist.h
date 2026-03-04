#pragma once

#include "cpbase.h"

#define LINKED_LIST_DEF(type, name)                                            \
    typedef struct name##_n {                                                  \
        type val;                                                              \
        b8 tomb;                                                               \
        struct name##_n *next;                                                 \
    } name##_node;                                                             \
    typedef struct {                                                           \
        name##_node *nodes;                                                    \
        u32 capacity;                                                          \
        u32 size;                                                              \
    } name;                                                                    \
    void name##_init(name *l, u32 size) {                                      \
        l->capacity = size * 2;                                                \
        l->nodes = malloc(l->capacity * sizeof(name##_node));                  \
        l->size = size;                                                        \
        for (u32 i = 0; i < size; i++) {                                       \
            if (i >= size - 1) {                                               \
                l->nodes[i].next = NULL;                                       \
                break;                                                         \
            }                                                                  \
            l->nodes[i].next = &l->nodes[i + 1];                               \
        }                                                                      \
    }                                                                          \
    void name##_destroy(name *l) {                                             \
        free(l->nodes);                                                        \
        l->capacity = 0;                                                       \
        l->size = 0;                                                           \
    }                                                                          \
    void name##_reserve(name *l, u32 capacity) {                               \
        l->nodes = malloc(capacity * sizeof(name##_node));                     \
        l->capacity = capacity;                                                \
        l->size = 0;                                                           \
    }                                                                          \
    void name##_add(name *l, type val) {                                       \
        if (l->capacity <= l->size) {                                          \
            name##_node *new_data =                                            \
                realloc(l->nodes, (u64)l->capacity * 2 * sizeof(name##_node)); \
            if (new_data != NULL) {                                            \
                l->capacity *= 2;                                              \
                l->nodes = new_data;                                           \
            }                                                                  \
        }                                                                      \
        if (l->size > 0) {                                                     \
            l->nodes[l->size - 1].next = &l->nodes[l->size];                   \
        }                                                                      \
        l->nodes[l->size++] = (name##_node){val, false, NULL};                 \
    }                                                                          \
    name##_node *name##_get(name *l, u32 i) { return &l->nodes[i]; }           \
    void name##_set(name *l, u32 i, type val) { l->nodes[i].val = val; }       \
    void name##_pop(name *l, u32 i) {                                          \
        if (i >= l->size) {                                                    \
            return;                                                            \
        }                                                                      \
        l->nodes[i].tomb = true;                                               \
        if (i == 0) {                                                          \
            return;                                                            \
        }                                                                      \
        i32 prev = (i32)i - 1;                                                 \
        while (prev >= 0 && l->nodes[prev].tomb)                               \
            prev--;                                                            \
        if (prev < 0)                                                          \
            return;                                                            \
        u32 next = i + 1;                                                      \
        while (next < l->size && l->nodes[next].tomb) {                        \
            next++;                                                            \
        }                                                                      \
        if (next >= l->size) {                                                 \
            l->nodes[prev].next = NULL;                                        \
        } else {                                                               \
            l->nodes[prev].next = &l->nodes[next];                             \
        }                                                                      \
    }
