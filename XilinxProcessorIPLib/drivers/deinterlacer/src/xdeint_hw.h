/******************************************************************************
*
* Copyright (C) 2011 - 2014 Xilinx, Inc.  All rights reserved.
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
*
* @file xdeint_hw.h
* @addtogroup deinterlacer_v3_2
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Video Deinterlacer core.
*
* For more information about the operation of this core, see the hardware
* specification and documentation in the higher level core xdeint.h source
* code file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a rjh    07/10/11 First release.
* 2.00a rjh    18/01/12 Updated for v_deinterlacer 2.00.
* 3.2   adk    02/13/14 Suffixed "_OFFSET" to all register offset macros.
*                       Added bit masks for the registers and added
*                       backward compatibility for macros.
*                       Swapped bit definitions of XDEINT_MODE_COLOUR_YUV
*                       and XDEINT_MODE_COLOUR_RGB.
*                       Modified bit definitions of version register.
* </pre>
*
******************************************************************************/

#ifndef XDEINT_HW_H
#define XDEINT_HW_H	/**< Prevent circular inclusions
			  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offsets:
 *  @{
 */
#define XDEINT_CONTROL_OFFSET		0x000	/**< Deinterlacer Main
						  *  Control */
#define XDEINT_MODE_OFFSET		0x004	/**< Deinterlacer internal
						  *  Modes */
#define XDEINT_IER_OFFSET		0x008	/**< Interrupt Enable
						  *  Control */
#define XDEINT_ISR_OFFSET		0x00C	/**< Interrupt Enable Status */
#define XDEINT_HEIGHT_OFFSET		0x010	/**< Height */
#define XDEINT_WIDTH_OFFSET		0x014	/**< Width */
#define XDEINT_THRESH1_OFFSET		0x018	/**< T1 Threshold */
#define XDEINT_THRESH2_OFFSET		0x01C	/**< T2 Threshold */
#define XDEINT_XFADE_OFFSET		0x020	/**< Cross Fade Ration */
#define XDEINT_BUFFER0_OFFSET		0x024	/**< VFBC Field Buffer 0
						  *  Base */
#define XDEINT_BUFFER1_OFFSET		0x028	/**< VFBC Field Buffer 1
						  *  Base */
#define XDEINT_BUFFER2_OFFSET		0x02C	/**< VFBC Field Buffer 2
						  *  Base */
#define XDEINT_BUFSIZE_OFFSET		0x030	/**< VFBC Field Buffer Page
						  *  size in 32bit Words */
#define XDEINT_VER_OFFSET		0x0F0	/**< Hardware Version ID */
#define XDEINT_RESET_OFFSET		0x100	/**< Soft Reset */
/*@}*/

/** @name Interrupt Status/Enable Register bit definitions:
 *  @{
 */
#define XDEINT_IXR_UPDATE_MASK		0x00000001	/**< Internal Register
							  *  update done */
#define XDEINT_IXR_LOCKED_MASK		0x00000002	/**< Deinterlacer is
							  *  locked to incoming
							  *  video */
#define XDEINT_IXR_UNLOCKED_MASK	0x00000004	/**< Deinterlacer has
							  *  lost lock to
							  *  incoming
							  *  video */
#define XDEINT_IXR_ERROR_MASK		0x00000008	/**< Deinterlacer
							  *  internal
							  *  FIFO error */
#define XDEINT_IXR_PULL_ON_MASK		0x00000010	/**< Pull down
							  *  activated */
#define XDEINT_IXR_PULL_OFF_MASK	0x00000020	/**< Pull down
							  *  cancelled */
#define XDEINT_IXR_FRAME_MASK		0x00000040	/**< Frame Tick */
#define XDEINT_IXR_FS_CFG_ERROR_MASK	0x00000100	/**< Frame store Write
							  *  setup error */
#define XDEINT_IXR_FS_WR_ERROR_MASK	0x00000200	/**< Frame store Write
							  *  FIFO overflow */
#define XDEINT_IXR_FS_RD_FIELD_ERROR_MASK 0x00000400	/**< Frame store Read
							  *  Field under run */
#define XDEINT_IXR_FS_RD_FRAME_ERROR_MASK 0x00000800	/**< Frame store Read
							  * Frame under run */

#define XDEINT_IXR_ALLINTR_MASK		(XDEINT_IXR_UPDATE_MASK | \
					XDEINT_IXR_LOCKED_MASK  | \
					XDEINT_IXR_UNLOCKED_MASK| \
					XDEINT_IXR_ERROR_MASK   | \
					XDEINT_IXR_PULL_ON_MASK | \
					XDEINT_IXR_PULL_OFF_MASK| \
					XDEINT_IXR_FRAME_MASK   | \
					XDEINT_IXR_FS_CFG_ERROR_MASK    | \
					XDEINT_IXR_FS_WR_ERROR_MASK     | \
					XDEINT_IXR_FS_RD_FIELD_ERROR_MASK | \
					XDEINT_IXR_FS_RD_FRAME_ERROR_MASK)
							/**< Mask for all
							  *  interrupts */
