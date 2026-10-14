/* ============================================================
 * bitmap.cpp
 * Implementation of the bitmap (bitset) data structure.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "bitmap/bitmap.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * Internal helpers
 * ============================================================ */

/* Clear trailing bits beyond n_bits in the last word. */
static void bm_clear_trailing(Bitmap* bm) {
    bm_size extra;

    extra = bm->n_words * BM_BITS_PER_WORD - bm->n_bits;
    if (extra > 0) {
        bm->words[bm->n_words - 1] &= ~0ULL >> extra;
    }
}

/* ============================================================
 * Error string
 * ============================================================ */

const char* bm_error_string(int err) {
    switch (err) {
        case BM_OK:           return "ok";
        case BM_ERR_NOMEM:    return "no memory";
        case BM_ERR_RANGE:    return "out of range";
        case BM_ERR_MISMATCH: return "size mismatch";
        case BM_ERR_INVALID:  return "not initialized";
    }
    return "unknown";
}

/* ============================================================
 * Lifecycle
 * ============================================================ */

int bm_init(Bitmap* bm, bm_size n_bits) {
    bm_size bytes;

    /* Empty bitmap: valid, but no storage. */
    if (n_bits == 0) {
        bm->words   = NULL;
        bm->n_bits  = 0;
        bm->n_words = 0;
        bm->count   = 0;
        return BM_OK;
    }

    bm->n_bits  = n_bits;
    bm->n_words = (n_bits + BM_BITS_PER_WORD - 1) / BM_BITS_PER_WORD;
    bm->count   = 0;

    bytes = bm->n_words * sizeof(u64);
    bm->words = (u64*) malloc(bytes);
    if (bm->words == NULL) {
        return BM_ERR_NOMEM;
    }
    memset(bm->words, 0, bytes);
    return BM_OK;
}

void bm_free(Bitmap* bm) {
    if (bm->words != NULL) {
        free(bm->words);
    }
    bm->words   = NULL;
    bm->n_bits  = 0;
    bm->n_words = 0;
    bm->count   = 0;
}

int bm_valid(const Bitmap* bm) {
    return bm != NULL && bm->words != NULL && bm->n_bits > 0;
}

/* ============================================================
 * Basic operations
 * ============================================================ */

int bm_set(Bitmap* bm, bm_size index) {
    bm_size word_idx;
    bm_size bit_idx;
    u64     mask;

    if (!bm_valid(bm))       return BM_ERR_INVALID;
    if (index >= bm->n_bits) return BM_ERR_RANGE;

    word_idx = index / BM_BITS_PER_WORD;
    bit_idx  = index % BM_BITS_PER_WORD;
    mask     = 1ULL << bit_idx;

    if ((bm->words[word_idx] & mask) == 0) {
        bm->words[word_idx] |= mask;
        bm->count++;
    }
    return BM_OK;
}

int bm_clear(Bitmap* bm, bm_size index) {
    bm_size word_idx;
    bm_size bit_idx;
    u64     mask;

    if (!bm_valid(bm))       return BM_ERR_INVALID;
    if (index >= bm->n_bits) return BM_ERR_RANGE;

    word_idx = index / BM_BITS_PER_WORD;
    bit_idx  = index % BM_BITS_PER_WORD;
    mask     = 1ULL << bit_idx;

    if ((bm->words[word_idx] & mask) != 0) {
        bm->words[word_idx] &= ~mask;
        bm->count--;
    }
    return BM_OK;
}

int bm_test(const Bitmap* bm, bm_size index) {
    bm_size word_idx;
    bm_size bit_idx;

    if (!bm_valid(bm))       return 0;
    if (index >= bm->n_bits) return 0;

    word_idx = index / BM_BITS_PER_WORD;
    bit_idx  = index % BM_BITS_PER_WORD;
    return (bm->words[word_idx] & (1ULL << bit_idx)) != 0;
}

