#ifndef XBOARD_OUT_CMDS_H
#define XBOARD_OUT_CMDS_H

/**
 * Value representing an xboard command sent to xboard.
 */
enum xb_out_cmd {
    XB_OUT_CMD_FEATURE,
    XB_OUT_CMD_PONG,
    XB_OUT_CMD_MOVE,
    XB_OUT_CMD_RESULT,
    XB_OUT_CMD_RESIGN,
    XB_OUT_CMD_OFFER_DRAW,
    XB_OUT_CMD_TELLOPPONENT,
    XB_OUT_CMD_TELLOTHERS,
    XB_OUT_CMD_TELLALL,
    XB_OUT_CMD_TELLUSER,
    XB_OUT_CMD_TELLUSERERROR,
    XB_OUT_CMD_ASKUSER,
    XB_OUT_CMD_TELLICS,
    XB_OUT_CMD_TELLICSNOALIAS,
    XB_OUT_CMD_SETUP,
    XB_OUT_CMD_PIECE,
    XB_OUT_CMD_HIGHLIGHT,
    XB_OUT_CMD_CLICK,

    XB_OUT_CMD_CNT, // number of output commands
};

/**
 * Send the specified command to xboard with the provided arguments.
 */
void xb_out_cmd(enum xb_out_cmd out_cmd, ...);

#endif // XBOARD_OUT_CMDS_H
