#include <string.h>
/******************************************************************************\
|* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
|* Copyright (c) 2024 by VeriSilicon Holdings Co., Ltd. ("VeriSilicon")       *|
|* All Rights Reserved.                                                       *|
|*                                                                            *|
|* The material in this file is confidential and contains trade secrets       *|
|* of VeriSilicon.  This is proprietary information owned or licensed by      *|
|* VeriSilicon.  No part of this work may be disclosed, reproduced, copied,   *|
|* transmitted, or used in any way for any purpose, without the express       *|
|* written permission of VeriSilicon.                                         *|
|*                                                                            *|
\******************************************************************************/

#include "oslayer.h"

/* Memory Pool Configuration */
#define OS_MAX_POOL_BLOCKS        128   /* Maximum number of allocations */
#define OS_MAX_BLOCK_SIZE         20480 /* Maximum size per allocation - INCREASED for MboxPostMsg (16432 bytes) */
#define OS_POOL_ALIGNMENT         8     /* Memory alignment */

/* Memory Pool Structure */
typedef struct {
	void *addr;      /* Block address */
	uint32_t size;      /* Block size */
	uint32_t used;      /* Usage flag */
} OsMemBlock_t;

/* Memory Pool Global Data */
static uint8_t g_memPool[OS_MAX_POOL_BLOCKS * OS_MAX_BLOCK_SIZE] __attribute__((aligned(
			OS_POOL_ALIGNMENT)));
static OsMemBlock_t g_memBlocks[OS_MAX_POOL_BLOCKS];
static uint32_t g_poolInitialized = 0;

/* Initialize memory pool */
static void osInitMemPool(void)
{
	uint32_t i;
	uint8_t *poolPtr = g_memPool;

	if (g_poolInitialized)
		return;

	for (i = 0; i < OS_MAX_POOL_BLOCKS; i++) {
		g_memBlocks[i].addr = poolPtr + (i * OS_MAX_BLOCK_SIZE);
		g_memBlocks[i].size = 0;
		g_memBlocks[i].used = 0;
	}

	g_poolInitialized = 1;
}

void *osMalloc(uint32_t size)
{
	uint32_t i;
	uint32_t alignedSize;
	uint32_t usedBlocks = 0;

	if (!g_poolInitialized)
		osInitMemPool();

	if (size == 0) {
		xil_printf("osMalloc: ERROR - Zero size requested\r\n");
		return NULL;
	}

	if (size > OS_MAX_BLOCK_SIZE) {
		xil_printf("osMalloc: ERROR - Size %lu exceeds max block size %d\r\n", size, OS_MAX_BLOCK_SIZE);
		return NULL;
	}

	/* Align size to OS_POOL_ALIGNMENT boundary */
	alignedSize = (size + OS_POOL_ALIGNMENT - 1) & ~(OS_POOL_ALIGNMENT - 1);

	/* Find free block */
	for (i = 0; i < OS_MAX_POOL_BLOCKS; i++) {
		if (!g_memBlocks[i].used) {
			g_memBlocks[i].used = 1;
			g_memBlocks[i].size = alignedSize;

			/* Clear the allocated memory */
			memset(g_memBlocks[i].addr, 0, alignedSize);

			/* Count used blocks for debugging */
			for (uint32_t j = 0; j < OS_MAX_POOL_BLOCKS; j++) {
				if (g_memBlocks[j].used)
					usedBlocks++;
			}

			return g_memBlocks[i].addr;
		}
	}

	/* Count used blocks for error reporting */
	for (i = 0; i < OS_MAX_POOL_BLOCKS; i++) {
		if (g_memBlocks[i].used)
			usedBlocks++;
	}

	/* No free blocks available */
	xil_printf("osMalloc: ERROR - Pool exhausted! Requested %lu bytes, used blocks: %lu/%d\r\n",
		   size, usedBlocks, OS_MAX_POOL_BLOCKS);
	osMemPoolPrintStats();
	return NULL;
}

