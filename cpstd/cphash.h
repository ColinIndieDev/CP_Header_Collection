#pragma once

#include <stdlib.h>
#include <string.h>

#include "cpmath.h"

#define HASH_EMPTY 0
#define HASH_OCCUPIED 1
#define HASH_TOMBSTONE 2

#define HASHMAP_DEF(key_type, val_type, name)                                  \
    typedef struct {                                                           \
        key_type key;                                                          \
        val_type value;                                                        \
        u8 state;                                                              \
    } name##_entry;                                                            \
    typedef struct {                                                           \
        name##_entry *data;                                                    \
        u32 capacity;                                                          \
        u32 size;                                                              \
    } name;                                                                    \
    static u32 name##_hash(key_type key) { return (u32)key * 2654435761u; }    \
    void name##_init(name *m, u32 capacity) {                                  \
        capacity = CPM_MAX(8, capacity);                                       \
        m->capacity = capacity;                                                \
        m->size = 0;                                                           \
        m->data = malloc(capacity * sizeof(name##_entry));                     \
        memset(m->data, 0, capacity * sizeof(name##_entry));                   \
    }                                                                          \
    void name##_destroy(name *m) {                                             \
        free(m->data);                                                         \
        m->capacity = 0;                                                       \
        m->size = 0;                                                           \
    }                                                                          \
    static u32 name##_probe(name *m, key_type key) {                           \
        u32 idx = name##_hash(key) % m->capacity;                              \
        while (m->data[idx].state == HASH_OCCUPIED &&                          \
               m->data[idx].key != key) {                                      \
            idx = (idx + 1) % m->capacity;                                     \
        }                                                                      \
        return idx;                                                            \
    }                                                                          \
    void name##_resize(name *m) {                                              \
        name##_entry *old_data = m->data;                                      \
        u32 old_capacity = m->capacity;                                        \
        u32 new_capacity = old_capacity * 2;                                   \
        m->data = malloc(new_capacity * sizeof(name##_entry));                 \
        memset(m->data, 0, new_capacity * sizeof(name##_entry));               \
        m->capacity = new_capacity;                                            \
        m->size = 0;                                                           \
        for (u32 i = 0; i < old_capacity; i++) {                               \
            if (old_data[i].state == HASH_OCCUPIED) {                          \
                u32 idx = name##_probe(m, old_data[i].key);                    \
                if (m->data[idx].state != HASH_OCCUPIED) {                     \
                    m->size++;                                                 \
                }                                                              \
                m->data[idx].key = old_data[i].key;                            \
                m->data[idx].value = old_data[i].value;                        \
                m->data[idx].state = HASH_OCCUPIED;                            \
            }                                                                  \
        }                                                                      \
        free(old_data);                                                        \
    }                                                                          \
    void name##_put(name *m, key_type key, val_type value) {                   \
        u32 idx = name##_probe(m, key);                                        \
        if (m->data[idx].state != HASH_OCCUPIED) {                             \
            m->size++;                                                         \
        }                                                                      \
        if ((f32)m->size / m->capacity > 0.7f) {                               \
            name##_resize(m);                                                  \
        }                                                                      \
        m->data[idx].key = key;                                                \
        m->data[idx].value = value;                                            \
        m->data[idx].state = HASH_OCCUPIED;                                    \
    }                                                                          \
    val_type *name##_get(name *m, key_type key) {                              \
        u32 idx = name##_hash(key) % m->capacity;                              \
        while (m->data[idx].state != HASH_EMPTY) {                             \
            if (m->data[idx].state == HASH_OCCUPIED &&                         \
                m->data[idx].key == key) {                                     \
                return &m->data[idx].value;                                    \
            }                                                                  \
            idx = (idx + 1) % m->capacity;                                     \
        }                                                                      \
        return NULL;                                                           \
    }                                                                          \
    void name##_remove(name *m, key_type key) {                                \
        u32 idx = name##_hash(key) % m->capacity;                              \
        while (m->data[idx].state != HASH_EMPTY) {                             \
            if (m->data[idx].state == HASH_OCCUPIED &&                         \
                m->data[idx].key == key) {                                     \
                m->data[idx].state = HASH_TOMBSTONE;                           \
                m->size--;                                                     \
                return;                                                        \
            }                                                                  \
            idx = (idx + 1) % m->capacity;                                     \
        }                                                                      \
    }                                                                          \
    name##_entry *name##_begin(name *m) { return m->data; }                    \
    name##_entry *name##_end(name *m) { return m->data + m->capacity; }        \
    b8 name##_empty(name *m) { return m->size <= 0; }

#define FOREACH_HM(m_type, it, m_ptr)                                          \
    for (m_type##_entry *it = m_type##_begin(m_ptr);                           \
         it != m_type##_end(m_ptr); it++)

#define HASHSET_DEF(key_type, name)                                            \
    typedef struct {                                                           \
        key_type key;                                                          \
        u8 state;                                                              \
    } name##_entry;                                                            \
    typedef struct {                                                           \
        name##_entry *data;                                                    \
        u32 capacity;                                                          \
        u32 size;                                                              \
    } name;                                                                    \
    static u32 name##_hash(key_type key) { return (u32)key * 2654435761u; }    \
    void name##_init(name *s, u32 capacity) {                                  \
        capacity = CPM_MAX(8, capacity);                                       \
        s->capacity = capacity;                                                \
        s->size = 0;                                                           \
        s->data = malloc(capacity * sizeof(name##_entry));                     \
        memset(s->data, 0, capacity * sizeof(name##_entry));                   \
    }                                                                          \
    void name##_destroy(name *s) {                                             \
        free(s->data);                                                         \
        s->capacity = 0;                                                       \
        s->size = 0;                                                           \
    }                                                                          \
    static u32 name##_probe(name *s, key_type key) {                           \
        u32 idx = name##_hash(key) % s->capacity;                              \
        while (s->data[idx].state == HASH_OCCUPIED &&                          \
               s->data[idx].key != key) {                                      \
            idx = (idx + 1) % s->capacity;                                     \
        }                                                                      \
        return idx;                                                            \
    }                                                                          \
    void name##_resize(name *s) {                                              \
        name##_entry *old_data = s->data;                                      \
        u32 old_capacity = s->capacity;                                        \
        u32 new_capacity = old_capacity * 2;                                   \
        s->data = malloc(new_capacity * sizeof(name##_entry));                 \
        memset(s->data, 0, new_capacity * sizeof(name##_entry));               \
        s->capacity = new_capacity;                                            \
        s->size = 0;                                                           \
        for (u32 i = 0; i < old_capacity; i++) {                               \
            if (old_data[i].state == HASH_OCCUPIED) {                          \
                u32 idx = name##_probe(s, old_data[i].key);                    \
                if (s->data[idx].state != HASH_OCCUPIED) {                     \
                    s->size++;                                                 \
                }                                                              \
                s->data[idx].key = old_data[i].key;                            \
                s->data[idx].state = HASH_OCCUPIED;                            \
            }                                                                  \
        }                                                                      \
        free(old_data);                                                        \
    }                                                                          \
    void name##_put(name *s, key_type key) {                                   \
        u32 idx = name##_probe(s, key);                                        \
        if (s->data[idx].state != HASH_OCCUPIED) {                             \
            s->size++;                                                         \
        }                                                                      \
        if ((f32)s->size / s->capacity > 0.7f) {                               \
            name##_resize(s);                                                  \
        }                                                                      \
        s->data[idx].key = key;                                                \
        s->data[idx].state = HASH_OCCUPIED;                                    \
    }                                                                          \
    b8 name##_contains(name *s, key_type key) {                                \
        u32 idx = name##_hash(key) % s->capacity;                              \
        while (s->data[idx].state != HASH_EMPTY) {                             \
            if (s->data[idx].state == HASH_OCCUPIED &&                         \
                s->data[idx].key == key) {                                     \
                return true;                                                   \
            }                                                                  \
            idx = (idx + 1) % s->capacity;                                     \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    void name##_remove(name *s, key_type key) {                                \
        u32 idx = name##_hash(key) % s->capacity;                              \
        while (s->data[idx].state != HASH_EMPTY) {                             \
            if (s->data[idx].state == HASH_OCCUPIED &&                         \
                s->data[idx].key == key) {                                     \
                s->data[idx].state = HASH_TOMBSTONE;                           \
                s->size--;                                                     \
                return;                                                        \
            }                                                                  \
            idx = (idx + 1) % s->capacity;                                     \
        }                                                                      \
    }                                                                          \
    name##_entry *name##_begin(name *s) { return s->data; }                    \
    name##_entry *name##_end(name *s) { return s->data + s->capacity; }        \
    b8 name##_empty(name *s) { return s->size <= 0; }

#define FOREACH_HS(s_type, it, s_ptr)                                          \
    for (s_type##_entry *it = s_type##_begin(s_ptr);                           \
         it != s_type##_end(s_ptr); it++)
