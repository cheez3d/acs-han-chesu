#ifndef BOARD_H
#define BOARD_H

#include "move.h"

#include <assert.h>
#include <stdbool.h>
#include <string.h>

/**
 * Value representing a rank on the board.
 */
enum rank {
    RK_1, RK_2, RK_3, RK_4, RK_5, RK_6, RK_7, RK_8,

    RK_CNT, // number of ranks
};

/**
 * Convert rank into a char representation.
 */
static inline char rank_to_char(enum rank r) {
    assert(r >= 0 && r < RK_CNT);

    return "12345678"[r];
}

/**
 * Convert a char representing a rank to rank.
 */
static inline enum rank char_to_rank(char c) {
    assert(c >= rank_to_char(RK_1) && c <= rank_to_char(RK_8));

    return c-rank_to_char(RK_1);
}

/**
 *  Value representing a file on the board.
 */
enum file {
    FL_A, FL_B, FL_C, FL_D, FL_E, FL_F, FL_G, FL_H,

    FL_CNT, // number of files
};

/**
 * Convert file into a char representation.
 */
static inline char file_to_char(enum file f) {
    assert(f >= 0 && f < FL_CNT);

    return "abcdefgh"[f];
}

/**
 * Convert a char representing a file to file.
 */
static inline enum file char_to_file(char c) {
    assert(c >= file_to_char(FL_A) && c <= file_to_char(FL_H));

    return c-file_to_char(FL_A);
}

/**
 *  Value representing a square on the board.
 */
enum square {
    /**
     * 56 57 58 59 60 61 62 63
     * 48 49 50 51 52 53 54 55
     * 40 41 42 43 44 45 46 47
     * 32 33 34 35 36 37 38 39
     * 24 25 26 27 28 29 30 31
     * 16 17 18 19 20 21 22 23
     *  8  9 10 11 12 13 14 15
     *  0  1  2  3  4  5  6  7
     */

    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,

    SQ_CNT, // number of squares

    SQ_NONE = -1,
};

/**
 * Infer rank from square.
 */
static inline enum rank square_to_rank(enum square s) {
    assert(s >= 0 && s < SQ_CNT);

    return s/RK_CNT;
}

/**
 * Infer file from square.
 */
static inline enum file square_to_file(enum square s) {
    assert(s >= 0 && s < SQ_CNT);

    return s%FL_CNT;
}

/**
 * Convert square to its string representation in coordinate notation.
 */
static inline const char * square_to_str(enum square s) {
    if (s == SQ_NONE) {
        return "-";
    }

    assert(s >= 0 && s < SQ_CNT);

    static const char *square_strs[SQ_CNT] = {
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    };

    return square_strs[s];
}

/**
 * Convert position from (rank, file) to square.
 */
static inline enum square rank_file_to_square(enum rank r, enum file f) {
    assert(r >= 0 && r < RK_CNT);
    assert(f >= 0 && f < FL_CNT);

    return RK_CNT*r+f;
}

/**
 * Convert a string representing a sqaure to square.
 */
static inline enum square str_to_square(const char *str) {
    if (str[0] == '-') {
        return SQ_NONE;
    }

    assert(str != NULL);
    assert(strlen(str) >= 2);
    assert(str[0] >= file_to_char(FL_A) && str[0] <= file_to_char(FL_H));
    assert(str[1] >= rank_to_char(RK_1) && str[1] <= rank_to_char(RK_8));

    enum file f = char_to_file(str[0]);
    enum rank r = char_to_rank(str[1]);

    return rank_file_to_square(r, f);
}

/**
 * Value representing piece color.
 */
enum color {
    WHITE, BLACK,
    
    COLOR_CNT, // number of piece colors

    COLOR_NONE = -1,
};

/**
 * Flip the given color.
 */
static inline enum color color_flip(enum color c) {
    assert(c >= 0 && c < COLOR_CNT);

    return c == WHITE ? BLACK : WHITE;
}

/**
 * Convert color into its char representation.
 */
static inline char color_to_char(enum color c) {
    if (c == COLOR_NONE) {
        return '-';
    }

    assert(c >= 0 && c < COLOR_CNT);

    return "wb"[c];
}

/**
 * Convert a char representing a piece to its color.
 */
