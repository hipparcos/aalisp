#ifndef H_LPARSER_
#define H_LPARSER_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "lerr.h"
#include "llexer.h"

/** last_type is the type of an AST node. */
enum last_type {
    LAST_ERR,   // is an error node.
    LAST_PROG,  // is a program node.
    LAST_EXPR,  // is an expression node.
    LAST_SEXPR, // is a S-Expression node.
    LAST_QEXPR, // is a Q-Expression node.
    LAST_NUM,   // is an integer number terminal node.
    LAST_DBL,   // is a double number terminal node.
    LAST_SYM,   // is a symbol terminal node.
    LAST_STR,   // is a string terminal node.
};

/** last is an AST node. */
struct last {
    /** last.type is the type of the node. */
    enum last_type type;
    /** last.err is the type of an error. */
    enum lerr_code err;
    /** last.content is the content of a terminal node.
     ** last.content is NULL for non-terminal nodes. */
    char* content;
    /**  last.len is the length of content. */
    size_t len;
    /** last.line is the line number where this node starts. */
    int line;
    /** last.col is the column number where this node starts. */
    int col;
    /** last.children is the list of all the children of this node.
     ** last.children is NULL for terminal nodes. */
    struct last** children;
    /** last.childrenc is the length of the list of children.
     ** last.childrenc is 0 for terminal nodes. */
    size_t childrenc;
};

/** lparse transforms a list of tokens into an ast.
 ** Returns the root of the ast.
 ** err is allocated in case of error.
 ** Caller is responsible for calling last_free() on ast. */
struct last* lparse(struct ltok* tokens, struct lerr** err);
/** last_free clears ast.
 ** ast must not be used afterwards. */
void last_free(struct last* ast);

/** last_are_equal tells if two ast nodes are equal. */
bool last_are_equal(const struct last* left, const struct last* right);
/** last_are_all_equal tells if two ast are equal. */
bool last_are_all_equal(const struct last* left, const struct last* right);

/** last_type_string returns type as string. */
const char* last_type_string(enum last_type type);
/** last_print_to prints the ast node to the file out. */
void last_print_to(const struct last* ast, FILE* out);
/** last_print prints the ast node to stdout. */
#define last_print(ast) last_print_to(ast, stdout)
/** last_print_all_to prints the ast to the file out. */
void last_print_all_to(const struct last* ast, FILE* out);
/** last_print_all prints the ast to stdout. */
#define last_print_all(ast) last_print_all_to(ast, stdout)

#endif
