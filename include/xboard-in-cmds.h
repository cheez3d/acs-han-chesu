#ifndef XBOARD_IN_CMDS_H
#define XBOARD_IN_CMDS_H

/**
 * Value representing an xboard command received form xboard.
 */
enum xb_in_cmd {
    XB_IN_CMD_XBOARD,
    XB_IN_CMD_PROTOVER,
    XB_IN_CMD_ACCEPTED,
    XB_IN_CMD_REJECTED,
    XB_IN_CMD_NEW,
    XB_IN_CMD_VARIANT,
    XB_IN_CMD_QUIT,
    XB_IN_CMD_RANDOM,
    XB_IN_CMD_FORCE,
    XB_IN_CMD_GO,

    // activated by `XB_FEATURE_PLAYOTHER`
    // (default: `false`)
    XB_IN_CMD_PLAYOTHER,

    // activated by `XB_FEATURE_COLORS`
    // (default: `true`)
    XB_IN_CMD_WHITE,
    XB_IN_CMD_BLACK,

    XB_IN_CMD_LEVEL,
    XB_IN_CMD_ST,
    XB_IN_CMD_SD,

    // activated by `XB_FEATURE_NPS`
    // (default: `true`)
    XB_IN_CMD_NPS,

    // activated by `XB_FEATURE_TIME`
    // (default: `true`)
    XB_IN_CMD_TIME,
    XB_IN_CMD_OTIM,

    // activated by `XB_FEATURE_USERMOVE`
    // (default: `false`)
    XB_IN_CMD_USERMOVE,

    XB_IN_CMD_QUESTIONMARK,

    // activated by `XB_FEATURE_PING`
    // (default: `false`)
    XB_IN_CMD_PING,

    XB_IN_CMD_DRAW,
    XB_IN_CMD_RESULT,

    // activated by `XB_FEATURE_SETBOARD`
    // (default: `false`)
    XB_IN_CMD_SETBOARD,

    // deactivated by `XB_FEATURE_SETBOARD`
    // (default: `false`)
    XB_IN_CMD_EDIT,

    XB_IN_CMD_HINT,
    XB_IN_CMD_BK,
    XB_IN_CMD_UNDO,
    XB_IN_CMD_REMOVE,
    XB_IN_CMD_HARD,
    XB_IN_CMD_EASY,
    XB_IN_CMD_POST,
    XB_IN_CMD_NOPOST,
    XB_IN_CMD_ANALYZE,

    // activated by `XB_FEATURE_NAME`
    // (default: `true` when playing on chess server)
    // (default: `false` when not playing on chess server)
    XB_IN_CMD_NAME,

    XB_IN_CMD_RATING,

    // activated by `XB_FEATURE_ICS`
    // (default: `false`)
    XB_IN_CMD_ICS,

    XB_IN_CMD_COMPUTER,

    // activated by `XB_FEATURE_PAUSE`
    // (default: `false`)
    XB_IN_CMD_PAUSE,
    XB_IN_CMD_RESUME,

    // activated by `XB_FEATURE_MEMORY`
    // (default: `false`)
    XB_IN_CMD_MEMORY,

    // activated by `XB_FEATURE_SMP`
    // (default: `false`)
    XB_IN_CMD_CORES,

    // activated by `XB_FEATURE_EGT`
    // (default: `false`)
    XB_IN_CMD_EGTPATH,

    XB_IN_CMD_OPTION,

    // activated by `XB_FEATURE_EXCLUDE`
    // (default: `false`)
    XB_IN_CMD_EXCLUDE,
    XB_IN_CMD_INCLUDE,

    // activated by `XB_FEATURE_SETSCORE`
    // (default: `false`)
    XB_IN_CMD_SETSCORE,

    // activated by `XB_FEATURE_HIGHLIGHT`
    // (default: `false`)
    XB_IN_CMD_LIFT,
    XB_IN_CMD_PUT,
    XB_IN_CMD_HOVER,

    XB_IN_CMD_CNT, // number of input commands

    XB_IN_CMD_UNKNOWN = -1,
};

/**
 * Initialize xboard input command hashes.
 */
void xb_in_cmd_hashes_init(void);

/**
 * Convert input string to an xboard input command.
 */
enum xb_in_cmd xb_str_to_in_cmd(const char *str);

/**
 * Convert an input command to its function.
 */
extern void (*xb_in_cmds[XB_IN_CMD_CNT])(void);

#endif // XBOARD_IN_CMDS_H
