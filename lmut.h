#ifndef _H_LMUT_
#define _H_LMUT_

#include <stdbool.h>

#include "lparser.h"
#include "lval.h"

/** lisp_mut mutates an ast into a lval sexpr.
 ** ast has to start with a LTAG_PROG node.
 ** error is set to the ast node that has generated an error or NULL.
 ** Caller is responsible for freeing returned lval. */
struct lval* lisp_mut(const struct last* ast, struct last** error);

#endif
