#include "llexer.h"

#include <stdio.h>
#include <string.h>

#include "lerr.h"

#define STRINGIFY(c) #c

const char* llex_type_string(enum ltok_type type) {
    switch (type) {
        case LTOK_EOF:  return "EOF";
        case LTOK_ERR:  return "error";
        case LTOK_OPAR: return STRINGIFY(LLEX_OPAR);
        case LTOK_CPAR: return STRINGIFY(LLEX_CPAR);
        case LTOK_OBRC: return STRINGIFY(LLEX_OBRC);
        case LTOK_CBRC: return STRINGIFY(LLEX_CBRC);
        case LTOK_DOLL: return STRINGIFY(LLEX_DOLL);
        case LTOK_SYM:  return "symbol";
        case LTOK_NUM:  return "number";
        case LTOK_DBL:  return "double";
        case LTOK_STR:  return "string";
    }
    return NULL;
}

struct lscanner {
    const char* input;
    size_t start;
    size_t pos;
    size_t width;
    int line;
    int col;
    enum ltok_type tok;
    enum lerr_code err;
};

static bool llex_is_whitespace(char c) {
    switch (c) {
    case ' ':  return true;
    case '\t': return true;
    case '\r': return true;
    case '\n': return true;
    }
    return false;
}

static bool llex_is_numeric(char c) {
    return (c >= '0' && c <= '9');
}

static bool llex_is_letter(char c) {
    return (c >= 'a' && c <= 'z')
        || (c >= 'A' && c <= 'Z');
}

static bool llex_is_symbolic(char c) {
    return c != '\0' && strchr(LLEX_SYMB, c); // strchr matches '\0'.
}

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

static void llex_skip_to_EOL(struct lscanner* scanner) {
    char c;
    while ((c = scanner->input[scanner->pos]) && c != '\n' && c != '\0') {
        llex_skip(scanner);
    }
    llex_reset(scanner);
}

