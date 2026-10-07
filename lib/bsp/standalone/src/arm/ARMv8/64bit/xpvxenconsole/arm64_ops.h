/*
Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
Copyright DornerWorks 2016

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
following conditions are met:
1.	 Redistributions of source code must retain the above copyright notice, this list of conditions and the
following disclaimer.

THIS SOFTWARE IS PROVIDED BY DORNERWORKS FOR USE ON THE CONTRACTED PROJECT, AND ANY EXPRESS OR IMPLIED WARRANTY
IS LIMITED TO THIS USE. FOR ALL OTHER USES THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DORNERWORKS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
* @file arm64_ops.h
* @brief ARM64 low-level operations for Xen hypercalls.
*/

#ifndef _ARM64_OPS_H_
#define _ARM64_OPS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** @brief Trigger a fatal assertion failure by executing an undefined instruction. */
#define BUG() while(1){__asm volatile (".word 0xe7f000f0\n");} /* Undefined instruction; will call our fault handler. */
/** @brief Assert that a condition holds; halts with a message if false.
 * @param [in] x Condition that must be true.
 */
#define ASSERT(x)                                              \
	do {                                                           \
		if (!(x)) {                                                \
			printk("ASSERTION FAILED: %s at %s:%d.\n",             \
			       # x ,                                           \
			       __FILE__,                                       \
			       __LINE__);                                      \
			BUG();                                                 \
		}                                                          \
	} while(0)

/** @brief Halt if the given condition is true.
 * @param [in] x Condition to test; halts if non-zero.
 */
#define BUG_ON(x) ASSERT(!(x))

/** @brief Return the current processor ID (always 0 in bare-metal). */
#define smp_processor_id() 0

/** @brief Compiler memory barrier; prevents instruction reordering by the compiler. */
#define barrier() __asm__ __volatile__("": : :"memory")

// disable interrupts
/** @brief Disable local IRQs on the current CPU. */
static inline void local_irq_disable(void)
{
	__asm__ __volatile__("cpsid i":::"memory");
}

// enable interrupts
/** @brief Enable local IRQs on the current CPU. */
static inline void local_irq_enable(void)
{
	__asm__ __volatile__("cpsie i":::"memory");
}

/** @brief Save IRQ flags to x and disable local IRQs.
 * @param [in] x Variable to save the CPU flags into.
 */
#define local_irq_save(x) { \
		__asm__ __volatile__("mrs %0, cpsr;cpsid i":"=r"(x)::"memory");    \
	}

/** @brief Restore IRQ flags previously saved by local_irq_save().
 * @param [in] x Previously saved CPU flags to restore.
 */
#define local_irq_restore(x) {    \
		__asm__ __volatile__("msr cpsr_c, %0"::"r"(x):"memory");    \
	}

/** @brief Save current IRQ flags to x without disabling IRQs.
 * @param [in] x Variable to save the CPU flags into.
 */
#define local_save_flags(x)    { \
		__asm__ __volatile__("mrs %0, cpsr":"=r"(x)::"memory");    \
	}

/** @brief Return non-zero if local IRQs are currently disabled. */
static inline int irqs_disabled(void)
{
	int x;
	local_save_flags(x);
	return x & 0x80;
}

/** @brief Data Synchronisation Barrier; ensures memory accesses complete.
 * @param [in] scope DSB scope (sy, ld, st, etc.).
 */
#define dsb(scope)      __asm volatile("dsb " #scope : : : "memory")

/* We probably only need "dmb" here, but we'll start by being paranoid. */
/** @brief Full memory barrier; prevents load/store reordering. */
#define mb()  dsb(sy)
/** @brief Read memory barrier; prevents load reordering. */
#define rmb() dsb(ld)
/** @brief Write memory barrier; prevents store reordering. */
#define wmb() dsb(st)

/** @brief Atomically exchange the value at ptr with v.
 * @param [in] ptr Pointer to the location to exchange.
 * @param [in] v New value to write.
 */
#define xchg(ptr,v) __atomic_exchange_n(ptr, v, __ATOMIC_SEQ_CST)

/**
 * test_and_clear_bit - Clear a bit and return its old value
 * @param[in] nr: Bit to clear
 * @param[in] addr: Address to count from
 *
 * Note that nr may be almost arbitrarily large; this function is not
 * restricted to acting on a single-word quantity.
 *
 * This operation is atomic.
 * If you need a memory barrier, use synch_test_and_clear_bit instead.
 */
