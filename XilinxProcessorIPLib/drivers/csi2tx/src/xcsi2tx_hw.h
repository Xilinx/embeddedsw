/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
* @file xcsi2tx_hw.h
* @addtogroup csi2tx_v1_0
* @{
*
* Hardware register & masks definition file. It defines the register interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 sss 07/15/16 Initial release
*     vsa 05/12/17 Add support for Clock Mode
* </pre>
*
*****************************************************************************/

#ifndef XCSI2TX_HW_H_
#define XCSI2TX_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/**************************** Include Files **********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register offset definitions. Register accesses are 32-bit.
 */
/** @name Device registers
 *  Register sets of MIPI CSI2 Tx Core
 *  @{
 */


#define XCSI2TX_CCR_OFFSET	0x00000000	/**< Core Configuration
						  *  Register */
#define XCSI2TX_PCR_OFFSET	0x00000004	/*Protocol Configuration
						  *  Register */
#define XCSI2TX_GIER_OFFSET	0x00000020	/**< Global Interrupt
						  *  Register */
#define XCSI2TX_ISR_OFFSET	0x00000024	/**< Interrupt Status
						  *  Register */
#define XCSI2TX_IER_OFFSET	0x00000028	/**< Interrupt Enable
						  *  Register */
#define XCSI2TX_SPKTR_OFFSET	0x00000030	/**< Generic Short Packet
						  *  Entry */
#define XCSI2TX_GSP_OFFSET	0x00000078	/* < GSP Status*/

/** @name Bitmasks and offsets of XCSI_GIER_OFFSET register
 *
 * This register contains the global interrupt enable bit.
 * @{
 */
#define XCSI2TX_GIER_GIE_MASK	0x00000001	/**< Global Interrupt
						  *  Enable bit */
#define XCSI2TX_GIER_GIE_SHIFT	0	/**< Shift bits for Global Interrupt
					  *  Enable */

#define XCSI2TX_GIER_SET	1	/**< Enable the Global Interrupts */
#define XCSI2TX_GIER_RESET 	0	/**< Disable the Global Interrupts */

/*@}*/

/** @name Bitmasks and offsets of XCSI_CCR_OFFSET register
 *
 * This register is used for the enabling/disabling and resetting
 * the core of CSI2 Tx Controller
 * @{
 */

#define XCSI2TX_CCR_COREENB_MASK	0x00000001 /* Enable/Disable core */
#define XCSI2TX_CCR_SOFTRESET_MASK	0x00000002 /* Soft Reset the core */
#define XCSI2TX_CSR_RIPCD_MASK		0x00000004 /* Core ready */
#define XCSI2TX_CCR_ULPS_MASK		0x00000008 /* ULPS */
#define XCSI2TX_CCR_CLKMODE_MASK	0x00000010 /* Clock Mode */
#define XCSI2TX_CCR_COREENB_SHIFT	0 	/* Shift bit for Core Enable*/
#define XCSI2TX_CCR_SOFTRESET_SHIFT	1 	/* Shift bit for Soft reset */
#define XCSI2TX_CSR_RIPCD_SHIFT		2 	/* Bit Shift for Core Ready */
#define XCSI2TX_CCR_ULPS_SHIFT 		3 	/* Shift bits for ulps */
#define XCSI2TX_CCR_CLKMODE_SHIFT	4 	/* Shift bits for clock mode */
/*@}*/

/** @name Bitmasks and offset of XCSI2TX_PCR_OFFSET register
 *
 * This register reports the number of lanes configured during core generation
 * and number of lanes actively used.
 * @{
 */
/* Mask bits */
#define XCSI2TX_PCR_LINEGEN_MASK	0x00008000 /* Line generation Mode */
#define XCSI2TX_PCR_PIXEL_MASK		0x00006000 /* Pixel Mode */
#define XCSI2TX_PCR_MAXLANES_MASK	0x00000018 /* Maximum lanes in core */
#define XCSI2TX_PCR_ACTLANES_MASK	0x00000003 /* Active  lanes in core */

/* Shift bits */
#define XCSI2TX_PCR_LINEGEN_SHIFT	15 	/* Line generation */
#define XCSI2TX_PCR_PIXEL_SHIFT		13 	/* Pixel Mode */
#define XCSI2TX_PCR_MAXLANES_SHIFT	3 	/* Max Lanes */
#define XCSI2TX_PCR_ACTLANES_SHIFT	0 	/* Active Lanes */

/*@}*/

#define XCSI2TX_GSP_MASK		0x00000001F /**< Number of GSPs can
						    be safely written to GSP
						    FIFO, before it goes full */
#define XCSI2TX_GSP_SHIFT		0 /** < Shift bit for GSP count*/

/** @name BitMasks interrupts
 *
 * @{
 */

#define XCSI2TX_IER_ALLINTR_MASK	0x0000003F /* All interrupts mask */
#define	XCSI2TX_ISR_ALLINTR_MASK	0x0000003F /* All interrupts mask */
#define XCSI2TX_UNDERRUN_PIXEL_MASK	(1<<0)	/* Underrun Pixel */
#define XCSI2TX_WRONG_DATATYPE_MASK	(1<<1)	/* Wrong data type */
#define XCSI2TX_LINE_BUFF_FULL_MASK	(1<<2)	/* Line buffer full */
#define XCSI2TX_DPHY_ULPS_MASK		(1<<3)	/* Dphy ulps */
#define XCSI_GPSFIFO_MASK		(1<<4)	/* GPS fifo full */
#define XCSI_INCORT_LANE_MASK		(1<<5)	/* Wrong lane configuration */

/*@}*/

/** @name BitMasks Short Packets
 *
 * @{
 */
#define XCSI2TX_SPKTR_DATA_MASK		0x00FFFF00 /* Short Packet byte0 and 1*/
#define XCSI2TX_SPKTR_DATA_SHIFT		8
#define XCSI2TX_SPKTR_VC_MASK		0x000000C0 /* Short Packet Virtual
								channel*/
#define XCSI2TX_SPKTR_VC_SHIFT		6
#define XCSI2TX_SPKTR_DT_MASK		0x0000003F /* Short Packet Data type*/
#define XCSI2TX_SPKTR_DT_SHIFT		0
/*@}*/


/*****************************************************************************/
/**
* Static inline function to read data from CSI register space
*
* @param	BaseAddress is the base address of CSI
* @param	RegOffset is the register offset.
*
* @return	Value of the register.
*
* @note		None
*
******************************************************************************/
static inline u32 XCsi2Tx_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return Xil_In32(BaseAddress + RegOffset);
}

/*****************************************************************************/
/**
* Static inline function to write data to CSI register space
*
* @param	BaseAddress is the base address of CSI
* @param	RegOffset is the register offset.
* @param	Data is the value to be written to the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XCsi2Tx_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
								u32 Data)
{
	Xil_Out32(BaseAddress + RegOffset, Data);
}

#ifdef __cplusplus
}
#endif
#endif /* end of protection macro */
/** @} */
