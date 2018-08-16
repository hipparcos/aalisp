#ifndef _H_LEVAL_
#define _H_LEVAL_

#include <stdbool.h>

#include "lval.h"
#include "lenv.h"

/** lisp_eval evaluates input and put result into r. */
bool lisp_eval(struct lenv* env,
        const char* restrict input, struct lval* r, int prompt_len);
/** lisp_eval_from_string evaluates input and prints result to stdout. */
void lisp_eval_from_string(struct lenv* env,
        const char* restrict input, int prompt_len);

/** leval evaluates v into r */
bool leval(struct lenv* env, const struct lval* v, struct lval* r);

#endif
