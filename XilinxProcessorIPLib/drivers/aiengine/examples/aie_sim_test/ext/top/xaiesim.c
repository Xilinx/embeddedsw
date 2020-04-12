/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiesim.c
* @{
*
* This file contains the low level layer interface to the AIE simulator.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  04/06/2018  Initial creation
* 1.1  Naresh  04/10/2018  Added the API XAieSim_Init
* 1.2  Naresh  04/18/2018  Modified workaround for CRVO#1696/CR#999680 to extend
*                          it to all types of registers and also for all the
*                          tiles i.e., not just specific to PL interface regs
* 1.3  Naresh  05/07/2018  Fixed CR#1001816
* 1.4  Naresh  05/23/2018  Updated code to fix CR#999693 and removed the static
*                          data structure allocation for the registers to be
*                          monitored, instead added dynamic memory allocation.
* 1.5  Naresh  06/13/2018  Fixed CR#1003905, CR#1004494
* 1.6  Naresh  07/11/2018  Updated copyright info
* 1.7  Hyun    10/16/2018  Added XAieSim_IO_Funcs to support dynamic backend
*                          select at runtime as a part of CDO generation
*                          support. CR-1012480
* 1.8  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.9  Hyun    01/08/2018  Add the MaskPoll
* 2.0  Hyun    04/05/2018  NPI support
* </pre>
*
******************************************************************************/
#include "stdlib.h"
#include "xaiesim.h"

#ifdef __AIESIM_SOCK__
#include "xsock.h"
#endif
#ifdef __AIESIM_ESS__
#include "main_rts.h"
#include "cdo_rts.h"
#endif

/***************************** Include Files *********************************/

/***************************** Macro Definitions *****************************/

/**************************** Type Definitions *******************************/
/*
 * This is internal IO function structure for different backends.
 */
typedef struct XAieSim_IO_Funcs {
	uint32 (*Read32)(uint64_t Addr);
	void (*Read128)(uint64_t Addr, uint32 *Data);
	void (*Write32)(uint64_t Addr, uint32 Data);
	void (*MaskWrite32)(uint64_t Addr, uint32 Mask, uint32 Data);
	void (*Write128)(uint64_t Addr, uint32 *Data);
	void (*WriteCmd)(uint8 Command, uint8 ColId, uint8 RowId,
			uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr);
	uint32 (*MaskPoll)(uint64_t Addr, uint32 Mask, uint32 Value,
			uint32 TimeOutUs);
	uint32(*NpiRead32)(uint64_t Addr);
	void(*NpiWrite32)(uint64_t Addr, uint32 Data);
	void(*NpiMaskWrite32)(uint64_t Addr, uint32 Mask, uint32 Data);
	uint32(*NpiMaskPoll)(uint64_t Addr, uint32 Mask, uint32 Value,
			uint32 TimeOutUs);
} XAieSim_IO_Funcs;

/************************** Variable Definitions *****************************/
#if defined(__AIESIM_ESS__)
static const XAieSim_IO_Funcs Ess_IO_Funcs;
static const XAieSim_IO_Funcs *IO_Funcs = &Ess_IO_Funcs;
#elif defined(__AIESIM_SOCK__)
static const XAieSim_IO_Funcs Sock_IO_Funcs;
static const XAieSim_IO_Funcs *IO_Funcs = &Sock_IO_Funcs;
#else
static const XAieSim_IO_Funcs *IO_Funcs = NULL;
#endif

#ifdef XAIE_WRKARND_CRVO1696

/* List of register addresses that needs to be state monitored locally */
XAieSim_RegState RegStateAddrs[XAIESIM_SAVE_REGSTATE_NUMADDRS] =
{
        {0x00033000U, 0x000000DB},
        {0x00033004U, 0x000006DB},
        {0x00033008U, 0x00000000}
};

/* Pointer to data structure to hold the registers that need to be monitored */
XAieSim_RegState *RegStatePtr = NULL;

/* Global variable to hold the total number of registers */
uint32 XAieSim_NumRegsState = 0U;

#endif

/************************** Function Definitions *****************************/
#if defined(__AIESIM_ESS__)

/*
 * CDO IO functions
 */

static inline uint32 XAieSim_CdoRead32(uint64_t Addr)
{
	/* no-op */
	return 0;
}

static inline void XAieSim_CdoRead128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XAieSim_CdoWrite32(uint64_t Addr, uint32 Data)
{
	cdo_Write32(Addr, Data);
}

static inline void XAieSim_CdoWrite128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XAieSim_CdoMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	cdo_MaskWrite32(Addr, Mask, Data);
}

