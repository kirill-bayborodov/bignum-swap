/* ------------------------------------------------------------------ */
/**
 * @file    test_bignum_swap_runner.c
 * @brief   Integration runner for bignum_swap.
 * @version 0.1.0
 * @details Revision 0.1.0: public-header/status and full-state swap smoke test.
 */
/* ------------------------------------------------------------------ */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "bignum_swap.h"

int main(void)
{
    bignum_t first = { { UINT64_C(1), UINT64_C(2), UINT64_C(3) }, 3U };
    bignum_t second = { { UINT64_C(7), UINT64_C(8) }, 2U };
    const bignum_t expected_first = second;
    const bignum_t expected_second = first;

    assert(bignum_swap(&first, &second) == BIGNUM_SWAP_SUCCESS);
    assert(memcmp(&first, &expected_first, sizeof(first)) == 0);
    assert(memcmp(&second, &expected_second, sizeof(second)) == 0);
    assert(bignum_swap(NULL, &first) == BIGNUM_SWAP_ERROR_NULL_ARG);

    puts("Integration bignum_swap runner passed");
    return 0;
}
