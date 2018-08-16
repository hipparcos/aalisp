#include "lval.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifdef OPTIM
#define INLINE inline
#else
#define INLINE
#endif

const char* const ltype_string[] = {
    "nil",
    "num",
    "bignum",
    "double",
    "error",
    "string",
    "symbol",
    "sexpr",
    "qexpr",
};
const char* const lerr_string[] = {
    "unknown error",
    "dead reference",
    "ast error",
    "evaluation error",
    "division by zero",
    "bad symbol",
    "bad operand",
    "too many arguments",
    "too few arguments",
};

/** ldata is the return type of an evalution. */
struct ldata {
    /** ldata.alive is a code used to detect mutation.
     ** This ldata is dead if set to 0. */
    int alive;
    /** refc is the number of lval referencing this ldata. */
    int refc;
    /** ldata.mutable tells if the ldata is mutable, associated lval can't be modified. */
    bool mutable;
    /** ldata.type to use the union. */
    enum ltype type;
    /** ldata.len value:
     ** LVAL_NIL = 0;
     ** LVAL_NUM, LVAL_BIGNUM, LVAL_DBL, LVAL_ERR = 1;
     ** LVAL_STR, LVAL_SYM = strlen(str);
     ** LVAL_SEXPR, LVAL_QEXPR = number of elements. */
    size_t len;
    /** ldata.payload must be considered according to ldata.type */
    union {
        long          num;
        mpz_t         bignum; // int > LONG_MAX.
        double        dbl;
        enum lerr     err;    // error code.
        char*         str;    // string or symbol.
        struct lval** cell;   // list of lval (can detect data mutation).
    } payload;
};

/* ldata.alive special status. */
#define DEAD      0
#define IMMORTAL -1

static INLINE bool lval_is_alive(const struct lval* v) {
    return v && v->alive != DEAD && v->alive == v->data->alive;
}

static INLINE bool lval_is_mutable(const struct lval* v) {
    return lval_is_alive(v) && v->data->mutable;
}

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
 static const struct ldata ldata_emptyq = {
    .alive        = IMMORTAL,
    .mutable      = false,
    .type         = LVAL_QEXPR,
    .len          = 0,
    .payload.cell = NULL
};
const struct lval lemptyq = {
    .alive = IMMORTAL,
    .data  = (struct ldata*) &ldata_emptyq
};
/** ldata_init is used as init data for lval.
 ** It is mutable but a new data is always allocated because refc starts at 1.*/
static struct ldata ldata_init = {
    .alive       = IMMORTAL,
    .mutable     = true,
    .refc        = 1, /* Here's the trick: is mutable but refc starts at 1. */
    .type        = LVAL_NIL,
    .len         = 0,
    .payload.num = 0
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
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        for (size_t c = 0; c < d->len; c++) {
            lval_free(d->payload.cell[c]);
        }
        free(d->payload.cell);
        d->payload.cell = NULL;
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
    struct ldata* data = calloc(1, sizeof(struct ldata));
    data->type = LVAL_NIL;
    data->mutable = true;
    ldata_clear(data);
    return data;
}

/** lval_connect connects a v to d.
 ** v is connected to lnil (immutable) by default. */
static void lval_connect(struct lval* v, struct ldata* d) {
    if (!d) {
        d = (struct ldata*) &ldata_init;
    }
    if (v->data == d) {
        v->alive = d->alive; // Keep alive updated.
        return;
    }
    v->data  = d;
    v->alive = d->alive;
    v->data->refc++;
}

static void lval_kill(struct lval* v);

/** lval_disconnect disconnects v from its ldata.
 ** v->data is reclaimed if refc = 0. */
static struct ldata* lval_disconnect(struct lval* v, bool reuse) {
    if (!v->data->mutable) {
        return NULL;
    }
    if (v->data->refc > 0) {
        v->data->refc--;
    }
    bool dead = v->data->refc == 0;
    struct ldata* data = v->data;
    if (reuse) {
        if (dead) {
            /* Reuse data. */
            ldata_clear(data);
        } else {
            /* Can't reuse ldata. */
            data = ldata_alloc();
        }
    } else {
        if (dead) {
            ldata_clear(data);
            if (data->alive != IMMORTAL) {
                free(data);
            }
        }
        data = NULL;
    }
    /* Kill handle. */
    v->data = NULL;
    lval_kill(v);
    return data;
}