static inline void  XAieSim_CdoWriteCmd(uint8 Command, uint8 ColId, uint8 RowId,
		uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr)
{
	/* no-op */
}

static inline uint32 XAieSim_CdoMaskPoll(uint64_t Addr, uint32 Mask,
		uint32 Value, uint32 TimeOutUs)
{
	/* Round up to msec */
	cdo_MaskPoll(Addr, Mask, Value, (TimeOutUs + 999) / 1000);
	return XAIESIM_SUCCESS;
}

static const XAieSim_IO_Funcs Cdo_IO_Funcs = {
	.Read32 = XAieSim_CdoRead32,
	.Read128 = XAieSim_CdoRead128,
	.Write32 = XAieSim_CdoWrite32,
	.MaskWrite32 = XAieSim_CdoMaskWrite32,
	.Write128 = XAieSim_CdoWrite128,
	.WriteCmd = XAieSim_CdoWriteCmd,
	.MaskPoll = XAieSim_CdoMaskPoll,
	.NpiRead32 = XAieSim_CdoRead32,
	.NpiWrite32 = XAieSim_CdoWrite32,
	.NpiMaskWrite32 = XAieSim_CdoMaskWrite32,
	.NpiMaskPoll = XAieSim_CdoMaskPoll,
};

/*
 * ESS IO functions
 */

static inline uint32 XAieSim_EssRead32(uint64_t Addr)
{
	return ess_Read32(Addr);
}

static inline void XAieSim_EssRead128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XAieSim_EssWrite32(uint64_t Addr, uint32 Data)
{
	ess_Write32(Addr, Data);
}

static inline void XAieSim_EssWrite128(uint64_t Addr, uint32 *Data)
{
	/* no-op */
}

static inline void XAieSim_EssMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	uint32 RegVal = ess_Read32(Addr);

	RegVal &= ~Mask;
	RegVal |= Data;
	ess_Write32(Addr, RegVal);
}

static inline void XAieSim_EssWriteCmd(uint8 Command, uint8 ColId, uint8 RowId,
		uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr)
{
	ess_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);

}

static inline uint32 XAieSim_EssMaskPoll(uint64_t Addr, uint32 Mask,
		uint32 Value, uint32 TimeOutUs)
{
	uint32 Ret = XAIESIM_FAILURE;

	while (TimeOutUs > 0U) {
		if ((ess_Read32(Addr) & Mask) == Value) {
			Ret = XAIESIM_SUCCESS;
			break;
		}
		usleep(1);
		TimeOutUs--;
	}

	return Ret;
}

static inline uint32 XAieSim_EssNpiRead32(uint64_t Addr)
{
	return ess_NpiRead32(Addr);
}

static inline void XAieSim_EssNpiWrite32(uint64_t Addr, uint32 Data)
{
	ess_NpiWrite32(Addr, Data);
}

static inline void XAieSim_EssNpiMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	uint32 RegVal = ess_NpiRead32(Addr);

	RegVal &= ~Mask;
	RegVal |= Data;
	ess_NpiWrite32(Addr, RegVal);
}

static inline uint32 XAieSim_EssNpiMaskPoll(uint64_t Addr, uint32 Mask,
	uint32 Value, uint32 TimeOutUs)
{
	uint32 Ret = XAIESIM_FAILURE;

	while (TimeOutUs > 0U) {
		if ((ess_NpiRead32(Addr) & Mask) == Value) {
			Ret = XAIESIM_SUCCESS;
			break;
		}
		usleep(1);
		TimeOutUs--;
	}

	return Ret;
}

static const XAieSim_IO_Funcs Ess_IO_Funcs = {
	.Read32 = XAieSim_EssRead32,
	.Read128 = XAieSim_EssRead128,
	.Write32 = XAieSim_EssWrite32,
	.MaskWrite32 = XAieSim_EssMaskWrite32,
	.Write128 = XAieSim_EssWrite128,
	.WriteCmd = XAieSim_EssWriteCmd,
	.MaskPoll = XAieSim_EssMaskPoll,
	.NpiRead32 = XAieSim_EssNpiRead32,
	.NpiWrite32 = XAieSim_EssNpiWrite32,
	.NpiMaskWrite32 = XAieSim_EssNpiMaskWrite32,
	.NpiMaskPoll = XAieSim_EssNpiMaskPoll,
};
#endif

