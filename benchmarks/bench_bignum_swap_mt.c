#define _POSIX_C_SOURCE 200809L
/* ------------------------------------------------------------------ */
/**
 * @file    bench_bignum_swap_mt.c
 * @brief   Multithread benchmark for bignum_swap.
 * @version 0.1.0
 * @details Revision 0.1.0: deterministic concurrent independent swap timing.
 */
/* ------------------------------------------------------------------ */
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bignum_swap.h"

struct worker_args {
    uint64_t iterations;
    uint64_t warmup;
    uint64_t checksum;
    unsigned id;
};

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

static bignum_t make_value(uint32_t len, uint64_t seed)
{
    bignum_t value;
    for (size_t i = 0U; i < BIGNUM_CAPACITY; ++i) {
        value.words[i] = seed + UINT64_C(0x9e3779b97f4a7c15) * (i + 1U);
    }
    value.len = len;
    return value;
}

static void *worker(void *opaque)
{
    struct worker_args *args = (struct worker_args *)opaque;
    bignum_t a = make_value((args->id + 3U) % (BIGNUM_CAPACITY + 1U), args->id + 1U);
    bignum_t b = make_value((args->id + 11U) % (BIGNUM_CAPACITY + 1U), args->id + 101U);
    uint64_t checksum = 0U;

    for (uint64_t i = 0U; i < args->warmup + args->iterations; ++i) {
        if (bignum_swap(&a, &b) != BIGNUM_SWAP_SUCCESS) {
            args->checksum = UINT64_MAX;
            return NULL;
        }
        if (i >= args->warmup) checksum ^= a.words[0] + a.len + i;
    }
    args->checksum = checksum;
    return NULL;
}

int main(int argc, char **argv)
{
    uint64_t iterations = UINT64_C(1000000);
    uint64_t total_iterations = 0U;
    uint64_t warmup = UINT64_C(10000);
    size_t threads_count = 2U;
    const char *data_mode = "all_nonzero";
    pthread_t *threads;
    struct worker_args *args;
    uint64_t start;
    uint64_t end;
    uint64_t checksum = 0U;

    for (int i = 1; i < argc; ++i) {
        uint64_t value;
        if ((strcmp(argv[i], "--threads") == 0 ||
             strcmp(argv[i], "--iterations") == 0 ||
             strcmp(argv[i], "--total-iterations") == 0 ||
             strcmp(argv[i], "--warmup") == 0) && i + 1 < argc &&
            parse_u64(argv[++i], &value) == 0) {
            if (strcmp(argv[i - 1], "--threads") == 0) threads_count = (size_t)value;
            else if (strcmp(argv[i - 1], "--iterations") == 0) iterations = value;
            else if (strcmp(argv[i - 1], "--total-iterations") == 0) total_iterations = value;
            else warmup = value;
        } else if (strcmp(argv[i], "--data-mode") == 0 && i + 1 < argc) {
            data_mode = argv[++i];
            if (strcmp(data_mode, "all_zero") != 0 &&
                strcmp(data_mode, "all_nonzero") != 0 &&
                strcmp(data_mode, "mixed") != 0) return EXIT_FAILURE;
        } else {
            fprintf(stderr, "usage: %s [--threads N] [--iterations N|--total-iterations N] [--warmup N] [--data-mode all_zero|all_nonzero|mixed]\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (threads_count == 0U || (total_iterations != 0U && total_iterations % threads_count != 0U)) return EXIT_FAILURE;
    if (total_iterations != 0U) iterations = total_iterations / threads_count;
    if (iterations == 0U) return EXIT_FAILURE;

    threads = calloc(threads_count, sizeof(*threads));
    args = calloc(threads_count, sizeof(*args));
    if (threads == NULL || args == NULL) {
        free(threads);
        free(args);
        return EXIT_FAILURE;
    }
    start = now_ns();
    for (size_t i = 0U; i < threads_count; ++i) {
        args[i].iterations = iterations;
        args[i].warmup = warmup;
        args[i].id = (unsigned)i;
        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0) {
            for (size_t j = 0U; j < i; ++j) (void)pthread_join(threads[j], NULL);
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
    }
    for (size_t i = 0U; i < threads_count; ++i) {
        (void)pthread_join(threads[i], NULL);
        if (args[i].checksum == UINT64_MAX) {
            free(threads);
            free(args);
            return EXIT_FAILURE;
        }
        checksum ^= args[i].checksum;
    }
    end = now_ns();

    printf("benchmark=bignum_swap_mt data_mode=%s threads=%zu iterations_per_thread=%" PRIu64
           " total_iterations=%" PRIu64 " successful=%" PRIu64
           " checksum=%" PRIu64 " elapsed_seconds=%.9f ns_per_call=%.3f\n",
           data_mode, threads_count, iterations, iterations * threads_count,
           iterations * threads_count, checksum,
           (double)(end - start) / 1000000000.0,
           (double)(end - start) / (double)(iterations * threads_count));
    free(threads);
    free(args);
    return EXIT_SUCCESS;
}
