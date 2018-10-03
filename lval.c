#include "lval.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lfunc.h"
#include "generic/mempool.h"

/** ldata is a dialecte value.
 ** A ldata initialized to {0} is a valid nil value. */
struct ldata {
    /** ldata.immutable tells if the ldata is immutable.
     ** If so, associated lval can't be modified. */
    bool immutable;
    /** ldata.refc is the number of references to this ldata. */
    int refc;
    /** ldata.type is the type of this ldata (to use the union). */
    enum ltype type;
    /** ldata.len value:
     ** LVAL_NIL = 0;
     ** LVAL_BOOL, LVAL_NUM, LVAL_BIGNUM, LVAL_DBL, LVAL_FUNC, LVAL_ERR = 1;
     ** LVAL_STR, LVAL_SYM = strlen(str);
     ** LVAL_SEXPR, LVAL_QEXPR = number of elements. */
    size_t len;
    /** The following fields are the payload. Depends on ldata.type. */
    union {
        bool          bol;
        long          num;
        mpz_t         bgn; // integer > LONG_MAX.
        double        dbl;
        char*         str; // string or symbol.
        lval_t*       val; // list of lval.
        struct lfunc* fun;
        struct lerr*  err;
    }; // Anonymous union, require C11.
};

/** ldata_cluster is the ldata memory pool instance. */
static struct mp_cluster* ldata_cluster;
/** ldata_per_cluster is the number of ldata per pool. */
static const size_t ldata_per_pool = 0xFFFF;

/** ldata_get returns a pointer to the ldata pointed by handle or NULL.
 ** ldata_get is an unsafe way to get/modify a ldata.
 ** The returned pointer must not escape from the caller scope.
 ** The returned pointer stays valid until handle is freed. */
static struct ldata* ldata_get(lval_t handle) {
    assert(ldata_cluster);
    return (struct ldata*)mp_get(ldata_cluster, handle);
}

/** ldata_put is a safe way to update the ldata pointed by handle.
 ** The content of data is copied to the ldata designated by handle. */
static bool ldata_put(lval_t handle, const struct ldata* data) {
    assert(ldata_cluster);
    static const struct ldata nil = {0};
    if (!data) {
        data = &nil;
    }
    return mp_put(ldata_cluster, handle, data);
}

/** ldata_clear clears the internal memory of d.
 ** d is set to nil.
 ** ldata_clear fails if d is immutable. */
static bool ldata_clear(struct ldata* d) {
    assert(d && !d->immutable);
    switch (d->type) {
    case LVAL_BIGNUM:
        mpz_clear(d->bgn);
        break;
    case LVAL_STR:
    case LVAL_SYM:
        if (d->str) { // d->str is NULL for 0-length strings.
            free(d->str);
            d->str = NULL;
        }
        break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        for (size_t c = 0; c < d->len; c++) {
            lval_free(&d->val[c]);
        }
        free(d->val);
        d->val = NULL;
        break;
    case LVAL_FUNC:
        lfunc_free(d->fun);
        d->fun = NULL;
        break;
    case LVAL_ERR:
        lerr_free(d->err);
        d->err = NULL;
        break;
    default: break;
    }
    d->immutable = false;
    d->refc    = 0;
    d->type    = LVAL_NIL;
    d->len     = 0;
    d->num     = 0;
    return true;
}

/** ldata_reference increments refc of the ldata designated by handle. */
static bool ldata_reference(lval_t handle) {
    struct ldata* data = ldata_get(handle);
    if (!data) {
        return false;
    }
    return ++data->refc;
}

/** ldata_dereference decrements refc of the ldata designated by handle.
 ** If refc == 0, the ldata is freed.
 ** handle is set to DEAD_REF. */
static bool ldata_dereference(lval_t* handle) {
    assert(ldata_cluster);
    struct ldata* data = ldata_get(*handle);
    if (!data) {
        return false;
    }
    if (--data->refc == 0) {
        ldata_clear(data);
        mp_free(ldata_cluster, *handle);
        /* Destroy cluster if it's the last lval. */
        if (mp_cluster_is_empty(ldata_cluster)) {
            mp_cluster_free(&ldata_cluster);
        }
    }
    *handle = DEAD_REF;
    return true;
}

/** ldata_copy copies src into dest.
 ** ldata_copy fails if dest is NULL or immutable. */
