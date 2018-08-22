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
    if (err->inner) {
        lerr_free(err->inner);
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
    if (src->inner) {
        dest->inner = lerr_alloc();
        lerr_copy(dest->inner, src->inner);
    }
}

struct lerr_ctx* lerr_throw_va(enum lerr code, const char* fmt, va_list va) {
    struct lerr_ctx* err = lerr_alloc();
    err->code = code;
    lerr_annotate_va(err, fmt, va);
    return err;
}

struct lerr_ctx* lerr_throw(enum lerr code, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    struct lerr_ctx* err = lerr_throw_va(code, fmt, va);
    va_end(va);
    return err;
}

struct lerr_ctx* lerr_propagate(struct lerr_ctx* inner, enum lerr code, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    struct lerr_ctx* outer = lerr_throw_va(code, fmt, va);
    va_end(va);
    lerr_wrap(outer, inner);
    return outer;
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

void lerr_wrap(struct lerr_ctx* outer, struct lerr_ctx* inner) {
    if (!outer || !inner || outer->inner) {
        return;
    }
    outer->inner = inner;
}

const struct lerr_ctx* lerr_cause(const struct lerr_ctx* err) {
    while (err->inner) { err = err->inner; }
    return err;
}

void lerr_print_to(const struct lerr_ctx* err, FILE* out) {
    if (!err) {
        return;
    }
    const struct lerr_ctx* cause = lerr_cause(err);
    if (cause->file)
        { fprintf(out, "<%s>:", cause->file); }
    if (cause->line && cause->col)
        { fprintf(out, "%d:%d:", cause->line, cause->col); }
    if (cause->code)
        { fprintf(out, "#%d", cause->code); }
    while (err) {
        fprintf(out, ": %s", err->message);
        err = err->inner;
    }
    fputc('\n', out);
}

static size_t lerr_sprint(const struct lerr_ctx* err, char* out, size_t len) {
    char dummy[2];
    if (!out) {
        out = &dummy[0];
        len = sizeof(dummy);
    }
    return snprintf(out, len, "Error #%d: %s", err->code, err->message);
}

size_t lerr_printlen(const struct lerr_ctx* err) {
    if (!err) {
        return 0;
    }
    err = lerr_cause(err);
    return 1 + lerr_sprint(err, NULL, 0);
}

/* Needed because of lval way of printing. */
void lerr_as_string(const struct lerr_ctx* err, char* out, size_t len) {
    if (!err) {
        return;
    }
    err = lerr_cause(err);
    lerr_sprint(err, out, len);
}
