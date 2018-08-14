#include "lparser.h"

#include <string.h>

const char* ltag_string[] = {
    "error",
    "program",
    "expr",
    "num",
    "double",
    "symbol",
    "string",
    "sexpr",
    "qexpr",
};

enum lparser_error {
    LPARSER_ERR_MISSING_OPAR,
    LPARSER_ERR_MISSING_CPAR,
    LPARSER_ERR_MISSING_CBRC,
    LPARSER_ERR_BAD_OPERAND,
    LPARSER_ERR_BAD_EXPR,
};
static const char* lparser_error_string[] = {
    "missing opening parenthesis",
    "missing closing parenthesis",
    "missing closing brace",
    "operands must be of types num|double|string|symbol|sexpr",
    "an expression must start with a symbol or a `(`",
};

static struct last* last_alloc(enum ltag tag, const char* content, struct ltok* tok) {
    struct last* ast = calloc(1, sizeof(struct last));
    ast->tag = tag;
    size_t len = strlen(content);
    ast->content = calloc(len + 1, sizeof(char));
    strncpy(ast->content, content, len);
    if (tok) {
        ast->line = tok->line;
        ast->col = tok->col;
    }
    return ast;
}

static bool last_attach(struct last* child, struct last* parent) {
    if (!child || !parent) {
        return NULL;
    }
    parent->children = realloc(parent->children, ++parent->childrenc * sizeof(struct last*));
    parent->children[parent->childrenc-1] = child;
    return true;
};

static struct last* lparse_symbol(struct ltok* tokens) {
    if (tokens->type != LTOK_SYM) {
        return NULL;
    }
    return last_alloc(LTAG_SYM, tokens->content, tokens);
}

static struct last* lparse_number(struct ltok* tokens) {
    if (tokens->type != LTOK_NUM) {
        return NULL;
    }
    return last_alloc(LTAG_NUM, tokens->content, tokens);
}

static struct last* lparse_double(struct ltok* tokens) {
    if (tokens->type != LTOK_DBL) {
        return NULL;
    }
    return last_alloc(LTAG_DBL, tokens->content, tokens);
}

static struct last* lparse_string(struct ltok* tokens) {
    if (tokens->type != LTOK_STR) {
        return NULL;
    }
    char* content = tokens->content;
    content++; // skip opening ".
    size_t len = strlen(content);
    len--; // skip closing ".
    char* buffer = calloc(len + 1, sizeof(char));
    /* Remove opening and closing " and escape \. */
    size_t lencpy = 0;
    char *curr = buffer, *end = NULL;
    while (NULL != (end = strstr(content, "\\\""))) {
        lencpy = end - content;
        strncpy(curr, content, lencpy);
        curr += lencpy;
        content += lencpy + 1; // skip \.
        len -= lencpy + 1;
    }
    strncpy(curr, content, len);
    struct last* node = last_alloc(LTAG_STR, buffer, tokens);
    free(buffer);
    return node;
}

static struct last* lparse_atom(struct ltok* curr, struct ltok** last) {
    if (curr->type == LTOK_EOF) {
        return NULL;
    }
    struct last* atom = NULL;
    switch (curr->type) {
    case LTOK_NUM:
        atom = lparse_number(curr);
        curr = curr->next;
        break;
    case LTOK_DBL:
        atom = lparse_double(curr);
        curr = curr->next;
        break;
    case LTOK_STR:
        atom = lparse_string(curr);
        curr = curr->next;
        break;
    case LTOK_SYM:
        atom = lparse_symbol(curr);
        curr = curr->next;
        break;
    default: break;
    }
    *last = curr;
    return atom;
}

static struct last* lparse_sexpr(struct ltok* first, struct ltok** last);
static struct last* lparse_qexpr(struct ltok* first, struct ltok** last);