static inline enum color char_to_color(char c) {
    switch (c) {
    case 'w':
    case 'R':
    case 'N':
    case 'B':
    case 'Q':
    case 'K':
    case 'P':
        return WHITE;

    case 'b':
    case 'r':
    case 'n':
    case 'q':
    case 'k':
    case 'p':
        return BLACK;

    default:
        return COLOR_NONE;
    }
}

/**
 * Value representing piece type.
 */
enum piece {
    ROOK, KNIGHT, BISHOP, QUEEN, KING, PAWN,

    PIECE_CNT, // number of piece types

    PIECE_NONE = -1,
};

/**
 * Convert piece into a char representation.
 */
static inline char piece_to_char(enum color c, enum piece p) {
    if (c == COLOR_NONE || p == PIECE_NONE) {
        return '-';
    }

    assert(c >= 0 && c < COLOR_CNT);
    assert(p >= 0 && p < PIECE_CNT);

    return (char *[]){"RNBQKP", "rnbqkp"}[c][p];
}

/**
 * Convert char representing a piece into a piece.
 */
static inline enum piece char_to_piece(char c) {
    switch (c) {
    case 'R':
    case 'r':
        return ROOK;

    case 'N':
    case 'n':
        return KNIGHT;

    case 'B':
    case 'b':
        return BISHOP;

    case 'Q':
    case 'q':
        return QUEEN;

    case 'K':
    case 'k':
        return KING;

    case 'P':
    case 'p':
        return PAWN;

    default:
        return PIECE_NONE;
    }
}

/**
 * Bits for representing castle rights.
 */
enum castle_right {
    CASTLE_RIGHT_WHITE_KING  = 1 << 0,
    CASTLE_RIGHT_WHITE_QUEEN = 1 << 1,
    CASTLE_RIGHT_BLACK_KING  = 1 << 2,
    CASTLE_RIGHT_BLACK_QUEEN = 1 << 3,

    CASTLE_RIGHT_WHITE = CASTLE_RIGHT_WHITE_KING | CASTLE_RIGHT_WHITE_QUEEN,
    CASTLE_RIGHT_BLACK = CASTLE_RIGHT_BLACK_KING | CASTLE_RIGHT_BLACK_QUEEN,

    CASTLE_RIGHT_NONE = 0,
    CASTLE_RIGHT_ALL = (1 << 4)-1,
};

#include "bitboard.h"

/**
 * Array index for accessing piece bitboard
 * for a particular color.
 */
enum bb_pieces_idx {
    BB_ROOKS   = ROOK,
    BB_KNIGHTS = KNIGHT,
    BB_BISHOPS = BISHOP,
    BB_QUEENS  = QUEEN,
    BB_KING    = KING,
    BB_PAWNS   = PAWN,

    BB_ALL, // index for accessing all pieces bitboard
            // (results from OR-ing together bitboards at indexes above)

    BB_PIECES_SZ, // array size
};

struct board {
    bb_t bb_pieces[COLOR_CNT][BB_PIECES_SZ]; // array for all bitboards used to store board state
    int pst_scores[COLOR_CNT];

    enum color color; // color that is on move

    enum castle_right castle_rights; // bitfield for storing castle rights

    enum square en_passant; // en passant target square

    size_t halfmove_clock; // number of plies since last capture or pawn move
    size_t fullmove_number; // number of full moves since game start
};

/**
 * Reset the board to the initial state.
 */
void board_reset(struct board *board);

/**
 * Initialize board state from a FEN string.
 */
void board_set_fen(struct board *board, const char *fen);

/**
 * Verify if the provided square is under attack from the
 * perspective of the specified color.
 */
bool board_square_under_attack(struct board *board, enum color c, enum square s);

/**
 * Verify if the provided color is currently in check.
 */
bool board_color_in_check(struct board *board, enum color c);

/**
 * Check if the specified color can castle king side.
 */
bool board_color_castle_king(struct board *board, enum color c);

/**
 * Check if the specified color can castle queen side.
 */
bool board_color_castle_queen(struct board *board, enum color c);

/**
 * Get an attacks bitboard given a piece type, its color and square,
 * taking into consideration the current game table state.
 */
bb_t board_get_attacks(struct board *board, enum color c, enum piece p, enum square s);

/**
 * Execute the provided move on the board for the active color.
 */
void board_do_move(struct board *board, struct move move);

/**
 * Print a visual representation of the board for debugging purposes.
 */
void board_print(struct board *board);

/**
 * Print a fancy visual representation of the board for debugging purposes.
 */
void board_print_fancy(struct board *board);

#endif // BOARD_H
