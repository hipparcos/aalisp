#ifndef _H_LENV_
#define _H_LENV_

#include "lval.h"

/** lenv associates a lval with a name. */
struct lenv;

/** lenv_alloc creates a new lenv.
 ** Caller is responsible for calling lenv_free. */
struct lenv* lenv_alloc(void);
/** lenv_free frees env.
 ** env must not be used afterwards.*/
void lenv_free(struct lenv* env);
/** lenv_lookup returns the lval associated to sym. */
bool lenv_lookup(const struct lenv* env,
        const struct lval* sym, struct lval* result);
/** lenv_put bind val to sym in env. */
bool lenv_put(struct lenv* env,
        const struct lval* sym, const struct lval* val);
/** lenv_put_builtin binds val to sym in env (helper function). */
bool lenv_put_builtin(struct lenv* env,
        const char* symbol, const lbuiltin func);

/** lenv_default fills env with default builtin functions. */
bool lenv_default(struct lenv* env);

#endif
