#ifndef _H_BUILTIN_EXEC_
#define _H_BUILTIN_EXEC_

#include "lval.h"
#include "lenv.h"

/* Condition: eval a condition for the given operands. */
typedef int (*lcondition)(const struct lenv*, const struct lval*);

/* Guard: a condition associated with an error. */
struct lguard {
    /** lguard.condition is the condition to be verified. */
    lcondition condition;
    /** lguard.argn is the number of the argument concerned
     **   >0 applied on nth args;
     **    0 applied on each args;
     **   -1 applied on all args. */
    int argn;
    /** lguard.error is the error returned. */
    enum lerr error;
};

/** descriptor describes a builtin function. */
struct ldescriptor {
    const char* symbol;
    /** lsym.max_argc is the maximum number of arguments. */
    int max_argc;
    /** lsym.min_argc is the minimum number of arguments. */
    int min_argc;
    /** lsym.accumulator tells if the function is of accumulator type. */
    bool accumulator;
    /** lsym.guards are functions that prevent execution of the symbol. */
    const struct lguard* guards;
    int guardc; /* Number of guards. */
    /** lsym.init_neutral tells if the initial value of acc must be the neutral elem. */
    bool init_neutral;
    /** lsym.neutral is the neutral element. */
    const struct lval* neutral;
    /** cnd_overflow tells if an operation on nums must be casted to bignum. */
    bool   (*cnd_overflow)(const long, const long);
    /** op_* operates on basic types: dbl > bignum > num. */
    long   (*op_num)(const long, const long);
    void   (*op_bignum)(mpz_t r, const mpz_t x, const mpz_t y);
    double (*op_dbl)(const double, const double);
    /** op_all operates of lval. If op_all is not null, only op_all is executed. */
    int    (*op_all)(struct lenv* env, struct lval* acc, const struct lval* args);
};

/** lbuiltin_exec is a lbuiltin.
 ** lbuiltin_exec returns:
 **   0 if success
 **  -1 if error
 **   n if nth argument generate an error */
int lbuiltin_exec(const struct ldescriptor* sym, struct lenv* env,
        const struct lval* args, struct lval* acc);

#endif
