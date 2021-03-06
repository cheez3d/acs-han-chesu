#ifndef UTILS_H
#define UTILS_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Hash the given string using the djb2 algorithm.
 */
static inline uint32_t hash_djb2(const char *str) {
    assert(str != NULL);

    uint32_t hash = 5381;

    char c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) ^ c;
    }

    return hash;
}

#endif // UTILS_H
