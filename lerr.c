#include "lerr.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* lerr_describe(enum lerr code) {
    switch (code) {
    case LERR_DEAD_REF:      return "dead reference";
    case LERR_AST:           return "ast error";
    case LERR_EVAL:          return "evaluation error";
    case LERR_DIV_ZERO:      return "division by zero";
    case LERR_BAD_SYMBOL:    return "bad symbol";
    case LERR_BAD_OPERAND:   return "bad operand";
    case LERR_TOO_MANY_ARGS: return "too many arguments";
    case LERR_TOO_FEW_ARGS:  return "too few arguments";
    default:                 return "unknown error";
    }
}

struct lerr_ctx* lerr_alloc(void) {
    struct lerr_ctx* err = calloc(1, sizeof(struct lerr_ctx));
    return err;
}

void lerr_free(struct lerr_ctx* err) {
    if (!err) {
        return;
    }
    if (err->file) {
        free(err->file);
    }
    free(err);
}

void lerr_copy(struct lerr_ctx* dest, const struct lerr_ctx* src) {
    if (!dest || !src) {
        return;
    }
    dest->code = src->code;
    lerr_annotate(dest, src->message);
    lerr_file_info(dest, src->file, src->line, src->col);
}

void lerr_annotate(struct lerr_ctx* err, const char* fmt, ...) {
    if (!fmt || strlen(fmt) == 0) {
        err->message[0] = '\0';
        return;
    }
    va_list va;
    va_start(va, fmt);
    lerr_annotate_va(err, fmt, va);
    va_end(va);
}

void lerr_annotate_va(struct lerr_ctx* err, const char* fmt, va_list va) {
    if (!fmt || strlen(fmt) == 0) {
        err->message[0] = '\0';
        return;
    }
    vsnprintf(err->message, sizeof(err->message), fmt, va);
}

void lerr_file_info(struct lerr_ctx* err, const char* file, int line, int col) {
    if (!err) {
        return;
    }
    if (err->file) {
        free(err->file);
        err->file = NULL;
    }
    if (file) {
        size_t len = strlen(file);
        err->file = calloc(len+1, 1);
        strncpy(err->file, file, len);
    }
    err->line = line;
    err->col = col;
}

#define FORMAT_LONG(err)  "Error #%d: %s: %s", err->code, lerr_describe(err->code), err->message
#define FORMAT_SHORT(err) "Error #%d: %s", err->code, lerr_describe(err->code)

static size_t lerr_sprint(const struct lerr_ctx* err, char* out, size_t len) {
    char dummy[2];
    char* dest = out;
    if (!dest) {
        dest = &dummy[0];
        len = sizeof(dummy);
    }
    if (strlen(err->message) > 0) {
        return snprintf(dest, len, FORMAT_LONG(err));
    } else {
        return snprintf(dest, len, FORMAT_SHORT(err));
    }
}

void lerr_print_to(const struct lerr_ctx* err, FILE* out) {
    if (!err) {
        return;
    }
    if (strlen(err->message) > 0) {
        fprintf(out, FORMAT_LONG(err));
    } else {
        fprintf(out, FORMAT_SHORT(err));
    }
    fputc('\n', out);
}

size_t lerr_printlen(const struct lerr_ctx* err) {
    if (!err) {
        return 0;
    }
    return 1 + lerr_sprint(err, NULL, 0);
}

/* Needed because of lval way of printing. */
void lerr_as_string(const struct lerr_ctx* err, char* out, size_t len) {
    if (!err) {
        return;
    }
    lerr_sprint(err, out, len);
}