static bool llex_scan_string(struct lscanner* scanner) {
    const char* c = &scanner->input[scanner->pos];
    if (*c == LLEX_OSTR) {
        llex_retain(scanner);
        c++; // Pass first quote.
    }
    /* Skip escaped quotes. */
    while ((*c != LLEX_CSTR || *(c-1) == '\\') && *c != '\0') {
        llex_retain(scanner);
        c++;
    }
    // No closing " seen before EOF.
    if (*c == '\0') {
        scanner->tok = LTOK_ERR;
        scanner->err = LERR_LEXER_MISSING_QUOTE;
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
    while (llex_is_letter(*c) || llex_is_numeric(*c) || llex_is_symbolic(*c)) {
        llex_retain(scanner);
        c++;
    }
    scanner->tok = LTOK_SYM;
    return true;
}

static char llex_peek(struct lscanner* scanner) {
    return scanner->input[scanner->pos+1];
}

static bool llex_next(struct lscanner* scanner) {
    llex_reset(scanner);
    llex_skip_whitespaces(scanner);
    char c = scanner->input[scanner->pos];
    /* Match special characters. */
    switch (c) {
    case LLEX_COMM: // Comment.
        llex_skip_to_EOL(scanner);
        return llex_next(scanner);
    case LLEX_OPAR:
        scanner->tok = LTOK_OPAR;
        llex_retain(scanner);
        return true;
    case LLEX_CPAR:
        scanner->tok = LTOK_CPAR;
        llex_retain(scanner);
        return true;
    case LLEX_OBRC:
        scanner->tok = LTOK_OBRC;
        llex_retain(scanner);
        return true;
    case LLEX_CBRC:
        scanner->tok = LTOK_CBRC;
        llex_retain(scanner);
        return true;
    case LLEX_DOLL:
        scanner->tok = LTOK_DOLL;
        llex_retain(scanner);
        return true;
    case LLEX_OSTR:
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
    if (llex_is_letter(c) || llex_is_symbolic(c)) {
        return llex_scan_symbol(scanner);
    }
    scanner->tok = LTOK_ERR;
    scanner->err = LERR_LEXER_UNKNOWN_CHAR;
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
        tok->content = NULL;
        return tok;
    }
    size_t len = scanner->width;
    tok->content = malloc(len+1);
    memcpy(tok->content, &scanner->input[scanner->start], len);
    tok->content[len] = '\0';
    tok->len = len;
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
    tok->content = malloc(2);
    memcpy(tok->content, STRINGIFY(LLEX_OPAR), 2);
    return tok;
}

static struct ltok* llex_emitCPAR() {
    struct ltok* tok = calloc(1, sizeof(struct ltok));
    tok->type = LTOK_CPAR;
    tok->content = malloc(2);
    tok->len = 1;
    memcpy(tok->content, STRINGIFY(LLEX_CPAR), 2);
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

static struct ltok* llex(const char* input, struct lerr** err, bool surround) {
    if (!input || strlen(input) == 0) {
        return llex_emitEOF();
    }
    struct lscanner scanner = {0};
    scanner.input = input;
    scanner.line = 1;
    scanner.col = 1;
    struct ltok *head = NULL, *curr = NULL, **last = &head, *error = NULL;
    while (input && llex_next(&scanner)) {
        curr = llex_emit(&scanner);
        last = llex_append(last, curr);
    }
    /* Check for lexing error & append EOF. */
    curr = llex_emit(&scanner);
    if (curr->type == LTOK_ERR) {
        error = curr;
        last = llex_append(last, curr);
        /* Force EOF emission. */
        llex_append(last, llex_emitEOF());
    } else {
        /* Optional: ensure that it's a sexpr. */
        if (head && surround && ((head && head->type != LTOK_OPAR)
                              || (curr && curr->type != LTOK_CPAR))) {
            struct ltok* opar = llex_emitOPAR();
            struct ltok* cpar = llex_emitCPAR();
            cpar->line = curr->line;
            cpar->col = curr->col++;
            llex_append(&opar, head);
            last = llex_append(last, cpar);
            head = opar;
        }
        /* Last call to emit returns a LTOK_EOF. */
        llex_append(last, curr);
    }
    /* Error handling. */
    if (error) {
        switch (scanner.err) {
        case LERR_LEXER_MISSING_QUOTE:
            *err = lerr_throw(scanner.err, "missing closing quotation mark");
            break;
        case LERR_LEXER_UNKNOWN_CHAR:
            *err = lerr_throw(scanner.err, "unknown character '%c'",
                    scanner.input[scanner.pos]);
            break;
        default:
            *err = lerr_throw(scanner.err, "unknown error");
            break;
        }
        lerr_set_location(*err, scanner.line, scanner.col);
    }
    return head;
}

struct ltok* llex_lex(const char* input, struct lerr** err) {
    return llex(input, err, false);
}

struct ltok* llex_lex_from(const FILE* input, struct lerr** err) {
    (void)input;
    (void)err;
    puts("llex_lex_from not implemented yet.");
    exit(EXIT_FAILURE);
}

void llex_free(struct ltok* tokens) {
    if (!tokens) {
        return;
    }
    struct ltok *curr, *next = tokens;
    while ((curr = next)) {
        next = curr->next;
        if (curr->content) {
            free(curr->content);
        }
        if (curr->type == LTOK_EOF) {
            free(curr);
            break;
        }
        free(curr);
    }
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

void llex_print_to(struct ltok* token, FILE* out) {
    if (!token) {
        return;
    }
    fprintf(out, "tok:{type: %s, %d:%d, content: \"%s\"}",
            llex_type_string(token->type), token->line, token->col, token->content);
    fputc('\n', out);
}

void llex_print_all_to(struct ltok* tokens, FILE* out) {
    if (!tokens) {
        return;
    }
    struct ltok *curr, *next = tokens;
    while ((curr = next) && curr->type != LTOK_EOF) {
        next = curr->next;
        llex_print_to(curr, out);
    }
}