int32_t osFree(void *p)
{
	uint32_t i;
	uint32_t usedBlocks = 0;

	if (p == NULL)
		return OSLAYER_OK;

	/* Find the block to free */
	for (i = 0; i < OS_MAX_POOL_BLOCKS; i++) {
		if (g_memBlocks[i].addr == p && g_memBlocks[i].used) {
			uint32_t freedSize = g_memBlocks[i].size;
			g_memBlocks[i].used = 0;
			g_memBlocks[i].size = 0;

			/* Count remaining used blocks */
			for (uint32_t j = 0; j < OS_MAX_POOL_BLOCKS; j++) {
				if (g_memBlocks[j].used)
					usedBlocks++;
			}

			return OSLAYER_OK;
		}
	}

	/* Block not found - this shouldn't happen in normal operation */
	return OSLAYER_ERROR;
}

int32_t osMemPoolGetStats(uint32_t *pUsedBlocks, uint32_t *pFreeBlocks,
			  uint32_t *pTotalMemory, uint32_t *pUsedMemory)
{
	uint32_t i;
	uint32_t usedBlocks = 0;
	uint32_t freeBlocks = 0;
	uint32_t usedMemory = 0;

	if (!g_poolInitialized)
		osInitMemPool();

	for (i = 0; i < OS_MAX_POOL_BLOCKS; i++) {
		if (g_memBlocks[i].used) {
			usedBlocks++;
			usedMemory += g_memBlocks[i].size;
		} else
			freeBlocks++;
	}

	if (pUsedBlocks)
		*pUsedBlocks = usedBlocks;
	if (pFreeBlocks)
		*pFreeBlocks = freeBlocks;
	if (pTotalMemory)
		*pTotalMemory = OS_MAX_POOL_BLOCKS * OS_MAX_BLOCK_SIZE;
	if (pUsedMemory)
		*pUsedMemory = usedMemory;

	return OSLAYER_OK;
}

int32_t osMemPoolPrintStats(void)
{
	uint32_t usedBlocks, freeBlocks, totalMemory, usedMemory;

	osMemPoolGetStats(&usedBlocks, &freeBlocks, &totalMemory, &usedMemory);

	xil_printf("Memory Pool Statistics:\r\n");
	xil_printf("  Used Blocks: %lu/%lu\r\n", usedBlocks, usedBlocks + freeBlocks);
	xil_printf("  Free Blocks: %lu\r\n", freeBlocks);
	xil_printf("  Used Memory: %lu bytes\r\n", usedMemory);
	xil_printf("  Total Memory: %lu bytes\r\n", totalMemory);
	xil_printf("  Memory Utilization: %lu%%\r\n",
		   totalMemory > 0 ? (usedMemory * 100) / totalMemory : 0);

	return OSLAYER_OK;
}

int32_t osMemPoolVerify(void)
{
	if (!g_poolInitialized)
		osInitMemPool();

	xil_printf("Memory Pool Verification:\r\n");
	xil_printf("  Pool Initialized: %s\r\n", g_poolInitialized ? "YES" : "NO");
	xil_printf("  Max Blocks: %d\r\n", OS_MAX_POOL_BLOCKS);
	xil_printf("  Max Block Size: %d bytes\r\n", OS_MAX_BLOCK_SIZE);
	xil_printf("  Total Pool Size: %d bytes\r\n", OS_MAX_POOL_BLOCKS * OS_MAX_BLOCK_SIZE);
	xil_printf("  Pool Address: 0x%08lX\r\n", (unsigned long)g_memPool);

	/* Test allocation and deallocation */
	void *testPtr = osMalloc(64);
	if (testPtr) {
		xil_printf("  Test Allocation: SUCCESS (64 bytes)\r\n");
		osFree(testPtr);
		xil_printf("  Test Deallocation: SUCCESS\r\n");
	} else {
		xil_printf("  Test Allocation: FAILED (64 bytes)\r\n");
		return OSLAYER_ERROR;
	}

	osMemPoolPrintStats();
	return OSLAYER_OK;
}

int32_t osSleep(uint32_t msec)
{

	if (msec < 1000)
		sleep(1);
	else
		sleep(msec / 1000);

	return OSLAYER_OK;
}


