#include "llexer.h"

#include <stdio.h>
#include <string.h>

const char* ltok_type_string[] = {
    "EOF",
    "error",
    "(",
    ")",
    "{",
    "}",
    "symbol",
    "number",
    "double",
    "string",
};

enum llex_error {
    LLEX_ERR_MISSING_QUOTE,
    LLEX_ERR_UNKNOWN_CHARACTER,
};
static const char* llex_error_format[] = {
    "missing closing quotation mark",
    "unknown character '%c'",
};

struct lscanner {
    const char* input;
    size_t start;
    size_t pos;
    size_t width;
    int line;
    int col;
    enum ltok_type tok;
    enum llex_error err;
};

static inline bool llex_is_whitespace(char c) {
    switch (c) {
    case ' ':  return true;
    case '\t': return true;
    case '\r': return true;
    case '\n': return true;
    }
    return false;
}
static bool llex_is_whitespace(char c);

static inline bool llex_is_numeric(char c) {
    return (c >= '0' && c <= '9');
}
static bool llex_is_numeric(char c);

static inline bool llex_is_letter(char c) {
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z');
}
static bool llex_is_letter(char c);

static inline bool llex_is_sign(char c) {
    return c != '\0' && strchr("+-*/%^?!:;,._#~<>=$§£¤µ", c); // strchr matches '\0'.
}
static bool llex_is_sign(char c);

static void llex_retain(struct lscanner* scanner) {
    if (scanner->input[scanner->pos] == '\n') {
        scanner->line++;
        scanner->col = 1;
    }
    scanner->pos++;
    scanner->width++;
}

static void llex_skip(struct lscanner* scanner) {
    if (scanner->input[scanner->pos] == '\n') {
        scanner->line++;
        scanner->col = 1;
    } else {
        scanner->col++;
    }
    scanner->pos++;
    scanner->width = 0;
}

static void llex_reset(struct lscanner* scanner) {
    scanner->tok = LTOK_EOF;
    scanner->col += scanner->width;
    scanner->start = scanner->pos;
    scanner->width = 0;
}

static void llex_skip_whitespaces(struct lscanner* scanner) {
    while (llex_is_whitespace(scanner->input[scanner->pos])) {
        llex_skip(scanner);
    }
    llex_reset(scanner);
}

static bool llex_scan_string(struct lscanner* scanner) {
    const char* c = &scanner->input[scanner->pos];
    if (*c == '\"') {
        llex_retain(scanner);
        c++; // Pass first quote.
    }
    /* Skip escaped quotes. */
    while ((*c != '"' || *(c-1) == '\\') && *c != '\0') {
        llex_retain(scanner);
        c++;
    }
    // No closing " seen before EOF.
    if (*c == '\0') {
        scanner->tok = LTOK_ERR;
        scanner->err = LLEX_ERR_MISSING_QUOTE;
        return false;
    }
    // Include last quote.
    llex_retain(scanner);
    scanner->tok = LTOK_STR;
    return true;
}

static bool llex_scan_number(struct lscanner* scanner) {
    const char* c = &scanner->input[scanner->pos];
    if (*c == '-') {
        llex_retain(scanner);
        c++;
    }
    /* Only one point allowed. */
    int once = 0;
    while (llex_is_numeric(*c) || (*c == '.' && once++ < 1)) {
        llex_retain(scanner);
        c++;
    }
    if (once) {
        scanner->tok = LTOK_DBL;
    } else {
        scanner->tok = LTOK_NUM;
    }
    return true;
}

static bool llex_scan_symbol(struct lscanner* scanner) {
    const char* c = &scanner->input[scanner->pos];
    while (llex_is_letter(*c) || llex_is_numeric(*c) || llex_is_sign(*c)) {
        llex_retain(scanner);
        c++;
    }
    scanner->tok = LTOK_SYM;
    return true;
}

static inline char llex_peek(struct lscanner* scanner) {
    return scanner->input[scanner->pos+1];
}
static char llex_peek(struct lscanner* scanner);

static bool llex_next(struct lscanner* scanner) {
    llex_reset(scanner);
    llex_skip_whitespaces(scanner);
    char c = scanner->input[scanner->pos];
    /* Match special characters. */
    switch (c) {
    case '(':
        scanner->tok = LTOK_OPAR;
        llex_retain(scanner);
        return true;
    case ')':
        scanner->tok = LTOK_CPAR;
        llex_retain(scanner);
        return true;
    case '{':
        scanner->tok = LTOK_OBRC;
        llex_retain(scanner);
        return true;
    case '}':
        scanner->tok = LTOK_CBRC;
        llex_retain(scanner);
        return true;
    case '"':
        return llex_scan_string(scanner);
    case '\0':
    case EOF:
        scanner->tok = LTOK_EOF;
        llex_retain(scanner);
        return false;
    }
    /* Match numbers. */
    if (llex_is_numeric(c) || (c == '-' && llex_is_numeric(llex_peek(scanner)))) {
        return llex_scan_number(scanner);
    }
    /* Match symbols. */
    if (llex_is_letter(c) || llex_is_sign(c)) {
        return llex_scan_symbol(scanner);
    }
    scanner->tok = LTOK_ERR;
    scanner->err = LLEX_ERR_UNKNOWN_CHARACTER;
    return false;
}

