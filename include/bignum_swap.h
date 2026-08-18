/* ------------------------------------------------------------------ */
/**
 * @file    bignum_swap.h
 * @brief   In-place обмен двух значений bignum_t.
 * @version 0.1.0
 * @details
 *   Revision 0.1.0: initial typed API and in-place swap contract.
 */
/* ------------------------------------------------------------------ */
#pragma once
#ifndef BIGNUM_SWAP_H
#define BIGNUM_SWAP_H

#include <stddef.h>
#include <stdint.h>

#include "bignum.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Результаты выполнения bignum_swap. */
typedef enum {
    BIGNUM_SWAP_SUCCESS        = 0,
    BIGNUM_SWAP_ERROR_NULL_ARG = -1,
    BIGNUM_SWAP_ERROR_OVERFLOW = -2
} bignum_swap_status_t;

/**
 * @brief Обменивает два значения bignum_t in-place.
 *
 * При успешном вызове меняются местами все поля объектов, включая `len` и
 * весь фиксированный массив слов. Дополнительная динамическая память не
 * выделяется; реализации используют временные регистры.
 *
 * NULL-аргументы возвращают `BIGNUM_SWAP_ERROR_NULL_ARG`. Некорректные
 * значения `len > BIGNUM_CAPACITY` возвращают `BIGNUM_SWAP_ERROR_OVERFLOW` и
 * не изменяют ни один объект. Exact self-swap (`a == b`) является успешной
 * no-op операцией. Объекты должны быть корректными normalized bignum_t.
 *
 * Для non-identical объектов действует `restrict` contract: области объектов
 * не должны частично перекрываться. Partial overlap не поддерживается.
 *
 * @param[in,out] a Первый объект.
 * @param[in,out] b Второй объект.
 * @return `BIGNUM_SWAP_SUCCESS`, `BIGNUM_SWAP_ERROR_NULL_ARG` или
 *         `BIGNUM_SWAP_ERROR_OVERFLOW`.
 */
bignum_swap_status_t bignum_swap(
    bignum_t *restrict a,
    bignum_t *restrict b);

#ifdef __cplusplus
}
#endif

#endif /* BIGNUM_SWAP_H */

/* SPDX-License-Identifier: MIT */
