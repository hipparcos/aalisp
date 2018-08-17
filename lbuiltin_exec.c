#include "lbuiltin_exec.h"

#include <string.h>

#include "lval.h"
#include "lenv.h"
#include "lbuiltin_condition.h"

static int lbuitin_check_guards(const struct ldescriptor* sym,
        const struct lguard* guards, size_t guardc,
        const struct lenv* env, const struct lval* args, struct lval* acc) {
    int s = 0;
    const struct lguard* guard = NULL;
    for (size_t g = 0; g < guardc; g++) {
        guard = guards+g;
        /* Guard applied on a specific argument. */
        if (guard->argn > 0) {
            struct lval* child = lval_alloc();
            lval_index(args, guard->argn-1, child);
            if (0 != (s = (guard->condition)(sym, env, child))) {
                lval_free(child);
                break;
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
                if (0 != (s = (guard->condition)(sym, env, child))) {
                    lval_free(child);
                    break;
                }
                lval_free(child);
            }
            if (s != 0) {
                break;
            }
            continue;
        }
        /* Guard applied on all args. */
        // guard->argn == -1;
        if (0 != (s = (guard->condition)(sym, env, args))) {
            break;
        }
    }
    if (s != 0 && guard) {
        lval_mut_err(acc, guard->error);
        return s;
    }
    return 0;
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
    if (sym->func) {
        return sym->func(env, arg, acc);
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

#define LENGTH(arr) sizeof(arr)/sizeof(arr[0])

static struct lguard lbuitin_guards[] = {
    {.condition= lbi_cond_max_argc,     .argn= -1, .error= LERR_TOO_MANY_ARGS},
    {.condition= lbi_cond_min_argc,     .argn= -1, .error= LERR_TOO_FEW_ARGS},
    {.condition= lbi_cond_func_pointer, .argn= -1, .error= LERR_EVAL},
};

int lbuiltin_exec(const struct ldescriptor* sym, struct lenv* env,
        const struct lval* args, struct lval* acc) {
    if (!sym) {
        lval_mut_err(acc, LERR_EVAL);
        return -1;
    }
    /* Guards */
    int s = 0;
    if (0 != (s = lbuitin_check_guards(sym,
                    &lbuitin_guards[0], LENGTH(lbuitin_guards),
                    env, args, acc))) {
        return s;
    }
    if (0 != (s = lbuitin_check_guards(sym,
                    sym->guards, sym->guardc,
                    env, args, acc))) {
        return s;
    }
    /* Argument execution. */
    if (!sym->accumulator) {
        return sym->func(env, args, acc);
    }
    /* Accumulator execution. */
    size_t len = lval_len(args);
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
