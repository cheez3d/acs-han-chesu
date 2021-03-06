#ifndef EVAL_H
#define EVAL_H

#include "board.h"
#include "movegen.h"
#include "pst.h"
#include <stdbool.h>
#include <assert.h>

/**
 * Inspiration source: https://www.chessprogramming.org/Evaluation
 * Another inspirations source for the centipawn evaluation: https://chess.fandom.com/wiki/Centipawn
 */

/**
 * Array of adjacent FILE bitboards for each file.
 */
extern bb_t adjacent_files[FL_CNT];

#define file_base (bb_t)0x101010101010101

/**
 * Values obtained by using http://cinnamonchess.altervista.org/bitboard_calculator/Calc.html
 */
#define black_squares (bb_t)0xAA55AA55AA55AA55
#define white_squares (bb_t)0x55AA55AA55AA55AA

/**
 * Bitboards of pawn shields for each color and position.
 */
extern bb_t pawn_shields[COLOR_CNT][SQ_CNT];

/**
 * Initializes the pawn shields bitboard.
 */
extern void init_shields();

/**
 *  Values of each piece. Used in score calculation.
 */
enum piece_value {
    PAWN_VALUE      = 100,
    KNIGHT_VALUE    = 350, // 320,
    BISHOP_VALUE    = 375, // 330,
    ROOK_VALUE      = 500, // 500,
    QUEEN_VALUE     = 1000, // 900
};

/**
 * Values representing bonus score.
 */
enum bonus {
    MOBILITY_BONUS          = 1,
    KING_PAWN_SHIELD_BONUS  = 7,
    BISHOP_PAIR_BONUS       = 10,
    ROOK_OPEN_FILE_BONUS    = 15
};

/**
 * Values representing a penalty to the overall score.
 */
enum penalty {
    ISOLATED_PAWN_PENALTY   = -30,
    DOUBLED_PAWN_PENALTY    = -25,
    BACKWARD_PAWN_PENALTY   = -20
};

extern enum piece_value get_piece_value(enum piece piece); // Returns the value of the given piece type.
extern int evaluate(struct board *board, enum color color); // Returns the score advantage of the given color in centipawns.

/**
 * Returns the number of legal moves available.
 * 
 * Used to determine the final MOBILITY_BONUS bonus score.
 */
extern int get_mobility(struct board *board, enum color color);

/**
 * Returns the number of pawns shielding the king of the given color.
 * 
 * Used to determine the final KIGN_PAWN_SHIELD_BONUS bonus score.
 */
extern int get_pawns_shielding_king(struct board *board, enum color color);

/**
 * Returns true if the player with the color `color` has at least one
 * bishop on a WHITE square and at least one bishop on a BLACK SQUARE.
 * 
 * Used to determine whether the BISHOP_PAIR_BONUS applies or not.
 */
extern bool has_bishop_pair(struct board *board, enum color color);

/**
 * Returns the number of rooks of the given color on open files.
 * 
 * Used to determine the final ROOK_OPEN_FILE_BONUS bonus score.
 */
extern int get_rooks_on_open_files(struct board *board, enum color color);

/**
 * Returns the number of isolated pawns of the given color.
 * 
 * Used to determine the final ISOLATED_PAWN_PENALTY penalty to the score.
 */
extern int get_isolated_pawns(struct board *board, enum color color);

/**
 * Returns the number of doubled pawns of the given color.
 * 
 * Used to determine the final DOUBLED_PAWN_PENALTY penalty to the score.
 * 
 * Double pawns are defined as two or more pawns on a single file.
 * For example if FILE_B has one pawn - then it has 0 double pawns, if it has
 * 2 pawns - then it has 1 double pawn, if it has 3 pawns - then it has 2 double
 * pawns, etc.
 */
extern int get_doubled_pawns(struct board *board, enum color color);

/**
 * Return the number of the backward pawns of the given color.
 * 
 * Used to determine the final BACKWARD_PAWN_PENALTY penalty to the score.
 * 
 * Backward pawns are pawns that are behind all the other pawns of the same color
 * on adjacent files and cannot be safely advanced.
 */
extern int get_backward_pawns(struct board *board, enum color color);

#endif // EVAL_H
