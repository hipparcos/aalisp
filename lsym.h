#ifndef _H_SYMBOL_
#define _H_SYMBOL_

#include "lval.h"
#include "lbuiltin.h"

/* Condition: eval a condition for the given operands. */
/** lcondition returns 0 if true, -1 if false,
 **   1 if first arg gives false, 2 if second arg gives false. */
typedef int (*lcondition)(const struct lval*, const struct lval*);

/* Guard: a condition associated with an error. */
struct lguard {
    lcondition condition;
    enum lerr error;
};

/* Symbol: generic */
struct lsym {
    const char* symbol;
    /** lsym.unary tells is the symbol takes only one argument. */
    bool unary;
    /** lsym.swap tells if the operands need to be swaped before execution. */
    bool swap;
    /** lsym.guards are functions that prevent execution of the symbol. */
    const struct lguard* guards;
    int guardc; /* Number of guards. */
    /** lsym.neutral is the neutral element. */
    const struct lval* neutral;
    /** cnd_overflow tells if an operation on nums must be casted to bignum. */
    bool   (*cnd_overflow)(const long, const long);
    /** op_* operates on basic types: dbl > bignum > num. */
    long   (*op_num)(const long, const long);
    void   (*op_bignum)(mpz_t r, const mpz_t x, const mpz_t y);
    double (*op_dbl)(const double, const double);
    /** op_all operates of lval. If op_all is not null, only op_all is executed. */
    int    (*op_all)(struct lval* acc, const struct lval* x);
};

/** lsym_table associates a symbol descriptor with a string.
 ** A valid lsym_table ends with a {NULL, NULL} row. */
struct lsym_table {
    const char*        symbol;
    const struct lsym* descriptor;
};

/** lsym_lookup returns the lsym associated to sym. */
const struct lsym* lsym_lookup(const struct lsym_table* table, const char* sym, size_t operands);

/** lsym_exec returns
 **   0 if success
 **  -1 if error
 **   1 if acc generate an error
 **   2 if x generate an error */
int lsym_exec(
    const struct lsym* sym,
    struct lval* acc, const struct lval* x);

#endif
