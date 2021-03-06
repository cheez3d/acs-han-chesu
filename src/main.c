#include "engine.h"
#include "xboard.h"

#include <libgen.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    (void)argc;

    char *engine_name = basename(argv[0]);

    engine_init(engine_name); // initialize engine
    atexit(engine_term); // register exit handler

    xb_loop(); // start xboard loop

    return EXIT_SUCCESS;
}
