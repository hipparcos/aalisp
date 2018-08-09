#ifndef _H_LEVAL_
#define _H_LEVAL_

#include <stdbool.h>

#include "lval.h"

bool lisp_eval(const char* restrict input, struct lval* r, int prompt_len);
void lisp_eval_from_string(const char* restrict input, int prompt_len);

#endif
