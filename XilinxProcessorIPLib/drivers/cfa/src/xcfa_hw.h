/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
*
* @file xcfa_hw.h
* @addtogroup cfa_v7_0
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Color Filter Array
* Interpolation (CFA) core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 7.0   adk    01/07/14 First release.
*                       Added the register offsets and bit masks for the
*                       registers and added backward compatibility for macros.
* </pre>
*
******************************************************************************/

#ifndef XCFA_HW_H_
#define XCFA_HW_H_	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name General control registers offsets
 *  @{
 */
#define XCFA_CONTROL_OFFSET	0x000	/**< Control */
#define XCFA_STATUS_OFFSET	0x004	/**< Status */
#define XCFA_ERROR_OFFSET	0x008	/**< Error */
#define XCFA_IRQ_EN_OFFSET	0x00C	/**< IRQ Enable */
#define XCFA_VERSION_OFFSET	0x010	/**< Version */
#define XCFA_SYSDEBUG0_OFFSET	0x014	/**< System Debug 0 */
#define XCFA_SYSDEBUG1_OFFSET	0x018	/**< System Debug 1 */
#define XCFA_SYSDEBUG2_OFFSET	0x01C	/**< System Debug 2 */

/* Timing control registers */
#define XCFA_ACTIVE_SIZE_OFFSET	0x020	/**< Active Size (V x H) */

/* Core specific registers offset */
#define XCFA_BAYER_PHASE_OFFSET	0x100	/**< Bayer_phase RW user register */
/*@}*/

/** @name Control register bit mask definition
 * @{
 */
#define XCFA_CTL_SW_EN_MASK	0x00000001	/**< Enable Mask */
#define XCFA_CTL_RUE_MASK	0x00000002	/**< Register Update Mask */
#define XCFA_CTL_BPE_MASK	0x00000010	/**< Bypass Mask */
#define XCFA_CTL_TPE_MASK	0x00000020	/**< Test pattern Mask */

#define XCFA_CTL_AUTORESET_MASK	0x40000000	/**< Software Reset -
						  *  Auto-synchronize to SOF
						  *  Mask */
#define XCFA_CTL_RESET_MASK	0x80000000	/**< Software Reset -
						  *  Instantaneous Mask */
/*@}*/

/** @name Interrupt Register Bit Masks. It is applicable for
 *	  Status and Irq_Enable Registers
 * @{
 */
#define XCFA_IXR_PROCS_STARTED_MASK	0x00000001	/**< Process Started
							  *  Mask */
#define XCFA_IXR_EOF_MASK	0x00000002	/**< End-Of-Frame Mask */
#define XCFA_IXR_SE_MASK	0x00010000	/**< Slave Error Mask */
#define XCFA_IXR_ALLINTR_MASK	0x00010003	/**< Interrupt All Error Mask
						  *  (ORing of all
						  *  Interrupt Mask) */
/*@}*/

/** @name Error Register bit mask definitions
 * @{
 */
#define XCFA_ERR_EOL_EARLY_MASK	0x00000001	/**< Error: End of line
						  *  Early Mask */
#define XCFA_ERR_EOL_LATE_MASK 	0x00000002	/**< Error: End of line
						  *  Late Mask */
#define XCFA_ERR_SOF_EARLY_MASK	0x00000004	/**< Error: Start of frame
						  *  Early Mask */
#define XCFA_ERR_SOF_LATE_MASK	0x00000008  	/**< Error: Start of frame
						  *  Late Mask */
/*@}*/

/** @name Version register bit definition and shifts
 * @{
 */
#define XCFA_VER_REV_NUM_MASK	0x000000FF	/**< Revision Number Mask */
#define XCFA_VER_PID_MASK	0x00000F00	/**< Patch ID Mask */
#define XCFA_VER_MINOR_MASK	0x00FF0000	/**< Version Minor Mask */
#define XCFA_VER_MAJOR_MASK	0xFF000000	/**< Version Major Mask */
#define XCFA_VER_REV_MASK	0x0000F000	/**< Version revision Mask */
#define XCFA_VER_MAJOR_SHIFT	24		/**< Version Major Shift */
#define XCFA_VER_MINOR_SHIFT	16		/**< Version Minor Shift */
#define XCFA_VER_INTERNAL_SHIFT	8		/**< Version Internal Shift */
#define XCFA_VER_REV_SHIFT	12		/**< Version Revision Shift */
/*@}*/

/** @name Active size register bit mask definition and shifts
 * @{
 */
#define XCFA_ACTSIZE_NUM_PIXEL_MASK	0x00001FFF	/**< Active size
							  *  Mask */
#define XCFA_ACTSIZE_NUM_LINE_MASK	0x1FFF0000	/**< Number of Active
							  *  lines per
							  *  Frame
							  *  (Vertical) */
#define XCFA_ACTSIZE_NUM_LINE_SHIFT	16		/**< Active size
							  *  Shift */
/*@}*/

/** @name Bayer Phase
 * @{
 */
#define XCFA_BAYER_PHASE_MASK	0x00000003	/**< Bayer Phase Mask */

/*@}*/

/** @name General purpose masks
 * @{
 */
#define XCFA_8_BIT_MASK		0x0FF		/**< Generic 8 bit Mask */
/*@}*/

/** @name Backward compatibility macros
 *  @{
 */
#define CFA_CONTROL		XCFA_CONTROL_OFFSET
#define CFA_STATUS		XCFA_STATUS_OFFSET
#define CFA_ERROR		XCFA_ERROR_OFFSET
#define CFA_IRQ_EN		XCFA_IRQ_EN_OFFSET
#define CFA_VERSION		XCFA_VERSION_OFFSET
#define CFA_SYSDEBUG0		XCFA_SYSDEBUG0_OFFSET
#define CFA_SYSDEBUG1		XCFA_SYSDEBUG1_OFFSET
#define CFA_SYSDEBUG2		XCFA_SYSDEBUG2_OFFSET
#define CFA_ACTIVE_SIZE		XCFA_ACTIVE_SIZE_OFFSET
#define CFA_BAYER_PHASE		XCFA_BAYER_PHASE_OFFSET
#define CFA_CTL_EN_MASK		XCFA_CTL_SW_EN_MASK
#define CFA_CTL_RUE_MASK 	XCFA_CTL_RUE_MASK
#define CFA_CTL_CS_MASK		XCFA_CTL_CS_MASK
#define CFA_RST_RESET		XCFA_CTL_RESET_MASK
#define CFA_RST_AUTORESET	XCFA_CTL_AUTORESET_MASK

#define CFA_In32		XCfa_In32
#define CFA_Out32		XCfa_Out32

#define CFA_ReadReg		XCfa_ReadReg
#define CFA_WriteReg		XCfa_WriteReg
/*@}*/

/** @name Interrupt Enable and Status Registers Offsets
 * @{
 */
/**
* Interrupt status register generates a interrupt if the corresponding bits of
* interrupt enable register bits are set.
*/
#define XCFA_ISR_OFFSET	XCFA_STATUS_OFFSET	/**< Interrupt Status Offset */
#define XCFA_IER_OFFSET	XCFA_IRQ_EN_OFFSET	/**< Interrupt Enable Offset */
 /*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define XCfa_In32	Xil_In32	/**< Cfa Input Operation. */
#define XCfa_Out32	Xil_Out32	/**< Cfa Output Operation. */

/*****************************************************************************/
/**
*
* This function macro reads the given register.
*
* @param	BaseAddress is the base address of the CFA core.
* @param	RegOffset is the register offset of the core (defined at
*		top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XCfa_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XCfa_ReadReg(BaseAddress, RegOffset) \
	XCfa_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This function macro writes the given register.
*
* @param	BaseAddress is the base address of the CFA core.
* @param	RegOffset is the register offset of the core (defined at
*		top of this file).
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XCfa_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XCfa_WriteReg(BaseAddress, RegOffset, Data) \
	XCfa_Out32((BaseAddress) + (u32)(RegOffset), (Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
