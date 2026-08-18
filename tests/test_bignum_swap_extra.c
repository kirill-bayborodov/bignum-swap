/* ------------------------------------------------------------------ */
/**
 * @file    test_bignum_swap_extra.c
 * @brief   Extended fuzz and stress tests for bignum_swap.
 * @version 0.1.0
 * @details Revision 0.1.0: 10,000 randomized full-state swap checks,
 *          invalid-length checks and exact self-swap stress.
 */
/* ------------------------------------------------------------------ */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bignum_swap.h"

static uint64_t next_random(uint64_t *state)
{
    *state = *state * UINT64_C(6364136223846793005) + UINT64_C(1442695040888963407);
    return *state;
}

static void fill_value(bignum_t *value, uint64_t *state, uint32_t len)
{
    for (size_t i = 0U; i < BIGNUM_CAPACITY; ++i) {
        value->words[i] = next_random(state);
    }
    value->len = len;
}

static void test_randomized_swaps(void)
{
    uint64_t state = UINT64_C(0x0123456789ABCDEF);
    for (unsigned iteration = 0U; iteration < 10000U; ++iteration) {
        bignum_t a;
        bignum_t b;
        fill_value(&a, &state, (uint32_t)(next_random(&state) % (BIGNUM_CAPACITY + 1U)));
        fill_value(&b, &state, (uint32_t)(next_random(&state) % (BIGNUM_CAPACITY + 1U)));
        const bignum_t before_a = a;
        const bignum_t before_b = b;

        assert(bignum_swap(&a, &b) == BIGNUM_SWAP_SUCCESS);
        assert(memcmp(&a, &before_b, sizeof(a)) == 0);
        assert(memcmp(&b, &before_a, sizeof(b)) == 0);
    }
    puts("test_randomized_swaps: PASSED (10000 cases)");
}

static void test_self_swap_stress(void)
{
    bignum_t value;
    uint64_t state = UINT64_C(0xCAFEBABE);
    fill_value(&value, &state, BIGNUM_CAPACITY);
    const bignum_t before = value;
    bignum_t *alias_a = &value;
    bignum_t *alias_b = &value;

    for (unsigned iteration = 0U; iteration < 100000U; ++iteration) {
        assert(bignum_swap(alias_a, alias_b) == BIGNUM_SWAP_SUCCESS);
    }
    assert(memcmp(&value, &before, sizeof(value)) == 0);
    puts("test_self_swap_stress: PASSED (100000 operations)");
}

static void test_partial_overlap_contract(void)
{
    union {
        max_align_t alignment;
        unsigned char bytes[sizeof(bignum_t) + sizeof(uint64_t)];
    } storage;
    const uintptr_t first = (uintptr_t)&storage.bytes[0];
    const uintptr_t second = (uintptr_t)&storage.bytes[sizeof(uint64_t)];
    const uintptr_t object_end = first + sizeof(bignum_t);

    /* The layouts overlap, but calling bignum_swap here would violate restrict. */
    assert(first < second);
    assert(second < object_end);
    puts("test_partial_overlap_contract: NOT INVOKED (outside restrict contract)");
}

static void test_invalid_lengths(void)
{
    bignum_t a;
    bignum_t b;
    uint64_t state = UINT64_C(0xF00D);
    fill_value(&a, &state, BIGNUM_CAPACITY + 1U);
    fill_value(&b, &state, BIGNUM_CAPACITY + 2U);
    const bignum_t before_a = a;
    const bignum_t before_b = b;

    assert(bignum_swap(&a, &b) == BIGNUM_SWAP_ERROR_OVERFLOW);
    assert(memcmp(&a, &before_a, sizeof(a)) == 0);
    assert(memcmp(&b, &before_b, sizeof(b)) == 0);
    puts("test_invalid_lengths: PASSED");
}

int main(void)
{
    puts("--- Starting extended bignum_swap tests ---");
    test_randomized_swaps();
    test_self_swap_stress();
    test_partial_overlap_contract();
    test_invalid_lengths();
    puts("--- All extended bignum_swap tests passed ---");
    return 0;
}