int bm_toggle(Bitmap* bm, bm_size index) {
    bm_size word_idx;
    bm_size bit_idx;
    u64     mask;

    if (!bm_valid(bm))       return BM_ERR_INVALID;
    if (index >= bm->n_bits) return BM_ERR_RANGE;

    word_idx = index / BM_BITS_PER_WORD;
    bit_idx  = index % BM_BITS_PER_WORD;
    mask     = 1ULL << bit_idx;

    if (bm->words[word_idx] & mask) {
        bm->words[word_idx] &= ~mask;
        bm->count--;
    } else {
        bm->words[word_idx] |= mask;
        bm->count++;
    }
    return BM_OK;
}

bm_size bm_size_of(const Bitmap* bm) {
    return bm->n_bits;
}

bm_size bm_count(const Bitmap* bm) {
    return bm->count;
}

/* ============================================================
 * Mass operations
 * ============================================================ */

int bm_set_all(Bitmap* bm) {
    bm_size i;

    if (!bm_valid(bm)) return BM_ERR_INVALID;

    for (i = 0; i < bm->n_words; i++) {
        bm->words[i] = ~0ULL;
    }
    bm_clear_trailing(bm);
    bm->count = bm->n_bits;
    return BM_OK;
}

int bm_clear_all(Bitmap* bm) {
    bm_size i;

    if (!bm_valid(bm)) return BM_ERR_INVALID;

    for (i = 0; i < bm->n_words; i++) {
        bm->words[i] = 0;
    }
    bm->count = 0;
    return BM_OK;
}

void bm_print(const Bitmap* bm) {
    bm_size i;

    if (!bm_valid(bm)) {
        printf("(invalid bitmap)\n");
        return;
    }

    printf("bitmap: %lu bits, %lu words, %lu set\n",
           (unsigned long) bm->n_bits,
           (unsigned long) bm->n_words,
           (unsigned long) bm->count);

    for (i = 0; i < bm->n_words; i++) {
        printf("  word %2lu: 0x%016llX\n",
               (unsigned long) i,
               (unsigned long long) bm->words[i]);
    }
}

/* ============================================================
 * Search and count
 * ============================================================ */

bm_size bm_find_first_set(const Bitmap* bm) {
    bm_size w;
    int     bit;
    bm_size global;

    if (!bm_valid(bm)) return BM_NOT_FOUND;

    for (w = 0; w < bm->n_words; w++) {
        /* Skip empty words: 64 bits at a time. */
        if (bm->words[w] == 0) continue;

        bit = __builtin_ctzll(bm->words[w]);
        global = w * BM_BITS_PER_WORD + (bm_size) bit;

        if (global >= bm->n_bits) return BM_NOT_FOUND;
        return global;
    }
    return BM_NOT_FOUND;
}

bm_size bm_find_first_clear(const Bitmap* bm) {
    bm_size w;
    u64      inverted;
    int      bit;
    bm_size  global;

    if (!bm_valid(bm)) return BM_NOT_FOUND;

    for (w = 0; w < bm->n_words; w++) {
        inverted = ~bm->words[w];
        if (inverted == 0) continue;

        bit = __builtin_ctzll(inverted);
        global = w * BM_BITS_PER_WORD + (bm_size) bit;

        if (global >= bm->n_bits) return BM_NOT_FOUND;
        return global;
    }
    return BM_NOT_FOUND;
}

bm_size bm_recount(const Bitmap* bm) {
    bm_size w;
    bm_size total;

    if (!bm_valid(bm)) return 0;

    total = 0;
    for (w = 0; w < bm->n_words; w++) {
        total += (bm_size) __builtin_popcountll(bm->words[w]);
    }
    return total;
}

/* ============================================================
 * Logical operations
 * ============================================================
 *
 * Result is written to c. c is freed first if it was in use.
 */

int bm_and(const Bitmap* a, const Bitmap* b, Bitmap* c) {
    bm_size i;
    int     err;

    if (!bm_valid(a) || !bm_valid(b)) return BM_ERR_INVALID;
    if (a->n_bits != b->n_bits)       return BM_ERR_MISMATCH;

    bm_free(c);
    err = bm_init(c, a->n_bits);
    if (err != BM_OK) return err;

    c->count = 0;
    for (i = 0; i < a->n_words; i++) {
        c->words[i] = a->words[i] & b->words[i];
        c->count += (bm_size) __builtin_popcountll(c->words[i]);
    }
    return BM_OK;
}

