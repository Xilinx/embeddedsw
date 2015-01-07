/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xfsbl_csu_dma.c
 *
 * This contains code for the CSU DMA driver.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00   kc  07/22/14 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_csu_dma.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
 *
 * This function will configure dst/src DMA address
 * and size and also start the CSU DMA.
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_CsuDmaStart(XFSBL_CSU_DMA_CHANNEL Channel, u32 Addr, u32 Size)
{
	XASSERT_VOID((Addr & 3) == 0);

	if(XFSBL_CSU_DMA_SRC == Channel)
	{
		XASSERT_VOID((Size & 2) == 0);
	}
	else
	{
		XASSERT_VOID((Size & 3) == 0);
	}

	XFsbl_Out32(CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_ADDR_REG, Addr);

	/**
	 *  Writes to SIZE to start the channel, this starts
	 *  the DMA.
	 */
	/** ASM Code */
	mb();
	XFsbl_Out32(CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_SIZE_REG, Size);
}

/*****************************************************************************/
/**
 *
 * This function will stop/pause the CSU DMA.
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_CsuDmaStop(u32 Flags)
{
	/**
	 * This function is TBD.
	 */

}

/*****************************************************************************/
/**
 *
 * This function will wait for CSU DMA done.
 *
 * @param	None
 *
 * @return	None
 *
 ******************************************************************************/
void XFsbl_CsuDmaWaitForDone(XFSBL_CSU_DMA_CHANNEL Channel)
{
	volatile u32 Status = 0;

	/** ASM Code */
	mb();
	/* Busy wait for the DMA.  */
	/**
	 * TBD: May need to have some timeout for this loop.
	 */
	do
	{
		Status = XFsbl_In32(CSUDMA_BASEADDR + Channel +
				XFSBL_CSU_DMA_STATUS_REG);

	} while (Status & XFSBL_CSU_DMA_STATUS_BUSY);

}

/*****************************************************************************/
/**
 *
 * This function will start the dma transfer
 * and wait for DST DMA done.
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
void XFsbl_CsuDmaXfer(u32 SrcAddr, u32 DestAddr, u32 ImgLen)
{
	XFsbl_Printf(DEBUG_INFO,"In XCbr_CsuDmaXfer: SrcAddr:0x%x "
		"DestAddr:0x%x  ImgLen:0x%x\r\n",SrcAddr,DestAddr,ImgLen);

	XFsbl_CsuDmaStart(XFSBL_CSU_DMA_DST, DestAddr, ((ImgLen/4) << 2) );

	/**
	 * In case of PSTP, the Src DMA channel is not used.
	 * For this function SrcAddr == 0 means we are calling
	 * this function for PS_TEST boot mode
	 */
	if(0 != SrcAddr)
	{
		XFsbl_CsuDmaStart(XFSBL_CSU_DMA_SRC, SrcAddr, (ImgLen/4) << 2);
		XFsbl_CsuDmaWaitForDone(XFSBL_CSU_DMA_SRC);
	}

	/**
	 * TBD: Need to enhance this to return
	 * timeout error
	 */
	XFsbl_CsuDmaWaitForDone(XFSBL_CSU_DMA_DST);

}
