#include "lbuiltin_exec.h"

#include <string.h>

#include "lval.h"
#include "lenv.h"

static int lbuitin_check_guards(const struct ldescriptor* sym, const struct lenv* env,
        const struct lval* args, struct lval* acc) {
    for (int i = 0; i < sym->guardc; i++) {
        const struct lguard* guard = &sym->guards[i];
        /* Guard applied on a specific argument. */
        if (guard->argn > 0) {
            struct lval* child = lval_alloc();
            lval_index(args, guard->argn-1, child);
            if (0 != (guard->condition)(env, child)) {
                lval_mut_err(acc, guard->error);
                lval_free(child);
                return guard->argn;
            }
            lval_free(child);
            continue;
        }
        /* Guard applied on each args. */
        if (guard->argn == 0) {
            size_t len = lval_len(args);
            for (size_t a = 0; a < len; a++) {
                struct lval* child = lval_alloc();
                lval_index(args, a, child);
                if (0 != (guard->condition)(env, child)) {
                    lval_mut_err(acc, guard->error);
                    lval_free(child);
                    return a + 1;
                }
                lval_free(child);
            }
            continue;
        }
        /* Guard applied on all args. */
        // guard->argn == -1;
        int s = 0;
        if (0 != (s = (guard->condition)(env, args))) {
            lval_mut_err(acc, guard->error);
            return s;
        }
    }
    return 0;
}

static int lbuiltin_exec_arg(const struct ldescriptor* sym, struct lenv* env,
        const struct lval* args, struct lval* acc) {
    if (!sym->op_all) {
        lval_mut_err(acc, LERR_EVAL);
        return -1;
    }
    /* Execution */
    return sym->op_all(env, acc, args);
}

#define EITHER_IS(type, a, b) (lval_type(a) == type || lval_type(b) == type)
static enum ltype typeof_op(const struct lval* a, const struct lval *b) {
    if (EITHER_IS(LVAL_DBL, a, b))    return LVAL_DBL;
    if (EITHER_IS(LVAL_BIGNUM, a, b)) return LVAL_BIGNUM;
    if (EITHER_IS(LVAL_NUM, a, b))    return LVAL_NUM;
    return LVAL_NIL;
}

static int lbuiltin_exec_acc(const struct ldescriptor* sym, struct lenv* env,
        const struct lval* arg, struct lval* acc) {
    if (sym->op_all) {
        return sym->op_all(env, acc, arg);
    }
    /* Accumulator based evaluation. */
    switch (typeof_op(acc, arg)) {
    case LVAL_DBL:
        {
        double a, b;
        lval_as_dbl(acc, &a);
        lval_as_dbl(arg, &b);
        lval_mut_dbl(acc, sym->op_dbl(a, b));
        return 0;
        }
    case LVAL_BIGNUM:
        {
        mpz_t a, b;
        mpz_init(a);
        mpz_init(b);
        lval_as_bignum(acc, a);
        lval_as_bignum(arg, b);
        mpz_t rbn;
        mpz_init(rbn);
        sym->op_bignum(rbn, a, b);
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
        lval_as_num(arg, &b);
        if (sym->cnd_overflow && sym->cnd_overflow(a, b)) {
            mpz_t bna;
            mpz_init_set_si(bna, a);
            lval_mut_bignum(acc, bna);
            int s = lbuiltin_exec_acc(sym, env, arg, acc);
            mpz_clear(bna);
            return s;
        }
        lval_mut_num(acc, sym->op_num(a, b));
        return 0;
        }
    default: break;
    }

    lval_mut_err(acc, LERR_EVAL);
    return -1;
}

int lbuiltin_exec(const struct ldescriptor* sym, struct lenv* env,
        const struct lval* args, struct lval* acc) {
    if (!sym) {
        lval_mut_err(acc, LERR_EVAL);
        return -1;
    }
    /* Check number of arguments. */
    size_t len = lval_len(args);
    int max = sym->max_argc;
    if (max != -1 && (int)len > max) {
        lval_mut_err(acc, LERR_TOO_MANY_ARGS);
        return -1;
    }
    int min = sym->min_argc;
    if (min != -1 && (int)len < min) {
        lval_mut_err(acc, LERR_TOO_FEW_ARGS);
        return -1;
    }
    /* Guards */
    int s = 0;
    if (0 != (s = lbuitin_check_guards(sym, env, args, acc))) {
        return s;
    }
    /* Argument execution. */
    if (!sym->accumulator) {
        return lbuiltin_exec_arg(sym, env, args, acc);
    }
    /* Accumulator execution. */
    if (len == 1) {
        /* Special case for unary operations. */
        struct lval* lx = lval_alloc();
        lval_index(args, 0, lx);      // lx is the first argument.
        lval_copy(acc, sym->neutral); // acc is the neutral element.
        int s = lbuiltin_exec_acc(sym, env, lx, acc);
        lval_free(lx);
        return s;
    }
    if (sym->init_neutral) {
        /* Init acc with neutral. */
        lval_copy(acc, sym->neutral);
    } else {
        /* Init acc with first argument. */
        lval_index(args, 0, acc);
    }
    for (size_t c = (sym->init_neutral) ? 0 : 1; c < len; c++) {
        struct lval* child = lval_alloc();
        lval_index(args, c, child);
        int err = lbuiltin_exec_acc(sym, env, child, acc);
        /* Break on error. */
        if (err != 0) {
            if (err == -1) {
                s = err;
            } else {
                s = c + 1;
            }
            break;
        }
        lval_free(child);
    }
    return s;
}
