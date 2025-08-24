/****************************************************************************
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * Copyright (c) 2014-2022 Vivante Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 ****************************************************************************/

/* VeriSilicon 2020 */

/**
 * @file    oslayer.h
 *
 * @brief   Operating System Abstraction Layer
 *
 *          Encapsulates and abstracts services from different operating
 *          system, including user-mode as well as kernel-mode services.
 ******************************************************************************/
#ifndef OSLAYER_H
#define OSLAYER_H

#include <builtins.h>
#include "ff.h"

/******************************************************************************/
/** @brief  Status codes of OS Abstraction Layer operation */
typedef enum _OSLAYER_STATUS {
	OSLAYER_OK = 0,   /*< success */
	OSLAYER_ERROR = -1,  /*< general error */
	OSLAYER_INVALID_PARAM = -2,  /*< invalid parameter supplied */
	OSLAYER_OPERATION_FAILED = -3,  /*< operation failed (i.e. current operation is interrupted) */
	OSLAYER_NOT_INITIALIZED = -4,  /*< resource object is not initialized */
	OSLAYER_TIMEOUT = -5,  /*< operation failed due to elapsed timeout */
	OSLAYER_SIGNAL_PENDING = -6   /*< operation interrupted due to pending signal for waiting thread/process */
} OSLAYER_STATUS;

/*****************************************************************************/
/** @brief  File object of OS Abstraction Layer */
typedef struct _osFile {
#ifdef LINUX
	FILE *file;
#else
	FIL file;
#endif
} osFile;

/******************************************************************************
 *  osMalloc()
 *****************************************************************************/
/**
 *  @brief  Allocate a continuous block of memory.
 *
 *  @param  size       Size of memory block to be allocated
 *
 ******************************************************************************/
extern void *osMalloc(uint32_t size);

/******************************************************************************
 *  osFree()
 *****************************************************************************/
/**
 *  @brief  Free a continuous block of memory.
 *
 *  @param  p       Pointer to previously allocated memory block
 *
 *  @return         always OSLAYER_OK
 ******************************************************************************/
extern int32_t osFree(void *p);

/******************************************************************************
 *  osMemPoolVerify()
 *****************************************************************************/
/**
 *  @brief  Verify memory pool initialization and perform basic tests
 *
 *  @return         OSLAYER_OK on success, OSLAYER_ERROR on failure
 ******************************************************************************/
extern int32_t osMemPoolVerify(void);

/******************************************************************************
 *  osMemPoolGetStats()
 *****************************************************************************/
/**
 *  @brief  Get memory pool statistics
 *
 *  @param  pUsedBlocks    Pointer to store number of used blocks
 *  @param  pFreeBlocks    Pointer to store number of free blocks
 *  @param  pTotalMemory   Pointer to store total memory size
 *  @param  pUsedMemory    Pointer to store used memory size
 *
 *  @return         OSLAYER_OK on success
 ******************************************************************************/
extern int32_t osMemPoolGetStats(uint32_t *pUsedBlocks, uint32_t *pFreeBlocks,
				 uint32_t *pTotalMemory, uint32_t *pUsedMemory);

/******************************************************************************
 *  osMemPoolPrintStats()
 *****************************************************************************/
/**
 *  @brief  Print memory pool statistics (debug function)
 *
 *  @return         OSLAYER_OK on success
 ******************************************************************************/
extern int32_t osMemPoolPrintStats(void);

extern int32_t osSleep(uint32_t msec);


/******************************************************************************
 *  osFopen()
 *****************************************************************************/
/**
 * @brief  Opens a file using the specified filename and mode.
 *
 * This function allocates and initializes an `osFile` structure, opens the
 * file with the given filename and mode, and associates the resulting `FILE/FIL*`
 * with the `osFile` structure.
 *
 * @param  filename  The name of the file to be opened.
 * @param  mode      The mode in which the file is to be opened (e.g., "r", "w").
 *
 * @return           A pointer to the opened `osFile` structure, or NULL if an
 *                   error occurred during file opening or memory allocation.
 */
