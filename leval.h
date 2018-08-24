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
/** lisp_eval_from_file evaluates the content of input and prints result to stdout. */
void lisp_eval_from_file(struct lenv* env, FILE* input);

/** leval evaluates v into r.
 ** env  is the global environment;
 ** v    is the program to execute;
 ** r    is the result. */
bool leval(struct lenv* env, const struct lval* v, struct lval* r);
/** leval_from_file calls leval with the content of input as argument. */
bool leval_from_file(struct lenv* env, FILE* input, struct lval* r);

#endif