/** llex_emit allocates a token and fills it using the last scan token. */
static struct ltok* llex_emit(struct lscanner* scanner) {
    struct ltok* tok = calloc(sizeof(struct ltok), 1);
    tok->type = scanner->tok;
    tok->line = scanner->line;
    tok->col  = scanner->col;
    if (tok->type == LTOK_EOF) {
        tok->content = NULL;
        return tok;
    }
    if (tok->type == LTOK_ERR) {
        const char* err_format = llex_error_format[scanner->err];
        size_t len = strlen(err_format) + 10 + 1;
        tok->content = calloc(sizeof(char), len);
        sprintf(tok->content, err_format, scanner->input[scanner->pos]);
        return tok;
    }
    tok->content = calloc(sizeof(char), scanner->width + 1);
    strncpy(tok->content, &scanner->input[scanner->start], scanner->width);
    return tok;
}

static struct ltok* llex_emitEOF() {
    struct ltok* tok = calloc(1, sizeof(struct ltok));
    tok->type = LTOK_EOF;
    tok->content = NULL;
    return tok;
}

static struct ltok* llex_emitOPAR() {
    struct ltok* tok = calloc(1, sizeof(struct ltok));
    tok->type = LTOK_OPAR;
    tok->content = calloc(2, 1);
    strncpy(tok->content, "(", 2);
    return tok;
}

static struct ltok* llex_emitCPAR() {
    struct ltok* tok = calloc(1, sizeof(struct ltok));
    tok->type = LTOK_CPAR;
    tok->content = calloc(2, 1);
    strncpy(tok->content, ")", 2);
    return tok;
}

/** llex_append appends token to list.
 ** Returns the address of the next element. */
static struct ltok** llex_append(struct ltok** last, struct ltok* curr) {
    if (*last) {
        (*last)->next = curr;
    } else {
        *last = curr;
    }
    return &((*last)->next);
}

static struct ltok* llex(const char* input, struct ltok** error, bool surround) {
    struct lscanner scanner = {0};
    scanner.input = input;
    scanner.line = 1;
    scanner.col = 1;
    struct ltok *head = NULL, *curr = NULL, **last = &head;
    *error = NULL;
    while (input && llex_next(&scanner)) {
        curr = llex_emit(&scanner);
        last = llex_append(last, curr);
    }
    /* Check for lexing error & append EOF. */
    curr = llex_emit(&scanner);
    if (curr->type == LTOK_ERR) {
        *error = curr;
        last = llex_append(last, curr);
        /* Force EOF emission. */
        llex_append(last, llex_emitEOF());
    } else {
        /* Optional: ensure that it's a sexpr. */
        if (surround && head && head->type != LTOK_OPAR) {
            struct ltok* opar = llex_emitOPAR();
            struct ltok* cpar = llex_emitCPAR();
            llex_append(&opar, head);
            last = llex_append(last, cpar);
            head = opar;
        }
        /* Last call to emit returns a LTOK_EOF. */
        llex_append(last, curr);
    }
    return head;
}

struct ltok* lisp_lex(const char* input, struct ltok** error) {
    return llex(input, error, false);
}

struct ltok* lisp_lex_surround(const char* input, struct ltok** error) {
    return llex(input, error, true);
}

void llex_free(struct ltok* tokens) {
    if (!tokens) {
        return;
    }
    struct ltok *curr, *next = tokens;
    while ((curr = next) && curr->type != LTOK_EOF) {
        next = curr->next;
        if (curr->content) {
            free(curr->content);
        }
        free(curr);
    }
    free(curr); // free LTOK_EOS.
}

bool llex_are_equal(struct ltok* left, struct ltok* right) {
    if (!left || !right) {
        return false;
    }
    return left->type == right->type && strcmp(left->content, right->content) == 0;
}

bool llex_are_all_equal(struct ltok* left, struct ltok* right) {
    if (!left || !right) {
        return false;
    }
    struct ltok *lcurr = NULL, *lnext = left;
    struct ltok *rcurr = NULL, *rnext = right;
    while ((lcurr = lnext) && (rcurr = rnext)
         && lcurr->type != LTOK_EOF && rcurr->type != LTOK_EOF) {
        lnext = lcurr->next;
        rnext = rcurr->next;
        if (!llex_are_equal(lcurr, rcurr)) {
            return false;
        }
    }
    return lcurr && lcurr->type == LTOK_EOF && rcurr && rcurr->type == LTOK_EOF;
}

static const char llex_to_string_format[] = "tok:{type: %s, %d:%d, content: \"%s\"}";
void llex_to_string(struct ltok* token, char* out) {
    if (!token) {
        return;
    }
    sprintf(out, llex_to_string_format,
            ltok_type_string[token->type], token->line, token->col, token->content);
}

size_t llex_printlen(struct ltok* token) {
    if (!token) {
        return 0;
    }
    size_t len = 0;
    len += sizeof(llex_to_string_format);
    len += 10; // + ltok->type.
    len += 10; // + ltok->line.
    len += 10; // + ltok->col.
    if (token->content) {
        len += strlen(token->content);
    }
    return len;
}

void llex_print_to(struct ltok* token, FILE* out) {
    if (!token) {
        return;
    }
    size_t len = llex_printlen(token);
    char* buffer = calloc(sizeof(char), len + 1);
    llex_to_string(token, buffer);
    fputs(buffer, out);
    free(buffer);
}

void llex_print_all_to(struct ltok* tokens, FILE* out) {
    if (!tokens) {
        return;
    }
    struct ltok *curr, *next = tokens;
    while ((curr = next) && curr->type != LTOK_EOF) {
        next = curr->next;
        llex_print_to(curr, out);
        fputc('\n', out);
    }
}
