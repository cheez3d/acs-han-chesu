#ifndef XBOARD_H
#define XBOARD_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Value representing an xboard feature.
 */
enum xb_feature {
    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    // recommended: `true`
    XB_FEATURE_PING,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    // recommended: `true`
    XB_FEATURE_SETBOARD,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    // recommended: `true`
    XB_FEATURE_PLAYOTHER,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_SAN,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_USERMOVE,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    // recommended: `true`
    XB_FEATURE_TIME,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    // recommended: `true`
    XB_FEATURE_DRAW,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    XB_FEATURE_SIGINT,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    XB_FEATURE_SIGTERM,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    // recommended: `true`
    XB_FEATURE_REUSE,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    // recommended: `true`
    XB_FEATURE_ANALYZE,

    // type: `XB_FEATURE_TYPE_STR`
    // default: determined from file name
    XB_FEATURE_MYNAME,

    // type: `XB_FEATURE_TYPE_STR`
    // default: all variants are assumed to be supported
    // recommended: "normal"
    XB_FEATURE_VARIANTS,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    // recommended: `false`
    XB_FEATURE_COLORS,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_ICS,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true` when playing on chess server
    // default: `false` when not playing on chess server
    XB_FEATURE_NAME,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_PAUSE,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `true`
    XB_FEATURE_NPS,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_DEBUG,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_MEMORY,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_SMP,

    // type: `XB_FEATURE_TYPE_STR`
    // default: end-game tables are assumed to not be supported
    XB_FEATURE_EGT,

    // type: `XB_FEATURE_TYPE_STR`
    XB_FEATURE_OPTION,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_EXCLUDE,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_SETSCORE,

    // type: `XB_FEATURE_TYPE_BOOL`
    // default: `false`
    XB_FEATURE_HIGHLIGHT,

    // type: `XB_FEATURE_TYPE_INT`
    // no default
    XB_FEATURE_DONE,

    XB_FEATURE_CNT, // number of features

    XB_FEATURE_UNKNOWN = -1,
};

/**
 * Value representing an xboard feature type.
 */
enum xb_feature_type {
    XB_FEATURE_TYPE_BOOL,
    XB_FEATURE_TYPE_INT,
    XB_FEATURE_TYPE_STR,

    XB_FEATURE_TYPE_CNT, // number of feature types
};

/**
 * Mapping of xboard features to their string representations.
 */
extern const char *xb_feature_strs[XB_FEATURE_CNT];

/**
 * Convert string to an xboard feature.
 */
enum xb_feature xb_str_to_feature(const char *str);

/**
 * Mapping for each xboard feature to its respective type.
 */
extern enum xb_feature_type xb_feature_types[XB_FEATURE_CNT];

/**
 * Union for storing the value of a feature.
 */
union xb_feature_val {
    bool b;
    int i;
    const char *s;
};

/**
 * Value representing an xboard result.
 */
enum xb_result {
    XB_RESULT_WHITE_WIN,
    XB_RESULT_BLACK_WIN,
    XB_RESULT_DRAW,
    XB_RESULT_OTHER,

    XB_RESULT_CNT, // number of results

    XB_RESULT_UNKNOWN = -1,
};

/**
 * Mapping of xboard results to their string representations.
 */
extern const char *xb_result_strs[XB_RESULT_CNT];

/**
 * Convert string to an xboard result.
 */
enum xb_result xb_str_to_result(const char *str);

struct xboard {
    int protover;

    /**
     * Allocate twice the required space of a feature value in order to
     * also store the pending feature value which is used for temporarily
     * storing the value of a feature requested through the
     * `XB_OUT_CMD_FEATURE` command until the `XB_IN_CMD_ACCEPTED`
     * or `XB_IN_CMD_REJECTED` response is received, at which point we will
     * know whether we should commit the requested value as the current value
     * or discard it.
     */
    union xb_feature_val features[XB_FEATURE_CNT][2];
};

/**
 * Initialize xboard.
 */
void xb_init(void);

/**
 * Terminate xboard.
 */
void xb_term(void);

/**
 * Read a string into automatically allocated space.
 * You should `free` the returned pointer after it is no longer needed.
 */
char * xb_read_str(const char *err_str);

/**
 * Read a comment into automatically allocated space.
 * You should `free` the returned pointer after it is no longer needed.
 */
char * xb_read_comment(const char *err_str);

/**
 * Read a comment FEN string into automatically allocated space.
 * You should `free` the returned pointer after it is no longer needed.
 */
char * xb_read_fen(const char *err_str);

/**
 * Read an integer.
 */
int xb_read_int(const char *err_str);

/**
 * Send a message to xboard without printing a newline.
 * (useful for constructing output commands step by step)
 */
void xb_print(const char *format, ...);

/**
 * Send a message to xboard on a new line.
 */
void xb_println(const char *format, ...);

/**
 * Print a comment without starting a new line
 * (if newline already started by a previous call) into xboard log.
 */
void xb_comment(const char *format, ...);

/**
 * Print a comment line into xboard log.
 */
void xb_commentln(const char *format, ...);

/**
 * Signal xboard that the engine has received an illegal move.
 */
void xb_ill(const char *move, const char *reason, ...);

/**
 * Signal xboard that an error has occured in the engine.
 */
void xb_err(const char *cmd, const char *type, ...);

/**
 * Start the xboard loop.
 */
void xb_loop(void);

#endif // XBOARD_H
