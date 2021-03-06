#ifndef _H_LVAL_
#define _H_LVAL_

#include <stdbool.h>
#include <stdio.h>

#include "vendor/mini-gmp/mini-gmp.h"

#include "lerr.h"

/** ltype is the type of a lval. */
enum ltype {
    LVAL_NIL = 0,
    LVAL_BOOL,
    LVAL_NUM,
    LVAL_BIGNUM,
    LVAL_DBL,
    LVAL_ERR,
    LVAL_STR,
    LVAL_SYM,
    LVAL_FUNC,
    LVAL_SEXPR,
    LVAL_QEXPR,
};

/* Forward declaration of lfunc, see lfunc.h */
struct lfunc;

/** lval is the public handle to a ldata.
 ** This level of indirection is used to prepare the work on a GC. */
struct lval {
    /** lval.alive is a code used to detect aliveness of the payload. */
    int alive;
    /** lval.data is a pointer to the actual ldata. */
    struct ldata* data;
    /** lval.ast is a pointer to the corresponding ast node. For error handling.
     ** Must not be used after the corresponding ast had been cleaned. */
    const struct last* ast;
};

/* Special lvals used in builtins. */
extern const struct lval lnil;    /** = nil */
extern const struct lval lzero;   /** = 0   */
extern const struct lval lone;    /** = 1   */
extern const struct lval lemptyq; /** = {}  */

/* Constructor & Destructor */
/** lval_alloc returns a handle to a new lval of type LVAL_NIL.
 ** Caller is responsible for calling lval_free. */
struct lval* lval_alloc(void);
/** lval_free reclaims v internal memory.
 ** v must not be used afterwards. */
bool lval_free(struct lval* v);

/* Memory management */
/** lval_clear mutates v to LVAL_NIL.
 ** It immediately clears v internal memory.
 ** v becomes of type LVAL_NIL afterwards. */
bool lval_clear(struct lval* v);
/** lval_dup links dest to the same underlying data than src.
 ** lval_dup fails when dest == src. */
bool lval_dup(struct lval* dest, const struct lval* src);
/** lval_copy does a deep copy of src into dest. */
bool lval_copy(struct lval* dest, const struct lval* src);

/* Mutators */
/** lval_mut_nil mutates v to LVAL_NIL type. */
bool lval_mut_nil(struct lval* v);
/** lval_mut_bool mutates v to LVAL_BOOL type. */
bool lval_mut_bool(struct lval* v, bool x);
/** lval_mut_num mutates v to LVAL_NUM type. */
bool lval_mut_num(struct lval* v, long x);
/** lval_mut_bignum mutates v to LVAL_BIGNUM type. x is copied. */
bool lval_mut_bignum(struct lval* v, const mpz_t x);
/** lval_mut_dbl mutates v to LVAL_DBL type. */
bool lval_mut_dbl(struct lval* v, double x);
/** lval_mut_err_code mutates v to LVAL_ERR type. */
bool lval_mut_err_code(struct lval* v, enum lerr_code code);
/** lval_mut_err mutates v to LVAL_ERR type.
 ** *err is copied. */
bool lval_mut_err(struct lval* v, const struct lerr* err);
/** lval_mut_err_ptr mutates v to LVAL_ERR type.
 ** *err is NOT copied. */
bool lval_mut_err_ptr(struct lval* v, struct lerr* err);
/** lval_mut_str mutates v to LVAL_STR type. str is copied. */
bool lval_mut_str(struct lval* v, const char* str);
/** lval_mut_sym mutates v to LVAL_SYM type. sym is copied */
bool lval_mut_sym(struct lval* v, const char* const sym);
/** lval_mut_func mutates v to LVAL_FUNC type. */
bool lval_mut_func(struct lval* v, const struct lfunc* func);
/** lval_mut_sexpr mutates v to LVAL_SEXPR type. */
bool lval_mut_sexpr(struct lval* v);
/** lval_mut_qexpr mutates v to LVAL_QEXPR type. */
bool lval_mut_qexpr(struct lval* v);
/** lval_mut_as mutates dest to the same type as src. */
bool lval_mut_as(struct lval* dest, const struct lval* src);
/** lval_cons add cell at the beginning of v.
 ** cell is safe to be freed by the caller after. */
bool lval_cons(struct lval* v, const struct lval* cell);
/** lval_push add cell to v. v must be of type sexpr or qexpr.
 ** cell is safe to be freed by the caller after. */
bool lval_push(struct lval* v, const struct lval* cell);
/** lval_pop remove cell c from v and returns it.
 ** Caller is responsible for calling free on returned value. */
