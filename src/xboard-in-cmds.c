#include "xboard-in-cmds.h"

#include "bitboard.h"
#include "board.h"
#include "engine.h"
#include "move.h"
#include "utils.h"
#include "xboard.h"
#include "xboard-out-cmds.h"

#include <errno.h>
#include <error.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static const char *xb_in_cmd_strs[XB_IN_CMD_CNT] = {
    [XB_IN_CMD_XBOARD]       = "xboard",
    [XB_IN_CMD_PROTOVER]     = "protover",
    [XB_IN_CMD_ACCEPTED]     = "accepted",
    [XB_IN_CMD_REJECTED]     = "rejected",
    [XB_IN_CMD_NEW]          = "new",
    [XB_IN_CMD_VARIANT]      = "variant",
    [XB_IN_CMD_QUIT]         = "quit",
    [XB_IN_CMD_RANDOM]       = "random",
    [XB_IN_CMD_FORCE]        = "force",
    [XB_IN_CMD_GO]           = "go",
    [XB_IN_CMD_PLAYOTHER]    = "playother",
    [XB_IN_CMD_WHITE]        = "white",
    [XB_IN_CMD_BLACK]        = "black",
    [XB_IN_CMD_LEVEL]        = "level",
    [XB_IN_CMD_ST]           = "st",
    [XB_IN_CMD_SD]           = "sd",
    [XB_IN_CMD_NPS]          = "nps",
    [XB_IN_CMD_TIME]         = "time",
    [XB_IN_CMD_OTIM]         = "otim",
    [XB_IN_CMD_USERMOVE]     = "usermove",
    [XB_IN_CMD_QUESTIONMARK] = "?",
    [XB_IN_CMD_PING]         = "ping",
    [XB_IN_CMD_DRAW]         = "draw",
    [XB_IN_CMD_RESULT]       = "result",
    [XB_IN_CMD_SETBOARD]     = "setboard",
    [XB_IN_CMD_EDIT]         = "edit",
    [XB_IN_CMD_HINT]         = "hint",
    [XB_IN_CMD_BK]           = "bk",
    [XB_IN_CMD_UNDO]         = "undo",
    [XB_IN_CMD_REMOVE]       = "remove",
    [XB_IN_CMD_HARD]         = "hard",
    [XB_IN_CMD_EASY]         = "easy",
    [XB_IN_CMD_POST]         = "post",
    [XB_IN_CMD_NOPOST]       = "nopost",
    [XB_IN_CMD_ANALYZE]      = "analyze",
    [XB_IN_CMD_NAME]         = "name",
    [XB_IN_CMD_RATING]       = "rating",
    [XB_IN_CMD_ICS]          = "ics",
    [XB_IN_CMD_COMPUTER]     = "computer",
    [XB_IN_CMD_PAUSE]        = "pause",
    [XB_IN_CMD_RESUME]       = "resume",
    [XB_IN_CMD_MEMORY]       = "memory",
    [XB_IN_CMD_CORES]        = "cores",
    [XB_IN_CMD_EGTPATH]      = "egtpath",
    [XB_IN_CMD_OPTION]       = "option",
    [XB_IN_CMD_EXCLUDE]      = "exclude",
    [XB_IN_CMD_INCLUDE]      = "include",
    [XB_IN_CMD_SETSCORE]     = "setscore",
    [XB_IN_CMD_LIFT]         = "lift",
    [XB_IN_CMD_PUT]          = "put",
    [XB_IN_CMD_HOVER]        = "hover",
};

static uint32_t xb_in_cmd_hashes[XB_IN_CMD_CNT];

void xb_in_cmd_hashes_init(void) {
    for (enum xb_in_cmd i = 0; i < XB_IN_CMD_CNT; ++i) {
        xb_in_cmd_hashes[i] = hash_djb2(xb_in_cmd_strs[i]);
    }
}

enum xb_in_cmd xb_str_to_in_cmd(const char *str) {
    assert(str != NULL);

