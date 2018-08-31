#ifndef H_LVAL_
#define H_LVAL_

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "vendor/mini-gmp/mini-gmp.h"

#include "lerr.h"

/* Forward declaration of lfunc, see lfunc.h */
struct lfunc;

/** lval is the public handle to a value. */
typedef uint64_t lval_t;
const uint64_t DEAD_REF = 0L;

/** ltype is the type of a lval. */
enum ltype {      // Underlying data is of type:
    LVAL_NIL = 0,
    LVAL_BOOL,    // bool
    LVAL_NUM,     // long
    LVAL_BIGNUM,  // mpz_t
    LVAL_DBL,     // double
    LVAL_ERR,     // lerr*
    LVAL_STR,     // char*
    LVAL_SYM,     // char*
    LVAL_FUNC,    // lfunc*
    LVAL_SEXPR,   // lval_t**
    LVAL_QEXPR,   // lval_t**
};

/* Memory management */
/** lval_alloc returns a handle to a new lval of type LVAL_NIL.
 ** It is safe to pass a lval_t by copy.
 ** Caller is responsible for calling lval_free. */
lval_t lval_alloc(void);
/** lval_free reclaims v internal memory.
 ** v is set to DEAD_REF. */
bool lval_free(lval_t* v);
/** lval_alloc_range allocates a list of length len in v.
 ** v must be a list (LVAL_SEXPR, LVAL_QEXPR or LVAL_STR).
 ** If v is DEAD_REF, v is allocated.
 ** v might be relocated. */
bool lval_alloc_range(lval_t* v, size_t len);

/* Mutators */
/** lval_copy copies src into dest.
 ** If dest == src, nothing happens.
 ** dest might be relocated. */
bool lval_copy(lval_t* dest, lval_t src);
/** lval_copy_range copies a range (from sfirst to but excluding) from src to
 ** dest (starting from dfirst).
 ** dest might be relocated. */
bool lval_copy_range(lval_t* dest, size_t dfirst,
        lval_t src, size_t sfirst, size_t slast);
/** lval_mut_nil mutates v to LVAL_NIL type.
 ** v might be relocated. */
bool lval_mut_nil(lval_t* v);
/** lval_mut_bool mutates v to LVAL_BOOL type.
 ** v might be relocated. */
bool lval_mut_bool(lval_t* v, bool x);
/** lval_mut_num mutates v to LVAL_NUM type.
 ** v might be relocated. */
bool lval_mut_num(lval_t* v, long x);
/** lval_mut_bignum mutates v to LVAL_BIGNUM type.
 ** x is copied.
 ** v might be relocated. */
bool lval_mut_bignum(lval_t* v, const mpz_t x);
/** lval_mut_dbl mutates v to LVAL_DBL type.
 ** v might be relocated. */
bool lval_mut_dbl(lval_t* v, double x);
/** lval_mut_err_code mutates v to LVAL_ERR type.
 ** v might be relocated. */
bool lval_mut_err_code(lval_t* v, enum lerr_code code);
/** lval_mut_err mutates v to LVAL_ERR type.
 ** err is copied.
 ** v might be relocated. */
bool lval_mut_err(lval_t* v, const struct lerr* err);
/** lval_mut_err_ptr mutates v to LVAL_ERR type.
 ** err is NOT copied.
 ** v might be relocated. */
bool lval_mut_err_ptr(lval_t* v, struct lerr* err);
/** lval_mut_str mutates v to LVAL_STR type.
 ** str is copied.
 ** v might be relocated. */
bool lval_mut_str(lval_t* v, const char* str);
/** lval_mut_sym mutates v to LVAL_SYM type.
 ** sym is copied
 ** v might be relocated. */
bool lval_mut_sym(lval_t* v, const char* const sym);
/** lval_mut_func mutates v to LVAL_FUNC type.
 ** fun is copied.
 ** v might be relocated. */
bool lval_mut_func(lval_t* v, const struct lfunc* fun);
/** lval_mut_sexpr mutates v to LVAL_SEXPR type.
 ** v might be relocated. */
bool lval_mut_sexpr(lval_t* v);
/** lval_mut_qexpr mutates v to LVAL_QEXPR type.
 ** v might be relocated. */
bool lval_mut_qexpr(lval_t* v);
/** lval_mut_as mutates dest to the same type as src.
 ** If dest == src, nothing happens.
 ** v might be relocated. */
bool lval_mut_as(lval_t* dest, lval_t src);

/* List manipulators */
/** lval_cons adds cell at the beginning of v. v must be a list.
 ** cell is safe to be freed by the caller after.
 ** v might be relocated. */
bool lval_cons(lval_t* v, lval_t cell);
/** lval_push adds cell at the end of v. v must be a list.
 ** cell is safe to be freed by the caller after.
 ** v might be relocated. */
