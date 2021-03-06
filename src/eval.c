#include "eval.h"
#include "engine.h"
#include "bitboard.h"

#include <assert.h>

bb_t adjacent_files[FL_CNT] = {
    [FL_A] = file_base << FL_A,
    [FL_B] = file_base << FL_A | file_base << FL_C,
    [FL_C] = file_base << FL_B | file_base << FL_D,
    [FL_D] = file_base << FL_C | file_base << FL_E,
    [FL_E] = file_base << FL_D | file_base << FL_F,
    [FL_F] = file_base << FL_E | file_base << FL_G,
    [FL_G] = file_base << FL_F | file_base << FL_H,
    [FL_H] = file_base << FL_G
};
bb_t pawn_shields[COLOR_CNT][SQ_CNT];

void init_shields(void){
    for(enum square sq = SQ_A1; sq < SQ_CNT; sq++){
        bb_t bb_square = bb_squares[sq];

        pawn_shields[WHITE][sq] = ((bb_square << 8) | ((bb_square << 7) & ~bb_files[FL_H])
                                | ((bb_square << 9) & ~bb_files[FL_A])) & bb_ranks[RK_2];
        pawn_shields[BLACK][sq] = ((bb_square >> 8) | ((bb_square >> 7) & ~bb_files[FL_A])
                                | ((bb_square >> 9) & ~bb_files[FL_H])) & bb_ranks[RK_7];
    }
}

enum piece_value get_piece_value(enum piece piece){
    assert(piece > PIECE_NONE && piece < PIECE_CNT);

    switch(piece){
        case PAWN:      return PAWN_VALUE;
        case KNIGHT:    return KNIGHT_VALUE;
        case BISHOP:    return BISHOP_VALUE;
        case ROOK:      return ROOK_VALUE;
        case QUEEN:     return QUEEN_VALUE;
        default:        return 0;
    }
}

int get_mobility(struct board *board, enum color color){
    assert(board != NULL);
    
    int total_mobility = 0;

    // Pawns have multiple move types so it's a special case.
    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];
    bb_t pawn_single_pushes = BB_EMPTY, pawn_double_pushes = BB_EMPTY, pawn_attacks = BB_EMPTY;
    bb_t bb_occ = board->bb_pieces[color][BB_ALL] | board->bb_pieces[color_flip(color)][BB_ALL];

    if(color == WHITE){
        pawn_single_pushes = (bb_pawns << 8) & ~bb_occ;
        pawn_double_pushes = ((pawn_single_pushes && bb_ranks[RK_3]) << 8) & ~bb_occ;
    }
    if(color == BLACK){
        pawn_single_pushes = (bb_pawns >> 8) & ~bb_occ;
        pawn_double_pushes = ((pawn_single_pushes && bb_ranks[RK_6]) >> 8) & ~bb_occ;
    }

    while(bb_pawns){
        enum square from = bb_pop_lsb(&bb_pawns);

        pawn_attacks |= bb_get_attacks(color, PAWN, from, BB_EMPTY);
    }

    // pawn_attacks &= ~bb_occ;
    pawn_attacks &= ~board->bb_pieces[color][BB_ALL];
    pawn_attacks &= board->bb_pieces[color_flip(color)][BB_ALL];

    total_mobility += bb_bit_cnt(pawn_single_pushes | pawn_double_pushes | pawn_attacks);

    // The rest of the pieces.
    for(enum piece p = 0; p < PAWN; p++){
        bb_t bb_piece = board->bb_pieces[color][p];

        while(bb_piece){
            enum square from = bb_pop_lsb(&bb_piece);

            bb_t bb_attacks = bb_get_attacks(color, p, from, bb_occ);
            bb_attacks &= ~board->bb_pieces[color][BB_ALL];

            total_mobility += bb_bit_cnt(bb_attacks);
        }
    }

    return total_mobility;
}

int get_pawns_shielding_king(struct board *board, enum color color){
    assert(board != NULL);

    bb_t bb_king = board->bb_pieces[color][BB_KING];
    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];

    enum square king_sq = bb_pop_lsb(&bb_king);
    
    return bb_bit_cnt(pawn_shields[color][king_sq] & bb_pawns);
}

bool has_bishop_pair(struct board *board, enum color color){
    assert(board != NULL);

    bool has_white_square_bishops, has_black_square_bishops;
    has_white_square_bishops = ((board->bb_pieces[color][BB_BISHOPS] & white_squares) != BB_EMPTY);
    has_black_square_bishops = ((board->bb_pieces[color][BB_BISHOPS] & black_squares) != BB_EMPTY);

    return (has_white_square_bishops && has_black_square_bishops);
}

int get_rooks_on_open_files(struct board *board, enum color color){
    assert(board != NULL);
    
    int rooks = 0;

    bb_t bb_rooks = board->bb_pieces[color][BB_ROOKS];
    bb_t bb_occ = board->bb_pieces[color][BB_ALL] | board->bb_pieces[color_flip(color)][BB_ALL];

    for(enum file file = FL_A; file < FL_CNT; file++){
        bb_t bb_file = bb_files[file];

        if((bb_file & bb_rooks) && ((bb_file & bb_rooks) == (bb_file & bb_occ)))
            rooks++;
    }

    return rooks;
}

