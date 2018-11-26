#include "lstring.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <uchar.h>

#include "vendor/utf8proc/utf8proc.h"

struct lstr* lstr(const char* src) {
    size_t len = strlen(src);
    return lstrn(src, len);
}

struct lstr* lstrn(const char* src, size_t len) {
    struct lstr* s = lbuf(len);
    s->len = len;
    memcpy(s->content, src, len);
    s->content[len] = '\0';
    return s;
}

struct lstr* lbuf(size_t cap) {
    assert(cap > 0);
    struct lstr* s = malloc(sizeof(struct lstr) + cap + 1);
    s->len = 0;
    s->cap = cap;
    s->content[0] = '\0';
    return s;
}

void lstr_free(struct lstr* s) {
    if (!s) {
        return;
    }
    free(s);
}

struct lstr* lstr_cat(struct lstr* buf, const struct lstr* str) {
    if (!buf) {
        return NULL;
    }
    if (!str || str->len == 0) {
        return NULL;
    }
    if (str->len > buf->cap - buf->len) {
        return NULL;
    }
    memcpy(buf->content + buf->len, str->content, str->len);
    buf->len += str->len;
    buf->content[buf->len] = '\0';
    return buf;
}

struct lstr* lstr_cat_cstr(struct lstr* buf, const char* str) {
    if (!buf) {
        return NULL;
    }
    if (!str) {
        return NULL;
    }
    size_t len = strlen(str);
    if (len == 0) {
        return NULL;
    }
    if (len > buf->cap - buf->len) {
        return NULL;
    }
    memcpy(buf->content + buf->len, str, len);
    buf->len += len;
    buf->content[buf->len] = '\0';
    return buf;
}

struct lstr* lstr_cat_codepoint(struct lstr* buf, int32_t codepoint) {
    if (!buf) {
        return NULL;
    }
    if (buf->cap == buf->len) {
        return NULL;
    }
    // Check UTF-8 validity.
    if (!utf8proc_codepoint_valid(codepoint)) {
        return NULL;
    }
    // Encode as string.
    uint8_t s[5] = {0};
    utf8proc_encode_char(codepoint, &s[0]);
    // Concatenate to buffer.
    return lstr_cat_cstr(buf, (char*)s);
}

int lstr_cmp(const struct lstr* left, const struct lstr* right) {
    if (!left) {
        return 1;
    }
    if (!right) {
        return -1;
    }
    size_t minlen = (left->len < right->len) ? left->len : right->len;
    return memcmp(left->content, right->content, minlen);
}

void lstr_print_to(const struct lstr* s, FILE* out) {
    if (!s) {
        return;
    }
    for (size_t i = 0; i < s->len; i++) {
        fputc(s->content[i], out); // Allow internal '\0'.
    }
}
