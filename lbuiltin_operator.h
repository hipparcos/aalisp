#ifndef _H_BUILTIN_OPERATOR_
#define _H_BUILTIN_OPERATOR_

#include <stdbool.h>

#include "vendor/mini-gmp/mini-gmp.h"

#include "lval.h"

int lbi_cond_is_not_zero(const struct lval* x);
int lbi_cond_is_positive(const struct lval* x);
int lbi_cond_is_integral(const struct lval* x);
int lbi_cond_is_numeric(const struct lval* x);
int lbi_cond_x_is_ul(const struct lval* x);

bool lbi_cond_num_add_overflow(const long a, const long b);
bool lbi_cond_num_sub_overflow(const long a, const long b);
bool lbi_cond_num_mul_overflow(const long a, const long b);
bool lbi_cond_num_pow_overflow(const long a, const long b);
bool lbi_cond_num_fac_overflow(const long a, const long b);

long lbi_op_num_add(const long a, const long b);
long lbi_op_num_sub(const long a, const long b);
long lbi_op_num_sub_unary(const long a, const long b);
long lbi_op_num_mul(const long a, const long b);
long lbi_op_num_div(const long a, const long b);
long lbi_op_num_mod(const long a, const long b);
long lbi_op_num_pow(const long a, const long b);
long lbi_op_num_fac(const long a, const long b);

double lbi_op_dbl_nop(const double a, const double b);
double lbi_op_dbl_add(const double a, const double b);
double lbi_op_dbl_sub(const double a, const double b);
double lbi_op_dbl_sub_unary(const double a, const double b);
double lbi_op_dbl_mul(const double a, const double b);
double lbi_op_dbl_div(const double a, const double b);
double lbi_op_dbl_pow(const double a, const double b);

void lbi_op_bignum_sub_unary(mpz_t r, const mpz_t a, const mpz_t b);
void lbi_op_bignum_fac(mpz_t r, const mpz_t a, const mpz_t b);
void lbi_op_bignum_pow(mpz_t r, const mpz_t a, const mpz_t b);

#endif
