#ifndef _H_LLEXER_
#define _H_LLEXER_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lerr.h"

enum ltok_type {
    LTOK_EOF= 0,
    LTOK_ERR,
    LTOK_OPAR,
    LTOK_CPAR,
    LTOK_OBRC,
    LTOK_CBRC,
    LTOK_SYM,
    LTOK_NUM,
    LTOK_DBL,
    LTOK_STR,
};

struct ltok {
    enum ltok_type type;
    char* content;
    int   line;
    int   col;
    /* singly linked list */
    struct ltok* next;
};

/** lisp_lex transforms the input into a list of tokens.
 ** Returns the first element of the list of ltok.
 ** The returned list always end with a LTOK_EOF token.
 ** err is allocated in case of error.
 ** Caller is responsible for calling llex_free() on tokens. */
struct ltok* lisp_lex(const char* input, struct lerr** err);
/** lisp_lex_surround acts as lisp_lex but surrounds tokens with `(` & `)`.
 ** It ensures that the output is a sexpr.  */
struct ltok* lisp_lex_surround(const char* input, struct lerr** err);
/** llex_free clears a list of tokens.
 ** The list of tokens must end with a token of type LTOK_EOS.
 ** tokens must not be used afterwards */
void llex_free(struct ltok* tokens);

/** llex_are_equal tells if two tokens are equal. */
bool llex_are_equal(struct ltok* left, struct ltok* right);
/** llex_are_all_equal tells if two list of tokens are equal. */
bool llex_are_all_equal(struct ltok* left, struct ltok* right);

/** llex_ty_string returns the type of a token as string. */
const char* llex_type_string(enum ltok_type type);
/** llex_print_to prints token to the out file. */
void llex_print_to(struct ltok* token, FILE* out);
/** llex_print prints token to stdout. */
#define llex_print(token) llex_print_to(token, stdout)
/** llex_print_all_to prints the list of tokens to the out file. */
void llex_print_all_to(struct ltok* tokens, FILE* out);
/** llex_print_all prints the list of tokens to stdout. */
#define llex_print_all(tokens) llex_print_all_to(tokens, stdout)

#endif
