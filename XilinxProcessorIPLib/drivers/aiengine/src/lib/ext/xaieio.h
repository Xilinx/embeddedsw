/******************************************************************************
* (c) Copyright 2018 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information
* of Xilinx, Inc. and is protected under U.S. and
* international copyright and other intellectual property
* laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any
* rights to the materials distributed herewith. Except as
* otherwise provided in a valid license issued to you by
* Xilinx, and to the maximum extent permitted by applicable
* law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
* WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
* AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
* BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
* INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
* (2) Xilinx shall not be liable (whether in contract or tort,
* including negligence, or under any other theory of
* liability) for any loss or damage of any kind or nature
* related to, arising under or in connection with these
* materials, including for any direct, or any indirect,
* special, incidental, or consequential loss or damage
* (including loss of data, profits, goodwill, or any type of
* loss or damage suffered as a result of any action brought
* by a third party) even if such damage or loss was
* reasonably foreseeable or Xilinx had been advised of the
* possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-
* safe, or for use in any application requiring fail-safe
* performance, such as life-support or safety devices or
* systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any
* other applications that could lead to death, personal
* injury, or severe property or environmental damage
* (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and
* liability of any use of Xilinx products in Critical
* Applications, subject only to applicable laws and
* regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
* PART OF THIS FILE AT ALL TIMES.
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
