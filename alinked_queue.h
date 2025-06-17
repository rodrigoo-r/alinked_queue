/*
 * This code is distributed under the terms of the GNU General Public License.
 * For more information, please refer to the LICENSE file in the root directory.
 * -------------------------------------------------
 * Copyright (C) 2025 Rodrigo R.
 * This program comes with ABSOLUTELY NO WARRANTY; for details type show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type show c' for details.
*/

#ifndef FLUENT_LIBC_A_LINKED_QUEUE_LIBRARY_H
#define FLUENT_LIBC_A_LINKED_QUEUE_LIBRARY_H

// ============= FLUENT LIB C =============
// Arena-Backed Linked Queue
// ----------------------------------------
// This header defines a minimal arena-allocated singly linked queue with append, prepend,
// and shift (pop from head) operations. Perfect for cases where performance and control
// over allocation are key.
//
// Macro-powered & Type-safe:
//   DEFINE_ALINKED_NODE(T, name) – creates a queue type and API for type `T`.
//
// Queue Features:
//   • Fast O(1) prepend, append, and shift operations.
//   • Built-in arena allocator (chunk-based memory efficiency).
//   • Optional free-list to reuse nodes (avoid arena fragmentation).
//
// API Usage:
// ----------------------------------------
//   DEFINE_ALINKED_NODE(int, int); // create int-based queue
//   alinked_queue_int_t q;
//   alinked_queue_int_init(&q, 512); // 512 chunks arena
//   alinked_queue_int_append(&q, 42);
//   int x = alinked_queue_int_shift(&q);
//   alinked_queue_int_destroy(&q);
//
// Internals:
//   - Each node is `struct { T data; next* }`
//   - Queue stores head/tail/len + arena + free-list
//   - Allocation done via arena_malloc or free-list reuse
//
// Dependencies:
//   - `arena.h` for memory pool
//   - `vector.h` for free-list (optional)
//   - `types.h` for `size_t`, `NULL`
//
// Notes:
//   - Generic fallback for `void*` queue is provided as `alinked_queue_generic_t`
//   - Non-thread safe by default (no locks)
//
// Great for:
//   - Embedded systems, game dev, or dataflow tasks where malloc is too slow
//   - Zero-GC, high-throughput situations

