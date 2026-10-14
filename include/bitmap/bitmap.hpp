/* ============================================================
 * bitmap.hpp
 * A low-level bitset with fixed size.
 *
 * Style: 80s C-flavoured C++.
 *   - Fixed-size Bitmap object.
 *   - Free functions bm_*.
 *   - Error codes via #define BM_ERR_*.
 *   - Explicit types, explicit prefixes, no exceptions.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#ifndef BITMAP_HPP
#define BITMAP_HPP

#include <stdint.h>
#include <stddef.h>

/* ============================================================
 * Constants
 * ============================================================ */

#define BM_BITS_PER_WORD   64
#define BM_NOT_FOUND       ((size_t)-1)

/* Error codes. Plain ints, as in old C. */
#define BM_OK              0
#define BM_ERR_NOMEM       1
#define BM_ERR_RANGE       2
#define BM_ERR_MISMATCH    3
#define BM_ERR_INVALID     4

/* ============================================================
 * Types
 * ============================================================ */

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef size_t    bm_size;

/* ============================================================
 * Bitmap structure
 * ============================================================
 *
 * words   - array of 64-bit words.
 * n_bits  - requested size in bits.
 * n_words - number of allocated words.
 * count   - cached number of set bits.
 *
 * Invariant: bits beyond n_bits in the last word are always 0.
 */

struct Bitmap {
    u64*     words;
    bm_size  n_bits;
    bm_size  n_words;
    bm_size  count;
};

/* ============================================================
 * Error string
 * ============================================================ */

const char* bm_error_string(int err);

/* ============================================================
 * Lifecycle
 * ============================================================ */

/* Allocate n_bits bits. All bits start at 0. */
int bm_init(Bitmap* bm, bm_size n_bits);

/* Free all memory and reset fields. */
void bm_free(Bitmap* bm);

/* True if the bitmap is usable. */
int bm_valid(const Bitmap* bm);

/* ============================================================
 * Basic operations
 * ============================================================ */

int bm_set(Bitmap* bm, bm_size index);
int bm_clear(Bitmap* bm, bm_size index);
int bm_test(const Bitmap* bm, bm_size index);
int bm_toggle(Bitmap* bm, bm_size index);

bm_size bm_size_of(const Bitmap* bm);
bm_size bm_count(const Bitmap* bm);

/* ============================================================
 * Mass operations
 * ============================================================ */

int bm_set_all(Bitmap* bm);
int bm_clear_all(Bitmap* bm);
void bm_print(const Bitmap* bm);

/* ============================================================
 * Search and count
 * ============================================================ */

bm_size bm_find_first_set(const Bitmap* bm);
bm_size bm_find_first_clear(const Bitmap* bm);
bm_size bm_recount(const Bitmap* bm);

/* ============================================================
 * Logical operations
 * ============================================================ */

int bm_and(const Bitmap* a, const Bitmap* b, Bitmap* c);
int bm_or (const Bitmap* a, const Bitmap* b, Bitmap* c);
int bm_xor(const Bitmap* a, const Bitmap* b, Bitmap* c);
int bm_not(const Bitmap* a, Bitmap* c);
int bm_copy(const Bitmap* a, Bitmap* c);

/* ============================================================
 * Diagnostics
 * ============================================================ */

u32 bm_check(const Bitmap* bm);

#endif /* BITMAP_HPP */
