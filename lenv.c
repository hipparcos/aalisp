#include "lenv.h"

#include <string.h>

#include "lsym.h"

const struct lsym* lenv_lookup(const struct lenv* symbols, const char* sym) {
    if (!symbols) {
        return NULL;
    }
    const struct lenv* row = &symbols[0];
    do {
        if (strcmp(sym, row->symbol) == 0) {
            return row->descriptor;
        }
    } while ((++row)->symbol);
    return NULL;
}
