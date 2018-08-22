#ifndef _H_LMUT_
#define _H_LMUT_

#include <stdbool.h>

#include "lparser.h"
#include "lval.h"
#include "lerr.h"

/** lisp_mut mutates an ast into a lval sexpr.
 ** ast has to start with a LTAG_PROG node.
 ** err is allocated in case of error.
 ** Caller is responsible for freeing returned lval. */
struct lval* lisp_mut(const struct last* ast, struct lerr** error);

#endif
