#ifndef _H_LLEXER_
#define _H_LLEXER_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum ltok_type {
    LTOK_EOF= 0,
    LTOK_ERR,
    LTOK_OPAR,
    LTOK_CPAR,
    LTOK_SYM,
    LTOK_NUM,
    LTOK_DBL,
    LTOK_STR,
};
extern const char* ltok_type_string[8];

struct ltok {
    enum ltok_type type;
    char* content;
    int   line;
    int   col;
    /* double linked list */
    struct ltok* prev;
    struct ltok* next;
};

/** lisp_lex transforms the input into a list of tokens.
 ** last element is always of type LTOK_EOS.
 ** Returns the first element of the list of ltok.
 ** Caller is responsible for calling llex_free() on tokens. */
struct ltok* lisp_lex(const char* input, struct ltok* last);
/** llex_free clears a list of tokens.
 ** The list of tokens must end with a token of type LTOK_EOS.
 ** tokens must not be used afterwards */
void llex_free(struct ltok* tokens);

/** llex_are_equals tells if two tokens are equal. */
bool llex_are_equals(struct ltok* left, struct ltok* right);
/** llex_are_all_equals tells if two list of tokens are equal. */
bool llex_are_all_equals(struct ltok* left, struct ltok* right);

/** llex_print prints a token.
 ** Caller is responsible for allocating the out string (see llex_printlen). */
void llex_to_string(struct ltok* token, char* out);
/** llex_printlen returns the len of the string that would be generated for
 ** token by llex_to_string. */
size_t llex_printlen(struct ltok* token);
/** llex_print_to prints token to the out file. */
void llex_print_to(struct ltok* token, FILE* out);
/** llex_print prints token to stdout. */
#define llex_print(token) llex_print_to(token, stdout);
/** llex_println prints token to stdout. A newline is added. */
#define llex_println(token) llex_print_to(token, stdout); putchar('\n');
/** llex_print_all_to prints the list of tokens to the out file. */
void llex_print_all_to(struct ltok* tokens, FILE* out);
/** llex_print_all prints the list of tokens to stdout. */
#define llex_print_all(tokens) llex_print_all_to(tokens, stdout);

#endif
