#ifndef SEARCH_H
#define SEARCH_H

#include "board.h"
#include "move.h"

#define SEARCH_DEPTH 6

enum other_score {
    CAPTURE_BONUS = 4000,
    PROMOTION_BONUS = 3000,
    KILLER1_BONUS = 2000,
    KILLER2_BONUS = 1000,
    QUIET_BONUS = 0
};

struct ordering_info {
    struct move killer1[50], killer2[50];
    int ply;
    int history[2][64][64];
};

struct move search_best_move(struct board *board);
struct move search_best_move2(struct board *board);
void init_other_moves_table();

#endif // SEARCH_H
