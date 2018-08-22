#include "lbuiltin.h"

#include "lfunc.h"
#include "lbuiltin_condition.h"
#include "lbuiltin_func.h"
#include "lbuiltin_operator.h"

/* Local macros. */
#define LENGTH(array) sizeof(array)/sizeof(array[0])
#define inline_ptr(type,val) (&((type[]){val})[0])

/* Operator: declaration */
static const struct lguard guards_op_add[] = {
    {.argn= 0, .condition= use_condition(must_be_numeric)},
};
const struct lfunc lbuiltin_op_add = {
    .symbol       = "+",
    .accumulator  = true,
    .neutral      = &lzero,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_add[0],
    .guardc       = LENGTH(guards_op_add),
    .func         = lbi_op_add,
};

static const struct lguard guards_op_sub[] = {
    {.argn= 0, .condition= use_condition(must_be_numeric)},
};
const struct lfunc lbuiltin_op_sub = {
    .symbol       = "-",
    .accumulator  = true,
    .neutral      = &lzero,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_sub[0],
    .guardc       = LENGTH(guards_op_sub),
    .func         = lbi_op_sub,
};

static const struct lguard guards_op_mul[] = {
    {.argn= 0, .condition= use_condition(must_be_numeric)},
};
const struct lfunc lbuiltin_op_mul = {
    .symbol       = "*",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_mul[0],
    .guardc       = LENGTH(guards_op_mul),
    .func         = lbi_op_mul,
};

static const struct lguard guards_op_div[] = {
    {.argn= 0, .condition= use_condition(must_be_numeric)},
    {.argn= -1, .condition= use_condition(divisor_must_be_non_zero)},
};
const struct lfunc lbuiltin_op_div = {
    .symbol       = "/",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_div[0],
    .guardc       = LENGTH(guards_op_div),
    .func         = lbi_op_div,
};

static const struct lguard guards_op_mod[] = {
    {.argn= 0, .condition= use_condition(must_be_integral)},
    {.argn= -1, .condition= use_condition(divisor_must_be_non_zero)},
};
const struct lfunc lbuiltin_op_mod = {
    .symbol       = "%",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_mod[0],
    .guardc       = LENGTH(guards_op_mod),
    .func         = lbi_op_mod,
};

static const struct lguard guards_op_fac[] = {
    {.argn= 0, .condition= use_condition(must_be_integral)},
    {.argn= 0, .condition= use_condition(must_be_positive)},
    {.argn= 0, .condition= use_condition(must_be_unsigned_long)},
};
const struct lfunc lbuiltin_op_fac = {
    .symbol       = "!",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_op_fac[0],
    .guardc       = LENGTH(guards_op_fac),
    .func         = lbi_op_fac,
};

static const struct lguard guards_op_pow[] = {
    {.argn= 0, .condition= use_condition(must_be_numeric)},
    {.argn= 0, .condition= use_condition(must_be_unsigned_long)},
};
const struct lfunc lbuiltin_op_pow = {
    .symbol       = "^",
    .accumulator  = true,
    .neutral      = &lone,
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_op_pow[0],
    .guardc       = LENGTH(guards_op_pow),
    .func         = lbi_op_pow,
};

const struct lfunc lbuiltin_op_eq = {
    .symbol       = "==",
    .accumulator  = true,
    .neutral      = &lnil,
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = NULL,
    .guardc       = 0,
    .func         = lbi_op_eq,
};

const struct lfunc lbuiltin_op_neq = {
    .symbol       = "!=",
    .accumulator  = true,
    .neutral      = &lnil,
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = NULL,
    .guardc       = 0,
    .func         = lbi_op_neq,
};

