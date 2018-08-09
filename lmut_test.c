#include "lmut.h"

#include "llexer.h"
#include "lparser.h"
#include "lval.h"

#include "vendor/snow/snow/snow.h"

describe(lmut, {

    it("works for (+ 1 1)", {
        const char* input = "(+ 1 1)";
        struct ltok *tokens = NULL, *lexer_error = NULL;
        defer(llex_free(tokens));
        assert(NULL != (tokens = lisp_lex(input, &lexer_error)));
        struct last *ast = NULL, *ast_error = NULL;
        defer(last_free(ast));
        assert(NULL != (ast = lisp_parse(tokens, &ast_error)));
        struct lval *got = NULL;
        defer(lval_free(got));
        assert(NULL != (got = lisp_mut(ast, &ast_error)));
        /* Expected lval construction. */
        struct lval *expected, *sexpr, *sym, *opr;
        expected = lval_alloc();
        sexpr = lval_alloc();
        sym = lval_alloc();
        opr = lval_alloc();
        defer(lval_free(expected));
        defer(lval_free(opr));
        defer(lval_free(sym));
        defer(lval_free(sexpr));
        lval_mut_num(opr, 1);
        lval_mut_sym(sym, "+");
        lval_mut_sexpr(sexpr);
        lval_mut_sexpr(expected);
        lval_push(sexpr, sym);
        lval_push(sexpr, opr);
        lval_push(sexpr, opr);
        lval_push(expected, sexpr);
        assert(lval_are_equal(got, expected));
    });

    it("works for NULL ast", {
        struct last* ast = NULL;
        struct last* error = NULL;
        struct lval* got = NULL;
        defer(lval_free(got));
        assert(NULL == (got = lisp_mut(ast, &error)));
    });

});

snow_main();
