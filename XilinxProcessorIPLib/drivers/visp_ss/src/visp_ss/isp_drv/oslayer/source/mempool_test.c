/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")     *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets of    *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "oslayer.h"
#include "xil_printf.h"

/**
 * @brief Test memory pool allocation patterns similar to actual usage
 */
int32_t osMemPoolStressTest(void)
{
	void *ptrs[32];  /* Test up to 32 allocations */
	uint32_t sizes[] = {64, 128, 256, 512, 1024, 2048};  /* Common allocation sizes */
	uint32_t numSizes = sizeof(sizes) / sizeof(sizes[0]);
	uint32_t allocCount = 0;
	uint32_t i, j;

	xil_printf("\n=== Memory Pool Stress Test ===\n");

	/* Initialize all pointers */
	for (i = 0; i < 32; i++)
		ptrs[i] = NULL;

	/* Verify pool before test */
	if (osMemPoolVerify() != OSLAYER_OK) {
		xil_printf("ERROR: Memory pool verification failed\n");
		return OSLAYER_ERROR;
	}

	/* Test various allocation sizes */
	for (j = 0; j < numSizes && allocCount < 32; j++) {
		uint32_t size = sizes[j];
		xil_printf("\nTesting allocation size: %lu bytes\n", size);

		/* Try to allocate 5 blocks of this size */
		for (i = 0; i < 5 && allocCount < 32; i++) {
			ptrs[allocCount] = osMalloc(size);
			if (ptrs[allocCount] == NULL) {
				xil_printf("FAILED: Allocation %lu of size %lu failed\n", allocCount, size);
				break;
			}
			allocCount++;
		}
	}

	xil_printf("\nSuccessfully allocated %lu blocks\n", allocCount);
	osMemPoolPrintStats();

	/* Free all allocated blocks */
	xil_printf("\nFreeing all allocated blocks...\n");
	for (i = 0; i < allocCount; i++) {
		if (ptrs[i] != NULL) {
			osFree(ptrs[i]);
			ptrs[i] = NULL;
		}
	}

	xil_printf("\nFinal pool state:\n");
	osMemPoolPrintStats();

	return OSLAYER_OK;
}

/**
 * @brief Simple memory pool test - call this early in your application
 */
int32_t osMemPoolQuickTest(void)
{
	xil_printf("\n=== Quick Memory Pool Test ===\n");

	/* Verify basic functionality */
	return osMemPoolVerify();
}
