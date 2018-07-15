#include "lsym.h"

struct lval lsym_exec(
    const struct lsym sym,
    const struct lval x, struct lval y
) {

    if (!sym.unary && y.type == LVAL_NIL) {
        y = *sym.neutral;
    }

    /* Guards */
    for (int i = 0; i < sym.guardc; i++) {
        if ((sym.guards[i]->condition)(x, y)) {
            return lval_err(sym.guards[i]->error);
        }
    }

    /* Eval: double */
    if (x.type == LVAL_DBL || y.type == LVAL_DBL) {
        double a = lval_as_dbl(x);
        double b = lval_as_dbl(y);
        return lval_dbl(sym.op_dbl(a, b));
    }
    /* Eval: bignum */
    if (x.type == LVAL_BIGNUM || y.type == LVAL_BIGNUM) {
        mpz_t a, b;
        lval_as_bignum(x, a);
        lval_as_bignum(y, b);
        mpz_t r;
        mpz_init(r);
        sym.op_bignum(r, a, b);
        mpz_clear(a);
        mpz_clear(b);
        struct lval ret = lval_bignum(r);
        mpz_clear(r);
        return ret;
    }
    /* Eval: num */
    if (x.type == LVAL_NUM && (y.type == LVAL_NUM || y.type == LVAL_NIL)) {
        long a = x.data.num;
        long b = y.data.num;
        if (sym.cnd_overflow && sym.cnd_overflow(a, b)) {
            mpz_t bna, bnb;
            lval_as_bignum(x, bna);
            lval_as_bignum(y, bnb);
            struct lval vbna = lval_bignum(bna);
            struct lval vbnb = lval_bignum(bnb);
            struct lval res = lsym_exec(sym, vbna, vbnb);
            mpz_clear(bna);
            mpz_clear(bnb);
            lval_clear(&vbna);
            lval_clear(&vbnb);
            return res;
        }
        return lval_num(sym.op_num(a, b));
    }

    return lval_err(LERR_EVAL);
}
