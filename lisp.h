#ifndef _H_LISP_
#define _H_LISP_

#include <stdbool.h>

struct lval;

bool lisp_eval(const char* restrict input, struct lval* r);
void lisp_eval_from_string(const char* restrict input);

#endif
