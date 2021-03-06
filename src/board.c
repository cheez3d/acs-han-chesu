#include "board.h"

#include "bitboard.h"
#include "engine.h"
#include "move.h"
#include "pst.h"
#include "xboard.h"

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Remove all pieces from the board.
 */
static void board_clear(struct board *board) {
    assert(board != NULL);

    for (enum color c = 0; c < COLOR_CNT; ++c) {
        for (enum bb_pieces_idx i = 0; i < BB_PIECES_SZ; ++i) {
            board->bb_pieces[c][i] = BB_EMPTY;
        }

        board->pst_scores[c] = 0;
    }
}

/**
 * Add a piece on the board.
 */
static void board_add_piece(struct board *board, enum color c, enum piece p, enum square s) {
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);
    assert(p >= 0 && p < PIECE_CNT);
    assert(s >= 0 && s < SQ_CNT);

    assert(!(board->bb_pieces[c][BB_ALL] & bb_squares[s]));

    board->bb_pieces[c][p] |= bb_squares[s];
    board->bb_pieces[c][BB_ALL] |= bb_squares[s];

    board->pst_scores[c] += pst_values[c][p][s];
}

/**
 * Remove a piece from the board.
 */
static void board_remove_piece(struct board *board, enum color c, enum piece p, enum square s) {
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);
    assert(p >= 0 && p < PIECE_CNT);
    assert(s >= 0 && s < SQ_CNT);

    // xb_commentln("board_remove_piece(%c, %c, %s)", color_to_char(c), piece_to_char(c, p), square_to_str(s));

    assert(board->bb_pieces[c][p] & bb_squares[s]);

    board->bb_pieces[c][p] ^= bb_squares[s];
    board->bb_pieces[c][BB_ALL] ^= bb_squares[s];

    board->pst_scores[c] -= pst_values[c][p][s];
}

/**
 * Move a piece on the board.
 */
static void board_move_piece(struct board *board,
                             enum color c, enum piece p,
                             enum square from, enum square to)
{
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);
    assert(p >= 0 && p < PIECE_CNT);
    assert(from >= 0 && from < SQ_CNT);
    assert(to >= 0 && to < SQ_CNT);

    assert(board->bb_pieces[c][p] & bb_squares[from]);
    assert(!(board->bb_pieces[c][BB_ALL] & bb_squares[to]));

    bb_t bb_mask = bb_squares[from] | bb_squares[to];

    board->bb_pieces[c][p] ^= bb_mask;
    board->bb_pieces[c][BB_ALL] ^= bb_mask;

    board->pst_scores[c] += pst_values[c][p][to]-pst_values[c][p][from];
}

void board_reset(struct board *board) {
    assert(board != NULL);

    board_clear(board);

    for (enum color c = 0; c < COLOR_CNT; ++c) {
        for (enum piece p = 0; p < PIECE_CNT; ++p) {
            for (enum square s = 0; s < SQ_CNT; ++s) {
                if (bb_pieces[c][p] & bb_squares[s]) {
                    board_add_piece(board, c, p, s);
                }
            }
        }
    }

    board->color = WHITE;

    board->castle_rights = CASTLE_RIGHT_ALL;

    board->en_passant = SQ_NONE;

    board->halfmove_clock = 0;
    board->fullmove_number = 1;
}

