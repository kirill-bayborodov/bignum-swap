#define _POSIX_C_SOURCE 200809L
/**
 * @file    bench_bignum_swap.c
 * @brief   Single-thread benchmark for bignum_swap.
 * @version 0.2.0
 * @details Revision 0.2.0: typed status checking and deterministic swap timing compatible with the template perf workflow.
 */
/* ------------------------------------------------------------------ */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bignum_swap.h"

static uint64_t now_ns(void)
{
    struct timespec ts;
    (void)clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * UINT64_C(1000000000) + (uint64_t)ts.tv_nsec;
}

static int parse_u64(const char *text, uint64_t *value)
{
    char *end = NULL;
    unsigned long long parsed = strtoull(text, &end, 0);
    if (end == text || *end != '\0') return -1;
    *value = (uint64_t)parsed;
    return 0;
}

int main(int argc, char **argv)
{
    uint64_t iterations = UINT64_C(1000000);
    uint64_t warmup = UINT64_C(10000);
    const char *data_mode = "all_nonzero";
    bignum_t source = { { 0U }, BIGNUM_CAPACITY };
    bignum_t destination = { { 0U }, 0U };
    uint64_t checksum = 0U;
    uint64_t start;
    uint64_t end;

    for (int i = 1; i < argc; ++i) {
        uint64_t value;
        if ((strcmp(argv[i], "--iterations") == 0 ||
             strcmp(argv[i], "--warmup") == 0) && i + 1 < argc &&
            parse_u64(argv[++i], &value) == 0) {
            if (strcmp(argv[i - 1], "--iterations") == 0) iterations = value;
            else warmup = value;
        } else if (strcmp(argv[i], "--data-mode") == 0 && i + 1 < argc) {
            data_mode = argv[++i];
            if (strcmp(data_mode, "all_zero") != 0 &&
                strcmp(data_mode, "all_nonzero") != 0 &&
                strcmp(data_mode, "mixed") != 0) return EXIT_FAILURE;
        } else {
            fprintf(stderr, "usage: %s [--iterations N] [--warmup N] [--data-mode all_zero|all_nonzero|mixed]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (iterations == 0U) return EXIT_FAILURE;
    source.len = strcmp(data_mode, "all_zero") == 0 ? 0U : BIGNUM_CAPACITY;
    for (size_t i = 0U; i < source.len; ++i) {
        source.words[i] = UINT64_C(0x9e3779b97f4a7c15) * (i + 1U);
    }
    for (uint64_t i = 0U; i < warmup; ++i) {
        if (bignum_swap(&destination, &source) != BIGNUM_SWAP_SUCCESS) return EXIT_FAILURE;
    }

    start = now_ns();
    for (uint64_t i = 0U; i < iterations; ++i) {
        if (bignum_swap(&destination, &source) != BIGNUM_SWAP_SUCCESS) return EXIT_FAILURE;
        checksum ^= destination.words[0] + destination.len + i;
    }
    end = now_ns();

    printf("benchmark=bignum_swap_st data_mode=%s src_len=%zu iterations=%" PRIu64
           " successful=%" PRIu64 " checksum=%" PRIu64
           " elapsed_seconds=%.9f ns_per_call=%.3f\n",
           data_mode, source.len, iterations, iterations, checksum,
           (double)(end - start) / 1000000000.0,
           (double)(end - start) / (double)iterations);
    return EXIT_SUCCESS;
}
