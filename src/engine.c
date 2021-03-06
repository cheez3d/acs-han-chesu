#include "engine.h"

#include "bitboard.h"
#include "board.h"
#include "book.h"
#include "eval.h"
#include "pst.h"
#include "search.h"
#include "xboard.h"
#include "xboard-out-cmds.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct engine engine;

void engine_init(const char *name) {
    assert(name != NULL);

    bb_init(); // initialize bitboard static data
    bk_init(); // initialize opening book
    xb_init(); // initialize xboard static data

    init_shields();
    init_pst();
    init_other_moves_table();

    engine.name = name;
    engine.name_other = NULL;

    // initialize xboard
    struct xboard *xboard = &engine.xboard;

    // set protover to 1; if protover > 1 it will be
    // set accordingly afterwards by the protover input command
    xboard->protover = 1;

    // set default feature values as described in xboard documentation
    xboard->features[XB_FEATURE_PING]->b      = false;
    xboard->features[XB_FEATURE_SETBOARD]->b  = false;
    xboard->features[XB_FEATURE_PLAYOTHER]->b = false;
    xboard->features[XB_FEATURE_SAN]->b       = false;
    xboard->features[XB_FEATURE_USERMOVE]->b  = false;
    xboard->features[XB_FEATURE_TIME]->b      = true;
    xboard->features[XB_FEATURE_DRAW]->b      = true;
    xboard->features[XB_FEATURE_SIGINT]->b    = true;
    xboard->features[XB_FEATURE_SIGTERM]->b   = true;
    xboard->features[XB_FEATURE_REUSE]->b     = true;
    xboard->features[XB_FEATURE_ANALYZE]->b   = true;
    xboard->features[XB_FEATURE_MYNAME]->s    = name;
    xboard->features[XB_FEATURE_VARIANTS]->s  = NULL;
    xboard->features[XB_FEATURE_COLORS]->b    = true;
    xboard->features[XB_FEATURE_ICS]->b       = false;
    xboard->features[XB_FEATURE_NAME]->b      = false;
    xboard->features[XB_FEATURE_PAUSE]->b     = false;
    xboard->features[XB_FEATURE_NPS]->b       = true;
    xboard->features[XB_FEATURE_DEBUG]->b     = false;
    xboard->features[XB_FEATURE_MEMORY]->b    = false;
    xboard->features[XB_FEATURE_SMP]->b       = false;
    xboard->features[XB_FEATURE_EGT]->s       = NULL;
    xboard->features[XB_FEATURE_OPTION]->s    = NULL;
    xboard->features[XB_FEATURE_EXCLUDE]->b   = false;
    xboard->features[XB_FEATURE_SETSCORE]->b  = false;
    xboard->features[XB_FEATURE_HIGHLIGHT]->b = false;
    xboard->features[XB_FEATURE_DONE]->i      = -1;

    engine.hard = true;
    engine.post = false;

    engine_reset();
}

void engine_term(void) {
    free(engine.name_other);

    xb_term();
    bk_term();
    bb_term();
}

void engine_reset(void) {
    engine.random = false;
    engine.force = false;
    engine.computer = false;

    engine.time = -1;
    engine.time_other = -1;

    board_reset(&engine.board);
}

void engine_recv_move(struct move move) {
    struct board *board = &engine.board;

    board_do_move(board, move);

    board_print_fancy(board);
}

void engine_send_move(void) {
    struct board *board = &engine.board;

    struct move move = bk_search(&engine.board);

    if (move.flags == MOVE_FLAG_INVALID) {
        xb_commentln("MOVE NOT IN BOOK");
        move = search_best_move(board);

        if (move.flags == MOVE_FLAG_INVALID) {
            xb_out_cmd(XB_OUT_CMD_RESIGN);
            return;
        }
    }

    board_do_move(board, move);

    xb_out_cmd(XB_OUT_CMD_MOVE, move); // send the move to xboard

    board_print_fancy(board);
}
