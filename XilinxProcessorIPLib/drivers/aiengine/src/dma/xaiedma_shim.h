/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaiedma_shim.h
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
* 1.2  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.3  Hyun    06/20/2019  Added APIs for individual BD / Channel reset
* 1.4  Hyun    06/20/2019  Add XAieDma_ShimSoftInitialize()
* 1.5  Dishita 02/07/2020  Resolved macro compilation error
* </pre>
*
******************************************************************************/
#ifndef XAIEDMA_SHIM_H
#define XAIEDMA_SHIM_H

/***************************** Include Files *********************************/
#include "xaiegbl_reginit.h"

/************************** Variable Declarations ****************************/
extern XAieGbl_RegShimDmaBd ShimBd[];
extern XAieGbl_RegShimDmaCh ShimDmaCh[];
extern XAieGbl_RegShimDmaSts ShimDmaSts[];

/***************************** Constant Definitions **************************/
#define XAIEDMA_SHIM_MAX_NUM_CHANNELS		4U
#define XAIEDMA_SHIM_MAX_NUM_DESCRS		16U
#define XAIEDMA_SHIM_MAX_NUM_LOCKS		16U

#define XAIEDMA_SHIM_CHNUM_S2MM0			0U
#define XAIEDMA_SHIM_CHNUM_S2MM1			1U
#define XAIEDMA_SHIM_CHNUM_MM2S0			2U
#define XAIEDMA_SHIM_CHNUM_MM2S1			3U

#define XAIEDMA_SHIM_STARTBD_RESET		0U

#define XAIEDMA_SHIM_CHCTRL_OFFSET		0x140U
#define XAIEDMA_SHIM_STARTQ_OFFSET		0x144U

#define XAIEDMA_SHIM_NUM_BD_WORDS		0x5U
#define XAIEDMA_SHIM_BD_VALID			0x1U
#define XAIEDMA_SHIM_BD_NEXTBD_INVALID		0xFFU
#define XAIEDMA_SHIM_BD_AXIBLEN_MASK		0x3U

#define XAIEDMA_SHIM_LKACQRELVAL_INVALID		0xFFU	/* Invalid value for lock acq/rel */
#define XAIEDMA_SHIM_ADDRLOW_ALIGN_MASK		0xFU	/* 128-bit aligned */
#define XAIEDMA_SHIM_TXFER_LEN32_OFFSET		2U
#define XAIEDMA_SHIM_TXFER_LEN32_MASK		3U

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
} XAieDma_ShimBdLk;

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
} XAieDma_ShimBdAxi;

/**
 * This typedef contains all the attributes for the BD configuration.
 */
typedef struct
{
	XAieDma_ShimBdLk Lock;		/**< Lock attributes for the BD */
	XAieDma_ShimBdAxi Axi;		/**< AXI attributes for the BD */
	u32 AddrL;			/**< Lower address (128-bit aligned) */
	u16 AddrH;			/**< Upper 16-bit base address bits */
	u32 Length;			/**< Transfer length (in 32-bit words) */
	u8 PktEn;			/**< Add packet header to data */
	u8 PktType;			/**< Packet type */
	u8 PktId;			/**< ID value for the packet */
	u8 NextBdEn;			/**< Continue with next BD after completion of current BD or stop */
	u8 NextBd;			/**< Next BD to continue with */
} XAieDma_ShimBd;

/**
 * This typedef is the Shim DMA instance. User is required to allocate memory for this Shim DMA instance
 * and a pointer of the same is passed to the Shim DMA driver functions. Each Shim DMA in the array is
 * required to have its own and unique instance structure.
 */
typedef struct
{
	u64 BaseAddress;					/**< Shim DMA base address pointing to the BD0 location */
	u8 BdStart[XAIEDMA_SHIM_MAX_NUM_CHANNELS];		/**< Start BD value for all the 4 channels */
	XAieDma_ShimBd Descrs[XAIEDMA_SHIM_MAX_NUM_DESCRS];	/**< Data structure to hold the 16 descriptors of the Shim DMA */
}XAieDma_Shim;

/***************************** Macro Definitions *****************************/
#define XAIEGBL_NOC_DMASTA_STA_IDLE		0x0U
#define XAIEGBL_NOC_DMASTA_STARTQ_MAX		0x4U

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
#define XAieDma_ShimChControl(DmaInstPtr, ChNum, PauseStrm, PauseMm, Enable)	\
                        XAieGbl_Write32(((DmaInstPtr)->BaseAddress +	        \
                        ShimDmaCh[ChNum].CtrlOff),                              \
                        (XAie_SetField(PauseStrm, ShimDmaCh[ChNum].PzStr.Lsb,    \
                        ShimDmaCh[ChNum].PzStr.Mask) |                          \
                        XAie_SetField(PauseMm, ShimDmaCh[ChNum].PzMem.Lsb,       \
                        ShimDmaCh[ChNum].PzMem.Mask) |                          \
                        XAie_SetField(Enable, ShimDmaCh[ChNum].En.Lsb,           \
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
#define XAieDma_ShimSetStartBd(DmaInstPtr, ChNum, StartBd)			\
                        XAieGbl_Write32(((DmaInstPtr)->BaseAddress +	        \
                        ShimDmaCh[ChNum].StatQOff),                             \
                        XAie_SetField(StartBd, ShimDmaCh[ChNum].StatQ.Lsb,       \
                        ShimDmaCh[ChNum].StatQ.Mask));                          \
                        (DmaInstPtr)->BdStart[ChNum] = StartBd

/************************** Function Prototypes  *****************************/
u32 XAieDma_ShimSoftInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Shim *DmaInstPtr);
void XAieDma_ShimInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Shim *DmaInstPtr);
u32 XAieDma_ShimChReset(XAieDma_Shim *DmaInstPtr, u8 ChNum);
u32 XAieDma_ShimChResetAll(XAieDma_Shim *DmaInstPtr);
void XAieDma_ShimBdSetLock(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 LockId, u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal);
void XAieDma_ShimBdSetAxi(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 Smid, u8 BurstLen, u8 Qos, u8 Cache, u8 Secure);
void XAieDma_ShimBdSetPkt(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 PktEn, u8 PktType, u8 PktId);
void XAieDma_ShimBdSetNext(XAieDma_Shim *DmaInstPtr, u8 BdNum, u8 NextBd);
void XAieDma_ShimBdSetAddr(XAieDma_Shim *DmaInstPtr, u8 BdNum, u16 AddrHigh, u32 AddrLow, u32 Length);
void XAieDma_ShimBdWrite(XAieDma_Shim *DmaInstPtr, u8 BdNum);
void XAieDma_ShimBdClear(XAieDma_Shim *DmaInstPtr, u8 BdNum);
void XAieDma_ShimBdClearAll(XAieDma_Shim *DmaInstPtr);
u8 XAieDma_ShimWaitDone(XAieDma_Shim *DmaInstPtr, u32 ChNum, u32 TimeOut);
u8 XAieDma_ShimPendingBdCount(XAieDma_Shim *DmaInstPtr, u32 ChNum);

#endif
