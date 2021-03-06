#include "search.h"

#include "board.h"
#include "move.h"
#include "movegen.h"

#include <limits.h>
#include <stddef.h>
#include <time.h>

#include "eval.h"

static clock_t start = 0;
static bool stop = false;
int other_attacks_table[5][6];

int quiescent_search(struct board *board, int alpha, int beta);

static bool check_limits() {
    if(clock() - start >= 5 * CLOCKS_PER_SEC) {
        return true;
    }

    return false;
}

static bool qhas_next(struct move_list *moves) {
    return moves->head < moves->captured_pieces;
}

static bool ghas_next(struct move_list *moves) {
    return moves->head < moves->count;
}

static void qscore_moves(struct move_list *moves) {
    for (size_t i = 0; i < moves->count; i++) {
        struct move move = moves->list[i];

        if (move.flags & MOVE_FLAG_CAPTURE) {
            moves->list[i].score = CAPTURE_BONUS + other_attacks_table[move.capture][move.piece];
        } else if (move.flags & MOVE_FLAG_PROMOTION) {
            moves->list[i].score = PROMOTION_BONUS + get_piece_value(move.piece);
        }
    }
}

static bool compare_move(struct move m1, struct move m2) {
    return (m1.capture == m2.capture) && (m1.flags == m2.flags) &&
    (m1.from == m2.from) && (m1.piece == m2.piece) && (m1.promotion == m2.promotion)
    && (m1.to == m2.to);
}

static void gscore_moves(struct move_list *moves, struct ordering_info *ordering_info, struct board *board) {
    for (size_t i = 0; i < moves->count; i++) {
        struct move move = moves->list[i];

        if (move.flags & MOVE_FLAG_CAPTURE) {
            moves->list[i].score = CAPTURE_BONUS + other_attacks_table[move.capture][move.piece];
        } else if (move.flags & MOVE_FLAG_PROMOTION) {
            moves->list[i].score = PROMOTION_BONUS + get_piece_value(move.promotion);
        } else if (compare_move(move, ordering_info->killer1[ordering_info->ply])) {
            moves->list[i].score = KILLER1_BONUS;
        } else if (compare_move(move, ordering_info->killer2[ordering_info->ply])) {
            moves->list[i].score = KILLER2_BONUS;
        } else { // Quiet
            moves->list[i].score = QUIET_BONUS + ordering_info->history[board->color][move.from][move.to];
        }
    }
}

void init_other_moves_table() {
    int current_score = 0;
    enum piece victims_low_to_high[] = {PAWN, KNIGHT, BISHOP, ROOK, QUEEN};
    enum piece victims_high_to_low[] = {KING, QUEEN, ROOK, BISHOP, KNIGHT, PAWN};

    for (int i = 0; i < 5; i ++) {
        for (int j = 0; j < 6; j++) {
            other_attacks_table[victims_low_to_high[i]][victims_high_to_low[j]] = current_score++;
        }
    }
}

static void init_qmove_picker(struct move_list *moves) {
    moves->head = 0;

    moves->captured_pieces = 0;

    for (size_t i = 0; i < moves->count; i++) {
        if (moves->list[i].flags & MOVE_FLAG_CAPTURE) {
            moves->captured_pieces++;
        }
    }

    qscore_moves(moves);
}

static void init_gmove_picker(struct move_list *moves, struct ordering_info *ordering_info, struct board *board) {
    moves->head = 0;

    gscore_moves(moves, ordering_info, board);
}

static struct move qget_next(struct move_list *moves) {
    size_t best_index = 0;
    int best_score = INT_MIN / 2;

    for (size_t i = moves->head; i < moves->count; i++) {
        int current_score = moves->list[i].score;

        if ((moves->list[i].flags & (MOVE_FLAG_CAPTURE | MOVE_FLAG_PROMOTION)) && (current_score > best_score)) {
            best_score = current_score;
            best_index = i;
        }
    }

    struct move aux = moves->list[moves->head];
    moves->list[moves->head] = moves->list[best_index];
    moves->list[best_index] = aux;

    return moves->list[moves->head++];
}

struct move gget_next(struct move_list *moves) {
    size_t best_index = 0;
    int best_score = INT_MIN / 2;

    for (size_t i = moves->head; i < moves->count; i++) {
        if (moves->list[i].score > best_score) {
            best_score = moves->list[i].score;
            best_index = i;
        }
    }

    struct move aux = moves->list[moves->head];
    moves->list[moves->head] = moves->list[best_index];
    moves->list[best_index] = aux;

    return moves->list[moves->head++];
}

