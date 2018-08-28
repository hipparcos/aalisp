#ifndef _H_LEVAL_
#define _H_LEVAL_

#include <stdbool.h>

#include "lval.h"
#include "lenv.h"
#include "lerr.h"

/** lisp_eval_from_string evaluates input and prints result to stdout. */
struct lerr* lisp_eval_from_string(struct lenv* env, const char* restrict input);
/** leval_from_string evaluates the content of input and puts result into r. */
struct lerr* leval_from_string(struct lenv* env, const char* restrict input, struct lval* r);
/** lisp_eval_from_file evaluates the content of input and prints result to stdout. */
struct lerr* lisp_eval_from_file(struct lenv* env, FILE* input);
/** leval_from_file evaluates the content of input and puts result into r. */
struct lerr* leval_from_file(struct lenv* env, FILE* input, struct lval* r);

/** leval evaluates v into r.
 ** env  is the global environment;
 ** v    is the program to execute;
 ** r    is the result. */
bool leval(struct lenv* env, const struct lval* v, struct lval* r);

#endif
