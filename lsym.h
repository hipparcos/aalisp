#ifndef _H_SYMBOL_
#define _H_SYMBOL_

#include "lval.h"

/* Condition: eval a condition for the given operands. */
/** lcondition_bin returns either true or false. */
typedef bool (*lcondition_bin)(const struct lval*, const struct lval*);
/** lcondition_tern returns 0 if true, -1 if false,
 **   1 if first arg gives false, 2 if second arg gives false. */
typedef int (*lcondition_tern)(const struct lval*, const struct lval*);

/* Guard: a condition associated with an error. */
struct lguard {
    lcondition_tern condition;
    enum lerr error;
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

/** lsym_exec returns
 **   0 if success
 **  -1 if error
 **   1 if acc generate an error
 **   2 if x generate an error */
int lsym_exec(
    const struct lsym sym,
    struct lval* acc, const struct lval* x);

#endif