/** lval_kill set v to DEAD REF. */
static void lval_kill(struct lval* v) {
    if (v->data) {
        lval_disconnect(v, false);
    }
    v->alive = DEAD;
    v->data = NULL;
    v->ast = NULL;
}

struct lval* lval_alloc(void) {
    struct lval* v = calloc(1, sizeof(struct lval));
    /* Don't alloc data yet, let mutation functions do it. */
    lval_connect(v, &ldata_init);
    v->ast = NULL;
    return v;
}

static struct lval* lval_alloc_handle(void) {
    struct lval* v = calloc(1, sizeof(struct lval));
    lval_kill(v);
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

bool lval_dup(struct lval* dest, const struct lval* src) {
    if (!dest || !lval_is_alive(src)) {
        return false;
    }
    if (dest == src) {
        return false;
    }
    lval_disconnect(dest, false);
    lval_connect(dest, src->data);
    dest->ast = src->ast;
    return true;
}

bool ldata_copy(struct ldata* dest, const struct ldata* src) {
    if (!dest->mutable) {
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
    case LVAL_QEXPR:
        dest->payload.cell = calloc(src->len, sizeof(struct lval*));
        for (size_t c = 0; c < src->len; c++) {
            struct lval* val = lval_alloc_handle();
            lval_connect(val, src->payload.cell[c]->data);
            dest->payload.cell[c] = val;
        }
        break;
    case LVAL_BIGNUM:
        mpz_init_set(dest->payload.bignum, src->payload.bignum);
        break;
    case LVAL_STR:
    case LVAL_SYM:
        dest->payload.str = calloc(src->len + 1, sizeof(char));
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
    if (!lval_clear(dest)) {
        return false;
    }
    if (!ldata_copy(dest->data, src->data)) {
        lval_connect(dest, dest->data);
        return false;
    }
    lval_connect(dest, dest->data);
    dest->ast = src->ast;
    return true;
}

bool lval_mut_nil(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    lval_clear(v); /* Default to nil, disconnect & connect. */
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
    if(!(data->payload.str = calloc(len + 1, sizeof(char)))) {
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
    return true;
}

bool lval_mut_sexpr(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_SEXPR;
    data->len = 0;
    lval_connect(v, data);
    return true;
}

bool lval_mut_qexpr(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_QEXPR;
    data->len = 0;
    lval_connect(v, data);
    return true;
}

bool lval_cons(struct lval* v, const struct lval* cell) {
    if (!lval_is_list(v) || !lval_is_alive(cell)) {
        return false;
    }
    /* Create a new handle. */
    struct lval* handle = lval_alloc_handle();
    lval_connect(handle, cell->data);
    handle->ast = cell->ast;
    /* Add it to the list. */
    v->data->payload.cell = realloc(v->data->payload.cell,
            sizeof(struct lval*) * (v->data->len+1));
    memmove(&v->data->payload.cell[1], &v->data->payload.cell[0],
            sizeof(struct lval*) * (v->data->len));
    v->data->payload.cell[0] = handle;
    v->data->len++;
    return true;
}

bool lval_push(struct lval* v, const struct lval* cell) {
    if (!lval_is_list(v) || !lval_is_alive(cell)) {
        return false;
    }
    /* Create a new handle. */
    struct lval* handle = lval_alloc_handle();
    lval_connect(handle, cell->data);
    handle->ast = cell->ast;
    /* Add it to the list. */
    v->data->payload.cell = realloc(v->data->payload.cell,
            sizeof(struct lval*) * (v->data->len+1));
    v->data->payload.cell[v->data->len] = handle;
    v->data->len++;
    return true;
}

struct lval* lval_pop(struct lval* v, size_t c) {
    if (!lval_is_list(v)) {
        return false;
    }
    if (c >= v->data->len) {
        return NULL;
    }
    struct lval* val = v->data->payload.cell[c];
    memmove(&v->data->payload.cell[c], &v->data->payload.cell[c+1],
            sizeof(struct lval*) * (v->data->len-1 - c));
    v->data->payload.cell = realloc(v->data->payload.cell,
            sizeof(struct lval*) * (v->data->len-1));
    v->data->len--;
    return val;
}

bool lval_index(const struct lval* v, size_t c, struct lval* dest) {
    if (!lval_is_list(v)) {
        return false;
    }
    if (!lval_is_mutable(dest)) {
        return false;
    }
    if (c >= v->data->len) {
        return false;
    }
    lval_disconnect(dest, false);
    struct lval* e = v->data->payload.cell[c];
    lval_connect(dest, e->data);
    dest->ast = e->ast;
    return true;
}

size_t lval_len(const struct lval* v) {
    if (!lval_is_list(v)) {
        return 0;
    }
    return v->data->len;
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

static size_t lval_list_as_str(
        const struct lval* v, char* out, size_t len,
        char opening, char closing) {
    if (!lval_is_list(v)) {
        return 0;
    }
    if (len < 3) {
        return 0;
    }
    char* s = out;
    *s++ = opening;
    struct ldata* data = v->data;
    for (size_t c = 0; c < data->len; c++) {
        if (c > 0) *s++ = ' ';
        size_t clen = lval_printlen(data->payload.cell[c]);
        if (clen > len-2) clen = len;
        lval_as_str(data->payload.cell[c], s, clen);
        s += clen-1; // - '\0'.
        len -= clen;
        if (len < 2) {
            break;
        }
    }
    *s++ = closing;
    *s   = '\0';
    return strlen(s);
}

bool lval_as_str(const struct lval* v, char* r, size_t len) {
    if (!lval_is_alive(v)) {
        r = NULL;
        return false;
    }
    switch (v->data->type) {
    case LVAL_NIL:
        snprintf(r, len, "nil");
        break;
    case LVAL_NUM:
        snprintf(r, len, "%li", v->data->payload.num);
        break;
    case LVAL_BIGNUM:
        mpz_get_str(r, 10, v->data->payload.bignum);
        break;
    case LVAL_DBL:
        snprintf(r, len, "%g", v->data->payload.dbl);
        break;
    case LVAL_SYM:
    case LVAL_STR:
        strncpy(r, v->data->payload.str,
                (len > v->data->len) ? v->data->len : len);
        break;
    case LVAL_QEXPR:
        lval_list_as_str(v, r, len, '{', '}');
        break;
    case LVAL_SEXPR:
        lval_list_as_str(v, r, len, '(', ')');
        break;
    case LVAL_ERR:
        {
        size_t idx = v->data->payload.err % sizeof(lerr_string);
        const char* format = lerr_string[idx];
        snprintf(r, len, "%s", format);
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
            v->data->type == LVAL_NIL ||
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
    default:          return 0;
    };
}

bool lval_is_list(const struct lval* v) {
    return lval_type(v) == LVAL_SEXPR || lval_type(v) == LVAL_QEXPR;
}

bool lval_are_equal(const struct lval* x, const struct lval* y) {
    if (!lval_is_alive(x))              return false;
    if (!lval_is_alive(y))              return false;
    if (x->data == y->data)             return true;
    if (x->data->type != y->data->type) return false;
    if (x->data->len != y->data->len)   return false;
    static const double epsilon = 0.000001;
    switch (x->data->type) {
    case LVAL_NIL:    return x->data->type == y->data->type;
    case LVAL_ERR:    return x->data->payload.err == y->data->payload.err;
    case LVAL_NUM:    return x->data->payload.num == y->data->payload.num;
    case LVAL_BIGNUM: return mpz_cmp(x->data->payload.bignum, y->data->payload.bignum) == 0;
    case LVAL_DBL:    return fabs(x->data->payload.dbl - y->data->payload.dbl) < epsilon;
    case LVAL_SYM:
    case LVAL_STR:    return strcmp(x->data->payload.str, y->data->payload.str) == 0;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        for (size_t c = 0; c < x->data->len; c++) {
            if (!lval_are_equal(x->data->payload.cell[c], y->data->payload.cell[c])) {
                return false;
            }
        }
        return true;
    default: return false;
    }
}

static size_t length_of_long(long l) {
    if (l == 0) {
        return 1;
    }
    size_t len = 0;
    len = (size_t)floor(log10(labs(l))) + 1;
    if (l < 0) {
        len++;
    }
    return len;
}

size_t lval_printlen(const struct lval* v) {
    if (!lval_is_alive(v)) {
        return 0;
    }
    size_t len = 0;
    switch (v->data->type) {
    case LVAL_NIL:    len = 3;   break;
    case LVAL_ERR:
        len = strlen(lerr_string[v->data->payload.err]);
        break;
    case LVAL_NUM:
        len = length_of_long(v->data->payload.num);
        break;
    case LVAL_BIGNUM: len = mpz_sizeinbase(v->data->payload.bignum, 10); break;
    case LVAL_DBL:
        {
        char dummy[2]; // minimum output size is 2 to suppress compiler warning.
        len = snprintf(dummy, sizeof(dummy), "%g", v->data->payload.dbl);
        }
        break;
    case LVAL_SYM:
    case LVAL_STR:    len = v->data->len; break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        len = 1 + 1; // ( )
        if (v->data->len > 0) {
            len += v->data->len - 1; // ' '.
        }
        for (size_t c = 0; c < v->data->len; c++) {
            len += lval_printlen(v->data->payload.cell[c]);
            len--; // - '\0'.
        }
        break;
    }
    return len + 1; // + '\0'
}

static size_t _lval_printlen_debug(const struct lval* v, bool recursive, int indent) {
    size_t len = 256 + lval_printlen(v);
    if (recursive && lval_type(v) == LVAL_SEXPR) {
        for (size_t c = 0; c < v->data->len; c++) {
            len += 8 * indent;
            len += _lval_printlen_debug(v->data->payload.cell[c], recursive, --indent);
        }
    }
    return len;
}
size_t lval_printlen_debug(const struct lval* v, bool recursive) {
    return _lval_printlen_debug(v, recursive, 0);
}

#define INDENT(out, indent) \
    do { int i = indent; while (i-- > 0) { *out++ = ' '; *out++ = ' '; } } while (0);

static void _lval_debug(const struct lval* v, char* out, bool recursive, int indent) {
    if (lval_is_alive(v)) {
        size_t lenpl = lval_printlen(v);
        char* payload = NULL;
        if (lenpl > 0) {
            payload = calloc(lenpl, sizeof(char));
            lval_as_str(v, payload, lenpl);
        }
        INDENT(out, indent);
        sprintf(out,
                "lval{data: %p, alive: 0x%x}->\n",
                v->data, v->alive);
        out += strlen(out);
        INDENT(out, indent);
        sprintf(out,
                "  ldata{type: %s, len: %ld, alive: 0x%x, refc: %d, mutable: %s,\n",
                ltype_string[v->data->type], v->data->len, v->data->alive, v->data->refc,
                v->data->mutable ? "true" : "false");
        out += strlen(out);
        INDENT(out, indent);
        sprintf(out,
                "    payload as string: '%s'\n",
                payload);
        out += strlen(out);
        INDENT(out, indent);
        sprintf(out, "  }\n");
        out += strlen(out);
        if (payload) {
            free(payload);
        }
        if (recursive && lval_type(v) == LVAL_SEXPR) {
            for (size_t c = 0; c < v->data->len; c++) {
                size_t len = strlen(out);
                _lval_debug(v->data->payload.cell[c], out+len, recursive, indent+1);
            }
        }
    } else {
        sprintf(out,
                "lval{data: %p, alive: 0x%x} DEAD REF\n",
                v->data, v->alive);
    }
}

void lval_debug(const struct lval* v, char* out, bool recursive) {
    _lval_debug(v, out, recursive, 0);
}

void lval_debug_print_to(const struct lval* v, FILE* out) {
    size_t len = lval_printlen_debug(v, true);
    if (len == 0) {
        return;
    }
    char* str = calloc(len, sizeof(char));
    lval_debug(v, str, true);
    fputs(str, out);
    free(str);
}

void lval_print_to(const struct lval* v, FILE* out) {
    size_t len = lval_printlen(v);
    if (len == 0) {
        return;
    }
    char* str = calloc(len, sizeof(char));
    lval_as_str(v, str, len);
    fputs(str, out);
    free(str);
}
