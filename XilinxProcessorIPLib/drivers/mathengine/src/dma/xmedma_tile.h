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
* @file xmedma_tile.h
* @{
*
* Header file for the Tile DMA functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Naresh  03/14/2018  Initial creation
* 1.1  Naresh  06/20/2018  Fixed CR#1005445
* 1.2  Naresh  07/11/2018  Updated copyright info
* </pre>
*
******************************************************************************/
#ifndef XMEDMA_TILE_H
#define XMEDMA_TILE_H

/***************************** Include Files *********************************/
#include "xmegbl_reginit.h"

/************************** Variable Declarations ****************************/
extern XMeGbl_RegTileDmaBd TileBd[];
extern XMeGbl_RegTileDmaCh TileDmaCh[];

/***************************** Constant Definitions **************************/
#define XMEDMA_TILE_MAX_NUM_CHANNELS		4U
#define XMEDMA_TILE_MAX_NUM_DESCRS		16U
#define XMEDMA_TILE_MAX_NUM_LOCKS		16U

#define XMEDMA_TILE_CHNUM_S2MM0			0U
#define XMEDMA_TILE_CHNUM_S2MM1			1U
#define XMEDMA_TILE_CHNUM_MM2S0			2U
#define XMEDMA_TILE_CHNUM_MM2S1			3U

#define XMEDMA_TILE_BD0_OFFSET			0x1D000U
#define XMEDMA_TILE_BD_SIZE			0x20U

#define XMEDMA_TILE_BD_2DDMA_X			0U
#define XMEDMA_TILE_BD_2DDMA_Y			1U
#define XMEDMA_TILE_BD_ADDRA			0U
#define XMEDMA_TILE_BD_ADDRB			1U

#define XMEDMA_TILE_STARTBD_RESET		0xFFU

#define XMEDMA_TILE_CHCTRL_OFFSET		0xE00U
#define XMEDMA_TILE_STARTQ_OFFSET		0xE04U

#define XMEDMA_TILE_CHSTS_IDLE                  0U
#define XMEDMA_TILE_CHSTS_STARTING              1U
#define XMEDMA_TILE_CHSTS_RUNNING               2U
#define XMEDMA_TILE_CHSTS_INVALID               0xFFU
#define XMEDMA_TILE_CHSTS_POLL_TIMEOUT          0x8000U

#define XMEDMA_TILE_BD_VALID			1U

#define XMEDMA_TILE_2DX_DEFAULT_INCR		0U
#define XMEDMA_TILE_2DX_DEFAULT_WRAP		255U
#define XMEDMA_TILE_2DX_DEFAULT_OFFSET		1U
#define XMEDMA_TILE_2DY_DEFAULT_INCR		255U
#define XMEDMA_TILE_2DY_DEFAULT_WRAP		255U
#define XMEDMA_TILE_2DY_DEFAULT_OFFSET		256U

#define XMEDMA_TILE_LOCK_ACQRELVAL_INVALID	0xFFU

#define XMEDMA_TILE_INTLVCNT_ALIGN_MASK		0x3U
#define XMEDMA_TILE_ADDRAB_ALIGN_MASK		0x3U
#define XMEDMA_TILE_ADDRAB_ALIGN_OFFSET		2U

#define XMEDMA_TILE_LENGTH128_OFFSET		0x4U
#define XMEDMA_TILE_LENGTH128_MASK		0xFU
#define XMEDMA_TILE_LENGTH32_OFFSET		0x2U
#define XMEDMA_TILE_LENGTH32_MASK		0x3U

#define XMEDMA_TILE_NUM_BD_WORDS		7U
#define XMEDMA_TILE_BD_NEXTBD_INVALID		0xFFU

/* Offsets for the Channel control register bit fields */
#define XMEDMA_TILE_CHCTRL_RESET_SHIFT		1U	
#define XMEDMA_TILE_CHCTRL_ENABLE_SHIFT		0U

/**************************** Type Definitions *******************************/
/**
 * This typedef contains the lock and base address attributes of the double buffer for the BD.
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
	u16 BaseAddr;			/**< Base address */
} XMeDma_TileBdLock;

/**
 * This typedef contains the X/Y 2D addressing attributes for the BD.
 */
typedef struct
{
	u16 Incr;			/**< Increment value for 2D X/Y addressing */
	u16 Wrap;			/**< Wrap value for 2D X/Y addressing */
	u16 Offset;			/**< Offset value for 2D X/Y addressing */
} XMeDma_TileBdXy;

/**
 * This typedef contains all the attributes for the BD configuration.
 */
