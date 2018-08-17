#ifndef _H_LBUILTIN_CONDITION_
#define _H_LBUILTIN_CONDITION_

#include "lval.h"
#include "lenv.h"
#include "lbuiltin_exec.h"

/* Conditions on ldescriptor. */
int lbi_cond_max_argc(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* args);
int lbi_cond_min_argc(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* args);
int lbi_cond_func_pointer(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* args);

/* Operators conditions. */
int lbi_cond_is_not_zero(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_is_positive(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_is_integral(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_is_numeric(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_x_is_ul(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);

/* List conditions. */
int lbi_cond_qexpr(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_list_of_sym(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_qexpr_or_nil(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);
int lbi_cond_list(
        const struct ldescriptor* sym, const struct lenv* env, const struct lval* arg);

#endif