static struct last* lparse_expr(struct ltok* first, struct ltok** last) {
    struct ltok* curr = first;
    struct last* expr = NULL;
    struct last* sexpr = NULL;
    switch (curr->type) {
    case LTOK_OPAR: // Start of SEXPR.
        sexpr = lparse_sexpr(curr, &curr);
        if (sexpr->tag == LTAG_ERR) {
            expr = sexpr;
            break;
        }
        expr = last_alloc(LTAG_EXPR, "", curr);
        last_attach(sexpr, expr);
        break;
    case LTOK_SYM:
        expr = last_alloc(LTAG_EXPR, "", curr);
        /* Symbol. */
        last_attach(lparse_symbol(curr), expr);
        curr = curr->next;
        /* Operands. */
        while (curr->type != LTOK_CPAR && curr->type != LTOK_EOF) {
            struct last* operand = NULL;
            if (!(operand = lparse_atom(curr, &curr))) {
                switch (curr->type) {
                case LTOK_OPAR:
                    operand = lparse_sexpr(curr, &curr);
                    break;
                case LTOK_OBRC:
                    operand = lparse_qexpr(curr, &curr);
                    break;
                default:
                    operand = last_alloc(LTAG_ERR,
                            lparser_error_string[LPARSER_ERR_BAD_OPERAND],
                            curr);
                    break;
                }
            }
            // Error = break.
            if (operand->tag == LTAG_ERR) {
                last_attach(expr, operand);
                expr = operand;
                break;
            }
            last_attach(operand, expr);
        }
        break;
    case LTOK_EOF:
        break;
    default:
        expr = last_alloc(LTAG_ERR,
                lparser_error_string[LPARSER_ERR_BAD_EXPR],
                curr);
        break;
    }
    *last = curr;
    return expr;
}

static struct last* lparse_sexpr(struct ltok* first, struct ltok** last) {
    struct ltok* curr = first;
    struct last* sexpr;
    struct last* expr = NULL;
    if (curr->type == LTOK_EOF) {
        *last = curr;
        return NULL;
    }
    // (.
    if (curr->type != LTOK_OPAR) {
        sexpr = last_alloc(LTAG_ERR,
                lparser_error_string[LPARSER_ERR_MISSING_OPAR],
                curr);
        *last = curr;
        return sexpr;
    }
    curr = curr->next; // Skip (.
    // Inner expr.
    expr = lparse_expr(curr, &curr);
    // Error = break.
    if (expr && expr->tag == LTAG_ERR) {
        return expr;
    }
    // ) or error.
    if (curr->type != LTOK_CPAR) {
        sexpr = last_alloc(LTAG_ERR,
                lparser_error_string[LPARSER_ERR_MISSING_CPAR],
                curr);
    } else {
        sexpr = last_alloc(LTAG_SEXPR, "", curr);
        curr = curr->next; // Skip ).
    }
    last_attach(expr, sexpr);
    *last = curr;
    return sexpr;
}

static struct last* lparse_qexpr(struct ltok* first, struct ltok** last) {
    struct ltok* curr = first;
    if (curr->type == LTOK_EOF) {
        *last = curr;
        return NULL;
    }
    curr = curr->next; // Skip {.
    // Inner list.
    struct last* qexpr = last_alloc(LTAG_QEXPR, "", curr);
    while (curr->type != LTOK_CBRC && curr->type != LTOK_EOF) {
        struct last* operand = NULL;
        if (!(operand = lparse_atom(curr, &curr))) {
            switch (curr->type) {
            case LTOK_OPAR:
                operand = lparse_sexpr(curr, &curr);
                break;
            case LTOK_OBRC:
                operand = lparse_qexpr(curr, &curr);
                break;
            default:
                operand = last_alloc(LTAG_ERR,
                        lparser_error_string[LPARSER_ERR_BAD_OPERAND],
                        curr);
                break;
            }
        }
        // Error = break.
        if (operand->tag == LTAG_ERR) {
            last_attach(qexpr, operand);
            qexpr = operand;
            break;
        }
        last_attach(operand, qexpr);
    }
    // Error = break.
    if (qexpr && qexpr->tag == LTAG_ERR) {
        return qexpr;
    }
    // } or error.
    if (curr->type != LTOK_CBRC) {
        struct last* err = last_alloc(LTAG_ERR,
                lparser_error_string[LPARSER_ERR_MISSING_CBRC],
                curr);
        last_attach(qexpr, err);
        return err;
    }
    curr = curr->next; // Skip }.
    *last = curr;
    return qexpr;
}

