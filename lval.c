#include "lval.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

const char* const ltype_string[] = {
    "nil",
    "num",
    "bignum",
    "double",
    "error",
    "string",
    "symbol",
    "sexpr",
};
const char* const lerr_string[7] = {
    "unknown error",
    "dead reference",
    "evaluation error",
    "division by zero",
    "bad symbol",
    "bad operand",
    "too many arguments",
};

/** ldata is the return type of an evalution. */
struct ldata {
    /** ldata.alive is a code used to detect mutation.
     ** This ldata is dead if set to 0. */
    int alive;
    /** refc is the number of lval referencing this ldata. */
    int refc;
    /** ldata.mutable tells if the ldata is mutable. */
    bool mutable;
    /** ldata.type to use the union. */
    enum ltype type;
    /** ldata.len value:
     ** LVAL_NIL = 0;
     ** LVAL_NUM, LVAL_BIGNUM, LVAL_DBL, LVAL_ERR, LVAL_SYM = 1;
     ** LVAL_STR = strlen(str);
     ** LVAL_SEXPR = number of elements. */
    size_t len;
    /** ldata.payload must be considered according to ldata.type */
    union {
        long           num;
        mpz_t          bignum; // int > LONG_MAX.
        double         dbl;
        enum lerr      err;    // error code.
        char*          str;    // string or symbol.
        struct ldata** cell;   // list.
    } payload;
};

/** lval is the public handle to a ldata.
 ** This level of indirection is used to prepare the work on a GC. */
struct lval {
    /** lval.alive is a code used to detect aliveness of the payload. */
    int alive;
    /** lval.data is a pointer to the actual ldata. */
    struct ldata* data;
};

/* ldata.alive special status. */
#define DEAD      0
#define IMMORTAL -1 /** and immutable. */

static inline bool lval_is_alive(const struct lval* v) {
    return v && v->alive != DEAD && v->alive == v->data->alive;
}
static bool lval_is_alive(const struct lval*);

static inline bool lval_is_immortal(const struct lval* v) {
    return lval_is_alive(v) && v->alive == IMMORTAL;
}
static bool lval_is_immortal(const struct lval*);

static inline bool lval_is_mutable(const struct lval* v) {
    return lval_is_alive(v) && !lval_is_immortal(v) && v->data->mutable;
}
static bool lval_is_mutable(const struct lval*);

/* Special lval definitions. */
static const struct ldata ldata_nil = {
    .alive       = IMMORTAL,
    .mutable     = false,
    .type        = LVAL_NIL,
    .len         = 0,
    .payload.num = 0
};
const struct lval lnil = {
    .alive = IMMORTAL,
    .data  = (struct ldata*) &ldata_nil
};
static const struct ldata ldata_zero = {
    .alive       = IMMORTAL,
    .mutable     = false,
    .type        = LVAL_NUM,
    .len         = 1,
    .payload.num = 0
};
const struct lval lzero = {
    .alive = IMMORTAL,
    .data  = (struct ldata*) &ldata_zero
};
 static const struct ldata ldata_one = {
    .alive       = IMMORTAL,
    .mutable     = false,
    .type        = LVAL_NUM,
    .len         = 1,
    .payload.num = 1
};
const struct lval lone = {
    .alive = IMMORTAL,
    .data  = (struct ldata*) &ldata_one
};

/** lvalp_unique returns a hopefully unique number. */
static int lval_unique() {
    static int last = 0;
    return ++last; /* Does the trick for now. */
}

/** ldata_clear clears the internal memory of d.
 ** d is set to nil. */
static bool ldata_clear(struct ldata* d) {
    if (!d || !d->mutable) {
        return false;
    }
    switch (d->type) {
    case LVAL_BIGNUM:
        mpz_clear(d->payload.bignum);
        break;
    case LVAL_STR:
    case LVAL_SYM:
        if (d->payload.str) {
            free(d->payload.str);
            d->payload.str = NULL;
        }
        break;
    default: break;
    }
    d->alive       = lval_unique();
    d->mutable     = true;
    d->refc        = 0;
    d->type        = LVAL_NIL;
    d->len         = 0;
    d->payload.num = 0;
    return true;
}