bool ldata_copy(struct ldata* dest, const struct ldata* src) {
    assert(dest && dest->immutable);
    switch (src->type) {
    case LVAL_NIL:
        break;
    case LVAL_BOOL:
        dest->bol = src->bol;
        break;
    case LVAL_NUM:
        dest->num = src->num;
        break;
    case LVAL_DBL:
        dest->dbl = src->dbl;
        break;
    case LVAL_ERR:
        dest->err = lerr_alloc();
        lerr_copy(dest->err, src->err);
        break;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        dest->val = calloc(src->len, sizeof(lval_t));
        for (size_t c = 0; c < src->len; c++) {
            dest->val[c] = src->val[c];
            ldata_reference(dest->val[c]);
        }
        break;
    case LVAL_BIGNUM:
        mpz_init_set(dest->bgn, src->bgn);
        break;
    case LVAL_STR:
    case LVAL_SYM:
        dest->payload.str = malloc(src->len+1);
        if (!strncpy(dest->payload.str, src->payload.str, src->len+1)) {
            free(dest->payload.str);
        }
        src->payload.str[src->len] = '\0';
        break;
    case LVAL_FUNC:
        dest->fun = lfunc_alloc();
        lfunc_copy(dest->fun, src->fun);
        break;
    }
    dest->type = src->type;
    dest->len = src->len;
    return true;
}

static bool lval_is_mutable(lval_t handle) {
    struct ldata* data = ldata_get(handle);
    return data && !data->immutable;
}

lval_t lval_alloc(void) {
    if (!ldata_cluster) {
        ldata_cluster = mp_cluster_alloc(
                mp_pool_alloc(ldata_per_pool, sizeof(struct ldata)));
    }
    lval_t handle;
    if (!mp_alloc(ldata_cluster, &handle)) {
        return DEAD_REF;
    }
    ldata_put(handle, NULL);
    ldata_reference(handle);
    return handle;
}

bool lval_free(lval_t* v) {
    return ldata_dereference(v);
}

bool lval_clear(lval_t* v) {
    if (!lval_is_mutable(*v)) {
        return false;
    }
    ldata_dereference(v);
    *v = lval_alloc();
    return *v != DEAD_REF;
}

static void lval_ensure_data_ownership(lval_t* v) {
    assert(v && *v != DEAD_REF);
    struct ldata* data = ldata_get(*v);
    if (data && data->refc > 1) {
        ldata_dereference(v);
        *v = lval_alloc();
        ldata_copy(ldata_get(*v), data);
    }
}

bool lval_copy(lval_t* dest, lval_t src) {
    assert(dest && *dest != DEAD_REF);

}

bool lval_mut_nil(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    lval_clear(v); /* Default to nil, disconnect & connect. */
    return true;
}

bool lval_mut_bool(struct lval* v, bool x) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_BOOL;
    data->payload.boolean = x;
    data->len = 1;
    lval_connect(v, data);
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

