#ifndef BITBOARD_H
#define BITBOARD_H

#include <assert.h>
#include <stdint.h>

/**
 * A bitboard is a 64-bit word.
 */
typedef uint_fast64_t bb_t;

#include "board.h"

/**
 * Empty bitboard.
 */
extern const bb_t BB_EMPTY;

/**
 * Bitboard used for bit shifts.
 */
extern const bb_t BB_ONE;

/**
 * Array for converting rank to its bitboard representation.
 */
extern bb_t bb_ranks[RK_CNT];

/**
 * Array for converting file to its bitboard representation.
 */
extern bb_t bb_files[FL_CNT];

/**
 * Array for converting square to its bitboard representation.
 */
extern bb_t bb_squares[SQ_CNT];

/**
 * Array for storing initial positions for pieces in bitboard representation.
 */
extern bb_t bb_pieces[COLOR_CNT][PIECE_CNT];

/**
 * Count the number of bits that are set in a bitboard.
 */
static inline int bb_bit_cnt(bb_t bb) {
    return __builtin_popcountll(bb);
}

/**
 * Pop the least significant bit set in a bitboard (set it to 0)
 * and return its position.
 */
static inline enum square bb_pop_lsb(bb_t *bb) {
    if (*bb == BB_EMPTY) {
        return SQ_NONE;
    }

    enum square s =  __builtin_ctzll(*bb);

    *bb &= *bb-1; // pop least significant bit

    return s;
}

/**
 * Return the position of the least significant bit set in a bitboard.
 */
static inline enum square bb_scan_lsb(bb_t bb) {
    if (bb == BB_EMPTY) {
        return SQ_NONE;
    }

    return __builtin_ctzll(bb);
}

/**
 * Return the position of the most significant bit set in a bitboard.
 */
static inline enum square bb_scan_msb(bb_t bb) {
    if (bb == 0) {
        return SQ_NONE;
    }

    return 8*sizeof(bb_t)-1-__builtin_clzll(bb);
}

/**
 * Initialize various useful bitboards.
 */
void bb_init(void);

/**
 * Free memory occupied by bitboards.
 */
void bb_term(void);

/**
 * Get an attacks bitboard given a piece type, its color, square
 * and the occupancy of the game table.
 */
bb_t bb_get_attacks(enum color c, enum piece p, enum square s, bb_t bb_occ);

/**
 * Print a visual representation of the bitboard for debugging purposes.
 */
void bb_print(bb_t bb);

/**
 * Print a fancy visual representation of the bitboard for debugging purposes.
 */
void bb_print_fancy(bb_t bb);

#endif // BITBOARD_H
