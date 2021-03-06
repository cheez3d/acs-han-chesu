#include "xboard.h"

#include "engine.h"
#include "utils.h"
#include "xboard-in-cmds.h"

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *xb_feature_strs[XB_FEATURE_CNT] = {
    [XB_FEATURE_PING]      = "ping",
    [XB_FEATURE_SETBOARD]  = "setboard",
    [XB_FEATURE_PLAYOTHER] = "playother",
    [XB_FEATURE_SAN]       = "san",
    [XB_FEATURE_USERMOVE]  = "usermove",
    [XB_FEATURE_TIME]      = "time",
    [XB_FEATURE_DRAW]      = "draw",
    [XB_FEATURE_SIGINT]    = "sigint",
    [XB_FEATURE_SIGTERM]   = "sigterm",
    [XB_FEATURE_REUSE]     = "reuse",
    [XB_FEATURE_ANALYZE]   = "analyze",
    [XB_FEATURE_MYNAME]    = "myname",
    [XB_FEATURE_VARIANTS]  = "variants",
    [XB_FEATURE_COLORS]    = "colors",
    [XB_FEATURE_ICS]       = "ics",
    [XB_FEATURE_NAME]      = "name",
    [XB_FEATURE_PAUSE]     = "pause",
    [XB_FEATURE_NPS]       = "nps",
    [XB_FEATURE_DEBUG]     = "debug",
    [XB_FEATURE_MEMORY]    = "memory",
    [XB_FEATURE_SMP]       = "smp",
    [XB_FEATURE_EGT]       = "egt",
    [XB_FEATURE_OPTION]    = "option",
    [XB_FEATURE_EXCLUDE]   = "exclude",
    [XB_FEATURE_SETSCORE]  = "setscore",
    [XB_FEATURE_HIGHLIGHT] = "highlight",
    [XB_FEATURE_DONE]      = "done",
};

static uint32_t xb_feature_hashes[XB_FEATURE_CNT];

/**
 * Initialize xboard feature hashes.
 */
static void xb_feature_hashes_init(void) {
    for (enum xb_feature i = 0; i < XB_FEATURE_CNT; ++i) {
        xb_feature_hashes[i] = hash_djb2(xb_feature_strs[i]);
    }
}

enum xb_feature xb_str_to_feature(const char *str) {
    assert(str != NULL);

    uint32_t hash = hash_djb2(str);

    for (enum xb_feature i = 0; i < XB_FEATURE_CNT; ++i) {
        if (xb_feature_hashes[i] == hash) {
            return i;
        }
    }

    return XB_FEATURE_UNKNOWN;
}

enum xb_feature_type xb_feature_types[XB_FEATURE_CNT] = {
    [XB_FEATURE_PING]      = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_SETBOARD]  = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_PLAYOTHER] = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_SAN]       = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_USERMOVE]  = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_TIME]      = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_DRAW]      = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_SIGINT]    = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_SIGTERM]   = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_REUSE]     = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_ANALYZE]   = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_MYNAME]    = XB_FEATURE_TYPE_STR,
    [XB_FEATURE_VARIANTS]  = XB_FEATURE_TYPE_STR,
    [XB_FEATURE_COLORS]    = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_ICS]       = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_NAME]      = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_PAUSE]     = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_NPS]       = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_DEBUG]     = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_MEMORY]    = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_SMP]       = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_EGT]       = XB_FEATURE_TYPE_STR,
    [XB_FEATURE_OPTION]    = XB_FEATURE_TYPE_STR,
    [XB_FEATURE_EXCLUDE]   = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_SETSCORE]  = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_HIGHLIGHT] = XB_FEATURE_TYPE_BOOL,
    [XB_FEATURE_DONE]      = XB_FEATURE_TYPE_INT,
};

const char *xb_result_strs[XB_RESULT_CNT] = {
    [XB_RESULT_WHITE_WIN] = "1-0",
    [XB_RESULT_BLACK_WIN] = "0-1",
    [XB_RESULT_DRAW]      = "1/2-1/2",
    [XB_RESULT_OTHER]     = "*",
};

static uint32_t xb_result_hashes[XB_RESULT_CNT];

/**
 * Initialize xboard result hashes.
 */
static void xb_result_hashes_init(void) {
    for (enum xb_result i = 0; i < XB_RESULT_CNT; ++i) {
        xb_result_hashes[i] = hash_djb2(xb_result_strs[i]);
    }
}

enum xb_result xb_str_to_result(const char *str) {
    assert(str != NULL);