bool lval_mut_err_code(struct lval* v, enum lerr_code code) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_ERR;
    struct lerr* err = lerr_alloc();
    err->code = code;
    data->payload.err = err;
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_err(struct lval* v, const struct lerr* err) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_ERR;
    struct lerr* errp = lerr_alloc();
    lerr_copy(errp, err);
    data->payload.err = errp;
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_err_ptr(struct lval* v, struct lerr* errp) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_ERR;
    data->payload.err = errp;
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
    if(!(data->payload.str = malloc(len+1))) {
        free(data);
        return false;
    }
    if (!strncpy(data->payload.str, str, len+1)) {
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

bool lval_mut_func(struct lval* v, const struct lfunc* func) {
    if (!lval_is_mutable(v) || !func) {
        return false;
    }
    struct ldata* data = NULL;
    if (!(data = lval_disconnect(v, true))) {
        return false;
    }
    data->type = LVAL_FUNC;
    data->payload.func = lfunc_alloc();
    lfunc_copy(data->payload.func, func);
    data->len = 1;
    lval_connect(v, data);
    return true;
}

bool lval_mut_sexpr(struct lval* v) {
    if (!lval_is_mutable(v)) {
        return false;
    }
    if (lval_type(v) == LVAL_SEXPR) {
        return true;
    }
    if (lval_type(v) == LVAL_QEXPR && v->data->refc == 1) {
        v->data->type = LVAL_SEXPR;
        return true;
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
    if (lval_type(v) == LVAL_QEXPR) {
        return true;
    }
    if (lval_type(v) == LVAL_SEXPR && v->data->refc == 1) {
        v->data->type = LVAL_QEXPR;
        return true;
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

bool lval_cons(struct lval* v, const struct lval* c) {
    if (!lval_is_list(v) || !lval_is_alive(c)) {
        return false;
    }
    /* Duplicate if multiple lval reference this data. */
    lval_ensure_data_ownership(v);
    /* Special case for strings. */
    if (v->data->type == LVAL_STR) {
        if (c->data->type != LVAL_STR) {
            return false;
        }
        size_t len_v = v->data->len;
        size_t len_c = c->data->len;
        char** payload = &v->data->payload.str;
        *payload = realloc(*payload, len_v+len_c+1);
        memmove((*payload)+len_c, *payload, len_v);
        memcpy(*payload, c->data->payload.str, len_c);
        v->data->len += len_c;
        *((*payload)+v->data->len) = '\0';
        return true;
    }
    /* Create a new handle. */
    struct lval* handle = lval_alloc_handle();
    lval_connect(handle, c->data);
    handle->ast = c->ast;
    /* Add it to the list. */
    v->data->payload.cell = realloc(v->data->payload.cell,
            sizeof(struct lval*) * (v->data->len+1));
    memmove(&v->data->payload.cell[1], &v->data->payload.cell[0],
            sizeof(struct lval*) * (v->data->len));
    v->data->payload.cell[0] = handle;
    v->data->len++;
    return true;
}

bool lval_push(struct lval* v, const struct lval* c) {
    if (!lval_is_list(v) || !lval_is_alive(c)) {
        return false;
    }
    /* Duplicate if multiple lval reference this data. */
    lval_ensure_data_ownership(v);
    /* Special case for strings. */
    if (v->data->type == LVAL_STR) {
        if (c->data->type != LVAL_STR) {
            return false;
        }
        size_t len_v = v->data->len;
        size_t len_c = c->data->len;
        char** payload = &v->data->payload.str;
        *payload = realloc(*payload, len_v+len_c+1);
        memcpy((*payload)+len_v, c->data->payload.str, len_c);
        v->data->len += len_c;
        *((*payload)+v->data->len) = '\0';
        return true;
    }
    /* Create a new handle. */
    struct lval* handle = lval_alloc_handle();
    lval_connect(handle, c->data);
    handle->ast = c->ast;
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
    /* Duplicate if multiple lval reference this data. */
    lval_ensure_data_ownership(v);
    /* Special case for strings. */
    if (v->data->type == LVAL_STR) {
        size_t len = v->data->len;
        char** payload = &v->data->payload.str;
        char popped = *(*payload + c);
        memmove(*payload+c, *payload+c+1, len-1 - c);
        if (len > 1) {
            v->data->len--;
            *payload = realloc(*payload, len);
            *((*payload)+len-1) = '\0';
        } else {
            v->data->len = 0;
            free(*payload);
            *payload = NULL;
        }
        /* Create lval. */
        char str[2] = {0};
        str[0] = popped; str[1] = '\0';
        struct lval* val = lval_alloc();
        lval_mut_str(val, &str[0]);
        return val;
    }
    /* Pop the cell and return it. */
    struct lval* val = v->data->payload.cell[c];
    memmove(&v->data->payload.cell[c], &v->data->payload.cell[c+1],
            sizeof(struct lval*) * (v->data->len-1 - c));
    if (v->data->len > 1) {
        v->data->payload.cell = realloc(v->data->payload.cell,
                sizeof(struct lval*) * (v->data->len-1));
    } else {
        free(v->data->payload.cell);
        v->data->payload.cell = NULL;
    }
    v->data->len--;
    return val;
}

bool lval_drop(struct lval* v, size_t c) {
    if (!lval_is_list(v)) {
        return false;
    }
    /* Duplicate if multiple lval reference this data. */
    lval_ensure_data_ownership(v);
    /* Drop. */
    struct lval* discarded = lval_pop(v, c);
    lval_free(discarded);
    return discarded != NULL;
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
    /* Special case for strings. */
    if (v->data->type == LVAL_STR) {
        char str[2];
        str[0] = *(v->data->payload.str+c);
        str[1] = '\0';
        struct ldata* data = lval_disconnect(dest, true);
        lval_connect(dest, data);
        lval_mut_str(dest, &str[0]);
        return true;
    }
    lval_disconnect(dest, false);
    struct lval* e = v->data->payload.cell[c];
    lval_connect(dest, e->data);
    dest->ast = e->ast;
    return true;
}

const struct lval* lval_index_ptr(const struct lval* v, size_t c) {
    if (lval_type(v) != LVAL_QEXPR) {
        return NULL;
    }
    if (c >= v->data->len) {
        return NULL;
    }
    return v->data->payload.cell[c];
}

bool lval_mut_as(struct lval* dest, const struct lval* src) {
    if (!lval_is_alive(dest)) {
        return false;
    }
    switch (lval_type(src)) {
    case LVAL_NIL:
    case LVAL_BOOL:
    case LVAL_NUM:
    case LVAL_BIGNUM:
    case LVAL_DBL:
    case LVAL_SYM:
    case LVAL_FUNC:
    case LVAL_ERR:
        lval_copy(dest, src);
        return true;
    case LVAL_STR:
        lval_mut_str(dest, "");
        return true;
    case LVAL_SEXPR:
        lval_mut_sexpr(dest);
        return true;
    case LVAL_QEXPR:
        lval_mut_qexpr(dest);
        return true;
    default:
        return false;
    }
}

bool lval_alloc_range(struct lval* dest, size_t len) {
    if (!lval_is_alive(dest)) {
        return false;
    }
    switch (lval_type(dest)) {
    case LVAL_STR:
        if (dest->data->payload.str) {
            free(dest->data->payload.str);
        }
        dest->data->payload.str = calloc(len+1, 1); // + '\0'.
        dest->data->len = len;
        return true;
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        dest->data->payload.cell = malloc(len * sizeof(struct lval*));
        for (size_t c = 0; c < len; c++) {
            dest->data->payload.cell[c] = lval_alloc();
        }
        dest->data->len = len;
        return true;
    default:
        return false;
    }
}

bool lval_copy_range(
        struct lval* dest, size_t dfirst,
        const struct lval* src, size_t sfirst, size_t slast) {
    if (!lval_is_list(src) || !lval_is_list(dest)) {
        return false;
    }
    if (lval_type(src) != lval_type(dest)) {
        return false;
    }
    size_t len_src = lval_len(src);
    if (len_src == 0) {
        return true;
    }
    if (sfirst >= len_src || slast < sfirst) {
        return false;
    }
    if (slast > len_src) {
        slast = len_src;
    }
    size_t len_range = slast - sfirst;
    size_t len_dest = lval_len(dest);
    if (len_dest == 0) {
        lval_alloc_range(dest, len_range);
        len_dest = lval_len(dest);
    }
    if (dfirst >= len_dest) {
        return false;
    }
    size_t dlast = len_dest;
    if (len_dest - dfirst < len_range) {
        len_range = len_dest - dfirst;
    }
    if (lval_type(src) == LVAL_STR) {
        strncpy(dest->data->payload.str+dfirst, src->data->payload.str+sfirst, len_range);
        dest->data->payload.str[len_dest] = '\0';
        return true;
    }
    size_t d = dfirst;
    size_t s = sfirst;
    while (d < dlast && s < slast) {
        lval_dup(dest->data->payload.cell[d++], src->data->payload.cell[s++]);
    }
    return true;
}

bool lval_reverse(struct lval* dest, const struct lval* src) {
    if (!lval_is_list(src) || !lval_is_alive(dest)) {
        return false;
    }
    lval_mut_as(dest, src);
    size_t len = lval_len(src);
    if (len == 0) {
        return true;
    }
    lval_alloc_range(dest, len);
    if (lval_type(src) == LVAL_STR) {
        size_t s = len, d = 0;
        while (s > 0) {
            dest->data->payload.str[d++] = src->data->payload.str[--s];
        }
        return true;
    }
    size_t s = len, d = 0;
    while (s > 0) {
        lval_dup(dest->data->payload.cell[d++], src->data->payload.cell[--s]);
    }
    return true;
}

bool lval_swap(struct lval* v, size_t i, size_t j) {
    if (!lval_is_list(v)) {
        return false;
    }
    size_t len = v->data->len;
    if (i >= len) { i = len-1; }
    if (j >= len) { j = len-1; }
    if (i == j) {
        return true;
    }
    /* Duplicate if multiple lval reference this data. */
    lval_ensure_data_ownership(v);
    /* Swap. */
    if (v->data->type == LVAL_STR) {
        char tmp = v->data->payload.str[i];
        v->data->payload.str[i] = v->data->payload.str[j];
        v->data->payload.str[j] = tmp;
        return true;
    }
    struct lval* tmp = v->data->payload.cell[i];
    v->data->payload.cell[i] = v->data->payload.cell[j];
    v->data->payload.cell[j] = tmp;
    return true;
}

static int qsort_compare_char(const void* p1, const void* p2) {
    char c1 = *(char*)p1;
    char c2 = *(char*)p2;
    return (c1 > c2) - (c1 < c2);
}
static int qsort_compare_lval(const void* p1, const void* p2) {
    return lval_compare(*(const struct lval**)p1, *(const struct lval**)p2);
}

bool lval_sort(struct lval* v) {
    if (!lval_is_list(v)) {
        return false;
    }
    /* Duplicate if multiple lval reference this data. */
    lval_ensure_data_ownership(v);
    /* Sort. */
    if (lval_type(v) == LVAL_STR) {
        qsort(v->data->payload.str, v->data->len, 1, qsort_compare_char);
        return true;
    }
    qsort(&v->data->payload.cell[0], v->data->len, sizeof(struct lval*),
            qsort_compare_lval);
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

const char* lval_type_string(enum ltype type) {
    switch (type) {
    case LVAL_NIL:    return "nil";
    case LVAL_BOOL:   return "bool";
    case LVAL_NUM:    return "num";
    case LVAL_BIGNUM: return "bignum";
    case LVAL_DBL:    return "double";
    case LVAL_ERR:    return "error";
    case LVAL_STR:    return "string";
    case LVAL_SYM:    return "symbol";
    case LVAL_FUNC:   return "function";
    case LVAL_SEXPR:  return "sexpr";
    case LVAL_QEXPR:  return "qexpr";
    }
    return "";
}

bool lval_as_err_code(const struct lval* v, enum lerr_code* r) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_ERR) {
        *r = LERR_DEAD_REF;
        return false;
    }
    *r = v->data->payload.err->code;
    return true;
}

struct lerr* lval_as_err(const struct lval* v) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_ERR) {
        return NULL;
    }
    return v->data->payload.err;
}

bool lval_as_bool(const struct lval* v) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_BOOL) {
        return false;
    }
    return v->data->payload.boolean;
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

