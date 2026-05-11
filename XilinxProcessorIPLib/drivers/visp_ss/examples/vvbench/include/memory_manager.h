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

#endif /* MEMORY_MANAGER_H */
