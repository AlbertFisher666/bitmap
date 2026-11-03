/* ============================================================
 * bitmap_demo.cpp
 * Field test: emulate a CP+ disk block allocator on top of
 * the bitmap library.
 *
 * Scenario:
 *   - 128 sectors (as in CP+ cpvm.disk).
 *   - Sectors 0-4 are system (boot, superblock, inode, bitmap).
 *   - Files occupy 1..8 contiguous-ish blocks.
 *   - Free a file, allocate a new one, verify integrity.
 *
 * Style: 80s C-flavoured C++.
 *
 * Made by Albert Ghosting Frangle (aka Albert Fisher)
 * ============================================================ */

#include "bitmap/bitmap.hpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define DISK_SECTORS     128
#define SYSTEM_SECTORS     5

/* ============================================================
 * Helpers
 * ============================================================ */

/* Mark a run of sectors as used. Returns 0 on success. */
static int mark_range(Bitmap* bm, bm_size from, bm_size count) {
    bm_size i;

    for (i = 0; i < count; i++) {
        int err = bm_set(bm, from + i);
        if (err != BM_OK) return err;
    }
    return BM_OK;
}

/* Free a run of sectors. Returns 0 on success. */
static int free_range(Bitmap* bm, bm_size from, bm_size count) {
    bm_size i;

    for (i = 0; i < count; i++) {
        int err = bm_clear(bm, from + i);
        if (err != BM_OK) return err;
    }
    return BM_OK;
}

/* Allocate the first free sector. Returns BM_NOT_FOUND if full. */
static bm_size alloc_one(Bitmap* bm) {
    bm_size idx = bm_find_first_clear(bm);
    if (idx == BM_NOT_FOUND) return BM_NOT_FOUND;
    assert(bm_set(bm, idx) == BM_OK);
    return idx;
}

/* ============================================================
 * Demo
 * ============================================================ */

int main(void) {
    Bitmap disk;

    printf("=== CP+ disk block allocator demo ===\n");
    printf("Sectors: %d, system: %d\n\n", DISK_SECTORS, SYSTEM_SECTORS);

    /* 1. Init the bitmap. */
    assert(bm_init(&disk, DISK_SECTORS) == BM_OK);
    printf("disk bitmap: %lu bits, %lu words, %lu set\n",
           (unsigned long) bm_size_of(&disk),
           (unsigned long) disk.n_words,
           (unsigned long) bm_count(&disk));

    /* 2. Mark system sectors. */
    assert(mark_range(&disk, 0, SYSTEM_SECTORS) == BM_OK);
    printf("system sectors marked: count = %lu\n",
           (unsigned long) bm_count(&disk));
    assert(bm_count(&disk) == SYSTEM_SECTORS);

    /* 3. Allocate file A: 4 sectors. */
    printf("\nfile A: allocate 4 sectors\n");
    {
        bm_size start = alloc_one(&disk);
        assert(start == SYSTEM_SECTORS);   /* 5 */
        assert(mark_range(&disk, start + 1, 3) == BM_OK);
        printf("  file A at sectors %lu..%lu\n",
               (unsigned long) start,
               (unsigned long) (start + 3));
    }
    assert(bm_count(&disk) == SYSTEM_SECTORS + 4);

    /* 4. Allocate file B: 2 sectors. */
    printf("file B: allocate 2 sectors\n");
    {
        bm_size start = alloc_one(&disk);
        assert(start == SYSTEM_SECTORS + 4);   /* 9 */
        assert(mark_range(&disk, start + 1, 1) == BM_OK);
        printf("  file B at sectors %lu..%lu\n",
               (unsigned long) start,
               (unsigned long) (start + 1));
    }
    assert(bm_count(&disk) == SYSTEM_SECTORS + 6);

    /* 5. Allocate file C: 3 sectors. */
    printf("file C: allocate 3 sectors\n");
    {
        bm_size start = alloc_one(&disk);
        assert(start == SYSTEM_SECTORS + 6);   /* 11 */
        assert(mark_range(&disk, start + 1, 2) == BM_OK);
        printf("  file C at sectors %lu..%lu\n",
               (unsigned long) start,
               (unsigned long) (start + 2));
    }
    assert(bm_count(&disk) == SYSTEM_SECTORS + 9);

    /* 6. Show the bitmap. */
    printf("\nbitmap after 3 files:\n");
    bm_print(&disk);
    assert(bm_check(&disk) == 0);

    /* 7. Free file B (2 sectors at 9..10). */
    printf("free file B (sectors 9..10)\n");
    assert(free_range(&disk, SYSTEM_SECTORS + 4, 2) == BM_OK);
    assert(bm_count(&disk) == SYSTEM_SECTORS + 7);
    assert(bm_test(&disk, 9) == 0);
    assert(bm_test(&disk, 10) == 0);
    assert(bm_check(&disk) == 0);

    /* 8. Allocate file D: 2 sectors. Should take 9..10. */
    printf("file D: allocate 2 sectors\n");
    {
        bm_size start = alloc_one(&disk);
        assert(start == 9);
        assert(mark_range(&disk, start + 1, 1) == BM_OK);
        printf("  file D at sectors %lu..%lu\n",
               (unsigned long) start,
               (unsigned long) (start + 1));
    }
    assert(bm_count(&disk) == SYSTEM_SECTORS + 9);
    assert(bm_check(&disk) == 0);

    /* 9. Final state. */
    printf("\nfinal bitmap:\n");
    bm_print(&disk);

    /* 10. Sanity: total count. */
    assert(bm_count(&disk) == bm_recount(&disk));
    assert(bm_count(&disk) == SYSTEM_SECTORS + 9);

    /* 11. Free everything. */
    bm_free(&disk);
    assert(!bm_valid(&disk));

    printf("\n=== demo passed ===\n");
    return 0;
}
