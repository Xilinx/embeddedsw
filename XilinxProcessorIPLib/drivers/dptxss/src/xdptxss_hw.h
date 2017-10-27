/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
*
* @file xdptxss_hw.h
* @addtogroup dptxss_v5_0
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx DisplayPort Transmitter
* Subsystem core.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver
* xdptxss.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- --------------------------------------------------
* 1.00 sha 01/29/15 Initial release.
* </pre>
*
******************************************************************************/
#ifndef XDPTXSS_HW_H_
#define XDPTXSS_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* 0x000: LINK_BW_SET */
#define XDPTXSS_LINK_BW_SET_162GBPS	XDP_TX_LINK_BW_SET_162GBPS /**< 1.62
								     *  Gbps
								     * link
								     * rate. */
#define XDPTXSS_LINK_BW_SET_270GBPS	XDP_TX_LINK_BW_SET_270GBPS /**< 2.70
								     *  Gbps
								     * link
								     * rate. */
#define XDPTXSS_LINK_BW_SET_540GBPS	XDP_TX_LINK_BW_SET_540GBPS /**< 5.40
								     *  Gbps
								     * link
								     * rate. */

/* 0x001: LANE_COUNT_SET */
#define XDPTXSS_LANE_COUNT_SET_1	XDP_TX_LANE_COUNT_SET_1	/**< Lane count
								  *  of 1. */
#define XDPTXSS_LANE_COUNT_SET_2	XDP_TX_LANE_COUNT_SET_2	/**< Lane count
								  *  of 2. */
#define XDPTXSS_LANE_COUNT_SET_4	XDP_TX_LANE_COUNT_SET_4	/**< Lane count
								  *  of 4. */

/* 0x144: INTERRUPT_MASK */
#define XDPTXSS_INTERRUPT_MASK		XDP_TX_INTERRUPT_MASK	/**< Masks the
								  *  specified
								  *  interrupt
								  *  sources */
#define XDPTXSS_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK \
		XDP_TX_INTERRUPT_MASK_HPD_PULSE_DETECTED_MASK	/**< Mask HPD
								  *  pulse
								  *  detected
								  *  interrupt
								  */
#define XDPTXSS_INTERRUPT_MASK_HPD_EVENT_MASK \
		XDP_TX_INTERRUPT_MASK_HPD_EVENT_MASK	/**< Mask HPD event
							  *  interrupt. */

#define XDPTXSS_NUM_STREAMS		4	/**< Maximum number of
						  *  streams supported */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XDpTxSs_In32		Xil_In32	/**< Input Operations */
#define XDpTxSs_Out32		Xil_Out32	/**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a DisplayPort Transmitter Subsystem register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XDpTxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XDpTxSs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDpTxSs_ReadReg(BaseAddress, RegOffset) \
	XDpTxSs_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a DisplayPort Transmitter Subsystem register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XDpTxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpTxSs_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
*		u32 Data)
*
******************************************************************************/
#define XDpTxSs_WriteReg(BaseAddress, RegOffset, Data) \
	XDpTxSs_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
