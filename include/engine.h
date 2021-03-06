#ifndef ENGINE_H
#define ENGINE_H

#include "board.h"
#include "move.h"
#include "movegen.h"
#include "xboard.h"

#include <stdbool.h>

struct engine {
    const char *name;
    char *name_other;
    struct xboard xboard;

    bool random;
    bool force;
    bool computer;
    bool hard;
    bool post;

    int time;
    int time_other;

    struct board board;
};

/**
 * Variable that stores the engine.
 */
extern struct engine engine;

/**
 * Initialize the engine.
 */
void engine_init(const char *name);

/**
 * Terminate the engine.
 */
void engine_term(void);

/**
 * Reset the engine to the initial game state.
 */
void engine_reset(void);

/**
 * Instruct the engine to process the move received from xboard
 * and update its internal state accordingly.
 */
void engine_recv_move(struct move move);

/**
 * Calculate the next best move and make it and send it to xboard.
 */
void engine_send_move(void);

#endif // ENGINGE_H
