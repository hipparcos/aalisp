#include "lerr.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* lerr_describe(enum lerr_code code) {
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

struct lerr* lerr_alloc(void) {
    struct lerr* err = calloc(1, sizeof(struct lerr));
    return err;
}

void lerr_free(struct lerr* err) {
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

bool lerr_copy(struct lerr* dest, const struct lerr* src) {
    if (!dest || !src) {
        return false;
    }
    dest->code = src->code;
    lerr_annotate(dest, src->message);
    lerr_set_file(dest, src->file);
    lerr_set_location(dest, src->line, src->col);
    if (src->inner) {
        dest->inner = lerr_alloc();
        lerr_copy(dest->inner, src->inner);
    }
    return true;
}

struct lerr* lerr_throw_va(enum lerr_code code, const char* fmt, va_list va) {
    struct lerr* err = lerr_alloc();
    err->code = code;
    lerr_annotate_va(err, fmt, va);
    return err;
}

struct lerr* lerr_throw(enum lerr_code code, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    struct lerr* err = lerr_throw_va(code, fmt, va);
    va_end(va);
    return err;
}

struct lerr* lerr_propagate(struct lerr* inner, const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    struct lerr* outer = lerr_throw_va(0, fmt, va);
    va_end(va);
    lerr_wrap(outer, inner);
    return outer;
}

void lerr_annotate(struct lerr* err, const char* fmt, ...) {
    if (!fmt || strlen(fmt) == 0) {
        err->message[0] = '\0';
        return;
    }
    va_list va;
    va_start(va, fmt);
    lerr_annotate_va(err, fmt, va);
    va_end(va);
}

void lerr_annotate_va(struct lerr* err, const char* fmt, va_list va) {
    if (!fmt || strlen(fmt) == 0) {
        err->message[0] = '\0';
        return;
    }
    vsnprintf(err->message, sizeof(err->message), fmt, va);
}

void lerr_set_file(struct lerr* err, const char* file) {
    if (!err) {
        return;
    }
    struct lerr* cause = lerr_cause(err);
    if (cause->file) {
        free(cause->file);
        cause->file = NULL;
    }
    if (file) {
        size_t len = strlen(file);
        cause->file = calloc(len+1, 1);
        strncpy(cause->file, file, len);
    }
}

void lerr_set_location(struct lerr* err, int line, int col) {
    if (!err) {
        return;
    }
    struct lerr* cause = lerr_cause(err);
    cause->line = line;
    cause->col = col;
}

void lerr_wrap(struct lerr* outer, struct lerr* inner) {
    if (!outer || !inner || outer->inner) {
        return;
    }
    outer->inner = inner;
}

struct lerr* lerr_cause(struct lerr* err) {
    if (!err) {
        return NULL;
    }
    while (err->inner) { err = err->inner; }
    return err;
}

void lerr_print_marker_to(struct lerr* err, int indent, FILE* out) {
    struct lerr* cause = lerr_cause(err);
    int col = cause->col;
    if (col) {
        for (int i = -indent+1; i < col; i++) {
            fputc(' ', out);
        }
        fputc('^', out);
        fputc('\n', out);
    }
}

void lerr_print_to(struct lerr* err, FILE* out) {
    if (!err) {
        return;
    }
    struct lerr* cause = lerr_cause(err);
    if (cause->file)
        { fprintf(out, "<%s>:", cause->file); }
    if (cause->line && cause->col)
        { fprintf(out, "%d:%d:", cause->line, cause->col); }
    if (cause->code)
        { fprintf(out, "error #%d:", cause->code); }
    while (err) {
        fprintf(out, " %s", err->message);
        err = err->inner;
    }
    fputs(".\n", out);
}

void lerr_print_cause_to(struct lerr* err, FILE* out) {
    if (!err) {
        return;
    }
    struct lerr* cause = lerr_cause(err);
    fprintf(out, "Error: %s", cause->message);
}
