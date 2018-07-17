#ifndef _H_SYMBOL_
#define _H_SYMBOL_

#include "lval.h"

/* Condition: eval a condition for the given operands. */
typedef bool (*lcondition)(const struct lval*, const struct lval*);

/* Guard: a condition associated with an error. */
struct lguard {
    lcondition condition;
    enum lerr  error;
};

/* Symbol: generic */
struct lsym {
    const char*           symbol;
    bool                  unary;
    const struct lguard** guards;
    int                   guardc;
    const struct lval*    neutral;
    long   (*op_num)(const long, const long);
    bool   (*cnd_overflow)(const long, const long);
    void   (*op_bignum)(mpz_t r, const mpz_t x, const mpz_t y);
    double (*op_dbl)(const double, const double);
};

bool lsym_exec(
    const struct lsym sym,
    const struct lval* x, const struct lval* y, struct lval* r);

#endif