void board_set_fen(struct board *board, const char *fen) {
    assert(board != NULL);
    assert(fen != NULL);

    char *positions = NULL;
    char color;
    char *castle_rights = NULL;
    char *en_passant = NULL;
    size_t halfmove_clock;
    size_t fullmove_number;

    if (sscanf(fen, "%ms %c %ms %ms %zu %zu",
               &positions, &color, &castle_rights,
               &en_passant, &halfmove_clock, &fullmove_number) < 6)
    {
        error(EXIT_FAILURE, errno, "could not parse FEN string '%s'", fen);
    }

    board_clear(board);

    enum square s = SQ_A8;

    for (size_t i = 0; positions[i] != '\0'; ++i) {
        enum color c = char_to_color(positions[i]);
        enum piece p = char_to_piece(positions[i]);

        if (c == COLOR_NONE || p == PIECE_NONE) {
            if (positions[i] == '/') {
                s -= 16;
            } else {
                s += positions[i]-'0';
            }

            continue;
        }

        board_add_piece(board, c, p, s++);
    }

    free(positions);

    board->color = char_to_color(color);

    board->castle_rights = CASTLE_RIGHT_NONE;

    for (size_t i = 0; castle_rights[i] != '\0'; ++i) {
        switch (castle_rights[i]) {
        case 'K':
            board->castle_rights |= CASTLE_RIGHT_WHITE_KING;
            break;

        case 'Q':
            board->castle_rights |= CASTLE_RIGHT_WHITE_QUEEN;
            break;

        case 'k':
            board->castle_rights |= CASTLE_RIGHT_BLACK_KING;
            break;

        case 'q':
            board->castle_rights |= CASTLE_RIGHT_BLACK_QUEEN;
            break;
        }
    }

    free(castle_rights);

    board->en_passant = str_to_square(en_passant);

    free(en_passant);

    board->halfmove_clock = halfmove_clock;
    board->fullmove_number = fullmove_number;

    board_print_fancy(board);
}

bool board_square_under_attack(struct board *board, enum color c, enum square s) {
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);
    assert(s >= 0 && s < SQ_CNT);

    for (enum piece p = 0; p < PIECE_CNT; ++p) {
        bb_t bb_pieces = board->bb_pieces[color_flip(c)][p];

        while (bb_pieces) {
            enum square from = bb_pop_lsb(&bb_pieces);

            bb_t bb_attacks = board_get_attacks(board, color_flip(c), p, from);

            if (bb_squares[s] & bb_attacks) {
                return true;
            }
        }
    }

    return false;
}

bool board_color_in_check(struct board *board, enum color c) {
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);

    bb_t bb_king = board->bb_pieces[c][BB_KING];

    enum square s = bb_pop_lsb(&bb_king);
    
    if (s == SQ_NONE) {
        return false;
    }

    return board_square_under_attack(board, c, s);
}

bool board_color_castle_king(struct board *board, enum color c) {
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);

    if (!(board->castle_rights & (c == WHITE ? CASTLE_RIGHT_WHITE_KING : CASTLE_RIGHT_BLACK_KING))) {
        return false;
    }

    if (board_color_in_check(board, c)) {
        return false;
    }

    enum square ks = c == WHITE ? SQ_E1 : SQ_E8;

    bb_t bb_occ = board->bb_pieces[c][BB_ALL]
                | board->bb_pieces[color_flip(c)][BB_ALL];

    bb_t bb_castle_path = bb_squares[ks+1] | bb_squares[ks+2];

    if (bb_castle_path & bb_occ) {
        return false;
    }

    if (board_square_under_attack(board, c, ks+1) ||
        board_square_under_attack(board, c, ks+2))
    {
        return false;
    }

    return true;
}

bool board_color_castle_queen(struct board *board, enum color c) {
    assert(board != NULL);
    assert(c >= 0 && c < COLOR_CNT);

    if (!(board->castle_rights & (c == WHITE ? CASTLE_RIGHT_WHITE_QUEEN : CASTLE_RIGHT_BLACK_QUEEN))) {
        return false;
    }

    if (board_color_in_check(board, c)) {
        return false;
    }

    enum square ks = c == WHITE ? SQ_E1 : SQ_E8;

    bb_t bb_occ = board->bb_pieces[c][BB_ALL]
                | board->bb_pieces[color_flip(c)][BB_ALL];

    bb_t bb_castle_path = bb_squares[ks-1] | bb_squares[ks-2] | bb_squares[ks-3];

    if (bb_castle_path & bb_occ) {
        return false;
    }

    if (board_square_under_attack(board, c, ks-1) ||
        board_square_under_attack(board, c, ks-2) ||
        board_square_under_attack(board, c, ks-3))
    {
        return false;
    }

    return true;
}

