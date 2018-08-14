#ifndef _H_BUILTIN_FUNC_
#define _H_BUILTIN_FUNC_

#include "lval.h"

int lbi_cond_qexpr(const struct lval* x, const struct lval* y);
int lbi_cond_list(const struct lval* x, const struct lval* y);

/** lbi_func_head returns the first element of a Q-Expression. */
int lbi_func_head(struct lval* acc, const struct lval* x);
/** lbi_func_tail returns a Q-Expression without its head. */
int lbi_func_tail(struct lval* acc, const struct lval* x);
/** lbi_func_join push all children of x in acc. */
int lbi_func_join(struct lval* acc, const struct lval* x);

#endif
