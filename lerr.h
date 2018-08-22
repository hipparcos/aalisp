#ifndef H_LERR_
#define H_LERR_

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

/** lerr is an error code. */
enum lerr_code {
    LERR_UNKNOWN = 0,
    LERR_DEAD_REF,
    LERR_AST,
    LERR_EVAL,
    LERR_DIV_ZERO,
    LERR_BAD_SYMBOL,
    LERR_BAD_OPERAND,
    LERR_TOO_MANY_ARGS,
    LERR_TOO_FEW_ARGS,
};

/** lerr gathers informations about an error. */
struct lerr {
    /** lerr.code is the type of error. */
    enum lerr_code code;
    /** lerr.message is the actual error message displayed to the user. */
    char message[80];
    /** lerr.file is the file in which error occurs in lisp file. */
    char* file;
    /** lerr.line is the line at which error occurs in lisp file. */
    int line;
    /** lerr.line is the column at which error occurs in lisp file. */
    int col;
    /** lerr.inner is a pointer to a wrapped error. */
    struct lerr* inner;
};

/** lerr_alloc allocates an lerr.
 ** Caller is responsible for calling lerr_free on returned lerr. */
struct lerr* lerr_alloc(void);
/** lerr_free frees err.
 ** err must not be used afterwards.
 ** lerr_free must be called on the outermost error. */
void lerr_free(struct lerr* err);
/** lerr_copy copies src into dest (and inner errors). */
void lerr_copy(struct lerr* dest, const struct lerr* src);
/** lerr_throw allocates an error and describes it.
 ** Caller is responsible for calling lerr_free on returned lerr. */
struct lerr* lerr_throw(enum lerr_code code, const char* fmt, ...);
struct lerr* lerr_throw_va(enum lerr_code code, const char* fmt, va_list va);
/** lerr_propagate allocates an error, describes it and wraps an inner error.
 ** Caller is responsible for calling lerr_free on returned lerr. */
struct lerr* lerr_propagate(struct lerr* inner, enum lerr_code code, const char* fmt, ...);

/** lerr_describe returns a message describing the error code. */
const char* lerr_describe(enum lerr_code code);
/** lerr_annotate annotates err 'à la' printf. */
void lerr_annotate(struct lerr* err, const char* fmt, ...);
void lerr_annotate_va(struct lerr* err, const char* fmt, va_list va);
/** lerr_file_info adds file infos to err. */
void lerr_file_info(struct lerr* err, const char* file, int line, int col);
/** lerr_wrap encapsulates an error into an other. */
void lerr_wrap(struct lerr* outer, struct lerr* inner);
/** lerr_cause returns the innermost error.
 ** Returned pointer stays valid until lerr_free is called on the outermost error. */
const struct lerr* lerr_cause(const struct lerr* err);

/** lerr_printlen returns the length of the error when printed as string. */
size_t lerr_printlen(const struct lerr* err);
/** lerr_as_string returns the error as string printed in out. */
void lerr_as_string(const struct lerr* err, char* out, size_t len);
/** lerr_print_to prints err to  the file out. */
void lerr_print_to(const struct lerr* err, FILE* out);
/** lerr_print prints err to stdout. */
#define lerr_print(err,out) lerr_print_to(err, stdout)

#endif