/*@}*/

/** @name Error Status/ bit definitions:
 *  @{
 */
#define XDEINT_STS_ERROR		0x00000008	/**< Deinterlacer
							  *  internal FIFO
							  *  error */
#define XDEINT_STS_FS_CFG_ERROR		0x00000100	/**< Frame store Write
							  *  setup error */
#define XDEINT_STS_FS_WR_ERROR		0x00000200	/**< Frame store Write
							  *  FIFO overflow */
#define XDEINT_STS_FS_RD_FIELD_ERROR	0x00000400	/**< Frame store Read
							  *  Field under run */
#define XDEINT_STS_FS_RD_FRAME_ERROR	0x00000800	/**< Frame store Read
							  *  Frame under run */
/*@}*/

#define XDEINT_RESET_RESET_MASK		0x00000001	/**< Software Reset */

/** @name Deinterlacer Control Fields:
 *  @{
 */
#define XDEINT_VER_MAJOR_MASK		0xFF000000	/**< Major Version */
#define XDEINT_VER_MAJOR_SHIFT		24		/**< Major Bit Shift */
#define XDEINT_VER_MINOR_MASK		0x00F00000	/**< Minor Version */
#define XDEINT_VER_MINOR_SHIFT		20		/**< Minor Bit Shift */
#define XDEINT_VER_REV_MASK		0x000F0000	/**< Revision
							  *  Version */
#define XDEINT_VER_REV_SHIFT		16		/**< Revision Bit
							  *  Shift */
/*@}*/

/** @name Deinterlacer Control Fields:
 *  @{
 */
#define XDEINT_CTL_UPDATE_REQ	0x00000001	/**< Queue a register update
						  *  request */
#define XDEINT_CTL_ENABLE	0x00000002	/**< Enable/Disable
						  *  Deinterlacer algorithms */
#define XDEINT_CTL_ACCEPT_VIDEO	0x00000004	/**< Accept Video into the
						  *  Deinterlacer */
/*@}*/

/** @name Deinterlacer Mode Fields:
 *  @{
 */
#define XDEINT_MODE_ALGORITHM_0		0x00000001	/**< Deinterlacer
							  *  algorithm */
#define XDEINT_MODE_ALGORITHM_1		0x00000002	/**< Deinterlacer
							  *  algorithm */
#define XDEINT_MODE_COL			0x00000004	/**< Color Space */
#define XDEINT_MODE_PACKING_0		0x00000008	/**< XSVI Packing */
#define XDEINT_MODE_PACKING_1		0x00000010	/**< XSVI Packing */
#define XDEINT_MODE_FIELD_ORDER		0x00000020	/**< First field
							  *  order */
#define XDEINT_MODE_PSF_ENABLE		0x00000040	/**< PSF pass through
							  *  enable */
#define XDEINT_MODE_PULL_32_ENABLE	0x00000080	/**< Pull down 3:2
							  *  control enable */
#define XDEINT_MODE_PULL_22_ENABLE	0x00000100	/**< Pull down 2:2
							  *  control enable */
#define XDEINT_MODE_PULL_22_FIELDP	0x00000200	/**< Pull down 2:2
							  *  Field
							  *  Precedence */
#define XDEINT_MODE_COLOUR_YUV		0x00000000	/**< Deinterlacer
							  *  color space */
#define XDEINT_MODE_COLOUR_RGB		0x00000004	/**< Deinterlacer
							  *  color space */
#define XDEINT_MODE_ALGORITHM_RAW	0x00000000	/**< Deinterlacer
							  *  algorithm
							  *  option 0 */
#define XDEINT_MODE_ALGORITHM_DIAG	0x00000001	/**< Deinterlacer
							  *  algorithm
							  *  option 1 */
#define XDEINT_MODE_ALGORITHM_MOTION	0x00000002	/**< Deinterlacer
							  *  algorithm
							  *  option 2 */
#define XDEINT_MODE_ALGORITHM_FULL	0x00000003	/**< Deinterlacer
							  *  algorithm
							  *  option 3 */
