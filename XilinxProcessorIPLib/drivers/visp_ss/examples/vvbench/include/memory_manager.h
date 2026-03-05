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
void *mm_aligned_malloc(size_t size, size_t alignment, struct aligned_buf *metadata);
void mm_print_stats(void);
void mm_free(struct aligned_buf *buf);

#endif /* MEMORY_MANAGER_H */
