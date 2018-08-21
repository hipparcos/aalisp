#ifndef _H_BUILTIN_FUNC_
#define _H_BUILTIN_FUNC_

#include "lval.h"
#include "lenv.h"

/** lbi_func_head returns the first element of a Q-Expression. */
int lbi_func_head(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_tail returns a Q-Expression without its head. */
int lbi_func_tail(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_init returns a Q-Expression without its last element. */
int lbi_func_init(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_last returns the last element of a Q-Expression. */
int lbi_func_last(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_cons adds an element x at the beginning of acc. */
int lbi_func_cons(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_len returns the len of a Q-Expression. */
int lbi_func_len(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_join push all children of x in acc. */
int lbi_func_join(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_list push x in to acc. acc is mutated into a qexpr. */
int lbi_func_list(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_eval evaluates x and put the result into acc. */
int lbi_func_eval(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_def defines a symbol in the global environment. */
int lbi_func_def(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_put defines a symbol in the local environment. */
int lbi_func_put(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_fun defines a function in the global environment. */
int lbi_func_fun(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_lambda defines a function. */
int lbi_func_lambda(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_pack packs a function and its arguments together. */
int lbi_func_pack(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_unpack evals a function and its list of arguments. */
int lbi_func_unpack(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_print prints all its arguments. */
int lbi_func_print(struct lenv* env, const struct lval* args, struct lval* acc);

#endif