/** ldata_alloc allocates a new ldata. Initialized to LVAL_NIL. */
static struct ldata* ldata_alloc(void) {
    /* Heap allocation for the moment.
     * Allocation from a ldata pool later.
     * Memory is set to 0. */
    struct ldata* data = calloc(sizeof(struct ldata), 1);
    data->mutable = true;
    ldata_clear(data);
    return data;
}

/** lval_connect connects a v to d.
 ** v is connected to lnil (immutable) by default. */
static void lval_connect(struct lval* v, struct ldata* d) {
    if (!d) {
        d = (struct ldata*) &ldata_nil;
    }
    v->data  = d;
    v->alive = v->data->alive;
    v->data->refc++;
}

/** lval_disconnect disconnects v from its ldata.
 ** v->data is reclaimed if refc = 0. */
static struct ldata* lval_disconnect(struct lval* v, bool reuse) {
    if (!v->data->mutable) {
        lval_connect(v, (struct ldata*)&lnil);
        return NULL;
    }
    if (v->data->refc > 0) {
        v->data->refc--;
    }
    if (reuse) {
        /* Can't reuse ldata. */
        if (v->data->refc > 0) {
            struct ldata* data = ldata_alloc();
            return data;
        } else {
            ldata_clear(v->data);
            return v->data;
        }
    }
    ldata_clear(v->data);
    free(v->data);
    return NULL;
}

struct lval* lval_alloc(void) {
    struct ldata* data = ldata_alloc();
    struct lval* v = calloc(sizeof(struct lval), 1);
    lval_connect(v, data);
    return v;
}

bool lval_free(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    lval_disconnect(v, false);
    free(v);
    return true;
}

bool lval_clear(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    lval_connect(v, data);
    return true;
}

bool ldata_copy(struct ldata* dest, const struct ldata* src) {
    if (!ldata_clear(dest)) {
        return false;
    }
    switch (src->type) {
    case LVAL_NIL:
        break;
    case LVAL_NUM:
        dest->payload.num = src->payload.num;
        break;
    case LVAL_DBL:
        dest->payload.dbl = src->payload.dbl;
        break;
    case LVAL_ERR:
        dest->payload.err = src->payload.err;
        break;
    case LVAL_SEXPR:
        dest->payload.cell = src->payload.cell;
        break;
    case LVAL_BIGNUM:
        mpz_init_set(dest->payload.bignum, src->payload.bignum);
        break;
    case LVAL_STR:
    case LVAL_SYM:
        dest->payload.str = calloc(sizeof(char), src->len + 1);
        if (!strncpy(dest->payload.str, src->payload.str, src->len)) {
            free(dest->payload.str);
        }
        break;
    }
    dest->type = src->type;
    dest->len = src->len;
    return true;
}

bool lval_copy(struct lval* dest, const struct lval* src) {
    if (!lval_is_alive(src)) {
        return false;
    }
    if (!lval_is_mutable(dest)) {
        return false;
    }
    if (!lval_is_nil(dest)) {
        return false;
    }
    if (!ldata_copy(dest->data, src->data)) {
        lval_connect(dest, dest->data);
        return false;
    }
    lval_connect(dest, dest->data);
    return true;
}

bool lval_mut_nil(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    lval_clear(v); /* Default to nil. */
    return true;
}

bool lval_mut_num(struct lval* v, long x) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_NUM;
    data->payload.num = x;
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_bignum(struct lval* v, const mpz_t x) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_BIGNUM;
    mpz_init_set(data->payload.bignum, x);
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_dbl(struct lval* v, double x) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_DBL;
    data->payload.dbl = x;
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_err(struct lval* v, enum lerr e) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_ERR;
    data->payload.err = e;
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_str(struct lval* v, const char* const str) {
    if (!lval_is_mutable(v) || !str) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    size_t len = strlen(str);
    if(!(data->payload.str = calloc(sizeof(char), len + 1))) {
        free(data);
        return false;
    }
    if (!strncpy(data->payload.str, str, len)) {
        free(data->payload.str);
        free(data);
        return false;
    }
    data->type = LVAL_STR;
    data->len = len;
    lval_connect(v, data);
    return true;
}

