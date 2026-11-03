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
 * NULL semantics:
 *   - bm_init(NULL, n)      -> BM_ERR_INVALID
 *   - bm_free(NULL)         -> no-op
 *   - bm_size_of(NULL)      -> 0
 *   - bm_count(NULL)        -> 0
 *   - bm_check(NULL)        -> 1 (one problem)
 *   - bm_valid(NULL)        -> 0
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
    u64*     words   = nullptr;
    bm_size  n_bits  = 0;
    bm_size  n_words = 0;
    bm_size  count   = 0;
};

/* ============================================================
 * Error string
 * ============================================================ */

const char* bm_error_string(int err);

/* ============================================================
 * Lifecycle
 * ============================================================ */

/* Allocate n_bits bits. All bits start at 0.
 * bm may be empty (words == nullptr). bm must not be NULL.
 * If bm->words is not nullptr, it is freed first.
 * Returns BM_ERR_INVALID if bm is NULL. */
int bm_init(Bitmap* bm, bm_size n_bits);

/* Free all memory and reset fields.
 * bm may be NULL: no-op. */
void bm_free(Bitmap* bm);

/* True if the bitmap is usable.
 * Returns 0 for NULL, for a freed bitmap, and for an empty one. */
int bm_valid(const Bitmap* bm);

/* ============================================================
 * Basic operations
 * ============================================================ */

int bm_set(Bitmap* bm, bm_size index);
int bm_clear(Bitmap* bm, bm_size index);

/* Test bit at index.
 * Returns 1 if set, 0 if clear.
 * Returns 0 on error (invalid bm or index out of range).
 * Callers that need to distinguish "clear" from "error" must
 * check bm_valid / index separately. */
int bm_test(const Bitmap* bm, bm_size index);

int bm_toggle(Bitmap* bm, bm_size index);

/* Return 0 for NULL. */
bm_size bm_size_of(const Bitmap* bm);
bm_size bm_count(const Bitmap* bm);

/* ============================================================
 * Mass operations
 * ============================================================ */

int bm_set_all(Bitmap* bm);
int bm_clear_all(Bitmap* bm);

/* Print to stdout as hex, one line per word. */
void bm_print(const Bitmap* bm);

/* ============================================================
 * Search and count
 * ============================================================
 *
 * bm_find_first_set and bm_find_first_clear rely on the
 * invariant (trailing bits == 0). If the bitmap is corrupt,
 * they may return BM_NOT_FOUND early.
 */

bm_size bm_find_first_set(const Bitmap* bm);
bm_size bm_find_first_clear(const Bitmap* bm);
bm_size bm_recount(const Bitmap* bm);

/* ============================================================
 * Logical operations
 * ============================================================
 *
 * Result is written to c. c must not be NULL.
 * If c->words is not nullptr, it is freed first.
 * On success, c is initialised with a->n_bits bits.
 * On error, c is left unchanged.
 */

int bm_and(const Bitmap* a, const Bitmap* b, Bitmap* c);
int bm_or (const Bitmap* a, const Bitmap* b, Bitmap* c);
int bm_xor(const Bitmap* a, const Bitmap* b, Bitmap* c);
int bm_not(const Bitmap* a, Bitmap* c);
int bm_copy(const Bitmap* a, Bitmap* c);

/* ============================================================
 * Diagnostics
 * ============================================================ */

/* Verify internal consistency.
 * Checks: n_words == ceil(n_bits/64), count == recount,
 *         trailing bits == 0.
 * Returns number of problems found (0 = ok, 1 for NULL bm). */
u32 bm_check(const Bitmap* bm);

#endif /* BITMAP_HPP */
