#ifndef H_LMUTATOR_
#define H_LMUTATOR_

#include <stdbool.h>

#include "lerr.h"
#include "lparser.h"
#include "lval.h"

/** lmutate mutates an ast into a lval sexpr.
 ** ast has to start with a LTAG_PROG node.
 ** err is allocated in case of error.
 ** Caller is responsible for freeing returned lval. */
struct lval* lmutate(const struct last* ast, struct lerr** error);

#endif