typedef struct
{
	XMeDma_TileBdLock AddrA;	/**< AddressA lock and address attributes for double buffering */
	XMeDma_TileBdLock AddrB;	/**< AddressB lock and address attributes for double buffering */
	XMeDma_TileBdXy X2dCfg;		/**< 2D addressing attributes for X */
	XMeDma_TileBdXy Y2dCfg;		/**< 2D addressing attributes for Y */
	u8 PktEn;			/**< Packet enable */
	u8 PktType;			/**< Packet type */
	u8 PktId;			/**< ID value used in the packet */
	u8 IntlvMode;			/**< Interleave mode enable */
	u8 IntlvDb;			/**< Double buffer type (A or B) for interleaving */
	u8 IntlvCnt;			/**< Interleave count */
	u16 IntlvCur;			/**< Interleave current pointer */
	u8 AbMode;			/**< AB Double buffer mode enable */
	u8 FifoMode;			/**< FIFO mode enable */
	u8 NextBdEn;			/**< Use next BD */
	u8 NextBd;			/**< Next BD to be used */
	u16 Length;			/**< Length of the transfer in bytes */
	u8 ChNum;			/**< Channel number, ranging from 0-3 */
} XMeDma_TileBd;

/**
 * This typedef is the Tile DMA instance. User is required to allocate memory for this Tile DMA instance
 * and a pointer of the same is passed to the Tile DMA driver functions. Each Tile DMA in the array is
 * required to have its own and unique instance structure.
 */
typedef struct
{
	u64 BaseAddress;		                        /**< Tile DMA base address */
        u32 IsReady;		                                /**< Device is initialized and ready */
	u8 StartBd[XMEDMA_TILE_MAX_NUM_CHANNELS];		/**< Start BD value for all the 4 channels */
	XMeDma_TileBd Descrs[XMEDMA_TILE_MAX_NUM_DESCRS];	/**< Data structure to hold the 16 descriptors of the Tile DMA */
} XMeDma_Tile;

/***************************** Macro Definitions *****************************/
/*****************************************************************************/
/**
*
* Macro to set the StartBd for the selected Tile DMA channel.
*
* @param	DmaInstPtr - Pointer to the Tile DMA instance.
* @param	ChNum - Channel number (0-S2MM0,1-S2MM1,2-MM2S0,3-MM2S1).
* @param	BdStart - BD value for the channel to start with (range:0-15).
*
* @return	None.
*
* @note		None.
*
*******************************************************************************/
#define XMeDma_TileSetStartBd(DmaInstPtr, ChNum, BdStart)			\
                        if(BdStart != 0xFFU) {                                  \
				XMeGbl_Write32((DmaInstPtr->BaseAddress +	\
				TileDmaCh[ChNum].StatQOff),			\
				(XMe_SetField(BdStart,				\
				TileDmaCh[ChNum].StatQ.Lsb,			\
				TileDmaCh[ChNum].StatQ.Mask)));			\
                        }                                                       \
                        DmaInstPtr->StartBd[ChNum] = BdStart

/************************** Function Prototypes  *****************************/
u32 XMeDma_TileInitialize(XMeGbl_Tile *TileInstPtr, XMeDma_Tile *DmaInstPtr);
void XMeDma_TileBdSetLock(XMeDma_Tile *DmaInstPtr, u8 BdNum, u8 AbType, u8 LockId, u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal);
void XMeDma_TileBdSetXy2d(XMeDma_Tile *DmaInstPtr, u8 BdNum, u8 XyType, u16 Incr, u16 Wrap, u16 Offset);
void XMeDma_TileBdSetIntlv(XMeDma_Tile *DmaInstPtr, u8 BdNum, u8 IntlvMode, u8 IntlvDb, u8 IntlvCnt, u16 IntlvCur);
void XMeDma_TileBdSetPkt(XMeDma_Tile *DmaInstPtr, u8 BdNum, u8 PktEn, u8 PktType, u8 PktId);
void XMeDma_TileBdSetAdrLenMod(XMeDma_Tile *DmaInstPtr, u8 BdNum, u16 BaseAddrA, u16 BaseAddrB, u16 Length, u8 AbMode, u8 FifoMode);
void XMeDma_TileBdSetNext(XMeDma_Tile *DmaInstPtr, u8 BdNum, u8 NextBd);
void XMeDma_TileBdWrite(XMeDma_Tile *DmaInstPtr, u8 BdNum);
void XMeDma_TileBdClear(XMeDma_Tile *DmaInstPtr, u8 BdNum);
u32 XMeDma_TileChControl(XMeDma_Tile *DmaInstPtr, u8 ChNum, u8 Reset, u8 Enable);

#endif		/* end of protection macro */
/** @} */

