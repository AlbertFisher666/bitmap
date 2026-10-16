/* ============================================================
 * bitmap_test.cpp
 * Test suite for the bitmap library.
 *
 * Style: 80s C-flavoured C++. assert-based, no frameworks.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "bitmap/bitmap.hpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ============================================================
 * Section 1: empty bitmap
 * ============================================================ */

static void test_empty_init(void) {
    Bitmap bm;

    assert(bm_init(&bm, 0) == BM_OK);
    assert(bm.words == NULL);
    assert(bm.n_bits == 0);
    assert(bm.n_words == 0);
    assert(bm.count == 0);
    assert(!bm_valid(&bm));
    assert(bm_find_first_set(&bm) == BM_NOT_FOUND);
    assert(bm_find_first_clear(&bm) == BM_NOT_FOUND);

    bm_free(&bm);
    assert(bm.words == NULL);
    assert(bm.n_bits == 0);
}

/* ============================================================
 * Section 2: single bit
 * ============================================================ */

static void test_single_bit(void) {
    Bitmap bm;

    assert(bm_init(&bm, 1) == BM_OK);
    assert(bm_valid(&bm));
    assert(bm.n_words == 1);
    assert(bm.count == 0);

    assert(bm_set(&bm, 0) == BM_OK);
    assert(bm.count == 1);
    assert(bm_test(&bm, 0) == 1);
    assert(bm_find_first_set(&bm) == 0);
    assert(bm_find_first_clear(&bm) == BM_NOT_FOUND);

    /* Repeat set: count does not grow. */
    assert(bm_set(&bm, 0) == BM_OK);
    assert(bm.count == 1);

    /* Out of range. */
    assert(bm_set(&bm, 1) == BM_ERR_RANGE);
    assert(bm.count == 1);

    /* Clear. */
    assert(bm_clear(&bm, 0) == BM_OK);
    assert(bm.count == 0);
    assert(bm_test(&bm, 0) == 0);

    /* Repeat clear: count does not fall below zero. */
    assert(bm_clear(&bm, 0) == BM_OK);
    assert(bm.count == 0);

    bm_free(&bm);
}

/* ============================================================
 * Section 3: word boundaries
 * ============================================================ */

static void test_word_boundaries(void) {
    Bitmap bm;

    /* Exactly one word. */
    assert(bm_init(&bm, 64) == BM_OK);
    assert(bm.n_words == 1);

    assert(bm_set(&bm, 0) == BM_OK);
    assert(bm_set(&bm, 63) == BM_OK);
    assert(bm.count == 2);
    assert(bm_find_first_set(&bm) == 0);
    bm_free(&bm);

    /* One bit into the second word. */
    assert(bm_init(&bm, 65) == BM_OK);
    assert(bm.n_words == 2);

    assert(bm_set(&bm, 64) == BM_OK);
    assert(bm.count == 1);
    assert(bm_test(&bm, 64) == 1);
    assert(bm_test(&bm, 63) == 0);
    assert(bm_find_first_set(&bm) == 64);
    bm_free(&bm);

    /* Two full words. */
    assert(bm_init(&bm, 128) == BM_OK);
    assert(bm.n_words == 2);

    assert(bm_set(&bm, 0) == BM_OK);
    assert(bm_set(&bm, 64) == BM_OK);
    assert(bm_set(&bm, 127) == BM_OK);
    assert(bm.count == 3);
    bm_free(&bm);
}

/* ============================================================
 * Section 4: set / clear / test / toggle
 * ============================================================ */

static void test_basic_ops(void) {
    Bitmap bm;
    bm_size i;

    assert(bm_init(&bm, 200) == BM_OK);

    for (i = 0; i < 200; i += 17) {
        assert(bm_set(&bm, i) == BM_OK);
    }
    /* 0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187 = 12 */
    assert(bm.count == 12);

    for (i = 0; i < 200; i += 17) {
        assert(bm_test(&bm, i) == 1);
        assert(bm_test(&bm, i + 1) == 0);
    }

    /* Toggle on and off. */
    assert(bm_toggle(&bm, 100) == BM_OK);
    assert(bm_test(&bm, 100) == 1);
    assert(bm.count == 13);
    assert(bm_toggle(&bm, 100) == BM_OK);
    assert(bm_test(&bm, 100) == 0);
    assert(bm.count == 12);

    /* Toggle existing bit. */
    assert(bm_toggle(&bm, 0) == BM_OK);
    assert(bm_test(&bm, 0) == 0);
    assert(bm.count == 11);

    /* Toggle out of range. */
    assert(bm_toggle(&bm, 200) == BM_ERR_RANGE);
    assert(bm.count == 11);

    bm_free(&bm);
}

/* ============================================================
 * Section 5: set_all / clear_all with trailing bits
 * ============================================================ */

