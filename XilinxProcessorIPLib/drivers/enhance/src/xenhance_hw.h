/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
*
* @file xenhance_hw.h
* @addtogroup enhance_v7_1
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Video Image Enhancement
* core.
*
* For more information about the operation of this core, see the hardware
* specification and documentation in the higher level driver xenhance.h source
* code file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -------------------------------------------------------
* 7.0   adk 01/07/14 First release.
*                    Added the register offsets and bit masks for the
*                    registers and added backward compatibility for macros.
* </pre>
*
******************************************************************************/

#ifndef XENHANCE_HW_H_
#define XENHANCE_HW_H_		/**< Prevent circular inclusions by using
				  *  protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Control Registers
 * @{
 */
#define XENH_CONTROL_OFFSET	0x0000	/**< Control Offset */
#define XENH_STATUS_OFFSET	0x0004	/**< Status Offset */
#define XENH_ERROR_OFFSET	0x0008	/**< Error Offset */
#define XENH_IRQ_EN_OFFSET	0x000C	/**< IRQ Enable	Offset */
#define XENH_VERSION_OFFSET	0x0010	/**< Version Offset */
#define XENH_SYSDEBUG0_OFFSET	0x0014	/**< System Debug 0
					  *  Offset */
#define XENH_SYSDEBUG1_OFFSET	0x0018	/**< System Debug 1
					  *  Offset */
#define XENH_SYSDEBUG2_OFFSET	0x001C	/**< System Debug 2
					  *  Offset */
/*@}*/

/** @name Timing Control Registers
 * @{
 */
#define XENH_ACTIVE_SIZE_OFFSET	0x0020	/**< Horizontal and Vertical
					  *  Active Frame Size Offset */
/*@}*/

/** @name Core Specific Registers
 * @{
 */
#define XENH_NOISE_THRESHOLD_OFFSET	0x0100	/**< Noise Reduction
						  *  Control Active */
#define XENH_ENHANCE_STRENGTH_OFFSET	0x0104	/**< Edge Enhancement
						  *  Control Active */
#define XENH_HALO_SUPPRESS_OFFSET	0x0108	/**< Halo Suppression
						  *  Control Active */
/*@}*/

/** @name Enhance Control Register Bit Masks
 * @{
 */
#define XENH_CTL_SW_EN_MASK	0x00000001	/**< Enable Mask */
#define XENH_CTL_RUE_MASK	0x00000002	/**< Register
						  *  Update Enable Mask */
#define XENH_CTL_BPE_MASK	0x00000010	/**< Bypass Mask */
#define XENH_CTL_TPE_MASK	0x00000020	/**< Test Pattern Mask */
#define XENH_CTL_AUTORESET_MASK	0x40000000	/**< Software Reset -
						  *  Auto-synchronize to
						  *  SOF Mask */
#define XENH_CTL_RESET_MASK	0x80000000	/**< Software Reset -
						  *  Instantaneous
						  *  Mask */
/*@}*/

/** @name Interrupt Register Bit Masks. It is applicable for
 *	  Status and Irq_Enable Registers
 * @{
 */
#define XENH_IXR_PROCS_STARTED_MASK	0x00000001	/**< Process started
						  *  Mask */
#define XENH_IXR_EOF_MASK	0x00000002	/**< End-Of-Frame Mask */
#define XENH_IXR_SE_MASK	0x00010000	/**< Slave Error Mask */
#define XENH_IXR_ALLINTR_MASK	0x00010003	/**< OR'ing of all Mask */
/*@}*/

/** @name Enhance Error Register Bit Masks
 * @{
 */
#define XENH_ERR_EOL_EARLY_MASK	0x00000001	/**< Frame EOL early Mask */
#define XENH_ERR_EOL_LATE_MASK	0x00000002	/**< Frame EOL late Mask */
#define XENH_ERR_SOF_EARLY_MASK	0x00000004	/**< Frame SOF early Mask */
#define XENH_ERR_SOF_LATE_MASK	0x00000008	/**< Frame SOF late Mask */
/*@}*/

/** @name Enhance Version Register bit definition
 * @{
 */
#define XENH_VER_REV_NUM_MASK	0x000000FF	/**< Revision Number Mask */
#define XENH_VER_PID_MASK	0x00000F00	/**< Patch ID Mask */
#define XENH_VER_MINOR_MASK	0x00FF0000	/**< Version Minor Mask */
#define XENH_VER_MAJOR_MASK	0xFF000000	/**< Version Major Mask */
#define XENH_VER_REV_MASK	0x0000F000	/**< VersionRevision Mask */
#define XENH_VER_INTERNAL_SHIFT	8		/**< Version Internal Shift */
#define XENH_VER_REV_SHIFT	12		/**< Version Revision Shift */
#define XENH_VER_MINOR_SHIFT	16		/**< Version Minor Shift */
#define XENH_VER_MAJOR_SHIFT	24		/**< Version Major Shift */
/*@}*/

/** @name Enhance ActiveSize register Masks and Shifts
 * @{
 */
#define XENH_ACTSIZE_NUM_PIXEL_MASK	0x00001FFF	/**< Active size
							  *  Mask  */
#define XENH_ACTSIZE_NUM_LINE_MASK	0x1FFF0000	/**< Number of Active
							  *  lines per Frame
							  * (Vertical) Mask */
#define XENH_ACTSIZE_NUM_LINE_SHIFT	16		/**< Active size
							  *  Shift */
/*@}*/

/** @name Enhance Noise Threshold Register Bit Masks
 * @{
 */
#define XENH_NOISE_THRESHOLD_MASK 0x0000FFFF	/**< Noise Threshold
							  *  Mask */
/*@}*/

/** @name Enhance Strength Register Bit Masks
 * @{
 */
#define XENH_STRENGTH_MASK	0x0000FFFF	/**< Enhance Strength
							  *  Mask */
/*@}*/

/** @name Enhance Halo Suppress Register Bit Masks
* @{
*/
#define XENH_HALO_SUPPRESS_MASK	0x0000FFFF	/**< Halo Suppress
							  *  Mask */
/*@}*/

/**@name Backward compatibility macros
 * @{
 */
#define ENHANCE_CONTROL			XENH_CONTROL_OFFSET
#define ENHANCE_STATUS			XENH_STATUS_OFFSET
#define ENHANCE_ERROR			XENH_ERROR_OFFSET
#define ENHANCE_IRQ_ENABLE		XENH_IRQ_EN_OFFSET
#define ENHANCE_VERSION			XENH_VERSION_OFFSET
#define ENHANCE_SYSDEBUG0		XENH_SYSDEBUG0_OFFSET
#define ENHANCE_SYSDEBUG1		XENH_SYSDEBUG1_OFFSET
#define ENHANCE_SYSDEBUG2		XENH_SYSDEBUG2_OFFSET
#define ENHANCE_ACTIVE_SIZE		XENH_ACTIVE_SIZE_OFFSET
#define ENHANCE_NOISE_THRESHOLD		XENH_NOISE_THRESHOLD_OFFSET
#define ENHANCE_ENHANCE_STRENGTH	XENH_ENHANCE_STRENGTH_OFFSET
#define ENHANCE_HALO_SUPPRESS		XENH_HALO_SUPPRESS_OFFSET

#define ENHANCE_CTL_EN_MASK		XENH_CTL_SW_EN_MASK
#define ENHANCE_CTL_RU_MASK		XENH_CTL_RUE_MASK
#define ENHANCE_CTL_RESET		XENH_CTL_RESET_MASK
#define ENHANCE_CTL_AUTORESET		XENH_CTL_AUTORESET_MASK

#define ENHANCE_In32			XEnhance_In32
#define ENHANCE_Out32			XEnhance_Out32
#define ENHANCE_ReadReg			XEnhance_ReadReg
#define ENHANCE_WriteReg		XEnhance_WriteReg
/*@}*/

/** @name Interrupt Registers
 * @{
 */
/**
* Interrupt status register generates a interrupt if the corresponding bits of
* interrupt enable register bits are set.
*/

#define XENH_ISR_OFFSET	XENH_STATUS_OFFSET	/**< Interrupt Status
						  *  Register */
#define XENH_IER_OFFSET	XENH_IRQ_EN_OFFSET	/**< Interrupt Enable
						  *  Register corresponds
						  *  to Status bits */
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

#define XEnhance_In32	Xil_In32	/**< Enhance Input Operation. */
#define XEnhance_Out32	Xil_Out32	/**< Enhance Output Operation. */

/*****************************************************************************/
/**
*
* This function macro reads the given register.
*
* @param	BaseAddress is the base address of the Image Enhancement core.
* @param	RegOffset is the register offset of the core (defined at.
*		top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:2
* 		u32 XEnhance_ReadReg(u32 BaseAddress, u32 RegOffset).
*
******************************************************************************/
#define XEnhance_ReadReg(BaseAddress, RegOffset) \
	XEnhance_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This function macro writes the given register.
*
* @param	BaseAddress is the base address of the Image Enhancement core.
* @param	RegOffset is the register offset of the core (defined
*		at top of this file).
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XEnhance_WriteReg(u32 BaseAddress, u32 RegOffset,
*		u32 Data).
*
******************************************************************************/
#define XEnhance_WriteReg(BaseAddress, RegOffset, Data) \
	XEnhance_Out32((BaseAddress) + (u32)(RegOffset), (Data))

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif /* End of protection macro */
/** @} */