#ifdef __AIESIM_SOCK__

/*
 * Socket client IO functions
 */

static inline uint32 XAieSim_SockRead32(uint64_t Addr)
{
	return XSock_Read32(Addr);
}

static inline void XAieSim_SockRead128(uint64_t Addr, uint32 *Data)
{
	uint8 Idx;

	/* AIE sim no support yet, so do 32-bit reads for now */
	for(Idx = 0U; Idx < 4U; Idx++) {
		Data[Idx] = XSock_Read32(Addr + Idx*4U);
	}
}

static inline void XAieSim_SockWrite32(uint64_t Addr, uint32 Data)
{
	XSock_Write32(Addr, Data);
}

static inline void XAieSim_SockWrite128(uint64_t Addr, uint32 *Data)
{
	XSock_Write128(Addr, Data);
}

static inline void XAieSim_SockMaskWrite32(uint64_t Addr, uint32 Mask,
		uint32 Data)
{
	uint32 RegVal = XSock_Read32(Addr);

	RegVal &= ~Mask;
	RegVal |= Data;
	XSock_Write32(Addr, RegVal);
}

static inline void  XAieSim_SockWriteCmd(uint8 Command, uint8 ColId,
		uint8 RowId, uint32 CmdWd0, uint32 CmdWd1, uint8 *CmdStr)
{
	XSock_WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
}

static inline void XAieSim_SockMaskPoll(uint64_t Addr, uint32 Mask,
		uint32 Value, uint32 TimeOutUs)
{
	uint32 Ret = XAIESIM_FAILURE;

	while (TimeOutUs > 0U) {
		if ((ess_Read32(Addr) & Mask) == Value) {
			Ret = XAIESIM_SUCCESS;
			break;
		}
		usleep(1);
		TimeOutUs--;
	}

	return Ret;
}