static void test_mass_ops(void) {
    Bitmap bm;
    u64 trailing;
    bm_size extra;

    /* n_bits = 100: two words, second has 36 valid bits, 28 trailing. */
    assert(bm_init(&bm, 100) == BM_OK);
    assert(bm.n_words == 2);

    assert(bm_set_all(&bm) == BM_OK);
    assert(bm.count == 100);
    assert(bm_test(&bm, 0) == 1);
    assert(bm_test(&bm, 99) == 1);
    assert(bm_find_first_clear(&bm) == BM_NOT_FOUND);

    /* Trailing bits in the last word must be 0. */
    extra = bm.n_words * BM_BITS_PER_WORD - bm.n_bits;  /* 28 */
    trailing = bm.words[bm.n_words - 1] >> (64 - extra);
    assert(trailing == 0);

    assert(bm_clear_all(&bm) == BM_OK);
    assert(bm.count == 0);
    assert(bm_find_first_set(&bm) == BM_NOT_FOUND);

    bm_free(&bm);
}

/* ============================================================
 * Section 6: find_first_set / find_first_clear
 * ============================================================ */

static void test_find(void) {
    Bitmap bm;

    /* Empty bitmap: nothing set, everything clear. */
    assert(bm_init(&bm, 200) == BM_OK);
    assert(bm_find_first_set(&bm) == BM_NOT_FOUND);
    assert(bm_find_first_clear(&bm) == 0);

    /* One set bit in the middle. */
    assert(bm_set(&bm, 100) == BM_OK);
    assert(bm_find_first_set(&bm) == 100);
    assert(bm_find_first_clear(&bm) == 0);

    /* First bit set: first clear moves to 1. */
    assert(bm_set(&bm, 0) == BM_OK);
    assert(bm_find_first_set(&bm) == 0);
    assert(bm_find_first_clear(&bm) == 1);

    /* Sparse: clear at end. */
    for (bm_size i = 0; i < 200; i++) {
        assert(bm_set(&bm, i) == BM_OK);
    }
    assert(bm.count == 200);
    assert(bm_find_first_set(&bm) == 0);
    assert(bm_find_first_clear(&bm) == BM_NOT_FOUND);

    bm_free(&bm);
}

/* ============================================================
 * Section 7: recount
 * ============================================================ */

static void test_recount(void) {
    Bitmap bm;
    bm_size saved;

    assert(bm_init(&bm, 300) == BM_OK);

    for (bm_size i = 0; i < 300; i += 7) {
        assert(bm_set(&bm, i) == BM_OK);
    }

    saved = bm.count;
    assert(bm_recount(&bm) == saved);

    /* Corrupt the cached count and verify recount catches it. */
    bm.count = 999;
    assert(bm_recount(&bm) == saved);

    bm_free(&bm);
}

/* ============================================================
 * Section 8: logical operations
 * ============================================================ */

static void test_logic(void) {
    Bitmap a, b, c;

    assert(bm_init(&a, 10) == BM_OK);
    assert(bm_init(&b, 10) == BM_OK);
    c = {};

    assert(bm_set(&a, 0) == BM_OK);
    assert(bm_set(&a, 5) == BM_OK);
    assert(bm_set(&b, 5) == BM_OK);
    assert(bm_set(&b, 9) == BM_OK);

    /* AND: only 5. */
    assert(bm_and(&a, &b, &c) == BM_OK);
    assert(bm_count(&c) == 1);
    assert(bm_test(&c, 5) == 1);
    assert(bm_test(&c, 0) == 0);
    assert(bm_test(&c, 9) == 0);
    bm_free(&c);

    /* OR: 0, 5, 9. */
    assert(bm_or(&a, &b, &c) == BM_OK);
    assert(bm_count(&c) == 3);
    assert(bm_test(&c, 0) == 1);
    assert(bm_test(&c, 5) == 1);
    assert(bm_test(&c, 9) == 1);
    bm_free(&c);

    /* XOR: 0 and 9. */
    assert(bm_xor(&a, &b, &c) == BM_OK);
    assert(bm_count(&c) == 2);
    assert(bm_test(&c, 0) == 1);
    assert(bm_test(&c, 5) == 0);
    assert(bm_test(&c, 9) == 1);
    bm_free(&c);

    /* NOT of a: 10 - 2 = 8. */
    assert(bm_not(&a, &c) == BM_OK);
    assert(bm_count(&c) == 8);
    assert(bm_test(&c, 0) == 0);
    assert(bm_test(&c, 5) == 0);
    assert(bm_test(&c, 1) == 1);
    assert(bm_check(&c) == 0);
    bm_free(&c);

    /* COPY of a: same bits. */
    assert(bm_copy(&a, &c) == BM_OK);
    assert(bm_count(&c) == 2);
    assert(bm_test(&c, 0) == 1);
    assert(bm_test(&c, 5) == 1);
    bm_free(&c);

    /* Mismatch: different sizes. */
    assert(bm_init(&c, 20) == BM_OK);
    assert(bm_and(&a, &c, &b) == BM_ERR_MISMATCH);
    assert(bm_or(&a, &c, &b) == BM_ERR_MISMATCH);
    assert(bm_xor(&a, &c, &b) == BM_ERR_MISMATCH);

    bm_free(&a);
    bm_free(&b);
    bm_free(&c);
}

