; -----------------------------------------------------------------------------
; @file    bignum_swap.asm
; @brief   x86-64 in-place bignum_t swap implementation.
; @version 0.1.0
; @details Revision 0.1.0: typed status API, validation and register-only swap.
; System V AMD64 ABI: rdi = a, rsi = b, eax = bignum_swap_status_t.
; -----------------------------------------------------------------------------
; SPDX-License-Identifier: MIT
; -----------------------------------------------------------------------------

default rel

section .text
    align 16
    global bignum_swap

BIGNUM_CAPACITY                     equ 32
BIGNUM_WORD_SIZE                    equ 8
BIGNUM_OFFSET_WORDS                 equ 0
BIGNUM_OFFSET_LEN                   equ BIGNUM_CAPACITY * BIGNUM_WORD_SIZE
BIGNUM_SWAP_SUCCESS                 equ 0
BIGNUM_SWAP_ERROR_NULL_ARG          equ -1
BIGNUM_SWAP_ERROR_OVERFLOW          equ -2

; bignum_swap_status_t bignum_swap(
;     bignum_t *restrict a, bignum_t *restrict b)
bignum_swap:
    ; Validate both pointers before any memory access.
    mov     eax, BIGNUM_SWAP_ERROR_NULL_ARG
    test    rdi, rdi
    jz      .ret
    test    rsi, rsi
    jz      .ret

    ; Exact self-swap is a successful no-op.
    cmp     rdi, rsi
    je      .success

    ; Validate both lengths before modifying either object.
    mov     eax, BIGNUM_SWAP_ERROR_OVERFLOW
    cmp     dword [rdi + BIGNUM_OFFSET_LEN], BIGNUM_CAPACITY
    ja      .ret
    cmp     dword [rsi + BIGNUM_OFFSET_LEN], BIGNUM_CAPACITY
    ja      .ret

    ; Swap all fixed-capacity words using only registers and object storage.
    xor     r8d, r8d
.words:
    mov     rax, qword [rdi + r8 * BIGNUM_WORD_SIZE]
    xchg    rax, qword [rsi + r8 * BIGNUM_WORD_SIZE]
    mov     qword [rdi + r8 * BIGNUM_WORD_SIZE], rax
    inc     r8d
    cmp     r8d, BIGNUM_CAPACITY
    jb      .words

    ; Swap the lengths after validation and word exchange.
    mov     eax, dword [rdi + BIGNUM_OFFSET_LEN]
    xchg    eax, dword [rsi + BIGNUM_OFFSET_LEN]
    mov     dword [rdi + BIGNUM_OFFSET_LEN], eax

.success:
    xor     eax, eax
.ret:
    ret

section .note.GNU-stack noalloc noexec nowrite progbits
