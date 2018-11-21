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
* @file xmedma_shim.h
* @{
*
* Header file for the Shim DMA functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  07/11/2018  Updated copyright info
* </pre>
*
******************************************************************************/
#ifndef XMEDMA_SHIM_H
#define XMEDMA_SHIM_H

/***************************** Include Files *********************************/
#include "xmegbl_reginit.h"

/************************** Variable Declarations ****************************/
extern XMeGbl_RegShimDmaBd ShimBd[];
extern XMeGbl_RegShimDmaCh ShimDmaCh[];
extern XMeGbl_RegShimDmaSts ShimDmaSts[];

/***************************** Constant Definitions **************************/
#define XMEDMA_SHIM_MAX_NUM_CHANNELS		4U
#define XMEDMA_SHIM_MAX_NUM_DESCRS		16U
#define XMEDMA_SHIM_MAX_NUM_LOCKS		16U

#define XMEDMA_SHIM_CHNUM_S2MM0			0U
#define XMEDMA_SHIM_CHNUM_S2MM1			1U
#define XMEDMA_SHIM_CHNUM_MM2S0			2U
#define XMEDMA_SHIM_CHNUM_MM2S1			3U

#define XMEDMA_SHIM_STARTBD_RESET		0U

#define XMEDMA_SHIM_CHCTRL_OFFSET		0x140U
#define XMEDMA_SHIM_STARTQ_OFFSET		0x144U

#define XMEDMA_SHIM_NUM_BD_WORDS		0x5U
#define XMEDMA_SHIM_BD_VALID			0x1U
#define XMEDMA_SHIM_BD_NEXTBD_INVALID		0xFFU
#define XMEDMA_SHIM_BD_AXIBLEN_MASK		0x3U

#define XMEDMA_SHIM_LKACQRELVAL_INVALID		0xFFU	/* Invalid value for lock acq/rel */
#define XMEDMA_SHIM_ADDRLOW_ALIGN_MASK		0xFU	/* 128-bit aligned */
#define XMEDMA_SHIM_TXFER_LEN32_OFFSET		2U
#define XMEDMA_SHIM_TXFER_LEN32_MASK		3U

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the lock attributes for the BD.
 */
typedef struct
{
	u8 LockId;			/**< Lock ID value, ranging from 0-15 */
	u8 LkRelEn;			/**< Lock release enable */
	u8 LkRelVal;			/**< Lock release value */
	u8 LkRelValEn;			/**< Lock release value enable */
	u8 LkAcqEn;			/**< Lock acquire enable */
	u8 LkAcqVal;			/**< Lock acquire value */
	u8 LkAcqValEn;			/**< Lock acquire value enable */
} XMeDma_ShimBdLk;

/**
 * This typedef contains the AXI attributes for the BD.
 */
typedef struct
{
	u8 Smid;			/**< SMID value for the AXI-MM transfer */
	u8 BrstLen;			/**< AXI Burst length (AxLEN) for the AXI-MM transfer*/
	u8 Qos;				/**< AXI Qos bits (AxQOS) for the AXI-MM transfer */
	u8 Cache;			/**< AxCACHE bits for the AXI-MM transfer */
	u8 Secure;			/**< Secure/Non-secure AXI-MM transfer */
} XMeDma_ShimBdAxi;

/**
 * This typedef contains all the attributes for the BD configuration.
 */
typedef struct
{
	XMeDma_ShimBdLk Lock;		/**< Lock attributes for the BD */
	XMeDma_ShimBdAxi Axi;		/**< AXI attributes for the BD */
	u32 AddrL;			/**< Lower address (128-bit aligned) */
	u16 AddrH;			/**< Upper 16-bit base address bits */
	u32 Length;			/**< Transfer length (in 32-bit words) */
	u8 PktEn;			/**< Add packet header to data */
	u8 PktType;			/**< Packet type */
	u8 PktId;			/**< ID value for the packet */
	u8 NextBdEn;			/**< Continue with next BD after completion of current BD or stop */
	u8 NextBd;			/**< Next BD to continue with */
} XMeDma_ShimBd;

/**
 * This typedef is the Shim DMA instance. User is required to allocate memory for this Shim DMA instance
 * and a pointer of the same is passed to the Shim DMA driver functions. Each Shim DMA in the array is
 * required to have its own and unique instance structure.
 */
typedef struct
{
	u64 BaseAddress;					/**< Shim DMA base address pointing to the BD0 location */
	u8 BdStart[XMEDMA_SHIM_MAX_NUM_CHANNELS];		/**< Start BD value for all the 4 channels */
	XMeDma_ShimBd Descrs[XMEDMA_SHIM_MAX_NUM_DESCRS];	/**< Data structure to hold the 16 descriptors of the Shim DMA */
}XMeDma_Shim;

/***************************** Macro Definitions *****************************/
#define XMEGBL_NOC_DMASTA_STA_IDLE		0x0U
#define XMEGBL_NOC_DMASTA_STARTQ_MAX		0x4U

/*****************************************************************************/
/**
*
* Macro to configure the selected Shim DMA channel conrol bits.
*
* @param	DmaInstPtr - Pointer to the Shim DMA instance.
* @param	ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
* @param	PauseStrm - When set, pauses the stream traffic.
* @param	PauseMm - When set, pauses the issuing of new AXI-MM commands.
* @param	Enable - Enable channel (1-Enable,0-Disable).
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XMeDma_ShimChControl(DmaInstPtr, ChNum, PauseStrm, PauseMm, Enable)	\
                        XMeGbl_Write32((DmaInstPtr->BaseAddress +	        \
                        ShimDmaCh[ChNum].CtrlOff),                              \
                        (XMe_SetField(PauseStrm, ShimDmaCh[ChNum].PzStr.Lsb,    \
                        ShimDmaCh[ChNum].PzStr.Mask) |                          \
                        XMe_SetField(PauseMm, ShimDmaCh[ChNum].PzMem.Lsb,       \
                        ShimDmaCh[ChNum].PzMem.Mask) |                          \
                        XMe_SetField(Enable, ShimDmaCh[ChNum].En.Lsb,           \
                        ShimDmaCh[ChNum].En.Mask)))

/*****************************************************************************/
/**
*
* Macro to set the StartBd for the selected Shim DMA channel.
*
* @param	DmaInstPtr - Pointer to the Shmim DMA instance.
* @param	ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
* @param	BdStart - BD value for the channel to start with (range:0-15).
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XMeDma_ShimSetStartBd(DmaInstPtr, ChNum, StartBd)			\
                        XMeGbl_Write32((DmaInstPtr->BaseAddress +	        \
                        ShimDmaCh[ChNum].StatQOff),                             \
                        XMe_SetField(StartBd, ShimDmaCh[ChNum].StatQ.Lsb,       \
                        ShimDmaCh[ChNum].StatQ.Mask));                          \
                        DmaInstPtr->BdStart[ChNum] = StartBd

/************************** Function Prototypes  *****************************/
void XMeDma_ShimInitialize(XMeGbl_Tile *TileInstPtr, XMeDma_Shim *DmaInstPtr);
void XMeDma_ShimBdSetLock(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 LockId, u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal);
void XMeDma_ShimBdSetAxi(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 Smid, u8 BurstLen, u8 Qos, u8 Cache, u8 Secure);
void XMeDma_ShimBdSetPkt(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 PktEn, u8 PktType, u8 PktId);
void XMeDma_ShimBdSetNext(XMeDma_Shim *DmaInstPtr, u8 BdNum, u8 NextBd);
void XMeDma_ShimBdSetAddr(XMeDma_Shim *DmaInstPtr, u8 BdNum, u16 AddrHigh, u32 AddrLow, u32 Length);
void XMeDma_ShimBdWrite(XMeDma_Shim *DmaInstPtr, u8 BdNum);
void XMeDma_ShimBdClear(XMeDma_Shim *DmaInstPtr, u8 BdNum);
u8 XMeDma_ShimWaitDone(XMeDma_Shim *DmaInstPtr, u32 ChNum, u32 TimeOut);
u8 XMeDma_ShimPendingBdCount(XMeDma_Shim *DmaInstPtr, u32 ChNum);

#endif
