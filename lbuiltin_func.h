#ifndef _H_BUILTIN_FUNC_
#define _H_BUILTIN_FUNC_

#include "lval.h"
#include "lenv.h"

/** lbi_func_if is the conditional function. */
int lbi_func_if(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_loop is the while function. */
int lbi_func_loop(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_head returns the first element of a Q-Expression. */
int lbi_func_head(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_tail returns a Q-Expression without its head. */
int lbi_func_tail(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_init returns a Q-Expression without its last element. */
int lbi_func_init(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_last returns the last element of a Q-Expression. */
int lbi_func_last(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_index returns the ith element of a list. */
int lbi_func_index(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_elem tells if an element is present in the list. */
int lbi_func_elem(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_take creates a list with the first/last nth elements of a list. */
int lbi_func_take(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_drop drops the first/last ith element of a list. */
int lbi_func_drop(struct lenv* env, const struct lval* args, struct lval* acc);
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
/** lbi_func_map apply a function onto each element of a list. */
int lbi_func_map(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_filter creates a new list containing elements that check a condition. */
int lbi_func_filter(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_fold apply a function to all elements of a list. */
int lbi_func_fold(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_reverse reverses a list. */
int lbi_func_reverse(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_def defines a symbol in the global environment. */
int lbi_func_def(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_override overrides a symbol in env environment. */
int lbi_func_override(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_put defines a symbol in the local environment. */
int lbi_func_put(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_fun defines a function in the global environment. */
int lbi_func_fun(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_load loads a module in the global environment. */
int lbi_func_load(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_lambda defines a function. */
int lbi_func_lambda(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_pack packs a function and its arguments together. */
int lbi_func_pack(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_unpack evals a function and its list of arguments. */
int lbi_func_unpack(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_partial does partial application. */
int lbi_func_partial(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_print prints all its arguments. */
int lbi_func_print(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_debug_env returns the current environment as a Q-Expression. */
int lbi_func_debug_env(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_debug_fun returns a Q-Expr containing the arguments and the body of a function. */
int lbi_func_debug_fun(struct lenv* env, const struct lval* args, struct lval* acc);
/** lbi_func_debug_val returns a Q-Expr containing the type and the value of a value. */
int lbi_func_debug_val(struct lenv* env, const struct lval* args, struct lval* acc);

/** lbi_func_error throws an error from lisp. */
int lbi_func_error(struct lenv* env, const struct lval* args, struct lval* acc);

#endif
