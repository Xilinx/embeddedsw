/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xmesim.c
* @{
*
* This file contains the low level layer interface to the ME simulator.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  04/10/2018  Added the API XMeSim_Init
* 1.2  Naresh  04/18/2018  Modified workaround for CRVO#1696/CR#999680 to extend
*                          it to all types of registers and also for all the
*                          tiles i.e., not just specific to PL interface regs
* 1.3  Naresh  05/07/2018  Fixed CR#1001816
* 1.4  Naresh  05/23/2018  Updated code to fix CR#999693 and removed the static
*                          data structure allocation for the registers to be
*                          monitored, instead added dynamic memory allocation.
* 1.5  Naresh  06/13/2018  Fixed CR#1003905, CR#1004494
* 1.6  Naresh  07/11/2018  Updated copyright info
* 1.7  Hyun    10/16/2018  Added XMeSim_IO_Funcs to support dynamic backend
*                          select at runtime as a part of CDO generation
*                          support. CR-1012480
* </pre>
*
******************************************************************************/
#include "stdlib.h"
#include "xmesim.h"

#ifdef __MESIM_SOCK__
#include "xsock.h"
#endif
#ifdef __MESIM_ESS__
#include "main_rts.h"
#include "cdo_rts.h"
#endif

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/**************************** Type Definitions *******************************/
/*
 * This is internal IO function structure for different backends.
 */
typedef struct XMeSim_IO_Funcs {
	uint32 (*Read32)(uint64_t Addr);
	void (*Read128)(uint64_t Addr, uint32 *Data);
	void (*Write32)(uint64_t Addr, uint32 Data);
	void (*MaskWrite32)(uint64_t Addr, uint32 Mask, uint32 Data);
	void (*Write128)(uint64_t Addr, uint32 *Data);
	void (*WriteCmd)(uint8 Command, uint8 ColId, uint8 RowId,
			uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr);
} XMeSim_IO_Funcs;

/************************** Variable Definitions *****************************/
#if defined(__MESIM_ESS__)
static const XMeSim_IO_Funcs Ess_IO_Funcs;
static const XMeSim_IO_Funcs *IO_Funcs = &Ess_IO_Funcs;
#elif defined(__MESIM_SOCK__)
static const XMeSim_IO_Funcs Sock_IO_Funcs;
static const XMeSim_IO_Funcs *IO_Funcs = &Sock_IO_Funcs;
#else
static const XMeSim_IO_Funcs *IO_Funcs = NULL;
#endif

#ifdef XME_WRKARND_CRVO1696

/* List of register addresses that needs to be state monitored locally */
XMeSim_RegState RegStateAddrs[XMESIM_SAVE_REGSTATE_NUMADDRS] =
{
        {0x00033000U, 0x000000DB},
        {0x00033004U, 0x000006DB},
        {0x00033008U, 0x00000000}
};

/* Pointer to data structure to hold the registers that need to be monitored */
XMeSim_RegState *RegStatePtr = NULL;

/* Global variable to hold the total number of registers */
uint32 XMeSim_NumRegsState = 0U;

#endif

/************************** Function Definitions *****************************/
#if defined(__MESIM_ESS__)

/*
 * CDO IO functions
 */

static inline uint32 XMeSim_CdoRead32(uint64_t Addr)
{
	/* no-op */
	return 0;
}

static inline void XMeSim_CdoRead128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XMeSim_CdoWrite32(uint64_t Addr, uint32 Data)
{
	cdo_Write32(Addr, Data);
}

static inline void XMeSim_CdoWrite128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XMeSim_CdoMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	cdo_MaskWrite32(Addr, Mask, Data);
}

static inline void  XMeSim_CdoWriteCmd(uint8 Command, uint8 ColId, uint8 RowId,
		uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr)
{
	/* no-op */
}

static const XMeSim_IO_Funcs Cdo_IO_Funcs = {
	.Read32 = XMeSim_CdoRead32,
	.Read128 = XMeSim_CdoRead128,
	.Write32 = XMeSim_CdoWrite32,
	.MaskWrite32 = XMeSim_CdoMaskWrite32,
	.Write128 = XMeSim_CdoWrite128,
	.WriteCmd = XMeSim_CdoWriteCmd,
};