static const XAieSim_IO_Funcs Sock_IO_Funcs = {
	.Read32 = XAieSim_SockRead32,
	.Read128 = XAieSim_SockRead128,
	.Write32 = XAieSim_SockWrite32,
	.MaskWrite32 = XAieSim_SockMaskWrite32,
	.Write128 = XAieSim_SockWrite128,
	.WriteCmd = XAieSim_SockWriteCmd,
	.MaskPoll = XAieSim_SockMaskPoll,
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
uint32 XAieSim_Read32(uint64_t Addr)
{
#ifdef XAIE_WRKARND_CRVO1696
        uint32 Idx;

        /* Check if reg state is maintained locally and hence skip the read */
        for(Idx = 0U; Idx < XAieSim_NumRegsState; Idx++) {
                if((Addr & XAIESIM_TILE_BASE_ADDRMASK) ==
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
void XAieSim_Read128(uint64_t Addr, uint32 *Data)
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
void XAieSim_Write32(uint64_t Addr, uint32 Data)
{
#ifdef XAIE_WRKARND_CRVO1696
        uint32 Idx;

        /* Check if reg state needs to be maintained locally */
        for(Idx = 0U; Idx < XAieSim_NumRegsState; Idx++) {
                if((Addr & XAIESIM_TILE_BASE_ADDRMASK) ==
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
void XAieSim_MaskWrite32(uint64_t Addr, uint32 Mask, uint32 Data)
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
void XAieSim_Write128(uint64_t Addr, uint32 *Data)
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
void XAieSim_WriteCmd(uint8 Command, uint8 ColId, uint8 RowId, uint32 CmdWd0,
						uint32 CmdWd1, uint8 *CmdStr)
{
        IO_Funcs->WriteCmd(Command, ColId, RowId, CmdWd0, CmdWd1, CmdStr);
}

/*****************************************************************************/
/**
*
* This is the IO function to poll until the value at the address to be given
* masked value.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to read data.
* @param	Value: The expected value
* @param	TimeOutUs: Minimum timeout in usec.
*
* @return	XAIESIM_SUCCESS if successful, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XAieSim_MaskPoll(uint64_t Addr, uint32 Mask, uint32 Value, uint32 TimeOutUs)
{
        return IO_Funcs->MaskPoll(Addr, Mask, Value, TimeOutUs);
}

/*****************************************************************************/
/**
*
* This API sets the IO mode and switches to available backend for IO operations.
*
* @param	Mode: IO mode. Should be XAIESIM_IO_MODE_* in xaiesim.h
*
* @return	XAIESIM_SUCCESS on success.
*
* @note		None.
*
*******************************************************************************/
uint8 XAieSim_SetIOMode(uint8 Mode)
{
	switch (Mode) {
#if defined(__AIESIM_ESS__)
	case XAIESIM_IO_MODE_ESS:
		IO_Funcs = &Ess_IO_Funcs;
		break;
	case XAIESIM_IO_MODE_CDO:
		IO_Funcs = &Cdo_IO_Funcs;
		break;
#endif
#ifdef __AIESIM_SOCK__
	case XAIESIM_IO_MODE_SOCK:
		IO_Funcs = &Sock_IO_Funcs;
		break;
#endif
	default:
		return XAIESIM_FAILURE;
	}

	return XAIESIM_SUCCESS;
}

/*****************************************************************************/
/**
*
* This is the init routine to enable AIE sim workarounds.
*
* @param	NumCols: Number of columns.
* @param	NumRows: Number of rows.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieSim_Init(uint8 NumCols, uint8 NumRows)
{
#ifdef XAIE_WRKARND_CRVO1696
        uint32 Idx1;
        uint32 Idx2;
        uint32 Idx3;
        uint32 RegIdx = 0U;

        if(RegStatePtr != NULL) {
                /* Data structure already initalized */
                return;
        }

        XAieSim_NumRegsState = NumCols * (NumRows + 1) *
                                        XAIESIM_SAVE_REGSTATE_NUMADDRS;

        /* Allocate memory for the data structure of the registers */
        RegStatePtr = (XAieSim_RegState *)malloc(XAieSim_NumRegsState *
                                                sizeof(XAieSim_RegState));

        /* Initialize the register state data structures */
        for(Idx1 = 0U; Idx1 < NumCols; Idx1++) {
                for(Idx2 = 0U; Idx2 <= NumRows; Idx2++) {
                        for(Idx3 = 0U;
                                Idx3 < XAIESIM_SAVE_REGSTATE_NUMADDRS; Idx3++) {

                                RegIdx = (Idx1 * (NumRows + 1U) *
                                        XAIESIM_SAVE_REGSTATE_NUMADDRS) +
                                        (Idx2 * XAIESIM_SAVE_REGSTATE_NUMADDRS) +
                                        Idx3;

                                RegStatePtr[RegIdx].RegAddr =
                                        (Idx1 << XAIESIM_TILE_ADDR_COL_SHIFT) |
					(Idx2 << XAIESIM_TILE_ADDR_ROW_SHIFT) |
                                        RegStateAddrs[Idx3].RegAddr;

                                RegStatePtr[RegIdx].RegVal =
                                                RegStateAddrs[Idx3].RegVal;

                                XAieSim_print("Tile(col,row):(%d,%d), RegIdx:%d,"
                                        "RegAddr:%x, RegVal:%x\n", Idx1, Idx2,
                                        RegIdx, RegStatePtr[RegIdx].RegAddr,
                                        RegStatePtr[RegIdx].RegVal);
                        }
                }
        }
#endif
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the NPI space.
*
* @param	Addr: Address to read from.
*
* @return	32-bit read value.
*
* @note		None.
*
*******************************************************************************/
uint32 XAieSim_NPIRead32(uint64_t Addr)
{
	return IO_Funcs->NpiRead32(Addr);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the NPI space.
*
* @param	Addr: Address to write to.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
void XAieSim_NPIWrite32(uint64_t Addr, uint32 Data)
{
	IO_Funcs->NpiWrite32(Addr, Data);
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask-write 32bit data to the NPI space.
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
void XAieSim_NPIMaskWrite32(uint64_t Addr, uint32 Mask, uint32 Data)
{
	IO_Funcs->NpiMaskWrite32(Addr, Mask, Data);
}

/*****************************************************************************/
/**
*
* This is the IO function to poll until the value at the NPI address space to
* be given masked value.
*
* @param	Addr: Address to write to.
* @param	Mask: Mask to be applied to read data.
* @param	Value: The expected value
* @param	TimeOutUs: Minimum timeout in usec.
*
* @return	XAIESIM_SUCCESS if successful, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
uint32 XAieSim_NPIMaskPoll(uint64_t Addr, uint32 Mask, uint32 Value,
		uint32 TimeOutUs)
{
	return IO_Funcs->NpiMaskPoll(Addr, Mask, Value, TimeOutUs);
}

/** @} */

