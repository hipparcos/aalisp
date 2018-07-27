#ifndef _H_LEVAL_
#define _H_LEVAL_

#include <stdbool.h>

struct lval;

bool lisp_eval(const char* restrict input, struct lval* r);
void lisp_eval_from_string(const char* restrict input);

#endif