/*
 * ESS IO functions
 */

static inline uint32 XMeSim_EssRead32(uint64_t Addr)
{
	return ess_Read32(Addr);
}

static inline void XMeSim_EssRead128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XMeSim_EssWrite32(uint64_t Addr, uint32 Data)
{
	ess_Write32(Addr, Data);
}

static inline void XMeSim_EssWrite128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XMeSim_EssMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	uint32 RegVal = ess_Read32(Addr);

	RegVal &= ~Mask;
	RegVal |= Data;
	ess_Write32(Addr, RegVal);
}

static inline void XMeSim_EssWriteCmd(uint8 Command, uint8 ColId, uint8 RowId,
		uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr)
{
	ess_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
}

static const XMeSim_IO_Funcs Ess_IO_Funcs = {
	.Read32 = XMeSim_EssRead32,
	.Read128 = XMeSim_EssRead128,
	.Write32 = XMeSim_EssWrite32,
	.MaskWrite32 = XMeSim_EssMaskWrite32,
	.Write128 = XMeSim_EssWrite128,
	.WriteCmd = XMeSim_EssWriteCmd,
};
#endif

#ifdef __MESIM_SOCK__

/*
 * Socket client IO functions
 */

static inline uint32 XMeSim_SockRead32(uint64_t Addr)
{
	return XSock_Read32(Addr);
}

static inline void XMeSim_SockRead128(uint64_t Addr, uint32 *Data)
{
	uint8 Idx;

	/* ME sim no support yet, so do 32-bit reads for now */
	for(Idx = 0U; Idx < 4U; Idx++) {
		Data[Idx] = XSock_Read32(Addr + Idx*4U);
	}
}

static inline void XMeSim_SockWrite32(uint64_t Addr, uint32 Data)
{
	XSock_Write32(Addr, Data);
}

static inline void XMeSim_SockWrite128(uint64_t Addr, uint32 *Data)
{
	XSock_Write128(Addr, Data);
}

static inline void XMeSim_SockMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	uint32 RegVal = XSock_Read32(Addr);

	RegVal &= ~Mask;
	RegVal |= Data;
	XSock_Write32(Addr, RegVal);
}

static inline void  XMeSim_SockWriteCmd(uint8 Command, uint8 ColId,
		uint8 RowId, uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr)
{
	XSock_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
}

