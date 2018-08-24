#ifndef _H_LBUILTIN_CONDITION_
#define _H_LBUILTIN_CONDITION_

#include "lfunc.h"

/* Conditions on lfunc. */
define_condition(must_have_max_argc);
define_condition(must_have_min_argc);
define_condition(must_have_func_ptr);

/* Operators conditions. */
define_condition(must_be_numeric);
define_condition(must_be_integral);
define_condition(must_be_positive);
define_condition(divisor_must_be_non_zero);
define_condition(must_be_unsigned_long);

/* List conditions. */
define_condition(must_be_of_type);
define_condition(must_all_be_of_same_type);
define_condition(must_have_min_len);
define_condition(must_be_of_equal_len);
define_condition(must_be_list_of);
define_condition(must_be_a_list);

#endif
