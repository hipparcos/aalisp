#ifndef H_LERR_
#define H_LERR_

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

/** lerr is an error code. */
enum lerr {
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

/** lerr_ctx gathers informations about an error. */
struct lerr_ctx {
    enum lerr code;
    char message[80];
    /** Error location. */
    char* file;
    int line;
    int col;
};

/** lerr_alloc allocates an lerr_ctx.
 ** Caller is repsonsible for calling lerr_free on returned value. */
struct lerr_ctx* lerr_alloc(void);
/** lerr_free frees err.
 ** err must not be used afterwards. */
void lerr_free(struct lerr_ctx* err);
/** lerr_copy copies src into dest. */
void lerr_copy(struct lerr_ctx* dest, const struct lerr_ctx* src);

/** lerr_describe returns a message describing the error code. */
const char* lerr_describe(enum lerr code);
/** lerr_annotate annotates err 'à la' printf. */
void lerr_annotate(struct lerr_ctx* err, const char* fmt, ...);
/** lerr_annotate_va is the same as lerr_annotate but takes a va_list. */
void lerr_annotate_va(struct lerr_ctx* err, const char* fmt, va_list va);
/** lerr_file_info adds file infos to err. */
void lerr_file_info(struct lerr_ctx* err, const char* file, int line, int col);

/** lerr_printlen returns the length of the error when printed as string. */
size_t lerr_printlen(const struct lerr_ctx* err);
/** lerr_as_string returns the error as string printed in out. */
void lerr_as_string(const struct lerr_ctx* err, char* out, size_t len);
/** lerr_print_to prints err to  the file out. */
void lerr_print_to(const struct lerr_ctx* err, FILE* out);
/** lerr_print prints err to stdout. */
#define lerr_print(err,out) lerr_print_to(err, stdout)

#endif
