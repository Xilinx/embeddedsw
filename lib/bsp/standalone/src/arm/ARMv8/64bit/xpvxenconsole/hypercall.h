/*
Copyright DornerWorks 2016
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.

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
* @file hypercall.h
* @brief Xen hypercall interface for ARM64 guests.
*/


#ifndef __HYPERCALL_H__
#define __HYPERCALL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "xen.h"

/**
 * @brief Xen console I/O hypercall (read/write to the Xen console).
 * @param[in] cmd    Console I/O command (for example CONSOLEIO_write).
 * @param[in] count  Number of bytes to transfer.
 * @param[in,out] str Pointer to the command-specific data buffer.
 * @return For CONSOLEIO_read, the number of bytes read; for CONSOLEIO_write,
 *         0 on success. Returns a negative error code on failure.
 */
int HYPERVISOR_console_io(int cmd, int count, char *str);
/**
 * @brief Xen HVM operations hypercall.
 * @param[in] cmd       HVM operation code (for example HVMOP_*).
 * @param[in,out] param Pointer to operation-specific parameter structure.
 * @return 0 on success, negative error code on failure.
 */
int HYPERVISOR_hvm_op(int cmd, void* param);
/**
 * @brief Xen memory operations hypercall.
 * @param[in] cmd       Memory operation code (for example XENMEM_*).
 * @param[in,out] param Pointer to operation-specific parameter structure.
 * @return Operation-specific result, such as the number of extents processed
 *         for reservation operations; negative error codes where defined by cmd.
 */
int HYPERVISOR_memory_op(int cmd, void* param);
/**
 * @brief Xen event channel operations hypercall.
 * @param[in] cmd       Event channel operation code (for example EVTCHNOP_*).
 * @param[in,out] param Pointer to operation-specific parameter structure.
 * @return 0 on success, negative error code on failure.
 */
int HYPERVISOR_event_channel_op(int cmd, void* param);

#endif  /* __HYPERCALL_H__ */

#ifdef __cplusplus
}
#endif
