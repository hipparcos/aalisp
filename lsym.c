#include "lsym.h"

#include <string.h>

#include "lbuiltin.h"

struct lsym_table {
    const char*        symbol;
    size_t             operands;
    const struct lsym* descriptor;
};

static struct lsym_table registered_symbols[] = {
    {"+", 2, &lbuiltin_op_add},
    {"-", 1, &lbuiltin_op_sub_unary},
    {"-", 2, &lbuiltin_op_sub},
    {"*", 2, &lbuiltin_op_mul},
    {"/", 2, &lbuiltin_op_div},
    {"%", 2, &lbuiltin_op_mod},
    {"^", 2, &lbuiltin_op_pow},
    {"!", 1, &lbuiltin_op_fac},
    {NULL, 0, NULL},
};

const struct lsym* lsym_lookup(const char* sym, size_t operands) {
    struct lsym_table* table = &registered_symbols[0];
    do {
        if (strcmp(sym, table->symbol) == 0) {
            if (table->operands < 2 && table->operands != operands) {
                continue;
            }
            return table->descriptor;
        }
    } while ((++table)->symbol);
    return NULL;
}


#define EITHER_IS(type, a, b) (lval_type(a) == type || lval_type(b) == type)
static enum ltype typeof_op(const struct lval* a, const struct lval *b) {
    if (EITHER_IS(LVAL_DBL, a, b))    return LVAL_DBL;
    if (EITHER_IS(LVAL_BIGNUM, a, b)) return LVAL_BIGNUM;
    if (EITHER_IS(LVAL_NUM, a, b))    return LVAL_NUM;
    return LVAL_NIL;
}

int lsym_exec(
    const struct lsym sym,
    struct lval* acc, const struct lval* x
) {
    /* Local x operand */
    const struct lval* lx = x;
    if (lval_is_nil(acc)) {
        lval_dup(acc, x);
        if (sym.unary) {
            lx = &lnil;
        } else {
            lx = sym.neutral;
        }
    }
    if (sym.unary && !lval_is_nil(lx)) {
        lval_mut_err(acc, LERR_TOO_MANY_ARGS);
        return 2;
    }

    /* Guards */
    int s = 0;
    for (int i = 0; i < sym.guardc; i++) {
        if ((s = (sym.guards[i]->condition)(acc, lx)) != 0) {
            lval_mut_err(acc, sym.guards[i]->error);
            return s;
        }
    }

    /* Eval */
    switch (typeof_op(acc, lx)) {
    case LVAL_DBL:
        {
        double a, b;
        lval_as_dbl(acc, &a);
        lval_as_dbl(lx, &b);
        lval_mut_dbl(acc, sym.op_dbl(a, b));
        return 0;
        }
    case LVAL_BIGNUM:
        {
        mpz_t a, b;
        mpz_init(a);
        mpz_init(b);
        lval_as_bignum(acc, a);
        lval_as_bignum(lx, b);
        mpz_t rbn;
        mpz_init(rbn);
        sym.op_bignum(rbn, a, b);
        mpz_clear(a);
        mpz_clear(b);
        lval_mut_bignum(acc, rbn);
        mpz_clear(rbn);
        return 0;
        }
    case LVAL_NUM:
        {
        long a, b;
        lval_as_num(acc, &a);
        lval_as_num(lx, &b);
        if (sym.cnd_overflow && sym.cnd_overflow(a, b)) {
            mpz_t bna;
            mpz_init_set_si(bna, a);
            lval_mut_bignum(acc, bna);
            int s = lsym_exec(sym, acc, lx);
            mpz_clear(bna);
            return s;
        }
        lval_mut_num(acc, sym.op_num(a, b));
        return 0;
        }
    default: break;
    }

    lval_mut_err(acc, LERR_EVAL);
    return -1;
}