osFile *osFopen(const char *filename, const char *mode);

/******************************************************************************
 *  osFclose()
 *****************************************************************************/
/**
 * @brief  Closes the file associated with the given osFile structure.
 *
 * This function closes the file associated with the given `osFile` structure,
 * frees the allocated memory for the structure, and performs necessary cleanup.
 *
 * @param  file  A pointer to the `osFile` structure representing the file.
 */
void osFclose(osFile* pVsiFile);

/******************************************************************************
 *  osFread()
 *****************************************************************************/
/**
 * @brief  Read data from a file into a specified buffer.
 *
 * This function reads data from a file associated with the given `osFile` structure
 * into the specified buffer.
 *
 * @param  ptr    Pointer to the buffer where the read data will be stored.
 * @param  size   Size of each data item to be read (in bytes).
 * @param  count  Number of data items to read.
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Number of data items actually read.
 *                If the return value is less than `count`, it may indicate reaching
 *                the end of the file or encountering an error.
 */
size_t osFread(void* ptr, size_t size, size_t count, osFile* pVsiFile);

/******************************************************************************
 *  osFwrite()
 *****************************************************************************/
/**
 * @brief  Write data from a specified buffer to a file.
 *
 * This function writes data from the specified buffer into the file associated
 * with the given `osFile` structure.
 *
 * @param  ptr    Pointer to the buffer containing the data to be written to the file.
 * @param  size   Size of each data item to be written (in bytes).
 * @param  count  Number of data items to write.
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Number of data items actually written to the file.
 *                If the return value is less than `count`, it may indicate disk
 *                full or encountering an error.
 */
size_t osFwrite(const void* ptr, size_t size, size_t count, osFile* pVsiFile);

/******************************************************************************
 *  osFseek()
 *****************************************************************************/
/**
 * @brief  Set the file position indicator for the specified file.
 *
 * This function sets the file position indicator for the file associated with
 * the given `osFile` structure. The new position is determined by adding
 * `offset` bytes to the position specified by `origin`.
 *
 * @param  file    Pointer to the `osFile` structure representing the file.
 * @param  offset  Number of bytes to offset from `origin`.
 * @param  origin  Position from where offset is added (SEEK_SET, SEEK_CUR, SEEK_END).
 *
 * @return         0 if successful, non-zero on error.
 */
int osFseek(osFile* pVsiFile, long int offset, int origin);

/******************************************************************************
 *  osFtell()
 *****************************************************************************/
/**
 * @brief  Get the current file position indicator for the specified file.
 *
 * This function gets the current file position indicator for the file associated
 * with the given `osFile` structure.
 *
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Current file position indicator if successful, -1 on error.
 */
long int osFtell(osFile* pVsiFile);

/******************************************************************************
 *  osFilesize()
 *****************************************************************************/
/**
 * @brief  Get the size of the file associated with the specified `osFile` structure.
 *
 * This function returns the size of the file associated with the given `osFile` structure.
 *
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Size of the file if successful, -1 on error.
 */
long int osFileSize(osFile* pVsiFile);

/******************************************************************************
 *  osFeof()
 *****************************************************************************/
/**
 * @brief  Check if the end-of-file indicator is set for the specified file.
 *
 * This function checks whether the end-of-file indicator is set for the file
 * associated with the given `osFile` structure.
 *
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Non-zero value if the end-of-file indicator is set, 0 otherwise.
 */
int osFeof(osFile* pVsiFile);

/******************************************************************************
 *  osFerror()
 *****************************************************************************/
/**
 * @brief  Check if an error indicator is set for the specified file.
 *
 * This function checks whether an error indicator is set for the file
 * associated with the given `osFile` structure.
 *
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Non-zero value if an error indicator is set, 0 otherwise.
 */
int osFerror(osFile* pVsiFile);

/******************************************************************************
 *  osFgetc()
 *****************************************************************************/