    uint32_t hash = hash_djb2(str);

    for (enum xb_in_cmd i = 0; i < XB_IN_CMD_CNT; ++i) {
        if (xb_in_cmd_hashes[i] == hash) {
            return i;
        }
    }

    return XB_IN_CMD_UNKNOWN;
}

static void xb_in_cmd_xboard(void) {
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_DONE, 0);

    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_PING, true);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_SETBOARD, true);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_PLAYOTHER, false);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_SAN, false);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_USERMOVE, true);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_TIME, true);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_DRAW, true);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_SIGINT, false);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_SIGTERM, false);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_REUSE, false);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_ANALYZE, false);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_MYNAME, engine.name);
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_COLORS, false);

#ifdef DEBUG
    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_DEBUG, true);
#endif

    xb_out_cmd(XB_OUT_CMD_FEATURE, XB_FEATURE_DONE, 1);
}

static void xb_in_cmd_protover(void) {
    int protover = xb_read_int("could not read protocol version");

    if (protover < 2) {
        error(EXIT_FAILURE, errno, "unsupported protocol version %d", protover);
    }

    xb_commentln("protover is %d", protover);

    engine.xboard.protover = protover;
}

static void xb_in_cmd_accepted(void) {
    struct xboard *xboard = &engine.xboard;

    char *feature_str = xb_read_str("could not read accepted feature");

    enum xb_feature feature = xb_str_to_feature(feature_str);

    if (feature == XB_FEATURE_UNKNOWN) {
        error(EXIT_FAILURE, errno, "unkown feature '%s' received", feature_str);
    }

    // make pending feature change
    memcpy(xboard->features[feature],
           xboard->features[feature]+1,
           sizeof *xboard->features[feature]);

    xb_commentln("feature '%s' accepted", feature_str);

    free(feature_str);
}

static void xb_in_cmd_rejected(void) {
    char *feature_str = xb_read_str("could not read rejected feature");

    enum xb_feature feature = xb_str_to_feature(feature_str);

    if (feature == XB_FEATURE_UNKNOWN) {
        error(EXIT_FAILURE, errno, "unkown feature '%s' received", feature_str);
    }

    xb_commentln("feature '%s' rejected", feature_str);

    free(feature_str);

    error(EXIT_FAILURE, errno, "cannot continue without '%s' support", xb_feature_strs[feature]);
}

static void xb_in_cmd_new(void) {
    engine_reset();
}

static void xb_in_cmd_quit(void) {
    exit(EXIT_SUCCESS);
}

static void xb_in_cmd_random(void) {
    engine.random = !engine.random;
}

static void xb_in_cmd_force(void) {
    engine.force = true;
}

static void xb_in_cmd_go(void) {
    if (engine.force) {
        engine.force = false;
    }

    engine_send_move();
}

static void xb_in_cmd_level(void) {
    int mins_per_time_control = xb_read_int("could not read mps");
    char *base_str = xb_read_str("could not read base");
    int inc = xb_read_int("could not read inc");

    (void)mins_per_time_control;
    (void)base_str;
    (void)inc;

    free(base_str);
}

