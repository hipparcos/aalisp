#ifndef _H_LENV_
#define _H_LENV_

/** lenv associates a symbol descriptor with a string.
 ** A valid lenv ends with a {NULL, NULL} row. */
struct lenv {
    const char*        symbol;
    const struct lsym* descriptor;
};

/** lenv_lookup returns the lsym associated to sym. */
const struct lsym* lenv_lookup(const struct lenv* table, const char* sym);

#endif
