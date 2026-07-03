// Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
/* Header for memory_manager - implementation provided by libvisp.a */
#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stddef.h>
#include <stdint.h>

/* Aligned buffer structure */
struct aligned_buf {
	void *original_addr;    /* Original malloc address for freeing */
	void *aligned_addr;     /* Aligned address for hardware */
	uint32_t size;
	uint32_t alignment;
};

/* Memory manager functions provided by libvisp.a */
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
void *mm_aligned_malloc(size_t size, size_t alignment, struct aligned_buf *metadata);
void mm_print_stats(void);

/* ISP buffer-pool memory-region helpers (see BUFFER_POOL_REGION in lscript.ld).
 * The high 32 bits of the buffer-pool physical address select the addressing
 * mode that must be sent to the RPU / programmed into the HW DMA upper word:
 *   0x8 = high memory (DDR_LOW_1, ATM), 0x0 = low / 32-bit memory (DDR_LOW_0).
 * Both values auto-derive from where the linker places the buffer pools. */
uint64_t mm_get_high_mem_prefix(void);
int mm_is_high_mem(void);

#endif /* MEMORY_MANAGER_H */