static struct last* lparse_program(struct ltok* tokens, struct last** error) {
    if (tokens->type == LTOK_EOF) {
        return NULL;
    }
    struct last* prg = last_alloc(LTAG_PROG, "", NULL);
    struct ltok* curr = tokens;
    struct last* expr = NULL;
    *error = NULL;
    while ((expr = lparse_sexpr(curr, &curr))) {
        last_attach(expr, prg);
        if (expr->tag == LTAG_ERR) {
            *error = expr;
            break;
        }
    }
    return prg;
}

struct last* lisp_parse(struct ltok* tokens, struct last** error) {
    if (!tokens) {
        return NULL;
    }
    return lparse_program(tokens, error);
}

void last_free(struct last* ast) {
    if (!ast) {
        return;
    }
    if (ast->children) {
        size_t childrenc = ast->childrenc;
        for (size_t i = 0; i < childrenc; i++) {
            last_free(ast->children[i]);
        }
        free(ast->children);
    }
    if (ast->content) {
        free(ast->content);
    }
    free(ast);
}

bool last_are_equal(const struct last* left, const struct last* right) {
    if (!left || !right) {
        return false;
    }
    return left->tag == right->tag && strcmp(left->content, right->content) == 0;
}

bool last_are_all_equal(const struct last* left, const struct last* right) {
    if (!left || !right) {
        return false;
    }
    if (left->childrenc != right->childrenc) {
        return false;
    }
    if (!last_are_equal(left, right)) {
        return false;
    }
    for (size_t i = 0; i < left->childrenc; i++) {
        if (!last_are_all_equal(left->children[i], right->children[i])) {
            return false;
        }
    }
    return true;
}

static const char last_to_string_format_simple[] = "tag: %si, %d:%d";
static const char last_to_string_format[] = "tag: %s, %d:%d, content: \"%s\"";
void last_to_string(const struct last* ast, char* out) {
    if (!ast) {
        return;
    }
    if (!ast->content) {
        sprintf(out, last_to_string_format_simple,
                ltag_string[ast->tag], ast->line, ast->col);
    } else {
        sprintf(out, last_to_string_format,
                ltag_string[ast->tag], ast->line, ast->col, ast->content);
    }
}

size_t last_printlen(const struct last* ast) {
    if (!ast) {
        return 0;
    }
    size_t len = 0;
    len += sizeof(last_to_string_format);
    len += strlen(ltag_string[ast->tag]);
    len += 10; // + last->line.
    len += 10; // + last->col.
    if (ast->content) {
        len += strlen(ast->content);
    }
    return len;
}

static void last_print_to_indent(const struct last* ast, FILE* out, unsigned int indent) {
    if (!ast) {
        return;
    }
    size_t len = last_printlen(ast);
    char* buffer = calloc(len + 1, sizeof(char));
    last_to_string(ast, buffer);
    while (indent-- > 0) {
        fputs("  ", out);
    }
    fputs(buffer, out);
    free(buffer);
}

void last_print_to(const struct last* ast, FILE* out) {
    if (!ast) {
        return;
    }
    last_print_to_indent(ast, out, 0);
}

static void last_print_all_to_rec(const struct last* ast, FILE* out, unsigned int level) {
    if (!ast) {
        return;
    }
    last_print_to_indent(ast, out, level);
    fputc('\n', out);
    for (size_t i = 0; i < ast->childrenc; i++) {
        last_print_all_to_rec(ast->children[i], out, level + 1);
    }
}

void last_print_all_to(const struct last* ast, FILE* out) {
    if (!ast) {
        return;
    }
    last_print_all_to_rec(ast, out, 0);
}
