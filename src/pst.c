#include "pst.h"
#include "engine.h"

int pst_values[COLOR_CNT][PIECE_CNT][SQ_CNT];

static void arr_rev_copy(void *arr_copy, void *arr, size_t s, size_t c) {
    for (size_t i = 0; i < c; ++i) {
        memcpy(arr_copy + i*s, arr + (c - 1 - i) * s, s);
    }
}

/**
 * Values taken from https://www.chessprogramming.org/Simplified_Evaluation_Function
 */
void init_pst(void){
    static int pst_pawn[SQ_CNT] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5, 10, 25, 25, 10,  5,  5,
    0,  0,  0, 20, 20,  0,  0,  0,
    5, -5,-10,  0,  0,-10, -5,  5,
    5, 10, 10,-20,-20, 10, 10,  5,
    0,  0,  0,  0,  0,  0,  0,  0};
    static int pst_knight[SQ_CNT] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
    };
    static int pst_bishop[SQ_CNT] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
    };
    static int pst_rook[SQ_CNT] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    5, 10, 10, 10, 10, 10, 10,  5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
   -5,  0,  0,  0,  0,  0,  0, -5,
    0,  0,  0,  5,  5,  0,  0,  0
    };
    static int pst_queen[SQ_CNT] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
    };
    static int pst_king[SQ_CNT] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
    };

    memcpy(pst_values[WHITE][PAWN], pst_pawn, sizeof(pst_pawn));
    memcpy(pst_values[WHITE][KNIGHT], pst_knight, sizeof(pst_knight));
    memcpy(pst_values[WHITE][BISHOP], pst_bishop, sizeof(pst_bishop));
    memcpy(pst_values[WHITE][ROOK], pst_rook, sizeof(pst_rook));
    memcpy(pst_values[WHITE][QUEEN], pst_queen, sizeof(pst_queen));
    memcpy(pst_values[WHITE][KING], pst_king, sizeof(pst_king));

    arr_rev_copy(pst_values[BLACK][PAWN], pst_pawn, sizeof *pst_pawn, sizeof pst_pawn/sizeof *pst_pawn);
    arr_rev_copy(pst_values[BLACK][KNIGHT], pst_knight, sizeof *pst_knight, sizeof pst_knight/sizeof *pst_knight);
    arr_rev_copy(pst_values[BLACK][BISHOP], pst_bishop, sizeof *pst_bishop, sizeof pst_bishop/sizeof *pst_bishop);
    arr_rev_copy(pst_values[BLACK][ROOK], pst_rook, sizeof *pst_rook, sizeof pst_rook/sizeof *pst_rook);
    arr_rev_copy(pst_values[BLACK][QUEEN], pst_queen, sizeof *pst_queen, sizeof pst_queen/sizeof *pst_queen);
    arr_rev_copy(pst_values[BLACK][KING], pst_king, sizeof *pst_king, sizeof pst_king/sizeof *pst_king);
}

void pst_add_piece(enum color color, enum piece piece, enum square square){
    pst_scores[color] += pst_values[color][piece][square];
}

void pst_remove_piece(enum color color, enum piece piece, enum square square){
    pst_scores[color] -= pst_values[color][piece][square];
}

void pst_move_piece(enum color color, enum piece piece, enum square from, enum square to){
    pst_remove_piece(color, piece, from);
    pst_add_piece(color, piece, to);
}

void init_pst_score(void){
    struct board *board = &engine.board;

    pst_scores[WHITE] = 0;
    pst_scores[BLACK] = 0;

    for(enum piece p = 0; p < PIECE_CNT; p++){
        bb_t bb_whites = board->bb_pieces[WHITE][p];
        bb_t bb_blacks = board->bb_pieces[BLACK][p];

        for(enum square sq = 0; sq < SQ_CNT; sq++){
            if(bb_squares[sq] & bb_whites)
                pst_add_piece(WHITE, p, sq);
            
            if(bb_squares[sq] & bb_blacks)
                pst_add_piece(BLACK, p, sq);
        }
    }
}

int get_pst_score(enum color color){
    return pst_scores[color];
}