static const XMeSim_IO_Funcs Sock_IO_Funcs = {
	.Read32 = XMeSim_SockRead32,
	.Read128 = XMeSim_SockRead128,
	.Write32 = XMeSim_SockWrite32,
	.MaskWrite32 = XMeSim_SockMaskWrite32,
	.Write128 = XMeSim_SockWrite128,
	.WriteCmd = XMeSim_SockWriteCmd,
};
#endif

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	Addr: Address to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
uint32 XMeSim_Read32(uint64_t Addr)
{
#ifdef XME_WRKARND_CRVO1696
        uint32 Idx;

        /* Check if reg state is maintained locally and hence skip the read */
        for(Idx = 0U; Idx < XMeSim_NumRegsState; Idx++) {
                if((Addr & XMESIM_TILE_BASE_ADDRMASK) ==
                                        RegStatePtr[Idx].RegAddr) {
                        return(RegStatePtr[Idx].RegVal);
                }
        }
#endif

	return IO_Funcs->Read32(Addr);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 128b data from the specified address.
*
* @param	Addr: Address to read from.
* @param	Data: Pointer to the 128-bit buffer to store the read data.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_Read128(uint64_t Addr, uint32 *Data)
{
	IO_Funcs->Read128(Addr, Data);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_Write32(uint64_t Addr, uint32 Data)
{
#ifdef XME_WRKARND_CRVO1696
        uint32 Idx;

        /* Check if reg state needs to be maintained locally */
        for(Idx = 0U; Idx < XMeSim_NumRegsState; Idx++) {
                if((Addr & XMESIM_TILE_BASE_ADDRMASK) ==
                                        RegStatePtr[Idx].RegAddr) {
                        RegStatePtr[Idx].RegVal = Data;
                        break;
                }
        }
#endif

	IO_Funcs->Write32(Addr, Data);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask-write 32bit data to the address.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_MaskWrite32(uint64_t Addr, uint32 Mask, uint32 Data)
{
	IO_Funcs->MaskWrite32(Addr, Mask, Data);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 128bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: Pointer to the 128-bit data buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_Write128(uint64_t Addr, uint32 *Data)
{
	IO_Funcs->Write128(Addr, Data);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 128bit data to the specified address.
*
* @param	Addr: Address to write to.
* @param	Data: Pointer to the 128-bit data buffer.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_WriteCmd(uint8 Command, uint8 ColId, uint8 RowId, uint32 CmdWd0,
						uint32 CmdWd1, uint8 *CmdStr)
{
        IO_Funcs->WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
}

/*****************************************************************************/
/**
*
* This API sets the IO mode and switches to available backend for IO operations.
*
* @param	Mode: IO mode. Should be XMESIM_IO_MODE_* in xmesim.h
*
* @return	XMESIM_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
uint8 XMeSim_SetIOMode(uint8 Mode)
{
	switch (Mode) {
#if defined(__MESIM_ESS__)
	case XMESIM_IO_MODE_ESS:
		IO_Funcs = &Ess_IO_Funcs;
		break;
	case XMESIM_IO_MODE_CDO:
		IO_Funcs = &Cdo_IO_Funcs;
		break;
#endif
#ifdef __MESIM_SOCK__
	case XMESIM_IO_MODE_SOCK:
		IO_Funcs = &Sock_IO_Funcs;
		break;
#endif
	default:
		return XMESIM_FAILURE;
	}

	return XMESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the init routine to enable ME sim workarounds.
*
* @param	NumCols: Number of columns.
* @param	NumRows: Number of rows.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XMeSim_Init(uint8 NumCols, uint8 NumRows)
{
#ifdef XME_WRKARND_CRVO1696
        uint32 Idx1;
        uint32 Idx2;
        uint32 Idx3;
        uint32 RegIdx = 0U;

        if(RegStatePtr != NULL) {
                /* Data structure already initalized */
                return;
        }

        XMeSim_NumRegsState = NumCols * (NumRows + 1) *
                                        XMESIM_SAVE_REGSTATE_NUMADDRS;

        /* Allocate memory for the data structure of the registers */
        RegStatePtr = (XMeSim_RegState *)malloc(XMeSim_NumRegsState *
                                                sizeof(XMeSim_RegState));

        /* Initialize the register state data structures */
        for(Idx1 = 0U; Idx1 < NumCols; Idx1++) {
                for(Idx2 = 0U; Idx2 <= NumRows; Idx2++) {
                        for(Idx3 = 0U;
                                Idx3 < XMESIM_SAVE_REGSTATE_NUMADDRS; Idx3++) {

                                RegIdx = (Idx1 * (NumRows + 1U) *
                                        XMESIM_SAVE_REGSTATE_NUMADDRS) +
                                        (Idx2 * XMESIM_SAVE_REGSTATE_NUMADDRS) +
                                        Idx3;

                                RegStatePtr[RegIdx].RegAddr =
                                        (Idx1 << XMESIM_TILE_ADDR_COL_SHIFT) |
					(Idx2 << XMESIM_TILE_ADDR_ROW_SHIFT) |
                                        RegStateAddrs[Idx3].RegAddr;

                                RegStatePtr[RegIdx].RegVal =
                                                RegStateAddrs[Idx3].RegVal;

                                XMeSim_print("Tile(col,row):(%d,%d), RegIdx:%d,"
                                        "RegAddr:%x, RegVal:%x\n", Idx1, Idx2,
                                        RegIdx, RegStatePtr[RegIdx].RegAddr,
                                        RegStatePtr[RegIdx].RegVal);
                        }
                }
        }
#endif
}

/** @} */