static void xb_in_cmd_usermove(void) {
    char *move_str = xb_read_str("could not read opponent move coords");

    struct board *board = &engine.board;

    enum color color = board->color;
    enum color color_other = color_flip(color);

    struct move move;
    
    move.flags = MOVE_FLAG_NONE;

    move.from = str_to_square(&move_str[0]);
    move.to = str_to_square(&move_str[2]);

    if (!(board->bb_pieces[color][BB_ALL] & bb_squares[move.from])) {
        move.flags = MOVE_FLAG_INVALID;
        xb_ill(move_str, "%s empty", square_to_str(move.from));
        goto illegal_move;
    } else if (board->bb_pieces[color][BB_ALL] & bb_squares[move.to]) {
        move.flags = MOVE_FLAG_INVALID;
        xb_ill(move_str, "%s occupied by piece of same color", square_to_str(move.to));
        goto illegal_move;
    }

    for (enum piece p = 0; p < PIECE_CNT; ++p) {
        if (board->bb_pieces[color][p] & bb_squares[move.from]) {
            move.piece = p;
            break;
        }
    }

    for (enum piece p = 0; p < PIECE_CNT; ++p) {
        if (board->bb_pieces[color_other][p] & bb_squares[move.to]) {
            move.flags |= MOVE_FLAG_CAPTURE;
            move.capture = p;
            break;
        }
    }

    if (move.piece == KING &&
        move.from == (color == WHITE ? SQ_E1 : SQ_E8))
    {
        if (move.to == (color == WHITE ? SQ_G1 : SQ_G8)) {
            if (!(board->castle_rights & (color == WHITE ? CASTLE_RIGHT_WHITE_KING : CASTLE_RIGHT_BLACK_KING))) {
                move.flags = MOVE_FLAG_INVALID;
                xb_ill(move_str, "illegal kingside castle", square_to_str(move.from), square_to_str(move.to));
                goto illegal_move;
            }

            move.flags |= MOVE_FLAG_KING_CASTLE;
        } else if (move.to == (color == WHITE ? SQ_C1 : SQ_C8)) {
            if (!(board->castle_rights & (color == WHITE ? CASTLE_RIGHT_WHITE_QUEEN : CASTLE_RIGHT_BLACK_QUEEN))) {
                move.flags = MOVE_FLAG_INVALID;
                xb_ill(move_str, "illegal queenside castle", square_to_str(move.from), square_to_str(move.to));
                goto illegal_move;
            }

            move.flags |= MOVE_FLAG_QUEEN_CASTLE;
        }
    } else if (move.piece == PAWN) {
        if (bb_squares[move.from] & bb_ranks[color == WHITE ? RK_2 : RK_7] &&
            bb_squares[move.to] & bb_ranks[color == WHITE ? RK_4 : RK_5])
        {
            move.flags |= MOVE_FLAG_PAWN_DOUBLE_PUSH;
        } else if (!(move.flags & MOVE_FLAG_CAPTURE) && move.to == board->en_passant) {
            move.flags |= MOVE_FLAG_EN_PASSANT;
        }
    }

    if (move_str[4] != '\0') {
        move.flags |= MOVE_FLAG_PROMOTION;

        move.promotion = char_to_piece(move_str[4]);
    }

    free(move_str);

illegal_move:
    if (move.flags == MOVE_FLAG_INVALID) {
        error(EXIT_FAILURE, errno, "illegal move received");
    }

    engine_recv_move(move);

    if (!engine.force) {
        engine_send_move();
    }
}

static void xb_in_cmd_time(void) {
    int time = xb_read_int("could not read time");

    engine.time = time;
}

static void xb_in_cmd_otim() {
    int time_other = xb_read_int("could not read ping opponent time");

    engine.time_other = time_other;
}

static void xb_in_cmd_ping(void) {
    int n = xb_read_int("could not read ping value");

    xb_out_cmd(XB_OUT_CMD_PONG, n);
}

static void xb_in_cmd_result(void) {
    char *result_str = xb_read_str("could not read result");
    char *comment = xb_read_comment("could not read comment");

    engine.force = true;

    enum xb_result result = xb_str_to_result(result_str);

    free(result_str);

    xb_commentln("result '%s': %s", xb_result_strs[result], comment);

    free(comment);
}

static void xb_in_cmd_setboard(void) {
    char *fen = xb_read_fen("could not read FEN string");

    board_set_fen(&engine.board, fen);

    free(fen);
}

static void xb_in_cmd_hard(void) {
    engine.hard = true;
}

static void xb_in_cmd_easy(void) {
    engine.hard = false;
}

static void xb_in_cmd_post(void) {
    engine.post = true;
}

static void xb_in_cmd_nopost(void) {
    engine.post = false;
}

