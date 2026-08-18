/* ------------------------------------------------------------------ */
/**
 * @file    test_bignum_swap.c
 * @brief   Детерминированные тесты bignum_swap.
 * @version 0.1.0
 * @details Revision 0.1.0: typed status, full-state swap, self-swap and
 *          validation error tests.
 */
/* ------------------------------------------------------------------ */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bignum_swap.h"

static bignum_t make_value(uint32_t len, uint64_t seed)
{
    bignum_t value;
    for (size_t i = 0U; i < BIGNUM_CAPACITY; ++i) {
        value.words[i] = seed + UINT64_C(0x9E3779B97F4A7C15) * (i + 1U);
    }
    value.len = len;
    return value;
}

static void test_full_state_swap(void)
{
    bignum_t a = make_value(3U, UINT64_C(0x1111));
    bignum_t b = make_value(17U, UINT64_C(0xAAAA));
    const bignum_t expected_a = b;
    const bignum_t expected_b = a;

    assert(bignum_swap(&a, &b) == BIGNUM_SWAP_SUCCESS);
    assert(memcmp(&a, &expected_a, sizeof(a)) == 0);
    assert(memcmp(&b, &expected_b, sizeof(b)) == 0);
    puts("test_full_state_swap: PASSED");
}

static void test_zero_and_capacity(void)
{
    bignum_t zero = make_value(0U, UINT64_C(1));
    bignum_t full = make_value(BIGNUM_CAPACITY, UINT64_C(2));
    const bignum_t expected_zero = full;
    const bignum_t expected_full = zero;

    assert(bignum_swap(&zero, &full) == BIGNUM_SWAP_SUCCESS);
    assert(memcmp(&zero, &expected_zero, sizeof(zero)) == 0);
    assert(memcmp(&full, &expected_full, sizeof(full)) == 0);
    puts("test_zero_and_capacity: PASSED");
}

static void test_self_swap(void)
{
    bignum_t value = make_value(9U, UINT64_C(0x1234));
    const bignum_t before = value;
    bignum_t *alias_a = &value;
    bignum_t *alias_b = &value;

    assert(bignum_swap(alias_a, alias_b) == BIGNUM_SWAP_SUCCESS);
    assert(memcmp(&value, &before, sizeof(value)) == 0);
    puts("test_self_swap: PASSED");
}

static void test_null_arguments(void)
{
    bignum_t value = make_value(4U, UINT64_C(0x55));
    const bignum_t before = value;

    assert(bignum_swap(NULL, &value) == BIGNUM_SWAP_ERROR_NULL_ARG);
    assert(memcmp(&value, &before, sizeof(value)) == 0);
    assert(bignum_swap(&value, NULL) == BIGNUM_SWAP_ERROR_NULL_ARG);
    assert(memcmp(&value, &before, sizeof(value)) == 0);
    assert(bignum_swap(NULL, NULL) == BIGNUM_SWAP_ERROR_NULL_ARG);
    puts("test_null_arguments: PASSED");
}

static void test_overflow_preserves_both(void)
{
    bignum_t a = make_value(BIGNUM_CAPACITY + 1U, UINT64_C(0xA));
    bignum_t b = make_value(2U, UINT64_C(0xB));
    const bignum_t before_a = a;
    const bignum_t before_b = b;

    assert(bignum_swap(&a, &b) == BIGNUM_SWAP_ERROR_OVERFLOW);
    assert(memcmp(&a, &before_a, sizeof(a)) == 0);
    assert(memcmp(&b, &before_b, sizeof(b)) == 0);

    a = make_value(2U, UINT64_C(0xC));
    b = make_value(BIGNUM_CAPACITY + 1U, UINT64_C(0xD));
    const bignum_t before_a2 = a;
    const bignum_t before_b2 = b;
    assert(bignum_swap(&a, &b) == BIGNUM_SWAP_ERROR_OVERFLOW);
    assert(memcmp(&a, &before_a2, sizeof(a)) == 0);
    assert(memcmp(&b, &before_b2, sizeof(b)) == 0);
    puts("test_overflow_preserves_both: PASSED");
}

int main(void)
{
    puts("--- Starting deterministic bignum_swap tests ---");
    test_full_state_swap();
    test_zero_and_capacity();
    test_self_swap();
    test_null_arguments();
    test_overflow_preserves_both();
    puts("--- All deterministic bignum_swap tests passed ---");
    return 0;
}