bool lval_push(lval_t* v, lval_t cell);
/** lval_pop removes cell c from v and returns it. v must be a list.
 ** Caller is responsible for calling free on the returned value.
 ** v might be relocated. */
lval_t lval_pop(lval_t* v, size_t c);
/** lval_drop pops cell c from v then discards it. v must be a list.
 ** v might be relocated. */
bool lval_drop(lval_t* v, size_t c);
/** lval_reverse reverses v. v must be a list.
 ** If src != DEAD_REF, src is reversed into v (single pass).
 ** v might be relocated. */
bool lval_reverse(lval_t* v, lval_t src);
/** lval_swap swaps two elements of v.
 ** v might be relocated. */
bool lval_swap(lval_t* v, size_t i, size_t j);
/** lval_sort sorts v using the standard C qsort function.
 ** v might be relocated. */
bool lval_sort(lval_t* v);

/* Accessors */
/** lval_type returns v as an error code. v must be of type LVAL_ERR. */
bool lval_as_err_code(lval_t v, enum lerr_code* r);
/** lval_type returns v as an lerr. v must be of type LVAL_ERR.
 ** The pointer to the underlying error is returned.
 ** The returned value stays valid while v is alive. */
struct lerr* lval_as_err(lval_t v);
/** lval_as_bool returns true only when v is of type LVAL_BOOL and is true. */
bool lval_as_bool(lval_t v);
/** lval_type returns v as a long. v must be of type LVAL_NUM. */
bool lval_as_num(lval_t v, long* r);
/** lval_type returns v as a bignum. v must be of type LVAL_BIGNUM.
 ** r must be init by the caller. Caller is responsible for calling mpz_clear.
 ** r is a copy of the data. */
bool lval_as_bignum(lval_t v, mpz_t r);
/** lval_type returns v as a double. v must be of type LVAL_DBL. */
bool lval_as_dbl(lval_t v, double* r);
/** lval_type returns v as a string. v must be of type LVAL_STR.
 ** The pointed value is NOT a copy of the string.
 ** The pointer stays valid while v is alive. */
const char* lval_as_str(lval_t v);
/** lval_as_sym returns v as a string. v must be of type LVAL_SYM.
 ** The pointed value is NOT a copy of the symbol.
 ** The pointer stays valid while v is alive. */
const char* lval_as_sym(lval_t v);
/** lval_as_func returns v as a lbuiltin pointer.
 ** The pointed value is NOT a copy of the symbol.
 ** The pointer stays valid while v is alive. */
struct lfunc* lval_as_func(lval_t v);
/** lval_index returns the c-th child of v. v must be a list.
 ** Caller is responsible for calling free on the returned value. */
lval_t lval_index(lval_t v, size_t c);
/** lval_len returns the length of v. */
size_t lval_len(lval_t v);

/* Queriers */
/** lval_type returns the type of v. */
enum ltype lval_type(lval_t v);
/** lval_type_string returns the type as string. */
const char* lval_type_string(enum ltype type);
/** lval_is_nil tells if v is nil.
 ** lval_is_nil returns true when v == DEAD_REF. */
bool lval_is_nil(lval_t v);
/** lval_is_numeric tells if v is of types: LVAL_NUM, LVAL_BIGNUM, LVAL_DBL. */
bool lval_is_numeric(lval_t v);
/** lval_is_zero tells if v is zero for either of the numeric types. */
bool lval_is_zero(lval_t v);
/** lval_sign returns {-1, 0, 1} based on the sign of v.
 ** lval_sign returns 0 for non-numeric types. */
int lval_sign(lval_t v);
/** lval_is_list tells if v is of types: LVAL_SEXPR, LVAL_QEXPR or LVAL_STR. */
bool lval_is_list(lval_t v);
/** lval_are_equal tells if x and y data are equal.
 ** lval of distinct types are never equal, they must be casted beforehand. */
bool lval_are_equal(lval_t x, lval_t y);
/** lval_compare compares x to y:
 **   <0 if x < y
 **   =0 if x == y
 **   >0 if x > y */
int lval_compare(lval_t x, lval_t y);

/* Debugging */
/** lval_debug_print_to prints debug infos of v to out. all means recursively. */
void lval_debug_print_to(lval_t v, FILE* out);
void lval_debug_print_all_to(lval_t v, FILE* out);
/** lval_debug_print prints debug infos of v to stdout. all means recursively. */
#define lval_debug_print(v) lval_debug_print_to(v, stdout)
#define lval_debug_print_all(v) lval_debug_print_all_to(v, stdout)
/** lval_print_to prints v to out. */
void lval_print_to(lval_t v, FILE* out);
/** lval_print prints v to stdout. */
#define lval_print(v) lval_print_to(v, stdout)
#define lval_println(v) (lval_print_to(v, stdout), fputc('\n', stdout))

#endif
