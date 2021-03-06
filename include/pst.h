#ifndef PST_H
#define PST_H

#include "board.h"

/**
 * Inspirations source: https://www.chessprogramming.org/Piece-Square_Tables
 */

extern int pst_values[COLOR_CNT][PIECE_CNT][SQ_CNT];
int pst_scores[COLOR_CNT];


void pst_add_piece(enum color color, enum piece piece, enum square square);
void pst_remove_piece(enum color color, enum piece piece, enum square square);
void pst_move_piece(enum color color, enum piece piece, enum square from, enum square to);

extern void init_pst();
extern void init_pst_score();
extern int get_pst_score(enum color color);

#endif // PST_H