static __inline__ int test_and_clear_bit(int nr, volatile void *addr)
{
	uint8_t *byte = ((uint8_t *)addr) + (nr >> 3);
	uint8_t bit = 1 << (nr & 7);
	uint8_t orig;

	orig = __atomic_fetch_and(byte, ~bit, __ATOMIC_RELAXED);

	return (orig & bit) != 0;
}

/**
 * Atomically set a bit and return the old value.
 * Similar to test_and_clear_bit.
 */
static __inline__ int test_and_set_bit(int nr, volatile void *base)
{
	uint8_t *byte = ((uint8_t *)base) + (nr >> 3);
	uint8_t bit = 1 << (nr & 7);
	uint8_t orig;

	orig = __atomic_fetch_or(byte, bit, __ATOMIC_RELAXED);

	return (orig & bit) != 0;
}

/**
 * Test whether a bit is set. */
static __inline__ int test_bit(int nr, const volatile unsigned long *addr)
{
	const uint8_t *ptr = (const uint8_t *) addr;
	return ((1 << (nr & 7)) & (ptr[nr >> 3])) != 0;
}

/**
 * Atomically set a bit in memory (like test_and_set_bit but discards result).
 */
static __inline__ void set_bit(int nr, volatile unsigned long *addr)
{
	test_and_set_bit(nr, addr);
}

/**
 * Atomically clear a bit in memory (like test_and_clear_bit but discards result).
 */
static __inline__ void clear_bit(int nr, volatile unsigned long *addr)
{
	test_and_clear_bit(nr, addr);
}

/**
 * __ffs - find first (lowest) set bit in word.
 * @param[in] word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __inline__ unsigned long __ffs(unsigned long word)
{
	return __builtin_ctzl(word);
}

/********************* common arm32 and arm64  ****************************/

/* If *ptr == old, then store new there (and return new).
 * Otherwise, return the old value.
 * Atomic. */
/** @brief Atomic compare-and-exchange; stores new if *ptr == old.
 * @param [in] ptr Pointer to the value to compare and swap.
 * @param [in] old Expected current value.
 * @param [in] new Value to store if *ptr equals old.
 */
#define synch_cmpxchg(ptr, old, new) \
	({ __typeof__(*ptr) stored = old; \
		__atomic_compare_exchange_n(ptr, &stored, new, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST) ? new : old; \
	})

/* As test_and_clear_bit, but using __ATOMIC_SEQ_CST */
/**
 * @brief Atomically clear a bit and return the old value (sequential consistency).
 * @param[in] nr   Bit number to clear.
 * @param[in] addr Base address of the bit array.
 * @return Previous value of the bit (non-zero if it was set, 0 if clear).
 */
static __inline__ int synch_test_and_clear_bit(int nr, volatile void *addr)
{
	uint8_t *byte = ((uint8_t *)addr) + (nr >> 3);
	uint8_t bit = 1 << (nr & 7);
	uint8_t orig;

	orig = __atomic_fetch_and(byte, ~bit, __ATOMIC_SEQ_CST);

	return (orig & bit) != 0;
}

/**
 * @brief Atomically set a bit and return the old value (sequential consistency).
 * @param[in] nr Bit number to set.
 * @param[in] base Base address of the bit array.
 * @return Previous value of the bit (0 or non-zero).
 */
static __inline__ int synch_test_and_set_bit(int nr, volatile void *base)
{
	uint8_t *byte = ((uint8_t *)base) + (nr >> 3);
	uint8_t bit = 1 << (nr & 7);
	uint8_t orig;

	orig = __atomic_fetch_or(byte, bit, __ATOMIC_SEQ_CST);

	return (orig & bit) != 0;
}

/**
 * @brief Atomically set a bit (sequential consistency); discards old value.
 * @param[in] nr Bit number to set.
 * @param[in] addr Base address of the bit array.
 */
static __inline__ void synch_set_bit(int nr, volatile void *addr)
{
	synch_test_and_set_bit(nr, addr);
}

/**
 * @brief Atomically clear a bit (sequential consistency); discards old value.
 * @param[in] nr Bit number to clear.
 * @param[in] addr Base address of the bit array.
 */
static __inline__ void synch_clear_bit(int nr, volatile void *addr)
{
	synch_test_and_clear_bit(nr, addr);
}

/**
 * @brief Test a bit with a following memory barrier.
 * @param[in] nr Bit number to test.
 * @param[in] addr Base address of the bit array.
 * @return Non-zero if the bit is set, 0 otherwise.
 */
static __inline__ int synch_test_bit(int nr, volatile void *addr)
{
	int result;
	result = test_bit(nr, addr);
	barrier();
	return result;
}
#endif

#ifdef __cplusplus
}
#endif