int bm_or(const Bitmap* a, const Bitmap* b, Bitmap* c) {
    bm_size i;
    int     err;

    if (!bm_valid(a) || !bm_valid(b)) return BM_ERR_INVALID;
    if (a->n_bits != b->n_bits)       return BM_ERR_MISMATCH;

    bm_free(c);
    err = bm_init(c, a->n_bits);
    if (err != BM_OK) return err;

    c->count = 0;
    for (i = 0; i < a->n_words; i++) {
        c->words[i] = a->words[i] | b->words[i];
        c->count += (bm_size) __builtin_popcountll(c->words[i]);
    }
    return BM_OK;
}

int bm_xor(const Bitmap* a, const Bitmap* b, Bitmap* c) {
    bm_size i;
    int     err;

    if (!bm_valid(a) || !bm_valid(b)) return BM_ERR_INVALID;
    if (a->n_bits != b->n_bits)       return BM_ERR_MISMATCH;

    bm_free(c);
    err = bm_init(c, a->n_bits);
    if (err != BM_OK) return err;

    c->count = 0;
    for (i = 0; i < a->n_words; i++) {
        c->words[i] = a->words[i] ^ b->words[i];
        c->count += (bm_size) __builtin_popcountll(c->words[i]);
    }
    return BM_OK;
}

int bm_not(const Bitmap* a, Bitmap* c) {
    bm_size i;
    int     err;

    if (!bm_valid(a)) return BM_ERR_INVALID;

    bm_free(c);
    err = bm_init(c, a->n_bits);
    if (err != BM_OK) return err;

    for (i = 0; i < a->n_words; i++) {
        c->words[i] = ~a->words[i];
    }
    bm_clear_trailing(c);
    c->count = a->n_bits - a->count;
    return BM_OK;
}

int bm_copy(const Bitmap* a, Bitmap* c) {
    bm_size i;
    int     err;

    if (!bm_valid(a)) return BM_ERR_INVALID;

    bm_free(c);
    err = bm_init(c, a->n_bits);
    if (err != BM_OK) return err;

    for (i = 0; i < a->n_words; i++) {
        c->words[i] = a->words[i];
    }
    c->count = a->count;
    return BM_OK;
}

/* ============================================================
 * Diagnostics
 * ============================================================ */

u32 bm_check(const Bitmap* bm) {
    u32      problems;
    bm_size  expected_words;
    bm_size  real_count;
    bm_size  extra;
    u64      trailing;

    problems = 0;

    /* Freed or empty bitmap must have all-zero fields. */
    if (bm->words == NULL) {
        if (bm->n_bits != 0 || bm->n_words != 0 || bm->count != 0) {
            fprintf(stderr,
                "check: null words but non-zero fields\n");
            problems++;
        }
        return problems;
    }

    /* n_words must match n_bits. */
    expected_words = (bm->n_bits + BM_BITS_PER_WORD - 1) / BM_BITS_PER_WORD;
    if (bm->n_words != expected_words) {
        fprintf(stderr, "check: n_words %lu != %lu\n",
                (unsigned long) bm->n_words,
                (unsigned long) expected_words);
        problems++;
    }

    /* Cached count must match recount. */
    real_count = bm_recount(bm);
    if (bm->count != real_count) {
        fprintf(stderr, "check: count %lu != recount %lu\n",
                (unsigned long) bm->count,
                (unsigned long) real_count);
        problems++;
    }

    /* Trailing bits must be 0. */
    extra = bm->n_words * BM_BITS_PER_WORD - bm->n_bits;
    if (extra > 0) {
        trailing = bm->words[bm->n_words - 1] >> (64 - extra);
        if (trailing != 0) {
            fprintf(stderr, "check: trailing bits not clear\n");
            problems++;
        }
    }

    return problems;
}