const char* lval_as_str(const struct lval* v) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_STR) {
        return NULL;
    }
    return v->data->payload.str;
}

const char* lval_as_sym(const struct lval* v) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_SYM) {
        return NULL;
    }
    return v->data->payload.str;
}

struct lfunc* lval_as_func(const struct lval* v) {
    if (!lval_is_alive(v) || lval_type(v) != LVAL_FUNC) {
        return NULL;
    }
    return v->data->payload.func;
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
    enum ltype type = lval_type(v);
    return type == LVAL_SEXPR || type == LVAL_QEXPR || type == LVAL_STR;
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
    case LVAL_BOOL:   return x->data->payload.boolean == y->data->payload.boolean;
    case LVAL_ERR:
        {
        struct lerr* cause_x = lerr_cause(x->data->payload.err);
        struct lerr* cause_y = lerr_cause(y->data->payload.err);
        return cause_x->code == cause_y->code;
        }
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
    case LVAL_FUNC:
        return lfunc_are_equal(x->data->payload.func, y->data->payload.func);
    default: return false;
    }
}

#define payload(x) (x->data->payload)
#define compare(x,y) ((x > y) - (x < y))
#define compare_payload(x,y,m) compare(payload(x).m, payload(y).m)

int lval_compare(const struct lval* x, const struct lval* y) {
    if (!lval_is_alive(x))              return -1;
    if (!lval_is_alive(y))              return -1;
    if (x->data == y->data)             return 0;
    if (x->data->type != y->data->type) return -1;
    if (x->data->len != y->data->len)   return -1;
    switch (x->data->type) {
    case LVAL_NIL:
        return 0;
    case LVAL_BOOL:
        return compare_payload(x, y, boolean);
    case LVAL_NUM:
        return compare_payload(x, y, num);
    case LVAL_DBL:
        return compare_payload(x, y, dbl);
    case LVAL_BIGNUM:
        return mpz_cmp(payload(x).bignum, payload(y).bignum);
    case LVAL_STR:
    case LVAL_SYM:
        return strcmp(payload(x).str, payload(y).str);
    case LVAL_FUNC:
    case LVAL_ERR:
        return lval_are_equal(x, y);
    case LVAL_SEXPR:
    case LVAL_QEXPR:
        if (lval_len(x) != lval_len(y)) {
            return compare(lval_len(x), lval_len(y));
        }
        for (size_t c = 0; c < x->data->len; c++) {
            int s = 0;
            if (0 != (s = lval_compare(x->data->payload.cell[c], y->data->payload.cell[c]))) {
                return s;
            }
        }
        return 0;
    }
    return -1;
}