/*****************************************************************************/
/*  @brief  FreeRTOS platform (Software vitis, Hardware ZC702) */
#include "sleep.h"
BYTE convertMode(const char* mode)
{
	if (strcmp(mode, "r") == 0)
		return FA_READ;

	else if (strcmp(mode, "w") == 0)
		return FA_WRITE | FA_CREATE_ALWAYS;

	else if (strcmp(mode, "a") == 0)
		return FA_WRITE | FA_OPEN_APPEND;

	else if (strcmp(mode, "rb") == 0)
		return FA_READ;

	else if (strcmp(mode, "wb") == 0)
		return FA_WRITE | FA_CREATE_ALWAYS;

	else if (strcmp(mode, "ab") == 0)
		return FA_WRITE | FA_OPEN_APPEND;

	else if (strcmp(mode, "r+") == 0)
		return FA_READ | FA_WRITE;

	else if (strcmp(mode, "w+") == 0)
		return FA_READ | FA_WRITE | FA_CREATE_ALWAYS;

	else if (strcmp(mode, "w++") == 0)
		return FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_OPEN_APPEND;

	else if (strcmp(mode, "a+") == 0)
		return FA_READ | FA_WRITE | FA_OPEN_APPEND;

	else if (strcmp(mode, "r+b") == 0 || strcmp(mode, "rb+") == 0)
		return FA_READ | FA_WRITE;

	else if (strcmp(mode, "w+b") == 0 || strcmp(mode, "wb+") == 0)
		return FA_READ | FA_WRITE | FA_CREATE_ALWAYS;

	else if (strcmp(mode, "a+b") == 0 || strcmp(mode, "ab+") == 0)
		return FA_READ | FA_WRITE | FA_OPEN_APPEND;

	else
		return 0;
}

osFile *osFopen(const char *filename, const char *mode)
{
	osFile* pVsiFile = (osFile *)osMalloc(sizeof(osFile));
	if (pVsiFile == NULL) {
		printf("%s: pVsiFile allocate memory is failed\n", __func__);
		return NULL;
	}

	FRESULT res = f_open(&(pVsiFile->file), filename, convertMode(mode));
	if (res != FR_OK) {
		printf("%s: f_open is failed\n", __func__);
		osFree(pVsiFile);
		pVsiFile = NULL;
		return NULL;
	}
	return pVsiFile;
}

void osFclose(osFile* pVsiFile)
{
	if (pVsiFile != NULL) {
		f_close(&(pVsiFile->file));
		osFree(pVsiFile);
	} else
		printf("%s: pVsiFile is NULL\n", __func__);

}

size_t osFread(void* ptr, size_t size, size_t count, osFile* pVsiFile)
{
	if (pVsiFile == NULL) {
		printf("%s: pVsiFile is NULL\n", __func__);
		return -1;
	}

	unsigned int bytesRead;
	void *tempBuffer;
	tempBuffer = osMalloc(size * count);
	FRESULT res = f_read(&(pVsiFile->file), tempBuffer, size * count, &bytesRead);
	memcpy(ptr, tempBuffer, size * count);
	osFree(tempBuffer);
	if (res != FR_OK) {
		printf("%s: f_read is failed %d \n", __func__, res);
		return -1;
	}

	return bytesRead / size;
}

size_t osFwrite(const void* ptr, size_t size, size_t count, osFile* pVsiFile)
{
	if (pVsiFile == NULL) {
		printf("%s: pVsiFile is NULL\n", __func__);
		return -1;
	}

	unsigned int bytesWritten;
	FRESULT res = f_write(&(pVsiFile->file), ptr, size * count, &bytesWritten);
	if (res != FR_OK) {
		printf("%s: f_write is failed\n", __func__);
		return -1;
	}

	return bytesWritten / size;
}

int osFseek(osFile* pVsiFile, long int offset, int origin)
{
	if (pVsiFile == NULL) {
		printf("%s: pVsiFile is NULL\n", __func__);
		return -1;
	}
	if (origin == SEEK_END) {
		uint32_t NumBytesRead = 1;
		uint32_t fileReadLen = 0;
		char readBuf[256] = {0};
		while (NumBytesRead) {
			f_read(&(pVsiFile->file), (void*)readBuf, 256, &NumBytesRead);
			fileReadLen += NumBytesRead;
		}
		offset = fileReadLen + offset;
	} else if (origin == SEEK_CUR) {
		long int pos = f_tell(&(pVsiFile->file));
		offset = pos + offset;
	}

	FRESULT res = f_lseek(&(pVsiFile->file), offset);
	if (res != FR_OK) {
		printf("%s: f_lseek is failed\n", __func__);
		return -1;
	}

	return 0;
}

