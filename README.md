# bignum-swap

[![C/ASM CI](https://github.com/kirill-bayborodov/bignum-swap/actions/workflows/ci.yml/badge.svg)](https://github.com/kirill-bayborodov/bignum-swap/actions/workflows/ci.yml)
[![GitHub release](https://img.shields.io/github/v/release/kirill-bayborodov/bignum-swap?label=release)](https://github.com/kirill-bayborodov/bignum-swap/releases/latest)

`bignum-swap` is a standalone C/ASM module that exchanges two normalized `bignum_t` objects in-place. The representation uses a fixed little-endian array of 64-bit words and a `len` field containing the number of used words. The production path is an x86-64 YASM implementation conforming to the System V AMD64 ABI; a portable C11 implementation is retained as a reference and fallback.

The operation swaps the complete fixed-capacity word arrays and the length fields without dynamic allocation or a temporary heap buffer. Both objects are validated before any mutation. The API returns a typed `bignum_swap_status_t` result. NULL arguments and an invalid length greater than `BIGNUM_CAPACITY` are rejected. Exact self-swap is a successful no-op. Partial overlap is outside the `restrict` contract.

## Distribution

The module is intended to be used as a standalone component of the `bignum-lib` family. The required `bignum-core` component is included as a Git submodule at `libs/bignum-core`.

## Features

- **Dual implementation:** x86-64 YASM is the primary implementation and C11 is the portable reference implementation.
- **Typed status API:** `bignum_swap(bignum_t *restrict a, bignum_t *restrict b)` returns success, NULL-argument, or overflow status codes.
- **In-place operation:** all words and `len` are exchanged using registers and object storage; no dynamic memory is allocated.
- **Full-state exchange:** the fixed-capacity word array and length fields are both exchanged, including unused words.
- **Validation before mutation:** NULL pointers and oversized lengths are rejected before either object changes.
- **Exact self-swap protection:** `a == b` returns success without reading or writing the object.
- **Self-contained ASM:** the YASM path has no external library calls and uses register-level `xchg` operations.
- **Deterministic verification:** tests cover ordinary values, zero length, capacity boundary, self-swap, NULL status, overflow status, and full-state preservation.
- **Extended verification:** 10,000 randomized swaps, invalid-length cases, and 100,000 exact self-swap stress operations are covered.
- **Thread-safety testing:** independent object pairs are swapped concurrently in multiple threads.
- **Reproducible benchmarks:** single-thread and multithread benchmarks report data mode, iteration counts, checksums, elapsed time, and nanoseconds per call.
- **Perf workflow:** the unchanged template Makefile provides benchmark, runtime validation, repeated `perf stat`, raw profile retention, and comparison targets.

## Dependencies

| Dependency | Purpose |
|---|---|
| `make` | Build, test, lint, benchmark, and distribution targets |
| `gcc` | C compilation and linking |
| `yasm` | x86-64 assembly compilation |
| `cppcheck` | Static analysis |
| `perf` | Performance counters and sampling profiles |
| `taskset` | CPU affinity control |
| `pthread` | Multithreaded tests and benchmarks |

Clone the repository with its submodule:

```bash
git clone --recurse-submodules https://github.com/kirill-bayborodov/bignum-swap.git
cd bignum-swap
```

For an existing clone, initialize the submodule with:

```bash
git submodule update --init --recursive
```

## API

The public API is declared in `include/bignum_swap.h`:

```c
typedef enum {
    BIGNUM_SWAP_SUCCESS        = 0,
    BIGNUM_SWAP_ERROR_NULL_ARG = -1,
    BIGNUM_SWAP_ERROR_OVERFLOW = -2
} bignum_swap_status_t;

bignum_swap_status_t bignum_swap(
    bignum_t *restrict a,
    bignum_t *restrict b);
```

### Contract

| Condition | Result | Object behavior |
|---|---|---|
| Both objects are valid and non-overlapping | `BIGNUM_SWAP_SUCCESS` | Complete words and `len` fields are exchanged |
| `a == b` | `BIGNUM_SWAP_SUCCESS` | No memory is accessed; the object is unchanged |
| `a == NULL` or `b == NULL` | `BIGNUM_SWAP_ERROR_NULL_ARG` | Neither object is modified |
| `a->len > BIGNUM_CAPACITY` or `b->len > BIGNUM_CAPACITY` | `BIGNUM_SWAP_ERROR_OVERFLOW` | Neither object is modified |
| `a` and `b` partially overlap | Not supported | The `restrict` contract is violated |
| Unused words | Included in the operation | All fixed-capacity words are exchanged |

A successful call leaves the first object byte-for-byte equal to the previous second object and the second object byte-for-byte equal to the previous first object. The source/destination terminology used by copy modules does not apply: both parameters are mutable swap operands.

The normalized representation requires `len <= BIGNUM_CAPACITY`, `len == 0` for zero, and a nonzero highest used word when `len > 0`. The function validates capacity but leaves normalization of nonzero words to the caller.

Example:

```c
#include "bignum_swap.h"

bignum_swap_status_t exchange_values(
    bignum_t *restrict left,
    bignum_t *restrict right)
{
    return bignum_swap(left, right);
}
```

After `BIGNUM_SWAP_SUCCESS`, every word and the `len` field of `left` contain the corresponding pre-call value of `right`, and vice versa. No heap allocation is required.

## Build and test

Build the release object and submodule:

```bash
make build CONFIG=release
```

The production object is generated at:

```text
build/bignum_swap.o
```

Run the full deterministic, extended, multithreaded, and integration-runner suite against ASM:

```bash
make test CONFIG=release USE_ASM=yes
```

The expected summary is:

```text
=== Summary: 0 / 4 failed ===
```

To test the portable C implementation:

```bash
make clean
make test CONFIG=release USE_ASM=no
```

Run static analysis:

```bash
make lint
```

Optional sanitizer and race-oriented checks:

```bash
make test_sanitize CONFIG=release SAN=address USE_ASM=no
make test_sanitize CONFIG=release SAN=undefined USE_ASM=no
make test_helgrind CONFIG=release
```

For leak checking of the non-sanitized C test binaries:

```bash
make clean
make test CONFIG=release USE_ASM=no
for binary in bin/test_bignum_swap bin/test_bignum_swap_extra bin/test_bignum_swap_mt bin/test_bignum_swap_runner; do
  valgrind --leak-check=full --show-leak-kinds=all \
    --errors-for-leak-kinds=all --error-exitcode=1 "$binary"
done
```

The test files are organized as follows:

| File | Scope |
|---|---|
| `tests/test_bignum_swap.c` | Deterministic full-state exchange, self-swap, NULL/overflow statuses, zero and capacity tests |
| `tests/test_bignum_swap_extra.c` | 10,000 randomized swaps, invalid lengths, and 100,000 self-swap stress operations |
| `tests/test_bignum_swap_mt.c` | Concurrent independent object-pair swaps |
| `tests/test_bignum_swap_runner.c` | Public-header integration smoke test |

The tests invoke and stress exact alias (`a == b`) because it is a supported successful no-op. A dedicated diagnostic constructs two partially overlapping layouts and verifies the overlap geometry, but intentionally does not call the function: partial object overlap violates the `restrict` contract and is not a supported API case.

## Benchmarks

The benchmark sources are:

```text
benchmarks/bench_bignum_swap.c
benchmarks/bench_bignum_swap_mt.c
```

Each benchmark exchanges two deterministic objects and reports data mode, iteration count, successful-call count, checksum, elapsed time, and nanoseconds per call. The workload includes a warm-up phase and uses no heap allocation inside the swap operation.

### Single-thread CLI

```text
bin/bench_bignum_swap [--iterations N] [--warmup N] [--data-mode all_zero|all_nonzero|mixed]
```

Example:

```bash
./bin/bench_bignum_swap --iterations 1000000 --warmup 10000 --data-mode all_nonzero
```

### Multithread CLI

```text
bin/bench_bignum_swap_mt [--threads N] [--iterations N|--total-iterations N] [--warmup N] [--data-mode all_zero|all_nonzero|mixed]
```

Example:

```bash
./bin/bench_bignum_swap_mt --threads 2 --total-iterations 1000000 --warmup 10000 --data-mode mixed
```

Each worker owns an independent pair of bignum objects. Consequently, the benchmark measures concurrent in-place swaps without sharing mutable operands between threads.

For a fair one-thread/two-thread comparison, hold data mode, measured iterations, warm-up count, and object construction constant:

```bash
./bin/bench_bignum_swap_mt --threads 1 --total-iterations 1000000 --warmup 10000 --data-mode all_nonzero
./bin/bench_bignum_swap_mt --threads 2 --total-iterations 1000000 --warmup 10000 --data-mode all_nonzero
```

## Perf workflow

The current environment provides two logical CPUs. The corresponding MT settings are:

```make
MT_THREADS=2
MT_CPU_LIST=0-1
MT_TOTAL_ITERATIONS=3200000000
```

Run the complete ST/MT workflow:

```bash
make bench_full CONFIG=release \
  REPORT_NAME=baseline \
  PERF_RUNS=7 \
  KEEP_PERF=1
```

For targeted repeated counter measurements:

```bash
make bench_stat_st CONFIG=release REPORT_NAME=baseline_st PERF_RUNS=7
make bench_stat_mt CONFIG=release REPORT_NAME=baseline_mt \
  MT_THREADS=2 MT_CPU_LIST=0-1 \
  MT_TOTAL_ITERATIONS=3200000000 PERF_RUNS=7
```

Reports are written to `benchmarks/reports/`. With `KEEP_PERF=1`, raw profiles are retained as `.perf.data` files. Runtime validation checks the benchmark identifier, elapsed-time field, and successful-call count.

A reproducible optimization comparison must keep configuration, data mode, object construction, thread count, CPU affinity, warm-up count, and total iterations constant:

```bash
make clean
make test CONFIG=release USE_ASM=yes
make bench_full CONFIG=release REPORT_NAME=baseline PERF_RUNS=7 KEEP_PERF=1

# Change the implementation, then repeat the verification.
make clean
make test CONFIG=release USE_ASM=yes
make bench_full CONFIG=release REPORT_NAME=opt_v1 PERF_RUNS=7 KEEP_PERF=1
```

## Installation and distribution

Build the object-file distribution:

```bash
make install CONFIG=release
```

Build the single-header and static-library distribution:

```bash
make dist CONFIG=release
```

Remove generated artifacts:

```bash
make clean
```

The object-file distribution contains `bignum_swap.h`, the bundled `bignum-core` declarations, and the object file required by the module. The `dist` target additionally creates the single-header distribution and `libbignum_swap.a`, together with the license, README, and integration runner.

## Linking the object file

```bash
make build CONFIG=release

gcc your_app.c \
  build/bignum_swap.o \
  -I./include \
  -I./libs/bignum-core/include \
  -o your_app \
  -no-pie
```

The application must use the System V AMD64 ABI and include the `bignum_t` definition supplied by `bignum-core`. The ASM object is self-contained and does not call libc copy routines.

## Contributing

Contributions should preserve the typed status API, full-state exchange semantics, exact self-swap behavior, validation status codes, normalized-input preconditions, and the no-partial-overlap rule. Every behavior change must include deterministic tests and, where appropriate, randomized stress coverage. Changes should run both ASM and C test configurations, `make lint`, sanitizer checks, and the multithread test.

Performance changes should include reproducible benchmark parameters, matching single-thread and multithread evidence where applicable, and an explanation of the affected ASM hot path. The Makefile is part of the repository template and must not be modified without direct authorization.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
