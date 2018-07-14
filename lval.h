#ifndef _H_LVAL_
#define _H_LVAL_

#include <stdbool.h>
#include <stdio.h>

#include "vendor/mini-gmp/mini-gmp.h"

enum ltype {
    LVAL_NIL,
    LVAL_NUM,
    LVAL_BIGNUM,
    LVAL_DBL,
    LVAL_ERR
};

enum lerr {
    LERR_EVAL,
    LERR_DIV_ZERO,
    LERR_BAD_OP,
    LERR_BAD_NUM,
    LERR_TOO_MANY_ARGS
};

/* lval is the return type of an evalution. */
struct lval {
    enum ltype type;
    union {
        long num;
        mpz_t bignum;
        double dbl;
        enum lerr err;
    } data;
};

extern const struct lval lzero;
extern const struct lval lone;

struct lval lval_nil();
struct lval lval_num(long x);
struct lval lval_bignum(mpz_t x);
struct lval lval_dbl(double x);
struct lval lval_err(enum lerr err);
void lval_clear(struct lval*);

bool lval_is_zero(struct lval v);
double lval_as_dbl(struct lval v);
void lval_as_bignum(struct lval v, mpz_t r);
bool lval_equals(struct lval x, struct lval y);
bool lval_err_equals(struct lval x, struct lval y);

void lval_print_to(struct lval v, FILE* out);
void lval_to_string(struct lval v, char* out);

inline void lval_print(struct lval v)   { lval_print_to(v, stdout); }
inline void lval_println(struct lval v) { lval_print_to(v, stdout); putchar('\n'); }

#endif
