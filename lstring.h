#ifndef H_LSTRING_
#define H_LSTRING_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/** A string. */
struct lstr {
    /** lstr.len is the length in bytes of lstr.content. */
    size_t len;
    /** lstr.cap is the capacity in bytes of lstr.content. */
    size_t cap;
    /** lstr.content is the string. */
    char content[]; // C99 flexible array member, must be last.
};

/** lstr creates a new lstr by copying the content of src.
 ** Caller is responsible for calling lstr_free().
 ** src is a C standard \0 terminated string. */
struct lstr* lstr(const char* src);
/** lstrn create a new lstr of capacity & length len.
 ** Caller is responsible for calling lstr_free().
 ** src must be of size len or superior to avoid invalid read. */
struct lstr* lstrn(const char* src, size_t len);
/** lbuf creates a new lstr of capacity cap and len 0.
 ** lbuf is used to create buffer to concatenante strings using lstr_cat().
 ** Caller is responsible for calling lstr_free(). */
struct lstr* lbuf(size_t cap);
/** lstr_free frees s.
 ** s must not be used afterwaeds. */
void lstr_free(struct lstr* s);

/** lstr_cat concatenates str at the end of buf.
 ** buf capacity should be large enough to accomodate str (see lbuf()). */
struct lstr* lstr_cat(struct lstr* buf, const struct lstr* str);
/** lstr_cat_cstr concatenantes the '\0' terminated string str. to buf */
struct lstr* lstr_cat_cstr(struct lstr* buf, const char* str);
/** lstr_cat_char concatenates c at the end of buf. */
struct lstr* lstr_cat_char(struct lstr* buf, uint32_t c);
/** lstr_cmp compares two lstr.
 ** lstr_cmp returns:
 **   -1 if left>right
 **    0 if left=right
 **   +1 if left<right */
int lstr_cmp(const struct lstr* left, const struct lstr* right);

/** lstr_print_to prints s to the file out. */
void lstr_print_to(const struct lstr* s, FILE* out);
/** lstr_print prints s to stdout. */
#define lstr_print(s) lstr_print_to(s, stdout)

#endif
