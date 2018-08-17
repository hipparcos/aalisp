#ifndef _H_LENV_
#define _H_LENV_

#include <stdbool.h>

#include "lval.h"

/** lenv associates a lval with a name. */
struct lenv;

/* Forward declaration of lfunc, see lfunc.h */
struct lfunc;

/** lenv_alloc creates a new lenv.
 ** Caller is responsible for calling lenv_free. */
struct lenv* lenv_alloc(void);
/** lenv_free frees env.
 ** env must not be used afterwards.*/
void lenv_free(struct lenv* env);
/** lenv_copy copies src into dest. */
bool lenv_copy(struct lenv* dest, const struct lenv* src);
/** lenv_are_equal tells if two env contains the same symbols. */
bool lenv_are_equal(const struct lenv*, const struct lenv*);

/** lenv_set_parent set env parent to par. */
bool lenv_set_parent(struct lenv* env, struct lenv* par);
/** lenv_lookup returns the lval associated to sym. */
bool lenv_lookup(const struct lenv* env,
        const struct lval* sym, struct lval* result);
/** lenv_put binds val to sym in env. */
bool lenv_put(struct lenv* env,
        const struct lval* sym, const struct lval* val);
/** lenv_put_builtin binds val to sym in env (helper function). */
bool lenv_put_builtin(struct lenv* env,
        const char* symbol, const struct lfunc* func);
/** lenv_def binds val to sym in env outermost parent. */
bool lenv_def(struct lenv* env,
        const struct lval* sym, const struct lval* val);

/** lenv_default fills env with default builtin functions. */
bool lenv_default(struct lenv* env);

#endif