bb_t board_get_attacks(struct board *board, enum color c, enum piece p, enum square s) {
    bb_t bb_occ = p == ROOK || p == BISHOP || p == QUEEN
                ? board->bb_pieces[c][BB_ALL] | board->bb_pieces[color_flip(c)][BB_ALL]
                : BB_EMPTY;

    return bb_get_attacks(c, p, s, bb_occ) & ~board->bb_pieces[c][BB_ALL];
}

void board_do_move(struct board *board, struct move move) {
    assert(board != NULL);

    enum color color = board->color;
    enum color color_other = color_flip(color);

    if (move.flags & MOVE_FLAG_CAPTURE) {
        board_remove_piece(board, color_other, move.capture, move.to);

        if (move.capture == ROOK) {
            if (move.to == (color == WHITE ? SQ_H8 : SQ_H1)) {
                board->castle_rights &= ~(color == WHITE ? CASTLE_RIGHT_BLACK_KING : CASTLE_RIGHT_WHITE_KING);
            } else if (move.to == (color == WHITE ? SQ_A8 : SQ_A1)) {
                board->castle_rights &= ~(color == WHITE ? CASTLE_RIGHT_BLACK_QUEEN : CASTLE_RIGHT_WHITE_QUEEN);
            }
        }
    }

    board_move_piece(board, color, move.piece, move.from, move.to);

    if (move.flags & MOVE_FLAG_PAWN_DOUBLE_PUSH) {
        board->en_passant = move.to+(color == WHITE ? -FL_CNT : + FL_CNT);
    } else if (move.flags & MOVE_FLAG_KING_CASTLE) {
        board_move_piece(board, color, ROOK, color == WHITE ? SQ_H1 : SQ_H8, move.to-1);
    } else if (move.flags & MOVE_FLAG_QUEEN_CASTLE) {
        board_move_piece(board, color, ROOK, color == WHITE ? SQ_A1 : SQ_A8, move.to+1);
    } else if (move.flags & MOVE_FLAG_EN_PASSANT && board->en_passant != SQ_NONE) {
        board_remove_piece(board, color_other, PAWN, move.to+(color == WHITE ? -FL_CNT : +FL_CNT));
    } else if (move.flags & MOVE_FLAG_PROMOTION) {
        board_remove_piece(board, color, move.piece, move.to);
        board_add_piece(board, color, move.promotion, move.to);
    }

    if (move.piece == ROOK) {
        if (move.from == (color == WHITE ? SQ_H1 : SQ_H8)) {
            board->castle_rights &= ~(color == WHITE ? CASTLE_RIGHT_WHITE_KING : CASTLE_RIGHT_BLACK_KING);
        } else if (move.from == (color == WHITE ? SQ_A1 : SQ_A8)) {
            board->castle_rights &= ~(color == WHITE ? CASTLE_RIGHT_WHITE_QUEEN : CASTLE_RIGHT_BLACK_QUEEN);
        }
    } else if (move.piece == KING) {
        board->castle_rights &= ~(color == WHITE ? CASTLE_RIGHT_WHITE : CASTLE_RIGHT_BLACK);
    }

    if (!(move.flags & MOVE_FLAG_PAWN_DOUBLE_PUSH) && board->en_passant != SQ_NONE) {
        board->en_passant = SQ_NONE;
    }

    if (move.flags & MOVE_FLAG_CAPTURE || move.piece == PAWN) {
        board->halfmove_clock = 0;
    } else {
        ++board->halfmove_clock;
    }

    if (color == BLACK) {
        ++board->fullmove_number;
    }

    board->color = color_other;
}