#define INDENT(out, indent) \
    do { int i = indent; while (i-- > 0) { fputs("  ", out); } } while (0);

static void lval_debug(const struct lval* v, FILE* out, bool recursive, int indent) {
    if (!lval_is_alive(v)) {
        fprintf(out,
                "lval{data: %p, alive: 0x%x} DEAD REF\n",
                v->data, v->alive);
        return;
    }
    INDENT(out, indent);
    fprintf(out,
            "lval{data: %p, alive: 0x%x}->\n",
            v->data, v->alive);
    INDENT(out, indent);
    fprintf(out,
            "  ldata{type: %s, len: %ld, alive: 0x%x, refc: %d, mutable: %s,\n",
            lval_type_string(lval_type(v)), v->data->len, v->data->alive, v->data->refc,
            v->data->mutable ? "true" : "false");
    INDENT(out, indent);
    fputs("    payload as string: '", out);
    lval_print_to(v, out);
    fputs("'\n", out);
    INDENT(out, indent);
    fputs("  }\n", out);
    if (recursive && lval_type(v) == LVAL_SEXPR) {
        for (size_t c = 0; c < v->data->len; c++) {
            lval_debug(v->data->payload.cell[c], out, true, indent);
        }
    }
}

void lval_debug_print_to(const struct lval* v, FILE* out) {
    if (!v) {
        return;
    }
    lval_debug(v, out, false, 0);
}