bool lval_mut_sym(struct lval* v, const char* const sym) {
    if (!lval_mut_str(v, sym)) {
        return false;
    }
    v->data->type = LVAL_SYM;
    v->data->len = 1;
    return true;
}

bool lval_mut_sexpr(struct lval* v, const struct lval** cells, int cellc) {
    (void)(cells);
    (void)(cellc);
    if (!lval_is_mutable(v)) {
        return false;
    }
    ldata_clear(v->data);
    // TODO: implement list assignment.
    lval_connect(v, v->data);
    return false;
}

enum ltype lval_type(const struct lval* v) {
    if (!lval_is_alive(v)) {
        return LVAL_NIL;
    }
    return v->data->type;
}

bool lval_as_err(const struct lval* v, enum lerr* r) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_ERR) {
        *r = LERR_DEAD_REF;
        return false;
    }
    *r = v->data->payload.err;
    return true;
}

bool lval_as_num(const struct lval* v, long* r) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_NUM) {
        *r = 0;
        return false;
    }
    *r = v->data->payload.num;
    return true;
}

bool lval_as_bignum(const struct lval* v, mpz_t r) {
    if (!lval_is_alive(v) || !lval_is_numeric(v)) {
        mpz_set_ui(r, 0);
        return false;
    }
    /* Automatic casting */
    switch (v->data->type) {
    case LVAL_DBL: mpz_set_d(r, v->data->payload.dbl); break;
    case LVAL_NUM: mpz_set_si(r, v->data->payload.num); break;
    case LVAL_BIGNUM: mpz_set(r, v->data->payload.bignum); break;
    default: break;
    }
    return true;
}

bool lval_as_dbl(const struct lval* v, double* r) {
    if (!lval_is_alive(v) || !lval_is_numeric(v)) {
        *r = .0;
        return false;
    }
    /* Automatic casting */
    switch (v->data->type) {
    case LVAL_DBL: *r = v->data->payload.dbl; break;
    case LVAL_NUM: *r = (double) v->data->payload.num; break;
    /* No warranty. */
    case LVAL_BIGNUM: *r = mpz_get_d(v->data->payload.bignum); break;
    default: break;
    }
    return true;
}

bool lval_as_str(const struct lval* v, char* r, size_t len) {
    if (!lval_is_alive(v)) {
        r = NULL;
        return false;
    }
    switch (v->data->type) {
    case LVAL_NIL:
        sprintf(r, "nil");
        break;
    case LVAL_NUM:
        sprintf(r, "%li", v->data->payload.num);
        break;
    case LVAL_BIGNUM:
        mpz_get_str(r, 10, v->data->payload.bignum);
        break;
    case LVAL_DBL:
        sprintf(r, "%g", v->data->payload.dbl);
        break;
    case LVAL_SYM:
    case LVAL_STR:
        strncpy(r, v->data->payload.str,
                (len > v->data->len) ? v->data->len : len);
        break;
    case LVAL_SEXPR:
        // TODO: implement sexpr printing.
        sprintf(r, "<sexpr>");
        break;
    case LVAL_ERR:
        {
        size_t idx = v->data->payload.err % sizeof(lerr_string);
        const char* format = lerr_string[idx];
        sprintf(r, "%s", format);
        }
        break;
    }
    return true;
}

const char* lval_as_sym(const struct lval* v) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_SYM) {
        return NULL;
    }
    return v->data->payload.str;
}

bool lval_is_nil(const struct lval* v) {
    return !lval_is_alive(v) || v->data->type == LVAL_NIL;
}

bool lval_is_numeric(const struct lval* v) {
    return lval_is_alive(v) && (
            v->data->type == LVAL_NUM ||
            v->data->type == LVAL_BIGNUM ||
            v->data->type == LVAL_DBL
            );
}

bool lval_is_zero(const struct lval* v) {
    if (!lval_is_numeric(v)) {
        return false;
    }
    static mpz_t zero;
    mpz_init_set_si(zero, 0);
    switch (v->data->type) {
    case LVAL_NUM:    return v->data->payload.num == 0;
    case LVAL_DBL:    return fpclassify(v->data->payload.dbl) == FP_ZERO;
    case LVAL_BIGNUM: return mpz_cmp(v->data->payload.bignum, zero) == 0;
    default:          return false;
    };
}