void board_print(struct board *board) {
    assert(board != NULL);

    for (enum rank r = RK_CNT; r-- > 0; ) {
        for (enum file f = 0; f < FL_CNT; ++f) {
            enum square s = rank_file_to_square(r, f);

            char piece_char = '.';

            for (enum color c = 0; c < COLOR_CNT; ++c) {
                for (enum piece p = 0; p < PIECE_CNT; ++p) {
                    if (bb_squares[s] & board->bb_pieces[c][p]) {
                        piece_char = piece_to_char(c, p);

                        goto found;
                    }
                }
            }

        found:
            xb_comment("%c ", piece_char);
        }

        xb_comment("%c\n", rank_to_char(r));
    }

    for (enum file f = 0; f < FL_CNT; ++f) {
        xb_comment("%c ", file_to_char(f));
    }

    xb_comment("\n");

    xb_comment("%c " , color_to_char(board->color));

    xb_comment(
        "%c%c%c%c ",
        piece_to_char(WHITE, board->castle_rights & CASTLE_RIGHT_WHITE_KING ? KING : PIECE_NONE),
        piece_to_char(WHITE, board->castle_rights & CASTLE_RIGHT_WHITE_QUEEN ? QUEEN : PIECE_NONE),
        piece_to_char(BLACK, board->castle_rights & CASTLE_RIGHT_BLACK_KING ? KING : PIECE_NONE),
        piece_to_char(BLACK, board->castle_rights & CASTLE_RIGHT_BLACK_QUEEN ? QUEEN : PIECE_NONE)
    );

    xb_comment("%s ", square_to_str(board->en_passant));

    xb_comment("%zu ", board->halfmove_clock);

    xb_comment("%zu",  board->fullmove_number);

    xb_comment("\n");

    for (enum color c = 0; c < COLOR_CNT; ++c) {
        xb_comment("%d ", board->pst_scores[c]);
    }

    xb_comment("\n");
}

void board_print_fancy(struct board *board) {
    assert(board != NULL);

    for (enum rank r = RK_CNT; r-- > 0; ) {
        xb_comment("+---+---+---+---+---+---+---+---+   \n");

        for (enum file f = 0; f < FL_CNT; ++f) {
            enum square s = rank_file_to_square(r, f);

            char piece_char = ' ';

            for (enum color c = 0; c < COLOR_CNT; ++c) {
                for (enum piece p = 0; p < PIECE_CNT; ++p) {
                    if (bb_squares[s] & board->bb_pieces[c][p]) {
                        piece_char = piece_to_char(c, p);

                        goto found;
                    }
                }
            }

        found:
            xb_comment("| %c ", piece_char);
        }

        xb_comment("| %c \n", rank_to_char(r));
    }

    xb_comment("+---+---+---+---+---+---+---+---+   \n");

    for (enum file f = 0; f < FL_CNT; ++f) {
        xb_comment("  %c ", file_to_char(f));
    }

    xb_comment("    \n");

    xb_comment("+---+------+----+-------+-------+\n");

    xb_comment("| %c ", color_to_char(board->color));

    xb_comment(
        "| %c%c%c%c ",
        piece_to_char(WHITE, board->castle_rights & CASTLE_RIGHT_WHITE_KING ? KING : PIECE_NONE),
        piece_to_char(WHITE, board->castle_rights & CASTLE_RIGHT_WHITE_QUEEN ? QUEEN : PIECE_NONE),
        piece_to_char(BLACK, board->castle_rights & CASTLE_RIGHT_BLACK_KING ? KING : PIECE_NONE),
        piece_to_char(BLACK, board->castle_rights & CASTLE_RIGHT_BLACK_QUEEN ? QUEEN : PIECE_NONE)
    );

    xb_comment("| %-2s ", square_to_str(board->en_passant));

    xb_comment("| %-5zu ", board->halfmove_clock);

    xb_comment("| %-5zu ",  board->fullmove_number);

    xb_comment("|\n");

    xb_comment("+---+------+----+-------+-------+\n");

    for (enum color c = 0; c < COLOR_CNT; ++c) {
        xb_comment("| %-13d ", board->pst_scores[c]);
    }

    xb_comment("|\n");

    xb_comment("+---------------+---------------+\n");
}
