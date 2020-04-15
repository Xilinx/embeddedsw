/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/



/*****************************************************************************/
/**
* @file xaieio.h
* @{
*
* This file contains the generic definitions for the AIE simulator interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Hyun    07/12/2018  Initial creation
* 1.1  Nishad  12/05/2018  Renamed ME attributes to AIE
* </pre>
*
******************************************************************************/
#ifndef XAIEIO_H
#define XAIEIO_H

/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/
/****************************** Type Definitions *****************************/
typedef unsigned char			uint8;
typedef unsigned int			uint32;
typedef signed int                      sint32;
typedef unsigned long int               uint64_t;

/************************** Function Prototypes  *****************************/
void XAieIO_Finish(void);
void XAieIO_Init(void);

void XAieIO_IntrUnregisterIsr(int Offset);
int XAieIO_IntrRegisterIsr(int Offset, int (*Handler) (void *Data), void *Data);
void XAieIO_IntrEnable(void);
void XAieIO_IntrDisable(void);

void *_XAieIO_GetIO(void);

uint32 XAieIO_Read32(uint64_t Addr);
void XAieIO_Read128(uint64_t Addr, uint32 *Data);
void XAieIO_Write32(uint64_t Addr, uint32 Data);
void XAieIO_Write128(uint64_t Addr, uint32 *Data);

typedef struct XAieIO_Mem XAieIO_Mem;

void XAieIO_MemFinish(XAieIO_Mem *IO_MemInstPtr);
XAieIO_Mem *XAieIO_MemInit(uint8 idx);
void XAieIO_MemDetach(XAieIO_Mem *IO_MemInstPtr);
XAieIO_Mem *XAieIO_MemAttach(uint64_t Vaddr, uint64_t Paddr, uint64_t Size, uint64_t MemHandle);
void XAieIO_MemFree(XAieIO_Mem *IO_MemInstPtr);
XAieIO_Mem *XAieIO_MemAllocate(uint64_t Size, uint32 Attr);

uint8 XAieIO_MemSyncForCPU(XAieIO_Mem *IO_MemInstPtr);
uint8 XAieIO_MemSyncForDev(XAieIO_Mem *IO_MemInstPtr);

uint64_t XAieIO_MemGetSize(XAieIO_Mem *IO_MemInstPtr);
uint64_t XAieIO_MemGetVaddr(XAieIO_Mem *IO_MemInstPtr);
uint64_t XAieIO_MemGetPaddr(XAieIO_Mem *IO_MemInstPtr);
void XAieIO_MemWrite32(XAieIO_Mem *IO_MemInstPtr, uint64_t Addr, uint32 Data);
uint32 XAieIO_MemRead32(XAieIO_Mem *IO_MemInstPtr, uint64_t Addr);

uint32 XAieIO_NPIRead32(uint64_t Addr);
void XAieIO_NPIWrite32(uint64_t Addr, uint32 Data);

#endif		/* end of protection macro */
/** @} */
