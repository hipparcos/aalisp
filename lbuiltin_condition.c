#include "lbuiltin_condition.h"

#include <limits.h>

#include "lval.h"
#include "lerr.h"
#include "lfunc.h"

#define unused(x) ((void)x)

define_condition(must_have_max_argc) {
    unused(param);
    size_t len = lval_len(arg);
    int max = fun->max_argc;
    if (max != -1 && (int)len > max) {
        *err = lerr_throw(LERR_TOO_MANY_ARGS,
                "takes %d arguments at maximum", max);
        return -1;
    }
    return 0;
}

define_condition(must_have_min_argc) {
    unused(param);
    size_t len = lval_len(arg);
    int min = fun->min_argc;
    if (min != -1 && (int)len < min) {
        *err = lerr_throw(LERR_TOO_FEW_ARGS,
                "takes %d arguments at minimum", min);
        return -1;
    }
    return 0;
}

define_condition(must_have_func_ptr) {
    unused(param); unused(arg);
    if (!fun->accumulator && !fun->func) {
        *err = lerr_throw(LERR_EVAL,
                "incorrect builtin definition");
        return -1;
    }
    return 0;
}

define_condition(must_be_numeric) {
    unused(param); unused(fun);
    if (!lval_is_numeric(arg)) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be numeric");
        return 1;
    }
    return 0;
}

define_condition(must_be_integral) {
    unused(param); unused(fun);
    if ((lval_type(arg) != LVAL_NUM && lval_type(arg) != LVAL_BIGNUM)) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be integral");
        return 1;
    }
    return 0;
}

define_condition(must_be_positive) {
    unused(param); unused(fun);
    if (lval_sign(arg) < 0) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be positive");
        return 1;
    }
    return 0;
}

define_condition(divisor_must_be_non_zero) {
    unused(param); unused(fun);
    size_t len = lval_len(arg);
    struct lval* d = lval_alloc();
    for (size_t c = 1; c < len; c++) {
        lval_index(arg, c, d);
        if (lval_is_zero(d)) {
            *err = lerr_throw(LERR_DIV_ZERO,
                    "divisor must not be 0");
            lval_free(d);
            return c+1;
        }
    }
    lval_free(d);
    return 0;
}

define_condition(must_be_unsigned_long) {
    unused(param); unused(fun);
    if (lval_type(arg) == LVAL_NUM) {
        return 0;
    }
    if (lval_type(arg) == LVAL_DBL) {
        return 0;
    }
    if (lval_type(arg) != LVAL_BIGNUM) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be integral");
        return 1;
    }
    mpz_t r;
    mpz_init(r);
    lval_as_bignum(arg, r);
    if (mpz_cmp_ui(r, ULONG_MAX) > 0) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "is too large");
        mpz_clear(r);
        return 1;
    }
    mpz_clear(r);
    return 0;
}

define_condition(must_be_of_type) {
    unused(fun);
    enum ltype type = *((enum ltype*)param);
    if (lval_type(arg) != type) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be of type %s", lval_type_string(type));
        return 1;
    }
    return 0;
}

define_condition(must_all_be_of_same_type) {
    unused(fun); unused(param);
    size_t len = lval_len(arg);
    struct lval* child = lval_alloc();
    lval_index(arg, 0, child);
    enum ltype first_type = lval_type(child);
    for (size_t c = 1; c < len; c++) {
        lval_index(arg, c, child);
        if (lval_type(child) != first_type) {
            *err = lerr_throw(LERR_BAD_OPERAND,
                    "must be a list of %s", lval_type_string(first_type));
            lval_free(child);
            return c+1;
        }
    }
    lval_free(child);
    return 0;
}

define_condition(must_have_min_len) {
    unused(fun);
    size_t min = *((size_t*)param);
    if (lval_len(arg) < min) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must have a length of at least %ld", min);
        return 1;
    }
    return 0;
}

define_condition(must_be_of_equal_len) {
    unused(fun); unused(param);
    struct lval* list = lval_alloc();
    lval_index(arg, 0, list);
    size_t list_len = lval_len(list);
    size_t args_len = lval_len(arg);
    if (args_len-1 < list_len) {
        *err = lerr_throw(LERR_TOO_FEW_ARGS,
                "lenght must match: %ld symbol(s) and %ld value(s)", list_len, args_len-1);
        lval_free(list);
        return 1;
    }
    if (args_len-1 > list_len) {
        *err = lerr_throw(LERR_TOO_MANY_ARGS,
                "lenght must match: %ld symbol(s) and %ld value(s)", list_len, args_len-1);
        lval_free(list);
        return 1;
    }
    lval_free(list);
    return 0;
}

define_condition(must_be_a_list) {
    unused(param); unused(fun);
    if (!lval_is_list(arg)) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be a list");
        return 1;
    }
    return 0;
}

define_condition(must_be_list_of) {
    unused(fun);
    enum ltype type = *((enum ltype*)param);
    if (!lval_is_list(arg)) {
        *err = lerr_throw(LERR_BAD_OPERAND,
                "must be a list");
        return 1;
    }
    size_t len = lval_len(arg);
    struct lval* child = lval_alloc();
    for (size_t c = 0; c < len; c++) {
        lval_index(arg, c, child);
        if (lval_type(child) != type) {
            *err = lerr_throw(LERR_BAD_OPERAND,
                    "must be a list of %s", lval_type_string(type));
            lval_free(child);
            return c+1;
        }
    }
    lval_free(child);
    return 0;
}