int lval_sign(const struct lval* v) {
    if (!lval_is_numeric(v)) {
        return 0;
    }
    switch (v->data->type) {
    case LVAL_NUM:    return (v->data->payload.num > 0) - (v->data->payload.num < 0);
    case LVAL_DBL:    return (v->data->payload.dbl > .0) - (v->data->payload.dbl < .0);
    case LVAL_BIGNUM: return mpz_sgn(v->data->payload.bignum);
    default:          return false;
    };
}

bool lval_are_equal(const struct lval* x, const struct lval* y) {
    if (!lval_is_alive(x))              return false;
    if (!lval_is_alive(y))              return false;
    if (x->data == y->data)             return true;
    if (x->data->type != y->data->type) return false;
    static const double epsilon = 0.000001;
    switch (x->data->type) {
    case LVAL_NIL:    return x->data->type == y->data->type;
    case LVAL_ERR:    return x->data->payload.err == y->data->payload.err;
    case LVAL_NUM:    return x->data->payload.num == y->data->payload.num;
    case LVAL_BIGNUM: return mpz_cmp(x->data->payload.bignum, y->data->payload.bignum) == 0;
    case LVAL_DBL:    return fabs(x->data->payload.dbl - y->data->payload.dbl) < epsilon;
    case LVAL_SYM:
    case LVAL_STR:    return strcmp(x->data->payload.str, y->data->payload.str) == 0;
    case LVAL_SEXPR:  return false; // TODO: implement lval_are_equal for SEXPR.
    default: return false;
    }
}

size_t lval_printlen(const struct lval* v) {
    if (!lval_is_alive(v)) {
        return 0;
    }
    size_t len = 0;
    switch (v->data->type) {
    case LVAL_NIL:    len = 3;   break;
    case LVAL_ERR:    len = 128; break; // TODO: implement lval_printlen for LVAL_ERR.
    case LVAL_NUM:    len = 128; break; // TODO: implement lval_printlen for LVAL_NUM.
    case LVAL_BIGNUM: len = mpz_sizeinbase(v->data->payload.bignum, 10); break;
    case LVAL_DBL:    len = 128; break; // TODO: implement lval_printlen for LVAL_DBL.
    case LVAL_SYM:
    case LVAL_STR:    len = v->data->len; break;
    case LVAL_SEXPR:  len = 7; break; // TODO: implement lval_printlen for SEXPR.
    }
    return len + 1; // + '\0'
}

size_t lval_printlen_debug(const struct lval* v) {
    /* header length < 256 */
    return 256 + lval_printlen(v);
}

void lval_debug(const struct lval* v, char* out) {
    if (lval_is_alive(v)) {
        size_t lenpl = lval_printlen(v);
        char* payload = NULL;
        if (lenpl > 0) {
            payload = calloc(sizeof(char), lenpl);
            lval_as_str(v, payload, lenpl);
        }
        sprintf(out,
                "lval{data: %p, alive: 0x%x}->" \
                "ldata{type: %s, len: %ld, alive: 0x%x, refc: %d, mutable: %s, payload: '%s'}",
                v->data, v->alive,
                ltype_string[v->data->type], v->data->len, v->data->alive, v->data->refc,
                v->data->mutable ? "true" : "false", payload);
        if (payload) {
            free(payload);
        }
    } else {
        sprintf(out,
                "lval{data: %p, alive: 0x%x} DEAD REF",
                v->data, v->alive);
    }
}

void lval_debug_print_to(const struct lval* v, FILE* out) {
    size_t len = lval_printlen_debug(v);
    if (len == 0) {
        return;
    }
    char* str = calloc(sizeof(char), len);
    lval_debug(v, str);
    fputs(str, out);
    free(str);
}

void lval_print_to(const struct lval* v, FILE* out) {
    size_t len = lval_printlen(v);
    if (len == 0) {
        return;
    }
    char* str = calloc(sizeof(char), len);
    lval_as_str(v, str, len);
    fputs(str, out);
    free(str);
}
