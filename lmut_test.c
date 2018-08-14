#include "lmut.h"

#include <limits.h>
#include "vendor/mini-gmp/mini-gmp.h"

#include "llexer.h"
#include "lparser.h"
#include "lval.h"

#include "vendor/snow/snow/snow.h"

#define test_pass(msg, input, ...) \
    it("passes for " msg " `"input"`", { \
        struct lval* expected = lval_alloc(); \
        __VA_ARGS__ \
        defer(lval_free(expected)); \
        struct ltok *tokens = NULL, *lexer_error = NULL; \
        defer(llex_free(tokens)); \
        assert(NULL != (tokens = lisp_lex(input, &lexer_error))); \
        struct last *ast = NULL, *ast_error = NULL; \
        defer(last_free(ast)); \
        assert(NULL != (ast = lisp_parse(tokens, &ast_error))); \
        struct lval *got = NULL; \
        defer(lval_free(got)); \
        assert(NULL != (got = lisp_mut(ast, &ast_error))); \
        assert(lval_are_equal(got, expected)); \
    })

#define test_fail(msg, input) \
    it("fails for " msg, { \
        struct last *ast = NULL, *error = NULL; \
        defer(last_free(ast)); \
        struct lval* got = NULL; \
        defer(lval_free(got)); \
        assert(NULL == (got = lisp_mut(ast, &error))); \
    })

#define _STRINGIFY(str) #str
#define STRINGIFY(str) _STRINGIFY(str)

describe(lmut, {

    test_pass("num", "(+ 1)", {
        struct lval* sym = lval_alloc(); defer(lval_free(sym));
        lval_mut_sym(sym, "+");
        struct lval* opr = lval_alloc(); defer(lval_free(opr));
        lval_mut_num(opr, 1);
        struct lval* sexpr = lval_alloc(); defer(lval_free(sexpr));
        lval_mut_sexpr(sexpr);
        lval_push(sexpr, sym);
        lval_push(sexpr, opr);
        struct lval* prog = lval_alloc(); defer(lval_free(prog));
        lval_mut_sexpr(prog);
        lval_push(prog, sexpr);
        lval_dup(expected, prog);
    });

    test_pass("doubles", "(+ 1.0)", {
        struct lval* sym = lval_alloc(); defer(lval_free(sym));
        lval_mut_sym(sym, "+");
        struct lval* opr = lval_alloc(); defer(lval_free(opr));
        lval_mut_dbl(opr, 1.0);
        struct lval* sexpr = lval_alloc(); defer(lval_free(sexpr));
        lval_mut_sexpr(sexpr);
        lval_push(sexpr, sym);
        lval_push(sexpr, opr);
        struct lval* prog = lval_alloc(); defer(lval_free(prog));
        lval_mut_sexpr(prog);
        lval_push(prog, sexpr);
        lval_dup(expected, prog);
    });

    test_pass("bignum", "(+ 18446744073709551615)", {
        struct lval* sym = lval_alloc(); defer(lval_free(sym));
        lval_mut_sym(sym, "+");
        struct lval* opr = lval_alloc(); defer(lval_free(opr));
        mpz_t bn; mpz_init(bn);
        mpz_set_ui(bn, ULONG_MAX);
        lval_mut_bignum(opr, bn);
        mpz_clear(bn);
        struct lval* sexpr = lval_alloc(); defer(lval_free(sexpr));
        lval_mut_sexpr(sexpr);
        lval_push(sexpr, sym);
        lval_push(sexpr, opr);
        struct lval* prog = lval_alloc(); defer(lval_free(prog));
        lval_mut_sexpr(prog);
        lval_push(prog, sexpr);
        lval_dup(expected, prog);
    });

    test_pass("strings", "(+ \"test\")", {
        struct lval* sym = lval_alloc(); defer(lval_free(sym));
        lval_mut_sym(sym, "+");
        struct lval* opr = lval_alloc(); defer(lval_free(opr));
        lval_mut_str(opr, "test");
        struct lval* sexpr = lval_alloc(); defer(lval_free(sexpr));
        lval_mut_sexpr(sexpr);
        lval_push(sexpr, sym);
        lval_push(sexpr, opr);
        struct lval* prog = lval_alloc(); defer(lval_free(prog));
        lval_mut_sexpr(prog);
        lval_push(prog, sexpr);
        lval_dup(expected, prog);
    });

    test_pass("nested s-expressions", "(+ 1 (! 8))", {
        struct lval* symp = lval_alloc(); defer(lval_free(symp));
        lval_mut_sym(symp, "+");
        struct lval* opr1 = lval_alloc(); defer(lval_free(opr1));
        lval_mut_num(opr1, 1);
        struct lval* sexpr1 = lval_alloc(); defer(lval_free(sexpr1));
        lval_mut_sexpr(sexpr1);
        lval_push(sexpr1, symp);
        lval_push(sexpr1, opr1);
        struct lval* symb = lval_alloc(); defer(lval_free(symb));
        lval_mut_sym(symb, "!");
        struct lval* opr8 = lval_alloc(); defer(lval_free(opr8));
        lval_mut_num(opr8, 8);
        struct lval* sexpr2 = lval_alloc(); defer(lval_free(sexpr2));
        lval_mut_sexpr(sexpr2);
        lval_push(sexpr2, symb);
        lval_push(sexpr2, opr8);
        lval_push(sexpr1, sexpr2);
        struct lval* prog = lval_alloc(); defer(lval_free(prog));
        lval_mut_sexpr(prog);
        lval_push(prog, sexpr1);
        lval_dup(expected, prog);
    });

    test_pass("q-expressions", "(head {1 1 1})", {
        struct lval* sym = lval_alloc(); defer(lval_free(sym));
        lval_mut_sym(sym, "head");
        struct lval* opr1 = lval_alloc(); defer(lval_free(opr1));
        lval_mut_num(opr1, 1);
        struct lval* qexpr1 = lval_alloc(); defer(lval_free(qexpr1));
        lval_mut_qexpr(qexpr1);
        lval_push(qexpr1, opr1);
        lval_push(qexpr1, opr1);
        lval_push(qexpr1, opr1);
        struct lval* sexpr1 = lval_alloc(); defer(lval_free(sexpr1));
        lval_mut_sexpr(sexpr1);
        lval_push(sexpr1, sym);
        lval_push(sexpr1, qexpr1);
        struct lval* prog = lval_alloc(); defer(lval_free(prog));
        lval_mut_sexpr(prog);
        lval_push(prog, sexpr1);
        lval_dup(expected, prog);
    });

    test_fail("NULL AST", NULL);

});

snow_main();