int get_isolated_pawns(struct board *board, enum color color){
    assert(board != NULL);

    int isolated_pawns = 0;

    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];

    for(enum file file = FL_A; file < FL_CNT; file++){
        bb_t file_mask = bb_files[file];
        bb_t adjacent_files_mask = adjacent_files[file];

        if((file_mask & bb_pawns) && !(adjacent_files_mask & bb_pawns))
            isolated_pawns++;
    }

    return isolated_pawns;
}

int get_doubled_pawns(struct board *board, enum color color){
    assert(board != NULL);

    int doubled_pawns = 0;

    bb_t bb_pawns = board->bb_pieces[color][BB_PAWNS];

    for(enum file file = FL_A; file < FL_CNT; file++){
        int pawns_on_file = bb_bit_cnt(bb_files[file] & bb_pawns);

        if(pawns_on_file > 1)
            doubled_pawns += (pawns_on_file - 1);
    }

    return doubled_pawns;
}

int get_backward_pawns(struct board *board, enum color color){
    assert(board != NULL);

    bb_t bb_own_pawns = board->bb_pieces[color][BB_PAWNS];
    bb_t bb_op_pawns = board->bb_pieces[color_flip(color)][BB_PAWNS];
    bb_t bb_occ = board->bb_pieces[color][BB_ALL] | board->bb_pieces[color_flip(color)][BB_ALL];

    bb_t stop_squares = BB_EMPTY, own_pawn_attacks = BB_EMPTY, op_pawn_attacks = BB_EMPTY;

    if(color == WHITE){
        stop_squares = (bb_own_pawns << 8) & ~bb_occ;
        own_pawn_attacks = ((bb_own_pawns << 7) & ~bb_files[FL_H]) | ((bb_own_pawns << 9) & ~bb_files[FL_A]);
        op_pawn_attacks = ((bb_op_pawns >> 7) & ~bb_files[FL_A]) | ((bb_op_pawns >> 9) & ~bb_files[FL_H]);
    }
    if(color == BLACK){
        stop_squares = (bb_own_pawns >> 8) & ~bb_occ;
        own_pawn_attacks = ((bb_own_pawns >> 7) & ~bb_files[FL_A]) | ((bb_own_pawns >> 9) & ~bb_files[FL_H]);
        op_pawn_attacks = ((bb_op_pawns << 7) & ~bb_files[FL_H]) | ((bb_op_pawns << 9) & ~bb_files[FL_A]);
    }

    return bb_bit_cnt(stop_squares & ~own_pawn_attacks & op_pawn_attacks);
}

int evaluate(struct board *board, enum color color){
    assert(board != NULL);

    enum color color_other = color_flip(color);

    int total_score = 0;

    // Piece values

    total_score += PAWN_VALUE * (bb_bit_cnt(board->bb_pieces[color][BB_PAWNS]) 
                                - bb_bit_cnt(board->bb_pieces[color_other][BB_PAWNS]));

    total_score += KNIGHT_VALUE * (bb_bit_cnt(board->bb_pieces[color][BB_KNIGHTS]) 
                                - bb_bit_cnt(board->bb_pieces[color_other][BB_KNIGHTS]));

    total_score += BISHOP_VALUE * (bb_bit_cnt(board->bb_pieces[color][BB_BISHOPS]) 
                                - bb_bit_cnt(board->bb_pieces[color_other][BB_BISHOPS]));

    total_score += ROOK_VALUE * (bb_bit_cnt(board->bb_pieces[color][BB_ROOKS]) 
                                - bb_bit_cnt(board->bb_pieces[color_other][BB_ROOKS]));
                                
    total_score += QUEEN_VALUE * (bb_bit_cnt(board->bb_pieces[color][BB_QUEENS]) 
                                - bb_bit_cnt(board->bb_pieces[color_other][BB_QUEENS]));

    // PST scores
    total_score += board->pst_scores[color]-board->pst_scores[color_other];

    // Bonuses

    total_score += MOBILITY_BONUS * (get_mobility(board, color) - get_mobility(board, color_other));

    total_score += KING_PAWN_SHIELD_BONUS * (get_pawns_shielding_king(board, color) - get_pawns_shielding_king(board, color_other));

    total_score += has_bishop_pair(board, color) ? BISHOP_PAIR_BONUS : 0;

    total_score += ROOK_OPEN_FILE_BONUS * (get_rooks_on_open_files(board, color) - get_rooks_on_open_files(board, color_other));

    // Penalties

    total_score += ISOLATED_PAWN_PENALTY * (get_isolated_pawns(board, color) - get_isolated_pawns(board, color_other));

    total_score += DOUBLED_PAWN_PENALTY * (get_doubled_pawns(board, color) - get_doubled_pawns(board, color_other));

    total_score += BACKWARD_PAWN_PENALTY * (get_backward_pawns(board, color) - get_backward_pawns(board, color_other)); 

    total_score -= has_bishop_pair(board, color_other) ? BISHOP_PAIR_BONUS : 0;

    return total_score;
}