static void xb_in_cmd_name(void) {
    struct xboard *xboard = &engine.xboard;

    char *name_other = xb_read_str("could not read opponent name");

    xboard->features[XB_FEATURE_NAME]->b = true;

    engine.name_other = name_other;
}

static void xb_in_cmd_computer(void) {
    engine.computer = true;
}

void (*xb_in_cmds[XB_IN_CMD_CNT])(void) = {
    [XB_IN_CMD_XBOARD]       = xb_in_cmd_xboard,
    [XB_IN_CMD_PROTOVER]     = xb_in_cmd_protover,
    [XB_IN_CMD_ACCEPTED]     = xb_in_cmd_accepted,
    [XB_IN_CMD_REJECTED]     = xb_in_cmd_rejected,
    [XB_IN_CMD_NEW]          = xb_in_cmd_new,
    [XB_IN_CMD_VARIANT]      = NULL,
    [XB_IN_CMD_QUIT]         = xb_in_cmd_quit,
    [XB_IN_CMD_RANDOM]       = xb_in_cmd_random,
    [XB_IN_CMD_FORCE]        = xb_in_cmd_force,
    [XB_IN_CMD_GO]           = xb_in_cmd_go,
    [XB_IN_CMD_PLAYOTHER]    = NULL,
    [XB_IN_CMD_WHITE]        = NULL,
    [XB_IN_CMD_BLACK]        = NULL,
    [XB_IN_CMD_LEVEL]        = xb_in_cmd_level,
    [XB_IN_CMD_ST]           = NULL,
    [XB_IN_CMD_SD]           = NULL,
    [XB_IN_CMD_NPS]          = NULL,
    [XB_IN_CMD_TIME]         = xb_in_cmd_time,
    [XB_IN_CMD_OTIM]         = xb_in_cmd_otim,
    [XB_IN_CMD_USERMOVE]     = xb_in_cmd_usermove,
    [XB_IN_CMD_QUESTIONMARK] = NULL,
    [XB_IN_CMD_PING]         = xb_in_cmd_ping,
    [XB_IN_CMD_DRAW]         = NULL,
    [XB_IN_CMD_RESULT]       = xb_in_cmd_result,
    [XB_IN_CMD_SETBOARD]     = xb_in_cmd_setboard,
    [XB_IN_CMD_EDIT]         = NULL,
    [XB_IN_CMD_HINT]         = NULL,
    [XB_IN_CMD_BK]           = NULL,
    [XB_IN_CMD_UNDO]         = NULL,
    [XB_IN_CMD_REMOVE]       = NULL,
    [XB_IN_CMD_HARD]         = xb_in_cmd_hard,
    [XB_IN_CMD_EASY]         = xb_in_cmd_easy,
    [XB_IN_CMD_POST]         = xb_in_cmd_post,
    [XB_IN_CMD_NOPOST]       = xb_in_cmd_nopost,
    [XB_IN_CMD_ANALYZE]      = NULL,
    [XB_IN_CMD_NAME]         = xb_in_cmd_name,
    [XB_IN_CMD_RATING]       = NULL,
    [XB_IN_CMD_ICS]          = NULL,
    [XB_IN_CMD_COMPUTER]     = xb_in_cmd_computer,
    [XB_IN_CMD_PAUSE]        = NULL,
    [XB_IN_CMD_RESUME]       = NULL,
    [XB_IN_CMD_MEMORY]       = NULL,
    [XB_IN_CMD_CORES]        = NULL,
    [XB_IN_CMD_EGTPATH]      = NULL,
    [XB_IN_CMD_OPTION]       = NULL,
    [XB_IN_CMD_EXCLUDE]      = NULL,
    [XB_IN_CMD_INCLUDE]      = NULL,
    [XB_IN_CMD_SETSCORE]     = NULL,
    [XB_IN_CMD_LIFT]         = NULL,
    [XB_IN_CMD_PUT]          = NULL,
    [XB_IN_CMD_HOVER]        = NULL,
};