static const struct lguard guards_list_op[] = {
    {.argn= 1, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
};
const struct lfunc lbuiltin_head = {
    .symbol       = "head",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_list_op[0],
    .guardc       = LENGTH(guards_list_op),
    .func         = lbi_func_head,
};
const struct lfunc lbuiltin_tail = {
    .symbol       = "tail",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_list_op[0],
    .guardc       = LENGTH(guards_list_op),
    .func         = lbi_func_tail,
};
const struct lfunc lbuiltin_init = {
    .symbol       = "init",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_list_op[0],
    .guardc       = LENGTH(guards_list_op),
    .func         = lbi_func_init,
};
const struct lfunc lbuiltin_last = {
    .symbol       = "last",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_list_op[0],
    .guardc       = LENGTH(guards_list_op),
    .func         = lbi_func_last,
};

static const struct lguard guards_cons[] = {
    {.argn= 2, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
};
const struct lfunc lbuiltin_cons = {
    .symbol       = "cons",
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = &guards_cons[0],
    .guardc       = LENGTH(guards_cons),
    .func         = lbi_func_cons,
};

static const struct lguard guards_len[] = {
    {.argn= 1, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
};
const struct lfunc lbuiltin_len = {
    .symbol       = "len",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_len[0],
    .guardc       = LENGTH(guards_len),
    .func         = lbi_func_len,
};

static const struct lguard guards_join[] = {
    {.argn= 0, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
};
const struct lfunc lbuiltin_join = {
    .symbol       = "join",
    .accumulator  = true,
    .neutral      = &lemptyq,
    .init_neutral = true,
    .min_argc     =  0,
    .max_argc     = -1,
    .guards       = &guards_join[0],
    .guardc       = LENGTH(guards_join),
    .func         = lbi_func_join,
};

static const struct lguard guards_list[] = {
};
const struct lfunc lbuiltin_list = {
    .symbol       = "list",
    .accumulator  = true,
    .neutral      = &lemptyq,
    .init_neutral = true,
    .min_argc     =  0,
    .max_argc     = -1,
    .guards       = &guards_list[0],
    .guardc       = LENGTH(guards_list),
    .func         = lbi_func_list,
};

static const struct lguard guards_eval[] = {
};
const struct lfunc lbuiltin_eval = {
    .symbol       = "eval",
    .min_argc     =  1,
    .max_argc     =  1,
    .guards       = &guards_eval[0],
    .guardc       = LENGTH(guards_eval),
    .func         = lbi_func_eval,
};

static const struct lguard guards_def[] = {
    {.argn= 1, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
    {.argn= 1, .condition= use_condition(must_be_list_of),
        .param= inline_ptr(enum ltype, LVAL_SYM)},
    {.argn= -1, .condition= use_condition(must_be_of_equal_len)},
};
const struct lfunc lbuiltin_def = {
    .symbol       = "def",
    .min_argc     =  2,
    .max_argc     =  -1,
    .guards       = &guards_def[0],
    .guardc       = LENGTH(guards_def),
    .func         = lbi_func_def,
};
const struct lfunc lbuiltin_override = {
    .symbol       = "ovr",
    .min_argc     =  2,
    .max_argc     =  -1,
    .guards       = &guards_def[0],
    .guardc       = LENGTH(guards_def),
    .func         = lbi_func_override,
};
const struct lfunc lbuiltin_put = {
    .symbol       = "put",
    .min_argc     =  2,
    .max_argc     =  -1,
    .guards       = &guards_def[0],
    .guardc       = LENGTH(guards_def),
    .func         = lbi_func_put,
};

static const struct lguard guards_fun[] = {
    {.argn= 0, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
    {.argn= 1, .condition= use_condition(must_be_list_of),
        .param= inline_ptr(enum ltype, LVAL_SYM)},
    {.argn= 1, .condition= use_condition(must_have_min_len),
        .param= inline_ptr(size_t, 1)},
};
const struct lfunc lbuiltin_fun = {
    .symbol       = "fun",
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = &guards_fun[0],
    .guardc       = LENGTH(guards_fun),
    .func         = lbi_func_fun,
};

static const struct lguard guards_lambda[] = {
    {.argn= 0, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
    {.argn= 1, .condition= use_condition(must_be_list_of),
        .param= inline_ptr(enum ltype, LVAL_SYM)},
};
const struct lfunc lbuiltin_lambda = {
    .symbol       = "lambda",
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = &guards_lambda[0],
    .guardc       = LENGTH(guards_lambda),
    .func         = lbi_func_lambda,
};

static const struct lguard guards_pack[] = {
    {.argn= 1, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_FUNC)},
};
const struct lfunc lbuiltin_pack = {
    .symbol       = "pack",
    .min_argc     =  1,
    .max_argc     = -1,
    .guards       = &guards_pack[0],
    .guardc       = LENGTH(guards_pack),
    .func         = lbi_func_pack,
};

static const struct lguard guards_unpack[] = {
    {.argn= 1, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_FUNC)},
    {.argn= 2, .condition= use_condition(must_be_of_type),
        .param= inline_ptr(enum ltype, LVAL_QEXPR)},
};
const struct lfunc lbuiltin_unpack = {
    .symbol       = "unpack",
    .min_argc     =  2,
    .max_argc     =  2,
    .guards       = &guards_unpack[0],
    .guardc       = LENGTH(guards_unpack),
    .func         = lbi_func_unpack,
};

const struct lfunc lbuiltin_print = {
    .symbol       = "print",
    .min_argc     =  -1,
    .max_argc     =  -1,
    .guards       = NULL,
    .guardc       = 0,
    .func         = lbi_func_print,
};
