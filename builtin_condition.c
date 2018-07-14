#include "builtin_condition.h"

/* Conditions (inline see .h) */
bool cnd_y_is_zero(struct lval x, struct lval y);
bool cnd_either_is_dbl(struct lval x, struct lval y);
bool cnd_either_is_bignum(struct lval x, struct lval y);
bool cnd_dbl_and_bignum(struct lval x, struct lval y);
bool cnd_are_num(struct lval x, struct lval y);
bool _is_neg(struct lval x);
bool cnd_x_is_neg(struct lval x, struct lval y);
bool cnd_y_is_neg(struct lval x, struct lval y);
bool _too_big_for_ul(struct lval x);
bool cnd_x_too_big_for_ul(struct lval x, struct lval y);
bool cnd_y_too_big_for_ul(struct lval x, struct lval y);
