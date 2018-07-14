#ifndef _H_BUILTIN_
#define _H_BUILTIN_

#include <stdbool.h>

#include "lval.h"

/* Conditions: eval a condition for the given operands. */
typedef bool (*condition_func)(struct lval, struct lval);

/* Guard: a condition associated with an error. */
struct guard {
    condition_func condition;
    enum lerr error;
};

/* Operators: generic */
struct op_descriptor {
    const char*          symbol;
    bool                 unary;
    const struct guard** guards;
    int                  guardc;
    const struct lval*   neutral;
    long   (*op_num)(const long, const long);
    bool   (*cnd_overflow)(const long, const long);
    void   (*op_bignum)(mpz_t r, const mpz_t x, const mpz_t y);
    double (*op_dbl)(const double, const double);
};

struct lval lisp_builtin_op(
    const struct op_descriptor descriptor,
    const struct lval x, struct lval y);

extern const struct op_descriptor builtin_op_add;
extern const struct op_descriptor builtin_op_sub;
extern const struct op_descriptor builtin_op_mul;
extern const struct op_descriptor builtin_op_div;
extern const struct op_descriptor builtin_op_mod;
extern const struct op_descriptor builtin_op_fac;
extern const struct op_descriptor builtin_op_pow;

#endif
