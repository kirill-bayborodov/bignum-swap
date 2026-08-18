/* ------------------------------------------------------------------ */
/**
 * @file    bignum_swap.c
 * @brief   Эталонная переносимая C-реализация bignum_swap.
 * @version 0.1.0
 * @details Revision 0.1.0: typed in-place swap with validation.
 */
/* ------------------------------------------------------------------ */
#include <stddef.h>
#include <stdint.h>

#include "bignum_swap.h"

bignum_swap_status_t bignum_swap(
    bignum_t *restrict a,
    bignum_t *restrict b)
{
    if (a == NULL || b == NULL) {
        return BIGNUM_SWAP_ERROR_NULL_ARG;
    }
    if (a == b) {
        return BIGNUM_SWAP_SUCCESS;
    }
    if (a->len > BIGNUM_CAPACITY || b->len > BIGNUM_CAPACITY) {
        return BIGNUM_SWAP_ERROR_OVERFLOW;
    }

    uint64_t tmp;
    const size_t word_count = sizeof(a->words) / sizeof(a->words[0]);
    for (size_t i = 0U; i < word_count; ++i) {
        tmp = a->words[i];
        a->words[i] = b->words[i];
        b->words[i] = tmp;
    }

    const uint32_t len = a->len;
    a->len = b->len;
    b->len = len;

    return BIGNUM_SWAP_SUCCESS;
}
