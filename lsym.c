#include "lsym.h"

bool lsym_exec(
    const struct lsym sym,
    const struct lval* x, const struct lval* y, struct lval* r
) {
    /* local y operand */
    const struct lval* ly;
    if (sym.unary) {
        if (lval_type(y) != LVAL_NIL) {
            lval_mut_err(r, LERR_TOO_MANY_ARGS);
            return false;
        }
        ly = (struct lval*) &lnil;
    } else {
        ly = y;
        if (lval_type(y) == LVAL_NIL) {
            ly = sym.neutral;
        }
    }

    /* Guards */
    for (int i = 0; i < sym.guardc; i++) {
        if ((sym.guards[i]->condition)(x, ly)) {
            lval_mut_err(r, sym.guards[i]->error);
            return false;
        }
    }

    /* Eval: double */
    if (lval_type(x) == LVAL_DBL || lval_type(ly) == LVAL_DBL) {
        double a, b;
        lval_as_dbl(x, &a);
        lval_as_dbl(ly, &b);
        lval_mut_dbl(r, sym.op_dbl(a, b));
        return true;
    }
    /* Eval: bignum */
    if (lval_type(x) == LVAL_BIGNUM || lval_type(ly) == LVAL_BIGNUM) {
        mpz_t a, b;
        mpz_init(a);
        mpz_init(b);
        lval_as_bignum(x, a);
        lval_as_bignum(ly, b);
        mpz_t rbn;
        mpz_init(rbn);
        sym.op_bignum(rbn, a, b);
        mpz_clear(a);
        mpz_clear(b);
        lval_mut_bignum(r, rbn);
        mpz_clear(rbn);
        return true;
    }
    /* Eval: num */
    if (lval_type(x) == LVAL_NUM && (lval_type(ly) == LVAL_NUM || lval_type(ly) == LVAL_NIL)) {
        long a, b;
        lval_as_num(x, &a);
        lval_as_num(ly, &b);
        if (sym.cnd_overflow && sym.cnd_overflow(a, b)) {
            mpz_t bna, bnb;
            mpz_init_set_si(bna, a);
            struct lval* vbna = lval_alloc();
            lval_mut_bignum(vbna, bna);
            struct lval* vbnb = lval_alloc();
            if (!sym.unary) {
                mpz_init_set_si(bnb, b);
                lval_mut_bignum(vbnb, bnb);
            }
            bool ret = lsym_exec(sym, vbna, vbnb, r);
            if (!sym.unary) {
                mpz_clear(bnb);
            }
            mpz_clear(bna);
            lval_free(vbna);
            lval_free(vbnb);
            return ret;
        }
        lval_mut_num(r, sym.op_num(a, b));
        return true;
    }

    lval_mut_err(r, LERR_EVAL);
    return false;
}
