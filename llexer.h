#ifndef H_LLEXER_
#define H_LLEXER_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lerr.h"

#define LLEX_COMM ';' // is the comment mark.
#define LLEX_OPAR '(' // is the opening parenthesis.
#define LLEX_CPAR ')' // is the closing parenthesis.
#define LLEX_OBRC '{' // is the opening brace.
#define LLEX_CBRC '}' // is the closing brace.
#define LLEX_OSTR '"' // is the opening quote.
#define LLEX_CSTR '"' // is the closing quote.
#define LLEX_DOLL '$' // is the special $ character.
#define LLEX_SYMB "+-*/%^?!&|:,._#~<>=$§£¤µ\\" // is a list of characters allowed in symbols.

/** ltok_type is the type of a dialecte token. */
enum ltok_type {
    LTOK_EOF = 0, // is a terminal token, nothing afterwards.
    LTOK_ERR,     // signals the encounter of an unknown character.
    LTOK_OPAR,    // is an opening parenthesis.
    LTOK_CPAR,    // is a closing parenthesis.
    LTOK_OBRC,    // is an opening brace.
    LTOK_CBRC,    // is a closing brace.
    LTOK_DOLL,    // is the special $ syntax.
    LTOK_SYM,     // is a symbol.
    LTOK_NUM,     // is an integral number.
    LTOK_DBL,     // is a double number.
    LTOK_STR,     // is a string.
};

/** ltok is dialecte token or a list of dialecte tokens. */
struct ltok {
    /** ltok.type is the type of the token. */
    enum ltok_type type;
    /** ltok.content is the value of the token. */
    char* content;
    /** ltok.len is the length of ltok.content. */
    size_t len;
    /** ltok.line is the line where this token starts. */
    int   line;
    /** ltok.col is the column where this token starts. */
    int   col;
    /** ltok.next is the next element of the list (singly linked list). */
    /** ltok.next is NULL for LTOK_EOF tokens. */
    struct ltok* next;
};

/** llex_lex transforms the input into a list of tokens.
 ** Returns the first element of the list of ltok.
 ** The returned list always end with a LTOK_EOF token.
 ** err is allocated in case of error.
 ** Caller is responsible for calling llex_free() on the returned list of tokens. */
struct ltok* llex_lex(const char* input, struct lerr** err);
/** llex_lex_from calls llex_lex with the content of the file input. */
struct ltok* llex_lex_from(const FILE* input, struct lerr** err);
/** llex_free clears a list of tokens.
 ** The list of tokens must end with a token of type LTOK_EOF.
 ** tokens must not be used afterwards */
void llex_free(struct ltok* tokens);

/** llex_are_equal tells if two tokens are equal. */
bool llex_are_equal(struct ltok* left, struct ltok* right);
/** llex_are_all_equal tells if two lists of tokens are equal. */
bool llex_are_all_equal(struct ltok* left, struct ltok* right);

/** llex_type_string returns the type of a token as string. */
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