    uint32_t hash = hash_djb2(str);

    for (enum xb_result i = 0; i < XB_RESULT_CNT; ++i) {
        if (xb_result_hashes[i] == hash) {
            return i;
        }
    }

    return XB_RESULT_UNKNOWN;
}

void xb_init(void) {
    xb_in_cmd_hashes_init();
    xb_feature_hashes_init();
    xb_result_hashes_init();

    // disable buffering so that xboard receives
    // the engine responses immediately
    setbuf(stdout, NULL);
}

void xb_term(void) {}

char * xb_read_str(const char *err_str) {
    assert(err_str != NULL);

    char *str = NULL;

    if (scanf("%ms", &str) < 1) {
        error(EXIT_FAILURE, errno, "%s", err_str);
    }

    return str;
}

char * xb_read_comment(const char *err_str) {
    assert(err_str != NULL);

    char *comment = NULL;

    if (scanf(" {%m[^}]}", &comment) < 1) {
        error(EXIT_FAILURE, errno, "%s", err_str);
    }

    return comment;
}

char * xb_read_fen(const char *err_str) {
    assert(err_str != NULL);

    char *fen = NULL;

    if (scanf(" %m[^\n]", &fen) < 1) {
        error(EXIT_FAILURE, errno, "%s", err_str);
    }

    return fen;
}

int xb_read_int(const char *err_str) {
    assert(err_str != NULL);

    int d = -1;

    if (scanf("%d", &d) < 1) {
        error(EXIT_FAILURE, errno, "%s", err_str);
    }

    return d;
}

static bool xb_print_newline = true;

void xb_print(const char *format, ...) {
    assert(format != NULL);

    va_list args;
    va_start(args, format);

    vprintf(format, args);

    xb_print_newline = strrchr(format, '\n') != NULL;

    va_end(args);
}

void xb_println(const char *format, ...) {
    assert(format != NULL);

    va_list args;
    va_start(args, format);

    if (!xb_print_newline) {
        printf("\n");

        xb_print_newline = true;
    }

    vprintf(format, args);

    printf("\n");

    xb_print_newline = true;

    va_end(args);
}

#ifdef DEBUG
#define XB_COMMENT_SYMBOL "#"

#define XB_COMMENT_PREFIX XB_COMMENT_SYMBOL " >>> "

static bool xb_comment_newline = true;
#endif

void xb_comment(const char *format, ...) {
    assert(format != NULL);

    va_list args;
    va_start(args, format);

#ifdef DEBUG
    if (!engine.xboard.features[XB_FEATURE_DEBUG]->b) {
        return;
    }

    if (xb_comment_newline) {
        printf(XB_COMMENT_PREFIX);
    }

    vprintf(format, args);

    xb_comment_newline = strrchr(format, '\n') != NULL;
#endif

    va_end(args);
}

void xb_commentln(const char *format, ...) {
    assert(format != NULL);

    va_list args;
    va_start(args, format);

#ifdef DEBUG
    if (!engine.xboard.features[XB_FEATURE_DEBUG]->b) {
        return;
    }

    if (!xb_comment_newline) {
        printf("\n");

        xb_comment_newline = true;
    }

    printf(XB_COMMENT_PREFIX);

    vprintf(format, args);

    printf("\n");

    xb_comment_newline = true;
#endif

    va_end(args);
}

void xb_ill(const char *move, const char *reason, ...) {
    assert(move != NULL);
    assert(reason != NULL);

    va_list args;
    va_start(args, reason);

    printf("Illegal move");

    if (reason != NULL) {
        printf(" (");

        vprintf(reason, args);

        printf(")");
    }

    printf(": %s", move);

    printf("\n");

    va_end(args);
}

void xb_err(const char *cmd, const char *type, ...) {
    assert(cmd != NULL);
    assert(type != NULL);

    va_list args;
    va_start(args, type);

    printf("Error (");

    vprintf(type, args);

    printf("): %s", cmd);

    printf("\n");

    va_end(args);
}

void xb_loop(void) {
    while (true) {
        char *str = xb_read_str("could not read input command");

        xb_commentln("input command '%s'", str);

        enum xb_in_cmd in_cmd = xb_str_to_in_cmd(str);

        switch (in_cmd) {
        case XB_IN_CMD_UNKNOWN:
            xb_err(str, "unknown command");
            continue;

        default:
            if (xb_in_cmds[in_cmd] == NULL) {
                xb_err(str, "unimplemented command");
                continue;
            }

            xb_in_cmds[in_cmd]();
        }

        free(str);
    }
}