static int search_negamax(struct board *board, size_t depth, int alpha, int beta, struct ordering_info *ordering_info) {
    if (board->halfmove_clock >= 50) {
        return evaluate(board, board->color);
    }

    struct move_list moves;
    movegen_add_moves(&moves, board);

    init_gmove_picker(&moves, ordering_info, board);

    // checkmate or stalemate
    if (moves.count == 0) {
        return board_color_in_check(board, board->color) ? INT_MIN / 2 : 0;
    }

    if (depth == 0) {
        return quiescent_search(board, alpha, beta);
    }
    
    bool full_window = true;

    while(ghas_next(&moves)) {
        struct move move = gget_next(&moves);

        struct board board_copy = *board;
        board_do_move(&board_copy, move);

        int score;

        ordering_info->ply++;
        if (full_window) {
            score = -search_negamax(&board_copy, depth-1, -beta, -alpha, ordering_info);
        } else {
            score = -search_negamax(&board_copy, depth-1, -alpha - 1, -alpha, ordering_info);

            if (score > alpha) {
                score = -search_negamax(&board_copy, depth-1, -beta, -alpha, ordering_info);
            }
        }
        ordering_info->ply--;

        if (score >= beta) {
            // Add this move as a new killer move and update history if move is quiet
            ordering_info->killer2[ordering_info->ply] = ordering_info->killer1[ordering_info->ply];
              ordering_info->killer1[ordering_info->ply] = move;
            
            if (!(move.flags & MOVE_FLAG_CAPTURE)) {
                ordering_info->history[board->color][move.from][move.to] += depth * depth;
            }

            return beta;
        }

        if (score > alpha) {
            full_window = false;
            alpha = score;
        }
    }

    return alpha;
}
#include "xboard.h"
struct move search_best_move(struct board *board) {
    start = clock();
    stop = false;
    struct ordering_info ordering_info;
    ordering_info.ply = 0;
    memset(ordering_info.history, 0, sizeof(ordering_info.history));

    struct move_list moves;
    movegen_add_moves(&moves, board);

    init_gmove_picker(&moves, &ordering_info, board);

    struct move best_move = { .flags = MOVE_FLAG_INVALID };

    int current_score = 0;
    int alpha = INT_MIN / 2;
    int beta = INT_MAX / 2;

    bool full_window = true;

    while (ghas_next(&moves)) {
        xb_commentln("loop");
        xb_commentln("%i %i", moves.head, moves.count);

        struct move move = gget_next(&moves);

        struct board board_copy = *board;
        board_do_move(&board_copy, move);

        ordering_info.ply++;
        if (full_window) {
            current_score = -search_negamax(&board_copy, SEARCH_DEPTH-1, -beta, -alpha, &ordering_info);
        } else {
            current_score = -search_negamax(&board_copy, SEARCH_DEPTH-1, -alpha - 1, -alpha, &ordering_info);
            if (current_score > alpha) {
                current_score = -search_negamax(&board_copy, SEARCH_DEPTH-1, -beta, -alpha, &ordering_info);
            }
        }
        ordering_info.ply--;

        if (current_score > alpha) {
            full_window = false;
            best_move = move;
            alpha = current_score;

            if (current_score == INT_MAX/2) {
                break;
            }
        }
    }

    if (best_move.flags == MOVE_FLAG_INVALID) {
        best_move = moves.list[0];
    }

    xb_commentln("BEST MOVE SCORE :: %d", current_score);

    return best_move;
}

int quiescent_search(struct board *board, int alpha, int beta) {
    if (stop || check_limits()) {
        stop = true;
        return 0;
    }

    struct move_list moves;
    movegen_add_moves(&moves, board);

    // checkmate or stalemate
    if (moves.count == 0) {
        return board_color_in_check(board, board->color) ? INT_MIN : 0;
    }

    int stand_pat = evaluate(board, board->color);

    init_qmove_picker(&moves);

    if (!qhas_next(&moves)) {
        return stand_pat;
    }

    if (stand_pat >= beta) {
        return beta;
    }

    if (alpha < stand_pat) {
        alpha = stand_pat;
    }

    while (qhas_next(&moves)) {
        struct move move = qget_next(&moves);

        struct board board_copy = *board;
        board_do_move(&board_copy, move);

        int score = -quiescent_search(&board_copy, -beta, -alpha);

        if (score >= beta) {
            return beta;
        }

        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}


static int search_negamax2(struct board *board, size_t depth, clock_t time_start, struct ordering_info *ordering_info) {
    if (clock()-time_start >= 5*CLOCKS_PER_SEC) {
        return 0;
    }

    if (board->halfmove_clock >= 50) {
        return 0;
    }

    struct move_list moves;
    movegen_add_moves(&moves, board);
    init_gmove_picker(&moves, ordering_info, board);

    // checkmate or stalemate
    if (moves.count == 0) {
        return board_color_in_check(board, board->color) ? INT_MIN : 0;
    }

    if (depth == 0) {
        return evaluate(board, board->color);
    }

    int best_score = INT_MIN;

    while(ghas_next(&moves)) {
        xb_commentln("loop");
        xb_commentln("%i %i", moves.head, moves.count);

        struct move move = gget_next(&moves);

        struct board board_copy = *board;
        board_do_move(&board_copy, move);

        int score = -search_negamax2(&board_copy, depth-1, time_start, ordering_info);

        if (score > best_score) {
            best_score = score;
        }
    }

    return best_score;
}
// #include "xboard.h"
struct move search_best_move2(struct board *board) {
    clock_t time_start = clock();
    struct ordering_info ordering_info;
    ordering_info.ply = 0;
    memset(ordering_info.history, 0, sizeof(ordering_info.history));

    struct move_list moves;
    movegen_add_moves(&moves, board);

    init_gmove_picker(&moves, &ordering_info, board);

    struct move best_move = { .flags = MOVE_FLAG_INVALID };
    int best_score = INT_MIN;

    while(ghas_next(&moves)) {
        xb_commentln("loop");
        xb_commentln("%i %i", moves.head, moves.count);

        struct move move = gget_next(&moves);

        struct board board_copy = *board;
        board_do_move(&board_copy, move);

        int score = -search_negamax2(&board_copy, SEARCH_DEPTH-1, time_start, &ordering_info);

        if (score > best_score) {
            best_move = move;
            best_score = score;
        }
    }

    xb_commentln("BEST MOVE SCORE :: %d", best_score);

    return best_move;
}
