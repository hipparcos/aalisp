#include "lenv.h"

#include "vendor/snow/snow/snow.h"

#include "lval.h"

describe(lenv, {
    it("allocates an env and frees it", {
        struct lenv* env = lenv_alloc();
        assert(env != NULL);
        lenv_free(env);
    });

    it("allocates an env and fills it with default", {
        struct lenv* env = lenv_alloc();
        defer(lenv_free(env));
        assert(env != NULL);
        lenv_default(env);
        assert(lenv_len(env) > 0);
    });

    subdesc(lookup, {
        it("looks for `+` symbol into the default env", {
            struct lenv* env = lenv_alloc();
            defer(lenv_free(env));
            lenv_default(env);
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "+");
            struct lval* res = lval_alloc();
            defer(lval_free(res));
            assert(lenv_lookup(env, sym, res));
            assert(lval_as_func(res) != NULL);
        });

        it("looks for an undefined symbol into the default env", {
            struct lenv* env = lenv_alloc();
            defer(lenv_free(env));
            lenv_default(env);
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "undefined symbol");
            struct lval* res = lval_alloc();
            defer(lval_free(res));
            assert(!lenv_lookup(env, sym, res));
            assert(lval_type(res) == LVAL_ERR);
        });
    });

    subdesc(put, {
        it("puts a symbol into env and retrieves it", {
            struct lenv* env = lenv_alloc();
            defer(lenv_free(env));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_len(env) == 0);
            assert(lenv_put(env, sym, val));
            assert(lenv_len(env) == 1);
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lenv_lookup(env, sym, got));
            assert(lval_are_equal(got, val));
        });

        it("overrides a symbol into env and retrieves it", {
            struct lenv* env = lenv_alloc();
            defer(lenv_free(env));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_len(env) == 0);
            assert(lenv_put(env, sym, val));
            assert(lenv_len(env) == 1);
            lval_mut_num(val, 200);
            assert(lenv_put(env, sym, val));
            assert(lenv_len(env) == 1);
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lenv_lookup(env, sym, got));
            assert(lval_are_equal(got, val));
        });
    });

    subdesc(def, {
        it("defs a symbol into env and retrieves it", {
            struct lenv* env = lenv_alloc();
            defer(lenv_free(env));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_len(env) == 0);
            assert(lenv_def(env, sym, val));
            assert(lenv_len(env) == 1);
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lenv_lookup(env, sym, got));
            assert(lval_are_equal(got, val));
        });

        it("overrides a symbol into env and retrieves it", {
            struct lenv* env = lenv_alloc();
            defer(lenv_free(env));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_len(env) == 0);
            assert(lenv_put(env, sym, val));
            assert(lenv_len(env) == 1);
            lval_mut_num(val, 200);
            assert(lenv_def(env, sym, val));
            assert(lenv_len(env) == 1);
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lenv_lookup(env, sym, got));
            assert(lval_are_equal(got, val));
        });
    });

    subdesc(copy, {
        it("copies src into dest", {
            struct lenv* src = lenv_alloc();
            defer(lenv_free(src));
            struct lenv* dest = lenv_alloc();
            defer(lenv_free(dest));
            lenv_default(src);
            assert(lenv_copy(dest, src));
            assert(lenv_len(dest) == lenv_len(src));
        });
        it("copies src into dest then retrieves a sym after freeing src", {
            struct lenv* src = lenv_alloc();
            defer(if (src) lenv_free(src));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_put(src, sym, val));
            struct lenv* dest = lenv_alloc();
            defer(lenv_free(dest));
            assert(lenv_copy(dest, src));
            lenv_free(src); src = NULL;
            struct lval* got = lval_alloc();
            defer(lval_free(got));
            assert(lenv_lookup(dest, sym, got));
            assert(lval_are_equal(got, val));
        });
    });

    subdesc(are_equal, {
        it("compares default env", {
            struct lenv* left = lenv_alloc();
            defer(lenv_free(left));
            struct lenv* right = lenv_alloc();
            defer(lenv_free(right));
            lenv_default(left);
            lenv_default(right);
            assert(lenv_are_equal(left, right));
        });
        it("compares different envs", {
            struct lenv* left = lenv_alloc();
            defer(lenv_free(left));
            struct lenv* right = lenv_alloc();
            defer(lenv_free(right));
            lenv_default(left);
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_def(right, sym, val));
            assert(!lenv_are_equal(left, right));
        });
        it("compares different same lenght envs", {
            struct lenv* left = lenv_alloc();
            defer(lenv_free(left));
            struct lenv* right = lenv_alloc();
            defer(lenv_free(right));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 100);
            assert(lenv_def(left, sym, val));
            lval_mut_sym(sym, "y");
            lval_mut_num(val, 200);
            assert(lenv_def(right, sym, val));
            assert(!lenv_are_equal(left, right));
        });
        it("puts sym into left, defs sym into right then compare then", {
            struct lenv* left = lenv_alloc();
            defer(lenv_free(left));
            struct lenv* right = lenv_alloc();
            defer(lenv_free(right));
            struct lval* sym = lval_alloc();
            defer(lval_free(sym));
            lval_mut_sym(sym, "x");
            struct lval* val = lval_alloc();
            defer(lval_free(val));
            lval_mut_num(val, 100);
            assert(lenv_def(left, sym, val));
            assert(lenv_put(right, sym, val));
            assert(lenv_are_equal(left, right));
        });
    });