/**
 * @brief  Read a character from the specified file.
 *
 * This function reads a character from the file associated with the given
 * `osFile` structure. It returns when either a character is read, the
 * end-of-file is reached, or an error occurs, whichever comes first.
 *
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Integer of the read character on success, -1 on failure or end-of-file.
 */
int osFgetc(osFile* pVsiFile);

/******************************************************************************
 *  osFgets()
 *****************************************************************************/
/**
 * @brief  Read a line from the specified file into the specified buffer.
 *
 * This function reads a line from the file associated with the given `osFile` structure
 * into the specified buffer. It stops when either (n-1) characters are read, the newline
 * character is encountered, or the end-of-file is reached, whichever comes first.
 * The newline character, if encountered, is included in the buffer.
 *
 * @param  str    Pointer to the buffer where the line will be stored.
 * @param  size   Maximum number of characters to be read (including the null terminator).
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        Pointer to the read line on success, NULL on failure or end-of-file.
 */
char *osFgets(char* str, int size, osFile* pVsiFile);

/******************************************************************************
 *  osFputs()
 *****************************************************************************/
/**
 * @brief  Read a line from the specified file into the specified buffer.
 *
 * This function writes a string to the file associated with the given `osFile` structure
 *
 * @param  str    Pointer to the string which will be written into the file.
 * @param  file   Pointer to the `osFile` structure representing the file.
 *
 * @return        0 if successful, non-zero on error.
 */
int osFputs(const char* str, osFile* file);

/******************************************************************************
 *  osFscanf()
 *****************************************************************************/
/**
 * @brief  Read formatted data from the specified file.
 *
 * This function reads formatted data from the file associated with the given `osFile` structure,
 * under the control of the format string `format`. The variable arguments must correspond to
 * the placeholders in the format string.
 *
 * @param  file    Pointer to the `osFile` structure representing the file.
 * @param  format  Format string that specifies the expected format of the input.
 * @param  ...     Variable arguments corresponding to the placeholders in the format string.
 *
 * @return         Number of input items successfully matched and assigned, or EOF if an error occurs.
 */
int osFscanf(osFile* pVsiFile, const char* format, ...);

/******************************************************************************
 *  osFprintf()
 *****************************************************************************/
/**
 * @brief  Write formatted data to the specified file.
 *
 * This function writes formatted data to the file associated with the given `osFile` structure,
 * under the control of the format string `format`. The variable arguments must correspond to
 * the placeholders in the format string.
 *
 * @param  file    Pointer to the `osFile` structure representing the file.
 * @param  format  Format string that specifies the expected format of the output.
 * @param  ...     Variable arguments corresponding to the placeholders in the format string.
 *
 * @return         Number of characters written, or a negative value if an error occurs.
 */
int osFprintf(osFile* pVsiFile, const char* format, ...);


/******************************************************************************
 * osUSleep()
 *****************************************************************************/
/**
 * @brief  Sleeps the program for the specified number of microseconds.
 *
 * This function is used to make the program sleep for the specified number of microseconds.
 *
 * @param  microseconds  The number of microseconds to sleep.
 */
void osUSleep(unsigned int microseconds);


/******************************************************************************
 *  osTimeStampUs()
 *****************************************************************************/
/**
 *  @brief  Returns a 64-bit timestamp [us]
 *
 *  @param  pEvent         Reference of the timestamp object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Event successfully created
 *
 ******************************************************************************/
int32_t osTimeStampUs(int64_t *pTimeStamp);

/******************************************************************************
 *  osTimeStampNs()
 *****************************************************************************/
/**
 *  @brief  Returns a 64-bit timestamp [ns]
 *
 *  @param  pEvent         Reference of the timestamp object
 *
 *  @return                Status of operation
 *  @retval OSLAYER_OK     Event successfully created
 *
 ******************************************************************************/
int32_t osTimeStampNs(float32_t *pTimeStamp);


#endif //OSLAYER_H
