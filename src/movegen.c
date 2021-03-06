#include "movegen.h"

#include "board.h"
#include "engine.h"
#include "move.h"
#include "bitboard.h"

#include <string.h>

static void movegen_add_move_if_legal(struct move_list *moves, struct move move, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    struct board board_copy = *board;
    board_do_move(&board_copy, move);

    if (!board_color_in_check(&board_copy, board->color)) {
        assert(moves->count < MOVE_LIST_MAX);

        moves->list[moves->count++] = move;
    }
}

static void movegen_create_moves(struct move_list *moves,
                                 enum move_flag flags,
                                 enum square from, enum square to,
                                 enum piece piece,
                                 struct board *board)
{
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;
    enum color color_other = color_flip(color);

    struct move move;

    move.flags = flags;
    move.from = from;
    move.to = to;
    move.piece = piece;
    move.score = 0;

    if (bb_squares[to] & board->bb_pieces[color_other][BB_ALL]) {
        move.flags |= MOVE_FLAG_CAPTURE;

        for (enum piece p = 0; p < PIECE_CNT; ++p) {
            if (bb_squares[to] & board->bb_pieces[color_other][p]) {
                move.capture = p;
                break;
            }
        }
    }

    if (move.flags & MOVE_FLAG_PROMOTION) {
        for (enum piece p = 0; p < PIECE_CNT; ++p) {
            if (p == KING || p == PAWN) {
                continue;
            }

            move.promotion = p;

            movegen_add_move_if_legal(moves, move, board);
        }
    } else {
        movegen_add_move_if_legal(moves, move, board);
    }
}

static void movegen_add_rook_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;

    bb_t bb_rooks = board->bb_pieces[color][BB_ROOKS];

    while (bb_rooks) {
        enum square from = bb_pop_lsb(&bb_rooks);

        bb_t bb_attacks = board_get_attacks(board, color, ROOK, from);

        while (bb_attacks) {
            enum square to = bb_pop_lsb(&bb_attacks);

            movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, ROOK, board);
        }
    }
}

static void movegen_add_knight_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;

    bb_t bb_knights = board->bb_pieces[color][BB_KNIGHTS];

    while (bb_knights) {
        enum square from = bb_pop_lsb(&bb_knights);

        bb_t bb_attacks = board_get_attacks(board, color, KNIGHT, from);

        while (bb_attacks) {
            enum square to = bb_pop_lsb(&bb_attacks);

            movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, KNIGHT, board);
        }
    }
}

static void movegen_add_bishop_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;

    bb_t bb_bishops = board->bb_pieces[color][BB_BISHOPS];

    while (bb_bishops) {
        enum square from = bb_pop_lsb(&bb_bishops);

        bb_t bb_attacks = board_get_attacks(board, color, BISHOP, from);

        while (bb_attacks) {
            enum square to = bb_pop_lsb(&bb_attacks);

            movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, BISHOP, board);
        }
    }
}

static void movegen_add_queen_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;

    bb_t bb_queens = board->bb_pieces[color][BB_QUEENS];

    while (bb_queens) {
        enum square from = bb_pop_lsb(&bb_queens);

        bb_t bb_attacks = board_get_attacks(board, color, QUEEN, from);

        while (bb_attacks) {
            enum square to = bb_pop_lsb(&bb_attacks);

            movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, QUEEN, board);
        }
    }
}

static void movegen_add_king_castles(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;

    bb_t bb_king = board->bb_pieces[color][BB_KING];

    enum square ks = bb_pop_lsb(&bb_king);

    if (ks == SQ_NONE) {
        return;
    }

    if (board_color_castle_king(board, color)) {
        movegen_create_moves(moves, MOVE_FLAG_KING_CASTLE, ks, ks+2, KING, board);
    }

    if (board_color_castle_queen(board, color)) {
        movegen_create_moves(moves, MOVE_FLAG_QUEEN_CASTLE, ks, ks-2, KING, board);
    }
}

static void movegen_add_king_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;

    bb_t bb_king = board->bb_pieces[color][BB_KING];

    enum square from = bb_pop_lsb(&bb_king);

    if (from == SQ_NONE) {
        return;
    }

    bb_t bb_attacks = board_get_attacks(board, color, KING, from);

    while (bb_attacks) {
        enum square to = bb_pop_lsb(&bb_attacks);

        movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, KING, board);
    }

    movegen_add_king_castles(moves, board);
}