#define set_parent_init() \
    struct lenv* parent = lenv_alloc(); \
    defer(lenv_free(parent)); \
    struct lenv* child = lenv_alloc(); \
    defer(lenv_free(child)); \
    assert(lenv_set_parent(child, parent)); \
    struct lval* sym = lval_alloc(); \
    defer(lval_free(sym)); \
    struct lval* val = lval_alloc(); \
    defer(lval_free(val)); \
    struct lval* got = lval_alloc(); \
    defer(lval_free(got));

    subdesc(set_parent, {
        it("defines x in parent; lookup for it in both env", {
            set_parent_init();
            long r;
            /* Define. */
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 100);
            assert(lenv_def(parent, sym, val));
            /* Lookup. */
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(parent, sym, got));
            assert(lval_type(got) == LVAL_NUM);
            assert(lval_as_num(got, &r) && r == 100);
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(child, sym, got));
            assert(lval_as_num(got, &r) && r == 100);
        });
        it("defines x in parent, override it in child; lookup for it in both env", {
            set_parent_init();
            long r;
            /* Define. */
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 200);
            assert(lenv_def(parent, sym, val));
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 100);
            assert(lenv_def(child, sym, val));
            /* Lookups */
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(parent, sym, got));
            assert(lval_type(got) == LVAL_NUM);
            assert(lval_as_num(got, &r) && r == 100);
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(child, sym, got));
            assert(lval_type(got) == LVAL_NUM);
            assert(lval_as_num(got, &r) && r == 100);
        });
        it("defines x in parent, define x as local in child; lookup for it in both env", {
            set_parent_init();
            long r;
            /* Define. */
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 200);
            assert(lenv_put(parent, sym, val));
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 100);
            assert(lenv_put(child, sym, val));
            /* Lookups */
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(parent, sym, got));
            assert(lval_type(got) == LVAL_NUM);
            assert(lval_as_num(got, &r) && r == 200);
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(child, sym, got));
            assert(lval_type(got) == LVAL_NUM);
            assert(lval_as_num(got, &r) && r == 100);
        });
        it("defines x as local in child; lookup for it in both env", {
            set_parent_init();
            long r;
            /* Define. */
            lval_mut_sym(sym, "x");
            lval_mut_num(val, 100);
            assert(lenv_put(child, sym, val));
            lval_mut_sym(sym, "x"); r = 0;
            assert(!lenv_lookup(parent, sym, got));
            assert(lval_type(got) == LVAL_ERR);
            lval_mut_sym(sym, "x"); r = 0;
            assert(lenv_lookup(child, sym, got));
            assert(lval_type(got) == LVAL_NUM);
            assert(lval_as_num(got, &r) && r == 100);
        });
    });
});

snow_main();
