#ifndef BOOK_H
#define BOOK_H

#include "board.h"
#include "move.h"

void bk_init(void);

void bk_term(void);

struct move bk_search(struct board *board);

#endif // BOOK_H