#define XDEINT_MODE_PACKING_420		0x00000000	/**< XSVI Packing mode
							  *  420 */
#define XDEINT_MODE_PACKING_422		0x00000008	/**< XSVI Packing mode
							  *  422 */
#define XDEINT_MODE_PACKING_444		0x00000010	/**< XSVI Packing mode
							  *  444 */
#define XDEINT_MODE_FIELD_EVEN_FIRST	0x00000020	/**< First field of
							  *  frame
							  *  contains even
							  *  video lines */
#define XDEINT_MODE_FIELD_ODD_FIRST	0x00000000	/**< First field of
							  *  frame
							  *  contains odd
							  *  video lines */

/* RMW Masking Bits.*/
#define XDEINT_MODE_ALGORITHM_MASK	0x00000003	/**< Deinterlacer
							  *  algorithm */
#define XDEINT_MODE_COL_MASK		0x00000004	/**< Color Space */
#define XDEINT_MODE_PACKING_MASK	0x00000018	/**< XSVI Packing */
#define XDEINT_MODE_FIELD_ORDER_MASK	0x00000020	/**< First field
							  *  order */
#define XDEINT_MODE_PSF_ENABLE_MASK	0x00000040	/**< PSF pass through
							  *  enable */
#define XDEINT_MODE_PULL_ENABLE_MASK	0x00000180	/**< Pull down control
							  *  enable */
/*@}*/

/** @name Deinterlacer height bit definitions:
 *  @{
 */
#define XDEINT_HEIGHT_MASK	0x000007FF	/**< Deinterlacer height */
/*@}*/

/** @name Deinterlacer width bit definitions:
 *  @{
 */
#define XDEINT_WIDTH_MASK	0x000007FF	/**< Deinterlacer width */
/*@}*/

/** @name Deinterlacer Threshold T1 /T2 bit definitions:
 *  @{
 */
#define XDEINT_THRESHOLD_MASK	0x000003FF	/**< Deinterlacer threshold */
/*@}*/

/** @name Deinterlacer cross fade cycle bit definitions:
 *  @{
 */
#define XDEINT_XFADE_MASK	0x0000FFFF	/**< Deinterlacer Cross fade
						  *  cycle */
/*@}*/

/** @name Deinterlacer Buffer cycle bit definitions:
 *  @{
 */
#define XDEINT_BUF_SIZE_MASK	0x00FFFFFF	/**< Deinterlacer  Buffer
						  *  size mask */
/*@}*/

/* (4096*256) */
#define XDEINT_FADE_RATIO	1048576	/**< Fade ratio */

/**@name Backward compatibility macros
 * @{
 */
#define XDEINT_CONTROL	XDEINT_CONTROL_OFFSET
#define XDEINT_MODE	XDEINT_MODE_OFFSET
#define XDEINT_IER	XDEINT_IER_OFFSET
#define XDEINT_ISR	XDEINT_ISR_OFFSET
#define XDEINT_HEIGHT	XDEINT_HEIGHT_OFFSET
#define XDEINT_WIDTH	XDEINT_WIDTH_OFFSET
#define XDEINT_T1	XDEINT_THRESH1_OFFSET
#define XDEINT_T2	XDEINT_THRESH2_OFFSET
#define XDEINT_XFADE	XDEINT_XFADE_OFFSET
#define XDEINT_FS_BASE0	XDEINT_BUFFER0_OFFSET
#define XDEINT_FS_BASE1	XDEINT_BUFFER1_OFFSET
#define XDEINT_FS_BASE2	XDEINT_BUFFER2_OFFSET
#define XDEINT_FS_WORDS	XDEINT_BUFSIZE_OFFSET
#define XDEINT_VER	XDEINT_VER_OFFSET
#define XDEINT_RESET	XDEINT_RESET_OFFSET
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Core register I/O APIs:
 *  @{
 */
#define XDeint_In32	Xil_In32
#define XDeint_Out32	Xil_Out32

/*****************************************************************************/
/**
*
* This macro reads the given register.
*
* @param	BaseAddress is the base address of the core.
* @param	RegOffset is the register offset to be read.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XDeint_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDeint_ReadReg(BaseAddress, RegOffset) \
		XDeint_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This macro writes the given register.
*
* @param	BaseAddress is the base address of the core.
* @param	RegOffset is the register offset to be written.
* @param	Data is the 32-bit value to write to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XDeint_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XDeint_WriteReg(BaseAddress, RegOffset, Data) \
		XDeint_Out32((BaseAddress) + (u32)(RegOffset), (Data))

/*@}*/

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}

#endif

#endif	/* End of protection macro */
/** @} */
