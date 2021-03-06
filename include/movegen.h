#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "board.h"
#include "move.h"

#include <stddef.h>

/**
 * Max number of legal moves possible.
 */
#define MOVE_LIST_MAX 128

/**
 * Move list data structure to store all possible moves
 * for the current game state.
 */
struct move_list {
    struct move list[MOVE_LIST_MAX];

    size_t count;

    size_t head;

    size_t captured_pieces;
};

/**
 * Populate the provided move list for the provided board
 * with all the possible moves.
 */
void movegen_add_moves(struct move_list *moves, struct board *board);

#endif // MOVEGEN_H