static void movegen_add_pawn_pushes(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;
    enum color color_other = color_flip(color);

    bb_t bb_occ = board->bb_pieces[color][BB_ALL]
                | board->bb_pieces[color_other][BB_ALL];

    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];

    bb_t bb_pushes = (color == WHITE ? bb_pawns << FL_CNT : bb_pawns >> FL_CNT) & ~bb_occ;

    bb_t bb_promotions = bb_pushes & bb_ranks[color == WHITE ? RK_8 : RK_1];

    bb_pushes &= ~bb_promotions;

    while (bb_pushes) {
        enum square to = bb_pop_lsb(&bb_pushes);
        enum square from = to+(color == WHITE ? -FL_CNT : +FL_CNT);

        movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, PAWN, board);
    }

    while (bb_promotions) {
        enum square to = bb_pop_lsb(&bb_promotions);
        enum square from = to+(color == WHITE ? -FL_CNT : +FL_CNT);

        movegen_create_moves(moves, MOVE_FLAG_PROMOTION, from, to, PAWN, board);
    }
}

static void movegen_add_pawn_double_pushes(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;
    enum color color_other = color_flip(color);

    bb_t bb_occ = board->bb_pieces[color][BB_ALL]
                | board->bb_pieces[color_other][BB_ALL];

    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];

    bb_t bb_pushes = (color == WHITE ? bb_pawns << FL_CNT : bb_pawns >> FL_CNT) & ~bb_occ;
    bb_t bb_double_pushes = (color == WHITE ? bb_pushes << FL_CNT : bb_pushes >> FL_CNT) & ~bb_occ;

    bb_double_pushes &= bb_ranks[color == WHITE ? RK_4 : RK_5];

    while(bb_double_pushes) {
        enum square to = bb_pop_lsb(&bb_double_pushes);
        enum square from = to+(color == WHITE ? -2*FL_CNT : +2*FL_CNT);

        movegen_create_moves(moves, MOVE_FLAG_PAWN_DOUBLE_PUSH, from, to, PAWN, board);
    }
}

static void movegen_add_pawn_attacks(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    enum color color = board->color;
    enum color color_other = color_flip(color);

    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];

    while (bb_pawns) {
        enum square from = bb_pop_lsb(&bb_pawns);

        bb_t bb_attacks = board_get_attacks(board, color, PAWN, from);

        bb_attacks &= board->bb_pieces[color_other][BB_ALL];

        bb_t bb_promotions = bb_attacks & bb_ranks[color == WHITE ? RK_8 : RK_1];
        
        bb_attacks &= ~bb_ranks[color == WHITE ? RK_8 : RK_1];

        while (bb_attacks) {
            enum square to = bb_pop_lsb(&bb_attacks);

            movegen_create_moves(moves, MOVE_FLAG_NONE, from, to, PAWN, board);
        }

        while (bb_promotions) {
            enum square to = bb_pop_lsb(&bb_promotions);

            movegen_create_moves(moves, MOVE_FLAG_PROMOTION, from, to, PAWN, board);
        }
    }

    if (board->en_passant != SQ_NONE) {
        enum square to = board->en_passant;

        enum square from;

        from = to+(color == WHITE ? -(FL_CNT+1) : (FL_CNT-1));

        bb_t bb_en_passant_left = board->bb_pieces[color][BB_PAWNS] & bb_squares[from];

        bb_en_passant_left &= ~bb_files[FL_H];

        if (bb_en_passant_left) {
            movegen_create_moves(moves, MOVE_FLAG_EN_PASSANT, from, to, PAWN, board);
        }

        from = to+(color == WHITE ? -(FL_CNT-1) : +(FL_CNT+1));

        bb_t bb_en_passant_right = board->bb_pieces[color][BB_PAWNS] & bb_squares[from];

        bb_en_passant_right &= ~bb_files[FL_A];

        if (bb_en_passant_right) {
            movegen_create_moves(moves, MOVE_FLAG_EN_PASSANT, from, to, PAWN, board);
        }
    }
}

static void movegen_add_pawn_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    movegen_add_pawn_pushes(moves, board);
    movegen_add_pawn_double_pushes(moves, board);
    movegen_add_pawn_attacks(moves, board);
}

void movegen_add_moves(struct move_list *moves, struct board *board) {
    assert(moves != NULL);
    assert(board != NULL);

    moves->count = 0;

    movegen_add_rook_moves(moves, board);
    movegen_add_knight_moves(moves, board);
    movegen_add_bishop_moves(moves, board);
    movegen_add_queen_moves(moves, board);
    movegen_add_king_moves(moves, board);
    movegen_add_pawn_moves(moves, board);
}
