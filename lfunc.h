#ifndef _H_LFUNC_
#define _H_LFUNC_

#include "lval.h"
#include "lenv.h"

struct lfunc;

/** lcondition evaluates a condition for the given operands.
 **   0 if success
 **  -1 if error
 **   n if nth argument generate an error */
typedef int (*lcondition)(const struct lfunc*, const struct lenv*, const struct lval*);

/** lguard is a condition associated with an error. */
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

/** lbuiltin is a pointer to a builtin function.
 ** lbuiltin returns:
 **   0 if success
 **  -1 if error
 **   n if nth argument generates an error */
typedef int (*lbuiltin)(
        struct lenv* env, const struct lval* args, struct lval* result);

/** lfunc describes a builtin function. */
struct lfunc {
    const char* symbol;
    /** lfunc.max_argc is the maximum number of arguments. */
    int max_argc;
    /** lfunc.min_argc is the minimum number of arguments. */
    int min_argc;
    /** lfunc.accumulator tells if the function is of accumulator type. */
    bool accumulator;
    /** lfunc.guards are functions that prevent execution of the function. */
    const struct lguard* guards;
    int guardc; /* Number of guards. */
    /** lfunc.init_neutral tells if the initial value of acc must be the neutral elem. */
    bool init_neutral;
    /** lfunc.neutral is the neutral element. */
    const struct lval* neutral;
    /** lfunc.func is the associated builtin function. */
    lbuiltin func;
};

/** lfunc_exec is a lbuiltin.
 ** lfunc_exec returns:
 **   0 if success
 **  -1 if error
 **   n if nth argument generate an error */
int lfunc_exec(
        const struct lfunc* fun, struct lenv* env, const struct lval* args, struct lval* acc);

#endif
