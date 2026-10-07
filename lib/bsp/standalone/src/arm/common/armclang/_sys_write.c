/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
*
* SPDX-License-Identifier: MIT
*
******************************************************************************/
/**
 * @file _sys_write.c
 * @brief Semihosting write stub for ARM compilers.
 */
#include "xil_printf.h"
#include "xparameters.h"

/**
 * @brief Stub for semihosting write; outputs to STDOUT if configured.
 * @param[in] fh File handle (unused).
 * @param[in] buf Buffer of bytes to write.
 * @param[in] len Number of bytes to write.
 * @param[in] mode Transfer mode (unused).
 * @return 0 after standard console output; Xen console write result when Xen PV console is enabled.
 */
__attribute__((weak)) s32 _sys_write(__attribute__((unused)) u32 fh,
					const u8 *buf, u32 len,
					__attribute__((unused)) s32 mode)
{
#if HYP_GUEST && EL1_NONSECURE && XEN_USE_PV_CONSOLE
	return XPVXenConsole_Write(buf);
#else
#ifdef STDOUT_BASEADDRESS
	char8* LocalBuf = (char8 *)buf;
	u32 i;

	for (i = 0; i < len; i++) {
		/* wait if TNFUL */
		if(LocalBuf != NULL) {
			LocalBuf += i;
		}
		if(LocalBuf != NULL) {
			if (*LocalBuf == '\n') {
				outbyte('\r');
			}
			outbyte(*LocalBuf);
		}
		if(LocalBuf != NULL) {
			LocalBuf -= i;
		}
	}

	/* All bytes written; return the number NOT written */
	return 0;
#endif
#endif
	return 0;
}
