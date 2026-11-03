/* ============================================================
 * sieve.cpp
 * Sieve of Eratosthenes on top of Bitmap. Single translation unit.
 *
 * Style: 80s C-flavoured C++.
 *   - One file, no header, no header guard.
 *   - Static helpers internal to this file.
 *   - Explicit types, explicit prefixes, no exceptions.
 *
 * Bit convention: bit k is 1 if k is prime, 0 otherwise.
 *
 *  Contract for sieve_build:
 *   - primes must not be NULL.
 *   - primes must not alias another live Bitmap (Bitmap b = a;).
 *   - Passing an already-initialised bitmap is fine: bm_init frees
 *   - its previous storage.
 *   - n must not be (bm_size)-1 (n + 1 would wrap).
 *
 * Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "bitmap/bitmap.hpp"

#include <stdio.h>

/* ============================================================
 * Internal helpers
 * ============================================================ */

/* True if n + 1 would not fit in bm_size. */
static int sieve_n_overflow(bm_size n) {
    return n == (bm_size)-1;
}

/* Allocate and set all bits for numbers 0..n.
 * Precondition: primes->words == NULL (checked by caller). */
static int sieve_alloc(Bitmap* primes, bm_size n) {
    int err;

    err = bm_init(primes, n + 1);
    if (err != BM_OK) return err;

    err = bm_set_all(primes);
    if (err != BM_OK) {
        bm_free(primes);
        return err;
    }
    return BM_OK;
}

/* Cross out composites up to n.
 * Precondition: bitmap sized n + 1, all bits set. */
static void sieve_cross_out(Bitmap* primes, bm_size n) {
    bm_size i;
    bm_size j;

    bm_clear(primes, 0);
    if (n >= 1) {
        bm_clear(primes, 1);
    }

    /* i <= n / i avoids overflow in i * i. */
    for (i = 2; i <= n / i; i++) {
        if (!bm_test(primes, i)) continue;

        /* j = i*i is safe: guarded by i <= n / i above.
         * j += i is guarded by j > n - i below.
         * n - i is safe: i <= n for all i >= 2 and n >= 4,
         * and this loop body only runs when n >= 4. */
        for (j = i * i; j <= n; ) {
            bm_clear(primes, j);
            if (j > n - i) break;
            j += i;
        }
    }
}

/* ============================================================
 * Public entry point
 * ============================================================ */

/* Build a sieve up to n inclusive into *primes.
 * Returns BM_OK, BM_ERR_INVALID, BM_ERR_RANGE, or BM_ERR_NOMEM.
 * On success, bit k of *primes is 1 iff k is prime. */
int sieve_build(Bitmap* primes, bm_size n) {
    int err;

    if (primes == NULL)        return BM_ERR_INVALID;
    if (sieve_n_overflow(n))   return BM_ERR_RANGE;

    /* bm_init frees any previous storage. Aliasing (Bitmap b = a;)
     * is the caller's responsibility — see README of bitmap. */
    err = sieve_alloc(primes, n);
    if (err != BM_OK) return err;

    sieve_cross_out(primes, n);
    return BM_OK;
}

/* ============================================================
 * Small demo driver.
 * ============================================================ */

int main(void) {
    Bitmap   primes = {};        /* default-constructed: words == NULL */
    bm_size  n      = 100;
    bm_size  i;
    bm_size  total  = 0;
    int      err;

    err = sieve_build(&primes, n);
    if (err != BM_OK) {
        fprintf(stderr, "sieve_build failed: %s\n", bm_error_string(err));
        return 1;
    }

    if (bm_check(&primes) != 0) {
        fprintf(stderr, "bitmap inconsistent\n");
        bm_free(&primes);
        return 1;
    }

    printf("primes up to %lu:\n", (unsigned long) n);
    for (i = 0; i <= n; i++) {
        if (bm_test(&primes, i)) {
            printf("%lu ", (unsigned long) i);
            total++;
        }
    }
    printf("\n\ntotal primes: %lu\n", (unsigned long) total);
    printf("cached count : %lu\n", (unsigned long) bm_count(&primes));

    bm_free(&primes);
    return 0;
}
