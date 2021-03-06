#include "xboard-out-cmds.h"

#include "engine.h"
#include "move.h"
#include "xboard.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

static const char *xb_out_cmd_strs[XB_OUT_CMD_CNT] = {
    [XB_OUT_CMD_FEATURE]        = "feature",
    [XB_OUT_CMD_PONG]           = "pong",
    [XB_OUT_CMD_MOVE]           = "move",
    [XB_OUT_CMD_RESULT]         = "",
    [XB_OUT_CMD_RESIGN]         = "resign",
    [XB_OUT_CMD_OFFER_DRAW]     = "offer draw",
    [XB_OUT_CMD_TELLOPPONENT]   = "tellopponent",
    [XB_OUT_CMD_TELLOTHERS]     = "tellothers",
    [XB_OUT_CMD_TELLALL]        = "tellall",
    [XB_OUT_CMD_TELLUSER]       = "telluser",
    [XB_OUT_CMD_TELLUSERERROR]  = "tellusererror",
    [XB_OUT_CMD_ASKUSER]        = "askuser",
    [XB_OUT_CMD_TELLICS]        = "tellics",
    [XB_OUT_CMD_TELLICSNOALIAS] = "tellicsnoalias",
    [XB_OUT_CMD_SETUP]          = "setup",
    [XB_OUT_CMD_PIECE]          = "piece",
    [XB_OUT_CMD_HIGHLIGHT]      = "highlight",
    [XB_OUT_CMD_CLICK]          = "click",
};

static void xb_out_cmd_feature(va_list args) {
    enum xb_feature feature = va_arg(args, enum xb_feature);

    xb_print(" ");

    xb_print("%s=", xb_feature_strs[feature]);

    switch (xb_feature_types[feature]) {
    case XB_FEATURE_TYPE_BOOL: {
        bool val = va_arg(args, int);

        engine.xboard.features[feature][1].b = val;
        xb_print("%d", val);

        break;
    }

    case XB_FEATURE_TYPE_INT: {
        int val = va_arg(args, int);

        engine.xboard.features[feature][1].i = val;
        xb_print("%d", val);

        break;
    }

    case XB_FEATURE_TYPE_STR: {
        const char *val = va_arg(args, const char *);

        engine.xboard.features[feature][1].s = val;
        xb_print("\"%s\"", val);

        break;
    }

    default:
        return;
    }
}

static void xb_out_cmd_pong(va_list args) {
    int n = va_arg(args, int);

    xb_print(" ");

    xb_print("%d", n);
}

static void xb_out_cmd_move(va_list args) {
    struct move move = va_arg(args, struct move);

    xb_print(" ");

    xb_print("%s", square_to_str(move.from));
    xb_print("%s", square_to_str(move.to));

    if (move.flags & MOVE_FLAG_PROMOTION) {
        xb_print("%c", piece_to_char(BLACK, move.promotion));
    }
}

static void xb_out_cmd_result(va_list args) {
    enum xb_result result = va_arg(args, enum xb_result);
    const char *comment = va_arg(args, const char *);

    xb_print(xb_result_strs[result]);

    xb_print(" {");

    vprintf(comment, args);

    xb_print("}");
}

static void xb_out_cmd_resign(va_list args) {
    (void)args;
}

static void (*xb_out_cmds[XB_OUT_CMD_CNT])(va_list) = {
    [XB_OUT_CMD_FEATURE]        = xb_out_cmd_feature,
    [XB_OUT_CMD_PONG]           = xb_out_cmd_pong,
    [XB_OUT_CMD_MOVE]           = xb_out_cmd_move,
    [XB_OUT_CMD_RESULT]         = xb_out_cmd_result,
    [XB_OUT_CMD_RESIGN]         = xb_out_cmd_resign,
    [XB_OUT_CMD_OFFER_DRAW]     = NULL,
    [XB_OUT_CMD_TELLOPPONENT]   = NULL,
    [XB_OUT_CMD_TELLOTHERS]     = NULL,
    [XB_OUT_CMD_TELLALL]        = NULL,
    [XB_OUT_CMD_TELLUSER]       = NULL,
    [XB_OUT_CMD_TELLUSERERROR]  = NULL,
    [XB_OUT_CMD_ASKUSER]        = NULL,
    [XB_OUT_CMD_TELLICS]        = NULL,
    [XB_OUT_CMD_TELLICSNOALIAS] = NULL,
    [XB_OUT_CMD_SETUP]          = NULL,
    [XB_OUT_CMD_PIECE]          = NULL,
    [XB_OUT_CMD_HIGHLIGHT]      = NULL,
    [XB_OUT_CMD_CLICK]          = NULL,
};

void xb_out_cmd(enum xb_out_cmd out_cmd, ...) {
    assert(out_cmd >= 0 && out_cmd < XB_OUT_CMD_CNT);

    va_list args;
    va_start(args, out_cmd);

    xb_print("%s", xb_out_cmd_strs[out_cmd]);

    if (xb_out_cmds[out_cmd] != NULL) {
        xb_out_cmds[out_cmd](args);
    }

    xb_print("\n");

    va_end(args);
}
