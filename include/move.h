#ifndef MOVE_H
#define MOVE_H

/**
 * Value representing a move flag.
 */
enum move_flag {
    MOVE_FLAG_NULL             = 1 << 0,
    MOVE_FLAG_CAPTURE          = 1 << 1,
    MOVE_FLAG_PAWN_DOUBLE_PUSH = 1 << 2,
    MOVE_FLAG_KING_CASTLE      = 1 << 3,
    MOVE_FLAG_QUEEN_CASTLE     = 1 << 4,
    MOVE_FLAG_EN_PASSANT       = 1 << 5,
    MOVE_FLAG_PROMOTION        = 1 << 6,

    MOVE_FLAG_NONE = 0,
    MOVE_FLAG_INVALID = (1 << 7)-1,
};

/**
 * Structure represeting a move.
 */
struct move {
    enum move_flag flags   : 7;
    unsigned int from      : 6;
    unsigned int to        : 6;
    unsigned int piece     : 3;
    unsigned int capture   : 3;
    unsigned int promotion : 3;
    int score;
};

#endif // MOVE_H