struct lval* lval_pop(struct lval* v, size_t c);
/** lval_drop pops cell c from v then discards it. */
bool lval_drop(struct lval* v, size_t c);
/** lval_index returns the c-th child of a {s,q}expr in dest. */
bool lval_index(const struct lval* v, size_t c, struct lval* dest);
/** lval_index_ptr returns a pointer to the c-th element of v. v must be a qexpr.
 ** The pointer stays valid until v is freed or mutated. */
const struct lval* lval_index_ptr(const struct lval* v, size_t c);
/** lval_alloc_range allocates a list of len len in dest. */
bool lval_alloc_range(struct lval* dest, size_t len);
/** lval_copy_range copies a range from src to dest. */
bool lval_copy_range(
        struct lval* dest, size_t dfirst,
        const struct lval* src, size_t sfirst, size_t slast);
/** lval_reverse reverses a list or a string. */
bool lval_reverse(struct lval* dest, const struct lval* src);
/** lval_swap swaps two elements of v. */
bool lval_swap(struct lval* v, size_t i, size_t j);
/** lval_sort sorts v using the standard C qsort function. */
bool lval_sort(struct lval* v);
/** lval_len returns the length of an {s,q}expr. */
size_t lval_len(const struct lval* v);

/* Accessors */
/** lval_type returns the type of v. */
enum ltype lval_type(const struct lval* v);
/** lval_type_string returns the type of v as string. */
const char* lval_type_string(enum ltype type);
/** lval_type returns v as an error code. Its type must be LVAL_ERR. */
bool lval_as_err_code(const struct lval* v, enum lerr_code* r);
/** lval_type returns v as an lerr. Its type must be LVAL_ERR.
 ** The address pointer to the underlying error is returned.
 ** r stays valid until v is freed or mutated. */
struct lerr* lval_as_err(const struct lval* v);
/** lval_as_bool returns v as bool. */
bool lval_as_bool(const struct lval* v);
/** lval_type returns v as a long. Its type must be LVAL_NUM. */
bool lval_as_num(const struct lval* v, long* r);
/** lval_type returns v as a bignum. Its type must be LVAL_BIGNUM.
 ** r must be init by the caller. Caller is responsible for calling mpz_clear.
 ** r is a copy of the data. */
bool lval_as_bignum(const struct lval* v, mpz_t r);
/** lval_type returns v as a double. Its type must be LVAL_DBL. */
bool lval_as_dbl(const struct lval* v, double* r);
/** lval_type returns v as a string.
 ** v must be of type LVAL_STR.
 ** The pointed value is NOT a copy of the string.
 ** The pointer stays valid while v is alive. */
const char* lval_as_str(const struct lval* v);
/** lval_as_sym returns v as a string. Its type must be LVAL_SYM.
 ** The pointed value is NOT a copy of the symbol.
 ** The pointer stays valid while v is alive. */
const char* lval_as_sym(const struct lval* v);
/** lval_as_func returns v as a lbuiltin pointer.
 ** The pointed value is NOT a copy of the symbol.
 ** The pointer stays valid while v is alive. */
struct lfunc* lval_as_func(const struct lval* v);

/* Inquiries */
/** lval_is_nil returns true if v is nil. */
bool lval_is_nil(const struct lval* v);
/** lval_is_numeric returns true if v is of a numeric type.
 ** Numeric types are LVAL_NUM, LVAL_BIGNUM, LVAL_DBL. */
bool lval_is_numeric(const struct lval* v);
/** lval_is_zero returns true if v is zero for either of the numeric types. */
bool lval_is_zero(const struct lval* v);
/** lval_sign returns {-1, 0, 1} based on the sign of v. */
int lval_sign(const struct lval* v);
/** lval_is_list returns true is v is of type LVAL_SEXPR or LVAL_QEXPR. */
bool lval_is_list(const struct lval* v);
/** lval_are_equal returns true if x and y data are equal. */
bool lval_are_equal(const struct lval* x, const struct lval* y);
/** lval_compare compares x to y.
 ** lval_compare returns:
 **   <0 if x < y
 **   =0 if x == y
 **   >0 if x > y */
int lval_compare(const struct lval* x, const struct lval* y);

/* Printer */
/** lval_debug_print_to prints debug infos of v to out (FILE*). */
void lval_debug_print_to(const struct lval* v, FILE* out);
void lval_debug_print_all_to(const struct lval* v, FILE* out);
/** lval_debug_print prints debug infos of v to stdout. */
#define lval_debug_print(v) lval_debug_print_to(v, stdout)
#define lval_debug_print_all(v) lval_debug_print_all_to(v, stdout)
/** lval_print_to prints v to out (FILE*). */
void lval_print_to(const struct lval* v, FILE* out);
/** lval_print prints v to stdout. */
#define lval_print(v) lval_print_to(v, stdout)
#define lval_println(v) (lval_print_to(v, stdout), fputc('\n', stdout))

#endif