// ============= FLUENT LIB C++ =============
#if defined(__cplusplus)
extern "C"
{
#endif

// ============= INCLUDES =============
#ifndef FLUENT_LIBC_RELEASE
#   include <arena.h> // fluent_libc
#   include <types.h> // fluent_libc
#   include <vector.h> // fluent_libc
#else
#   include <fluent/vector/vector.h> // fluent_libc
#   include <fluent/arena/arena.h> // fluent_libc
#endif

#define DEFINE_ALINKED_NODE(V, NAME)                        \
    typedef struct alinked_node_##NAME##_t                  \
    {                                                       \
        V data;                                             \
        struct alinked_node_##NAME##_t *next;               \
    } alinked_node_##NAME##_t;                              \
                                                            \
    DEFINE_VECTOR(alinked_node_##NAME##_t *, _fluent_libc_list_##NAME); \
                                                            \
    typedef struct                                          \
    {                                                       \
        alinked_node_##NAME##_t *head;                      \
        alinked_node_##NAME##_t *tail;                      \
        size_t len;                                         \
        arena_allocator_t *allocator;                       \
        vector__fluent_libc_list_##NAME##_t *free_list;     \
    } alinked_queue_##NAME##_t;                             \
                                                            \
    static inline void alinked_queue_##NAME##_init(         \
        alinked_queue_##NAME##_t *queue,                    \
        const size_t arena_len                              \
    )                                                       \
    {                                                       \
        queue->allocator = arena_new(arena_len, sizeof(alinked_node_##NAME##_t)); \
                                                            \
        if (!queue->allocator)                              \
        {                                                   \
            queue->head = NULL;                             \
            queue->tail = NULL;                             \
            queue->len = 0;                                 \
            return;                                         \
        }                                                   \
                                                            \
        queue->head = NULL;                                 \
        queue->tail = NULL;                                 \
        queue->len = 0;                                     \
                                                            \
        queue->free_list = malloc(sizeof(vector__fluent_libc_list_##NAME##_t)); \
        if (queue->free_list)                               \
        {                                                   \
            vec__fluent_libc_list_##NAME##_init(queue->free_list, 15, 1.5); \
        }                                                   \
        else                                                \
        {                                                   \
            queue->free_list = NULL;                        \
        }                                                   \
    }                                                       \
                                                            \
    static inline void alinked_queue_##NAME##_destroy(      \
        alinked_queue_##NAME##_t *queue                     \
    )                                                       \
    {                                                       \
        if (queue->allocator)                               \
        {                                                   \
            destroy_arena(queue->allocator);                \
            queue->allocator = NULL;                        \
        }                                                   \
                                                            \
        queue->head = NULL;                                 \
        queue->tail = NULL;                                 \
        queue->len = 0;                                     \
                                                            \
        if (queue->free_list)                               \
        {                                                   \
            vec__fluent_libc_list_##NAME##_destroy(queue->free_list, NULL); \
            free(queue->free_list);                         \
            queue->free_list = NULL;                        \
        }                                                   \
    }                                                       \
    static alinked_node_##NAME##_t *__fluent_libc_##NAME##_linked_queue_suitable(const alinked_queue_##NAME##_t *queue) \
    {                                                       \
        if (queue->free_list && queue->free_list->length > 0) \
        {                                                   \
            alinked_node_##NAME##_t *node = vec__fluent_libc_list_##NAME##_pop(queue->free_list); \
            return node;                                    \
        }                                                   \
                                                            \
        alinked_node_##NAME##_t *node = (alinked_node_##NAME##_t *)arena_malloc(queue->allocator); \
        if (!node)                                          \
        {                                                   \
            return NULL;                                    \
        }                                                   \
                                                            \
        return node;                                        \
    }                                                       \
                                                            \
    static inline void alinked_queue_##NAME##_append(       \
        alinked_queue_##NAME##_t *queue,                    \
        V data                                              \
    )                                                       \
    {                                                       \
        alinked_node_##NAME##_t *node = __fluent_libc_##NAME##_linked_queue_suitable(queue); \
        if (!node)                                          \
        {                                                   \
            return;                                         \
        }                                                   \
                                                            \
        node->data = data;                                  \
        node->next = NULL;                                  \
                                                            \
        if (queue->len == 0)                                \
        {                                                   \
            queue->head = node;                             \
            queue->tail = node;                             \
        }                                                   \
        else                                                \
        {                                                   \
            queue->tail->next = node;                       \
            queue->tail = node;                             \
        }                                                   \
                                                            \
        queue->len++;                                       \
    }                                                       \
                                                            \
    static inline void alinked_queue_##NAME##_prepend(      \
        alinked_queue_##NAME##_t *queue,                    \
        V data                                              \
    )                                                       \
    {                                                       \
        alinked_node_##NAME##_t *node = __fluent_libc_##NAME##_linked_queue_suitable(queue); \
        if (!node)                                          \
        {                                                   \
            return;                                         \
        }                                                   \
                                                            \
        node->data = data;                                  \
        node->next = queue->head;                           \
                                                            \
        if (queue->len == 0)                                \
        {                                                   \
            queue->tail = node;                             \
        }                                                   \
                                                            \
        queue->head = node;                                 \
        queue->len++;                                       \
    }                                                       \
                                                            \
    static inline V alinked_queue_##NAME##_shift(           \
        alinked_queue_##NAME##_t *queue                     \
    )                                                       \
    {                                                       \
        alinked_node_##NAME##_t *node = queue->head;        \
                                                            \
        if (queue->head == queue->tail)                     \
        {                                                   \
            queue->head = NULL;                             \
            queue->tail = NULL;                             \
        }                                                   \
        else                                                \
        {                                                   \
            queue->head = node->next;                       \
        }                                                   \
                                                            \
        if (queue->free_list)                               \
        {                                                   \
            vec__fluent_libc_list_##NAME##_push(queue->free_list, node); \
        }                                                   \
                                                            \
        queue->len--;                                       \
        return node->data;                                  \
    }

#ifndef FLUENT_LIBC_A_LINKED_QUEUE_GENERIC_DEFINED
    DEFINE_ALINKED_NODE(void *, generic);
#   define FLUENT_LIBC_A_LINKED_QUEUE_GENERIC_DEFINED 1
#endif

// ============= FLUENT LIB C++ =============
#if defined(__cplusplus)
}
#endif

#endif //FLUENT_LIBC_A_LINKED_QUEUE_LIBRARY_H