long int osFtell(osFile* pVsiFile)
{
	if (pVsiFile == NULL) {
		printf("%s: pVsiFile is NULL\n", __func__);
		return -1;
	}

	long int pos = f_tell(&(pVsiFile->file));

	return pos;
}

long int osFileSize(osFile* pVsiFile)
{
	if (pVsiFile == NULL) {
		printf("%s: pVsiFile is NULL\n", __func__);
		return -1;
	}
	long int size = f_size(&(pVsiFile->file));

	return size;
}

int osFeof(osFile* pVsiFile)
{
	return f_eof(&(pVsiFile->file));
}

int osFerror(osFile* pVsiFile)
{
	return f_error(&(pVsiFile->file));
}

int osFgetc(osFile* pVsiFile)
{
	if (pVsiFile == NULL) {
		printf("%s: Invalid parameter\n", __func__);
		return -1;
	}

	unsigned int bytesRead;
	char tempBuffer;
	FRESULT result = f_read(&(pVsiFile->file), (void*)&tempBuffer, sizeof(char), &bytesRead);
	if (result != FR_OK) {
		if (!osFeof(pVsiFile)) {
			printf("%s: f_read error when reading\n", __func__);
			return -1;
		}
		if (bytesRead != 1) {
			printf("%s: f_read reaches end-of-file\n", __func__);
			return EOF;
		}
	}

	return (int)tempBuffer;
}

char *osFgets(char* str, int size, osFile* pVsiFile)
{
	if (str == NULL || size <= 0 || pVsiFile == NULL) {
		printf("%s: pVsiFile or str or size is NULL\n", __func__);
		return NULL;
	}

	char *result = f_gets(str, size, &(pVsiFile->file));
	if (result == NULL)
		return NULL;
	return result;
}

int osFputs(const char* str, osFile* pVsiFile)
{
	if (pVsiFile == NULL) {
		printf("%s: Invalid parameter\n", __func__);
		return -1;
	}

	return f_puts(str, &(pVsiFile->file));
}

int osFscanf(osFile* pVsiFile, const char* format, ...)
{
	va_list args;
	int result = EOF;

	va_start(args, format);

	char buffer[512];
	memset(buffer, 0, sizeof(buffer));

	if (f_gets(buffer, sizeof(buffer), &(pVsiFile->file)) != NULL)
		result = vsscanf(buffer, format, args);

	va_end(args);

	return result;
}

int osFprintf(osFile* pVsiFile, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[512];
	memset(buffer, 0, sizeof(buffer));

	int written_chars = vsnprintf(buffer, sizeof(buffer), format, args);

	if (written_chars > 0)
		f_write(&(pVsiFile->file), (char *)buffer, written_chars, NULL);

	else
		printf("%s Error in formatting the string\r\n", __func__);

	va_end(args);

	return written_chars;
}

char *osDirname(const char *path, char *Dir)
{
	if (path == NULL) {
		printf("%s: Invalid parameter\n", __func__);
		return NULL;
	}
	char *lastSlash = strrchr(path, '/');
	if (lastSlash == NULL) {
		Dir[0] = '.';
		Dir[1] = '\0';
		return Dir;
	}
	if (lastSlash == path) {
		Dir[0] = '/';
		Dir[1] = '\0';
		return Dir;
	}
	size_t len = lastSlash - path;
	strncpy(Dir, path, len);
	Dir[len] = '\0';
	return Dir;
}

void osUSleep(unsigned int microseconds)
{
	usleep(microseconds);
}

int32_t osTimeStampUs(int64_t *pTimeStamp)
{
	return OSLAYER_OK;
}

/******************************************************************************
 *  osTimeStampNs()
 *****************************************************************************/
/**
 *  @brief  Returns a 64-bit timestamp [ns]
 *
 *  @param   pTimeStamp       Reference of the timestamp object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Event successfully created
 *
 ******************************************************************************/
int32_t osTimeStampNs(float32_t *pTimeStamp)
{
	return OSLAYER_OK;
}
