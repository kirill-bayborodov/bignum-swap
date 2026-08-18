/* ------------------------------------------------------------------ */
/**
 * @file    test_bignum_swap_mt.c
 * @brief   Multithreaded tests for bignum_swap.
 * @version 0.1.0
 * @details Revision 0.1.0: concurrent independent in-place swap checks.
 */
/* ------------------------------------------------------------------ */
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bignum_swap.h"

#define THREAD_COUNT 8U
#define ITERATIONS   10000U

typedef struct {
    unsigned id;
    unsigned failures;
} worker_context_t;

static bignum_t make_value(uint32_t len, uint64_t seed)
{
    bignum_t value;
    for (size_t i = 0U; i < BIGNUM_CAPACITY; ++i) {
        value.words[i] = seed + UINT64_C(0xD6E8FEB86659FD93) * (i + 1U);
    }
    value.len = len;
    return value;
}

static void *worker(void *argument)
{
    worker_context_t *context = argument;
    bignum_t a = make_value((context->id + 3U) % (BIGNUM_CAPACITY + 1U), context->id + 1U);
    bignum_t b = make_value((context->id + 11U) % (BIGNUM_CAPACITY + 1U), context->id + 101U);
    const bignum_t original_a = a;
    const bignum_t original_b = b;

    for (unsigned iteration = 0U; iteration < ITERATIONS; ++iteration) {
        if (bignum_swap(&a, &b) != BIGNUM_SWAP_SUCCESS) {
            ++context->failures;
            return NULL;
        }
        if (iteration % 2U == 1U) {
            if (memcmp(&a, &original_a, sizeof(a)) != 0 ||
                memcmp(&b, &original_b, sizeof(b)) != 0) {
                ++context->failures;
                return NULL;
            }
        }
    }
    return NULL;
}

int main(void)
{
    pthread_t threads[THREAD_COUNT];
    worker_context_t contexts[THREAD_COUNT];

    puts("--- Starting multithreaded bignum_swap tests ---");
    for (unsigned i = 0U; i < THREAD_COUNT; ++i) {
        contexts[i] = (worker_context_t){ .id = i, .failures = 0U };
        assert(pthread_create(&threads[i], NULL, worker, &contexts[i]) == 0);
    }
    for (unsigned i = 0U; i < THREAD_COUNT; ++i) {
        assert(pthread_join(threads[i], NULL) == 0);
        assert(contexts[i].failures == 0U);
    }
    puts("--- Multithreaded bignum_swap test passed ---");
    return 0;
}