/* ============================================================
 * Section 9: NOT with trailing bits
 * ============================================================ */

static void test_not_trailing(void) {
    Bitmap a, c;

    /* n_bits = 10: one word, 54 trailing bits. */
    assert(bm_init(&a, 10) == BM_OK);

    /* Empty → full. */
    assert(bm_not(&a, &c) == BM_OK);
    assert(bm_count(&c) == 10);
    assert(bm_check(&c) == 0);

    /* Trailing bits must be 0 after NOT. */
    {
        u64 trailing = c.words[0] >> 10;
        assert(trailing == 0);
    }
    bm_free(&a);
    bm_free(&c);

    /* Full → empty. */
    assert(bm_init(&a, 10) == BM_OK);
    assert(bm_set_all(&a) == BM_OK);
    assert(bm_not(&a, &c) == BM_OK);
    assert(bm_count(&c) == 0);
    assert(bm_check(&c) == 0);
    bm_free(&a);
    bm_free(&c);
}

/* ============================================================
 * Section 10: check diagnostics
 * ============================================================ */

static void test_check(void) {
    Bitmap bm;

    /* Fresh bitmap: no problems. */
    assert(bm_init(&bm, 50) == BM_OK);
    assert(bm_check(&bm) == 0);

    /* Corrupt cached count: check must report. */
    bm.count = 5;
    assert(bm_check(&bm) > 0);
    bm.count = 0;

    /* Corrupt n_words: check must report. */
    bm.n_words = 0;
    assert(bm_check(&bm) > 0);
    bm.n_words = 1;

    printf("n_words=%lu count=%lu word[0]=0x%016llX\n",
		    (unsigned long) bm.n_words,
		    (unsigned long) bm.count,
		    (unsigned long long) bm.words[0]);

    /* Restore: no problems. */
    assert(bm_check(&bm) == 0);
    bm_free(&bm);

    /* Freed bitmap: no problems. */
    assert(bm_check(&bm) == 0);
}

/* ============================================================
 * Section 11: error strings
 * ============================================================ */

static void test_error_strings(void) {
    assert(strcmp(bm_error_string(BM_OK),           "ok") == 0);
    assert(strcmp(bm_error_string(BM_ERR_NOMEM),    "no memory") == 0);
    assert(strcmp(bm_error_string(BM_ERR_RANGE),    "out of range") == 0);
    assert(strcmp(bm_error_string(BM_ERR_MISMATCH), "size mismatch") == 0);
    assert(strcmp(bm_error_string(BM_ERR_INVALID),  "not initialized") == 0);
    assert(strcmp(bm_error_string(999),             "unknown") == 0);
}

/* ============================================================
 * Section 12: invalid calls
 * ============================================================ */

static void test_invalid(void) {
    Bitmap bm;

    /* Uninitialized: no words. */
    bm = {};
    assert(bm_set(&bm, 0) == BM_ERR_INVALID);
    assert(bm_clear(&bm, 0) == BM_ERR_INVALID);
    assert(bm_toggle(&bm, 0) == BM_ERR_INVALID);
    assert(bm_test(&bm, 0) == 0);
    assert(bm_valid(&bm) == 0);

    /* Invalid source for logic ops. */
    {
        Bitmap c;
        assert(bm_and(&bm, &bm, &c) == BM_ERR_INVALID);
        assert(bm_or(&bm, &bm, &c) == BM_ERR_INVALID);
        assert(bm_xor(&bm, &bm, &c) == BM_ERR_INVALID);
        assert(bm_not(&bm, &c) == BM_ERR_INVALID);
        assert(bm_copy(&bm, &c) == BM_ERR_INVALID);
    }
}

/* ============================================================
 * main
 * ============================================================ */

int main(void) {
    printf(">>> test_empty_init\n");    test_empty_init();
    printf(">>> test_single_bit\n");    test_single_bit();
    printf(">>> test_word_boundaries\n"); test_word_boundaries();
    printf(">>> test_basic_ops\n");     test_basic_ops();
    printf(">>> test_mass_ops\n");      test_mass_ops();
    printf(">>> test_find\n");          test_find();
    printf(">>> test_recount\n");       test_recount();
    printf(">>> test_logic\n");         test_logic();
    printf(">>> test_not_trailing\n");  test_not_trailing();
    printf(">>> test_check\n");         test_check();
    printf(">>> test_error_strings\n"); test_error_strings();
    printf(">>> test_invalid\n");       test_invalid();

    printf("all tests passed\n");
    return 0;
}
