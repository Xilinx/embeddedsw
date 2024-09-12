/******************************************************************************
*
* Copyright (C) 2021 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
******************************************************************************/

/*****************************************************************************/
/**
* @file sleep.h
*
*  This header file contains sleep related APIs.
*
* <pre>
* MODIFICATION HISTORY :
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
*  1.0  adk	 24/11/21 Initial release.
*  1.2  adk	 22/12/22 Fixed doxygen style and indentation issues.
*  2.1  adk      11/09/24 Added missing prototype for msleep() API.
*
* </pre>
*
******************************************************************************/

#ifndef SLEEP_H
#define SLEEP_H

#include "xil_types.h"
#include "xil_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/**
*
* This macro polls an address periodically until a condition is met or till the
* timeout occurs.
* The minimum timeout for calling this macro is 100us. If the timeout is less
* than 100us, it still waits for 100us. Also the unit for the timeout is 100us.
* If the timeout is not a multiple of 100us, it waits for a timeout of
* the next usec value which is a multiple of 100us.
*
* @param            IO_func - accessor function to read the register contents.
*                   Depends on the register width.
* @param            ADDR - Address to be polled
* @param            VALUE - variable to read the value
* @param            COND - Condition to checked (usually involves VALUE)
* @param            TIMEOUT_US - timeout in micro seconds
*
* @return
*		    - 0 - when the condition is met
*		    - 1 - when the condition is not met till the timeout period
*
* @note             none
*
*****************************************************************************/
#define Xil_poll_timeout(IO_func, ADDR, VALUE, COND, TIMEOUT_US) \
 ( {	  \
	u64 timeout = TIMEOUT_US/100;    \
	if(TIMEOUT_US%100!=0)	\
		timeout++;   \
	for(;;) { \
		VALUE = IO_func(ADDR); \
		if(COND) \
			break; \
		else {    \
			usleep(100);  \
			timeout--; \
			if(timeout==0) \
			break;  \
		}  \
	}    \
	(timeout>0) ? 0 : -1;  \
 }  )

void usleep(unsigned long useconds);
void msleep(unsigned long mseconds);
void sleep(unsigned int seconds);

#ifdef __cplusplus
}
#endif

#endif
