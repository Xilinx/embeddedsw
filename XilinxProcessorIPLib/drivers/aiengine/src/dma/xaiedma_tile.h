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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaiedma_tile.h
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
* 1.3  Nishad  12/05/2018  Renamed ME attributes to AIE
* 1.4  Hyun    06/20/2019  Add XAieDma_TileBdClearAll() that resets all sw BDs
* 1.5  Hyun    06/20/2019  Added APIs for individual BD / Channel reset
* 1.6  Hyun    06/20/2019  Add XAieDma_TileSoftInitialize()
* </pre>
*
******************************************************************************/
#ifndef XAIEDMA_TILE_H
#define XAIEDMA_TILE_H

/***************************** Include Files *********************************/
#include "xaiegbl_reginit.h"

/************************** Variable Declarations ****************************/
extern XAieGbl_RegTileDmaBd TileBd[];
extern XAieGbl_RegTileDmaCh TileDmaCh[];

/***************************** Constant Definitions **************************/
#define XAIEDMA_TILE_MAX_NUM_CHANNELS		4U
#define XAIEDMA_TILE_MAX_NUM_DESCRS		16U
#define XAIEDMA_TILE_MAX_NUM_LOCKS		16U

#define XAIEDMA_TILE_CHNUM_S2MM0			0U
#define XAIEDMA_TILE_CHNUM_S2MM1			1U
#define XAIEDMA_TILE_CHNUM_MM2S0			2U
#define XAIEDMA_TILE_CHNUM_MM2S1			3U

#define XAIEDMA_TILE_BD0_OFFSET			0x1D000U
#define XAIEDMA_TILE_BD_SIZE			0x20U

#define XAIEDMA_TILE_BD_2DDMA_X			0U
#define XAIEDMA_TILE_BD_2DDMA_Y			1U
#define XAIEDMA_TILE_BD_ADDRA			0U
#define XAIEDMA_TILE_BD_ADDRB			1U

#define XAIEDMA_TILE_STARTBD_RESET		0xFFU

#define XAIEDMA_TILE_CHCTRL_OFFSET		0xE00U
#define XAIEDMA_TILE_STARTQ_OFFSET		0xE04U

#define XAIEDMA_TILE_CHSTS_IDLE                  0U
#define XAIEDMA_TILE_CHSTS_STARTING              1U
#define XAIEDMA_TILE_CHSTS_RUNNING               2U
#define XAIEDMA_TILE_CHSTS_INVALID               0xFFU

#define XAIEDMA_TILE_BD_VALID			1U

#define XAIEDMA_TILE_2DX_DEFAULT_INCR		0U
#define XAIEDMA_TILE_2DX_DEFAULT_WRAP		255U
#define XAIEDMA_TILE_2DX_DEFAULT_OFFSET		1U
#define XAIEDMA_TILE_2DY_DEFAULT_INCR		255U
#define XAIEDMA_TILE_2DY_DEFAULT_WRAP		255U
#define XAIEDMA_TILE_2DY_DEFAULT_OFFSET		256U

#define XAIEDMA_TILE_LOCK_ACQRELVAL_INVALID	0xFFU

#define XAIEDMA_TILE_INTLVCNT_ALIGN_MASK		0x3U
#define XAIEDMA_TILE_ADDRAB_ALIGN_MASK		0x3U
#define XAIEDMA_TILE_ADDRAB_ALIGN_OFFSET		2U

#define XAIEDMA_TILE_LENGTH128_MASK		0x3U
#define XAIEDMA_TILE_LENGTH32_OFFSET		0x2U

#define XAIEDMA_TILE_FIFO_CNT0			0x2U
#define XAIEDMA_TILE_FIFO_CNT1			0x3U

#define XAIEDMA_TILE_NUM_BD_WORDS		7U
#define XAIEDMA_TILE_BD_NEXTBD_INVALID		0xFFU

/* Offsets for the Channel control register bit fields */
#define XAIEDMA_TILE_CHCTRL_RESET_SHIFT		1U	
#define XAIEDMA_TILE_CHCTRL_ENABLE_SHIFT		0U

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
} XAieDma_TileBdLock;

