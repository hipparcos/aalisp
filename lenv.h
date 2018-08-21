#ifndef _H_LENV_
#define _H_LENV_

#include <stdbool.h>

#include "lval.h"

/** lenv associates a lval with a name. */
struct lenv;

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
/** lenv_len returns the number of symbols contained into env and its parents. */
size_t lenv_len(const struct lenv* env);
/** lenv_print_to prints the list of defined symbols to the out file. */
void lenv_print_to(const struct lenv* env, FILE* out);
/** lenv_print prints th elist of defined symbols to stdout. */
#define lenv_print(env) lenv_print_to(env, stdout)

/** lenv_set_parent sets env parent to par. */
bool lenv_set_parent(struct lenv* env, struct lenv* par);
/** lenv_lookup returns the lval associated to sym. */
bool lenv_lookup(const struct lenv* env,
        const struct lval* sym, struct lval* result);
/** lenv_put binds val to sym in env. */
bool lenv_put(struct lenv* env,
        const struct lval* sym, const struct lval* val);
/** lenv_override tries to override sym in env and its parents. */
bool lenv_override(struct lenv* env,
        const struct lval* sym, const struct lval* val);
/** lenv_def binds val to sym in env outermost parent. */
bool lenv_def(struct lenv* env,
        const struct lval* sym, const struct lval* val);

/** lenv_default fills env with default builtin functions. */
bool lenv_default(struct lenv* env);

#endif