void lval_debug_print_all_to(const struct lval* v, FILE* out) {
    if (!v) {
        return;
    }
    lval_debug(v, out, true, 0);
}

static void lval_print_list(const struct lval* v, FILE* out, char opening, char closing) {
    fputc(opening, out);
    struct ldata* data = v->data;
    for (size_t c = 0; c < data->len; c++) {
        if (c > 0) {
            fputc(' ', out);
        }
        lval_print_to(data->payload.cell[c], out);
    }
    fputc(closing, out);
}

void lval_print_to(const struct lval* v, FILE* out) {
    if (!lval_is_alive(v)) {
        return;
    }
    switch (v->data->type) {
    case LVAL_NIL:
        fprintf(out, "nil");
        break;
    case LVAL_BOOL:
        fprintf(out, (v->data->payload.boolean) ? "true" : "false");
        break;
    case LVAL_NUM:
        fprintf(out, "%li", v->data->payload.num);
        break;
    case LVAL_BIGNUM:
        {
        size_t len = mpz_sizeinbase(v->data->payload.bignum, 10);
        char* buffer = malloc(len+1);
        mpz_get_str(buffer, 10, v->data->payload.bignum);
        fputs(buffer, out);
        free(buffer);
        break;
        }
    case LVAL_DBL:
        fprintf(out, "%g", v->data->payload.dbl);
        break;
    case LVAL_SYM:
        fputs(v->data->payload.str, out);
        break;
    case LVAL_STR:
        fputc('"', out);
        if (v->data->payload.str) {
            fputs(v->data->payload.str, out);
        }
        fputc('"', out);
        break;
    case LVAL_QEXPR:
        lval_print_list(v, out, '{', '}');
        break;
    case LVAL_SEXPR:
        lval_print_list(v, out, '(', ')');
        break;
    case LVAL_ERR:
        lerr_print_cause_to(v->data->payload.err, out);
        break;
    case LVAL_FUNC:
        lfunc_print_to(v->data->payload.func, out);
        break;
    }
}