/**
 * This typedef contains the X/Y 2D addressing attributes for the BD.
 */
typedef struct
{
	u16 Incr;			/**< Increment value for 2D X/Y addressing */
	u16 Wrap;			/**< Wrap value for 2D X/Y addressing */
	u16 Offset;			/**< Offset value for 2D X/Y addressing */
} XAieDma_TileBdXy;

/**
 * This typedef contains all the attributes for the BD configuration.
 */
typedef struct
{
	XAieDma_TileBdLock AddrA;	/**< AddressA lock and address attributes for double buffering */
	XAieDma_TileBdLock AddrB;	/**< AddressB lock and address attributes for double buffering */
	XAieDma_TileBdXy X2dCfg;		/**< 2D addressing attributes for X */
	XAieDma_TileBdXy Y2dCfg;		/**< 2D addressing attributes for Y */
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
} XAieDma_TileBd;

/**
 * This typedef is the Tile DMA instance. User is required to allocate memory for this Tile DMA instance
 * and a pointer of the same is passed to the Tile DMA driver functions. Each Tile DMA in the array is
 * required to have its own and unique instance structure.
 */
typedef struct
{
	u64 BaseAddress;		                        /**< Tile DMA base address */
        u32 IsReady;		                                /**< Device is initialized and ready */
	u8 StartBd[XAIEDMA_TILE_MAX_NUM_CHANNELS];		/**< Start BD value for all the 4 channels */
	XAieDma_TileBd Descrs[XAIEDMA_TILE_MAX_NUM_DESCRS];	/**< Data structure to hold the 16 descriptors of the Tile DMA */
} XAieDma_Tile;

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
#define XAieDma_TileSetStartBd(DmaInstPtr, ChNum, BdStart)			\
                        if(BdStart != 0xFFU) {                                  \
				XAieGbl_Write32((DmaInstPtr->BaseAddress +	\
				TileDmaCh[ChNum].StatQOff),			\
				(XAie_SetField(BdStart,				\
				TileDmaCh[ChNum].StatQ.Lsb,			\
				TileDmaCh[ChNum].StatQ.Mask)));			\
                        }                                                       \
                        DmaInstPtr->StartBd[ChNum] = BdStart

/************************** Function Prototypes  *****************************/
u32 XAieDma_TileSoftInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Tile *DmaInstPtr);
u32 XAieDma_TileInitialize(XAieGbl_Tile *TileInstPtr, XAieDma_Tile *DmaInstPtr);
void XAieDma_TileBdSetLock(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 AbType, u8 LockId, u8 LockRelEn, u8 LockRelVal, u8 LockAcqEn, u8 LockAcqVal);
void XAieDma_TileBdSetXy2d(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 XyType, u16 Incr, u16 Wrap, u16 Offset);
void XAieDma_TileBdSetIntlv(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 IntlvMode, u8 IntlvDb, u8 IntlvCnt, u16 IntlvCur);
void XAieDma_TileBdSetPkt(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 PktEn, u8 PktType, u8 PktId);
void XAieDma_TileBdSetAdrLenMod(XAieDma_Tile *DmaInstPtr, u8 BdNum, u16 BaseAddrA, u16 BaseAddrB, u16 Length, u8 AbMode, u8 FifoMode);
void XAieDma_TileBdSetNext(XAieDma_Tile *DmaInstPtr, u8 BdNum, u8 NextBd);
void XAieDma_TileBdWrite(XAieDma_Tile *DmaInstPtr, u8 BdNum);
void XAieDma_TileBdClear(XAieDma_Tile *DmaInstPtr, u8 BdNum);
void XAieDma_TileBdClearAll(XAieDma_Tile *DmaInstPtr);
u32 XAieDma_TileChControl(XAieDma_Tile *DmaInstPtr, u8 ChNum, u8 Reset, u8 Enable);
u32 XAieDma_TileChReset(XAieDma_Tile *DmaInstPtr, u8 ChNum);
u32 XAieDma_TileChResetAll(XAieDma_Tile *DmaInstPtr);

#endif		/* end of protection macro */
/** @} */

