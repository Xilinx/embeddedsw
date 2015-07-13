/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xosd_hw.h
* @addtogroup osd_v4_0
* @{
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx Video On-Screen-Display
* (OSD) core.
*
* For more information about the operation of this core, see the hardware
* specification and documentation in the higher level driver xosd.h source
* code file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- -------------------------------------------------------
* 1.00a xd     08/01/08 First release.
* 2.00a cm     06/12/12 14.1/14.2 release with address map updated to match
*                       video over AXI4-Stream Specification.
* 2.00a cjm    12/18/12 Converted from xio.h to xil_io.h, translating
*                       basic types, MB cache functions, exceptions and
*                       assertions to xil_io format.
* 4.0   adk    02/18/14 Suffixed "_OFFSET" to all register offset macros.
*                       Added register offsets, bit masks for the registers and
*                       added backward compatibility for macros.
*
*                       Removed following macros:
*                       XOSD_GIER_GIE_MASK, XOSD_IXR_GAO_MASK
*                       XOSD_IXR_GIE_MASK, XOSD_IXR_OOE_MASK,
*                       XOSD_IXR_IUE_MASK, XOSD_IXR_VBIE_MASK,
*                       XOSD_IXR_VBIS_MASK, XOSD_IXR_FE_MASK, XOSD_IXR_FD_MASK,
*                       XOSD_IXR_ALLIERR_MASK.
* </pre>
*
******************************************************************************/

#ifndef XOSD_HW_H_
#define XOSD_HW_H_		/**< Prevent circular inclusions by using
				  *  protection macros. */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offsets
 *  @{
 */
#define XOSD_CTL_OFFSET		0x000	/**< Control Offset */
#define XOSD_ACTIVE_SIZE_OFFSET	0x020	/**< Screen Size / Output Active_Size
					  *  Offset */
#define XOSD_BC0_OFFSET		0x100	/**< Background Color Channel 0
					  *  Offset */
#define XOSD_BC1_OFFSET		0x104	/**< Background Color Channel 1
					  *  Offset */
#define XOSD_BC2_OFFSET		0x108	/**< Background Color Channel 2
					  *  Offset */

#define XOSD_L0C_OFFSET		0x110	/**< Layer 0 Control Offset */
#define XOSD_L0P_OFFSET		0x114	/**< Layer 0 Position Offset */
#define XOSD_L0S_OFFSET		0x118	/**< Layer 0 Size Offset */

#define XOSD_L1C_OFFSET		0x120	/**< Layer 1 Control Offset */
#define XOSD_L1P_OFFSET		0x124	/**< Layer 1 Position Offset */
#define XOSD_L1S_OFFSET		0x128	/**< Layer 1 Size Offset */

#define XOSD_L2C_OFFSET		0x130	/**< Layer 2 Control Offset */
#define XOSD_L2P_OFFSET		0x134	/**< Layer 2 Position Offset */
#define XOSD_L2S_OFFSET		0x138	/**< Layer 2 Size Offset */

#define XOSD_L3C_OFFSET		0x140	/**< Layer 3 Control Offset */
#define XOSD_L3P_OFFSET		0x144	/**< Layer 3 Position Offset */
#define XOSD_L3S_OFFSET		0x148	/**< Layer 3 Size Offset */

#define XOSD_L4C_OFFSET		0x150	/**< Layer 4 Control Offset */
#define XOSD_L4P_OFFSET		0x154	/**< Layer 4 Position Offset */
#define XOSD_L4S_OFFSET		0x158	/**< Layer 4 Size Offset */

#define XOSD_L5C_OFFSET		0x160	/**< Layer 5 Control Offset */
#define XOSD_L5P_OFFSET		0x164	/**< Layer 5 Position Offset */
#define XOSD_L5S_OFFSET		0x168	/**< Layer 5 Size Offset */

#define XOSD_L6C_OFFSET		0x170	/**< Layer 6 Control Offset */
#define XOSD_L6P_OFFSET		0x174	/**< Layer 6 Position Offset */
#define XOSD_L6S_OFFSET		0x178	/**< Layer 6 Size Offset */

#define XOSD_L7C_OFFSET		0x180	/**< Layer 7 Control Offset */
#define XOSD_L7P_OFFSET		0x184	/**< Layer 7 Position Offset */
#define XOSD_L7S_OFFSET		0x188	/**< Layer 7 Size Offset */

#define XOSD_GCWBA_OFFSET	0x190	/**< GPU Write Bank Address Offset */
#define XOSD_GCABA_OFFSET	0x194	/**< GPU Active Bank Address Offset */
#define XOSD_GCD_OFFSET		0x198	/**< GPU Data Offset */

#define XOSD_VER_OFFSET		0x010	/**< Version Offset */
#define XOSD_RST_OFFSET		XOSD_CTL_OFFSET /**< Software Reset Offset */

/**
 * Interrupt status register generates a interrupt if the corresponding bits
 * of interrupt enable register bits are set.
 */
#define XOSD_ISR_OFFSET		XOSD_STATUS_OFFSET/**< Interrupt Status
						    *  Register Offset */
#define XOSD_IER_OFFSET		0x00C		/**< Interrupt Enable
						  *  Register Offset */

#define XOSD_STATUS_OFFSET	0x004	/**< Status Offset */
#define XOSD_ERROR_OFFSET	0x008	/**< Error Offset */

#define XOSD_OPENC_OFFSET	0x028	/**< Output Encoding Offset */
/*@}*/

/** @name Memory footprint of layers
 *  @{
 */
#define XOSD_LAYER_SIZE	0x10	/**< Total number of Layers */
#define XOSD_LXC	0x00	/**< Layer Control */
#define XOSD_LXP	0x04	/**< Layer Position */
#define XOSD_LXS	0x08	/**< Layer Size */
/*@}*/

/** @name Graphics Controller Bank related constants
 *  @{
 */
#define XOSD_GC_BANK_NUM	2	/**< The number of Banks in each
					  *  Graphics controller */
/*@}*/

/** @name Active Size Register bit definition and Shift
 *  @{
 */
#define XOSD_ACTSIZE_NUM_PIXEL_MASK	0x00000FFF /**< Horizontal Width of
						      * OSD Output Mask */
#define XOSD_ACTSIZE_NUM_LINE_MASK	0x0FFF0000 /**< Vertical Height of
						      * OSD Output Mask */
#define XOSD_ACTSIZE_NUM_LINE_SHIFT	16	    /**< Vertical Height of
						      * OSD Output Shift */
/*@}*/

/** @name Background Color Channel 0 Mask
 *  @{
 */
#define XOSD_BC0_YG_MASK	0x00000FFF	/**< Y (luma) or Green Mask */
/*@}*/

/** @name Background Color Channel 1 Mask
 *  @{
 */
#define XOSD_BC1_UCBB_MASK	0x00000FFF	/**< U (Cb) or Blue Mask */
/*@}*/

/** @name Background Color Channel 2 Mask
 *  @{
 */
#define XOSD_BC2_VCRR_MASK	0x00000FFF	/**< V(Cr) or Red Mask */
/*@}*/

/** @name Maximum number of the layers
 *  @{
 */
#define XOSD_MAX_NUM_OF_LAYERS	8		/**< The max number of
						  *  layers */
/*@}*/

/** @name Layer Control (Layer 0 through (XOSD_MAX_NUM_OF_LAYERS - 1)) bit
 *  definition and Shifts
 *  @{
 */
#define XOSD_LXC_ALPHA_MASK	0x1FFF0000	/**< Global Alpha Value Mask */
#define XOSD_LXC_ALPHA_SHIFT	16		/**< Bit shift number of
						  *  Global Alpha Value */
#define XOSD_LXC_PRIORITY_MASK	0x00000700	/**< Layer Priority Mask */
#define XOSD_LXC_PRIORITY_SHIFT	8		/**< Bit shift number of
						  *  Layer Priority */
#define XOSD_LXC_GALPHAEN_MASK	0x00000002	/**< Global Alpha Enable
						  *  Mask */
#define XOSD_LXC_EN_MASK	0x00000001	/**< Layer Enable Mask */
/*@}*/

/** @name Layer Position (Layer 0 through (XOSD_MAX_NUM_OF_LAYERS - 1)) bit
 *  definition and Shifts
 *  @{
 */
#define XOSD_LXP_YSTART_MASK	0x0FFF0000	/**< Vertical start line of
						  *  origin of layer Mask */
#define XOSD_LXP_YSTART_SHIFT	16		/**< Bit shift of vertical start
						  *  line of origin of layer */
#define XOSD_LXP_XSTART_MASK	0x00000FFF	/**< Horizontal start pixel of
						  *  origin of layer Mask */
/*@}*/

/** @name Layer Size (Layer 0 through (XOSD_MAX_NUM_OF_LAYERS - 1)) bit
 *  definition and Shift
 *  @{
 */
#define XOSD_LXS_YSIZE_MASK	0x0FFF0000	/**< Vertical size of layer
						  *   Mask */
#define XOSD_LXS_YSIZE_SHIFT	16		/**< Bit shift number of
						  *  vertical size of layer */
#define XOSD_LXS_XSIZE_MASK	0x00000FFF	/**< Horizontal size of
						  *  layer Mask */
/*@}*/

/** @name Graphics Controller Write Bank Address bit definition and Shift
 *  @{
 */
#define XOSD_GCWBA_GCNUM_MASK	0x00000700	/**< Graphics Controller
						  *  Number Mask */
#define XOSD_GCWBA_GCNUM_SHIFT	8		/**< Bit shift of Graphics
						  *  Controller Number */
#define XOSD_GCWBA_BANK_MASK	0x00000007	/**< Controls which bank to
						  *  write GPU instructions and
						  *  Color LUT data into */
#define XOSD_GCWBA_INS0		0x00000000	/**< Instruction RAM 0 */
#define XOSD_GCWBA_INS1		0x00000001	/**< Instruction RAM 1 */
#define XOSD_GCWBA_COL0		0x00000002	/**< Color LUT RAM 0 */
#define XOSD_GCWBA_COL1		0x00000003	/**< Color LUT RAM 1 */
#define XOSD_GCWBA_TXT0		0x00000004	/**< Text RAM 0 */
#define XOSD_GCWBA_TXT1		0x00000005	/**< Text RAM 1 */
#define XOSD_GCWBA_CHR0		0x00000006	/**< Character Set RAM 0 */
#define XOSD_GCWBA_CHR1		0x00000007	/**< Character Set RAM 1 */
/*@}*/

/** @name Graphics Controller Active Bank Address bit definition and Shifts
 *  @{
 */
#define XOSD_GCABA_CHR_MASK	0xFF000000	/**< Set the active Character
						  *  Bank Mask */
#define XOSD_GCABA_CHR_SHIFT	24		/**< Bit shift of active
						  * Character Bank */
#define XOSD_GCABA_TXT_MASK	0x00FF0000	/**< Set the active Text
						  *  Bank Mask */
#define XOSD_GCABA_TXT_SHIFT	16		/**< Bit shift of active Text
						  *  Bank */
#define XOSD_GCABA_COL_MASK	0x0000FF00	/**< Set the active Color Table
						  *  Bank Mask */
#define XOSD_GCABA_COL_SHIFT	8		/**< Bit shift of active Color
						  *  Table Bank */
#define XOSD_GCABA_INS_MASK	0x000000FF	/**< Set the active instruction
						  *  Bank Mask */
/*@}*/

/** @name Version Register bit definition and Shifts
 *  @{
 */
#define XOSD_VER_MAJOR_MASK	0xFF000000	/**< Major Version Mask */
#define XOSD_VER_MAJOR_SHIFT	24		/**< Major Version Shift */
#define XOSD_VER_MINOR_MASK	0x00FF0000	/**< Minor Version Mask */
#define XOSD_VER_MINOR_SHIFT	16		/**< Minor Version Bit Shift */
#define XOSD_VER_REV_MASK	0x0000F000	/**< Revision Version Mask */
#define XOSD_VER_REV_SHIFT	12		/**< Revision Bit Shift */
/*@}*/

/** @name Software Reset
 *  @{
 */
#define XOSD_RST_RESET	XOSD_CTL_SW_RST_MASK	/**< Software Reset */
/*@}*/

/** @name Interrupt Register Bit Masks. It is applicable for ISR and IER.
 *  @{
 */
#define XOSD_IXR_LAYER0_ERROR_MASK	0x00000010	/**< Layer 0 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER1_ERROR_MASK	0x00000020	/**< Layer 1 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER2_ERROR_MASK	0x00000040	/**< Layer 2 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER3_ERROR_MASK	0x00000080	/**< Layer 3 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER4_ERROR_MASK	0x00000100	/**< Layer 4 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER5_ERROR_MASK	0x00000200	/**< Layer 5 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER6_ERROR_MASK	0x00000400	/**< Layer 6 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_LAYER7_ERROR_MASK	0x00000800	/**< Layer 7 Error
							  *  interrupt enable
							  *  Mask */
#define XOSD_IXR_PROC_STARTED_MASK	0x00000001	/**< OSD Processing started
							  *  Mask */
#define XOSD_IXR_EOF_MASK	0x00000002	/**< OSD End of frame mask */
#define XOSD_IXR_ALLERR_MASK	(XOSD_IXR_LAYER0_ERROR_MASK | \
				XOSD_IXR_LAYER1_ERROR_MASK | \
				XOSD_IXR_LAYER2_ERROR_MASK | \
				XOSD_IXR_LAYER3_ERROR_MASK | \
				XOSD_IXR_LAYER4_ERROR_MASK | \
				XOSD_IXR_LAYER5_ERROR_MASK | \
				XOSD_IXR_LAYER6_ERROR_MASK | \
				XOSD_IXR_LAYER7_ERROR_MASK )
						/**< OSD Layer 0  to Layer 7
						  *  Error Mask */

#define XOSD_IXR_ALLINTR_MASK	(XOSD_IXR_PROC_STARTED_MASK | \
				XOSD_IXR_EOF_MASK | \
				XOSD_IXR_ALLERR_MASK)	/**< OR'ing of all
							  * Masks */
/*@}*/

/** @name Layer Types
 *  @{
 */
#define XOSD_LAYER_TYPE_DISABLE	0	/**< Layer is disabled */
#define XOSD_LAYER_TYPE_GPU	1	/**< Layer's type is GPU */
#define XOSD_LAYER_TYPE_VFBC	2	/**< Layer's type is VFBC */
/*@}*/

/** @name Supported instruction numbers given an instruction memory size
 *  @{
 */
#define XOSD_INS_MEM_SIZE_TO_NUM	1	/**< Conversion to the number
						  *  of instructions from the
						  *  instruction memory size */
/*@}*/

/** @name GC instruction word offset definition
 *  @{
 */
#define XOSD_INS0	0	/**< Instruction word 0 offset */
#define XOSD_INS1	1	/**< Instruction word 1 offset */
#define XOSD_INS2	2	/**< Instruction word 2 offset */
#define XOSD_INS3	3	/**< Instruction word 3 offset */
#define XOSD_INS_SIZE	4	/**< Size of an instruction in words */
/*@}*/

/** @name GC Instruction word 0 definition and Shifts
 *  @{
 */
#define XOSD_INS0_OPCODE_MASK	0xF0000000	/**< Operation Code (OpCode)
						  *  Mask */
#define XOSD_INS0_OPCODE_SHIFT	28		/**< Bit shift number of
						  *  OpCode */
#define XOSD_INS0_GCNUM_MASK	0x07000000	/**< Graphics controller number
						  *  (GC#) Mask */
#define XOSD_INS0_GCNUM_SHIFT	24		/**< Bit shift number of GC# */
#define XOSD_INS0_XEND_MASK	0x00FFF000	/**< Horizontal end pixel of
						  *  the object Mask */

#define XOSD_INS0_XEND_SHIFT	12		/**< Bit shift number of
						  *  Horizontal end pixel of
						  *  the object */
#define XOSD_INS0_XSTART_MASK	0x00000FFF	/**< Horizontal start pixel of
						  *  the Object Mask */
/*@}*/

/** @name GC Instruction word 1 definition
 *  @{
 */
#define XOSD_INS1_TXTINDEX_MASK	0x0000000F	/**< String Index */
/*@}*/

/** @name GC Instruction word 2 definition and Shifts
 *  @{
 */
#define XOSD_INS2_OBJSIZE_MASK	0xFF000000	/**< Object Size Mask */
#define XOSD_INS2_OBJSIZE_SHIFT	24		/**< Bit shift number of Object
						  * Size */
#define XOSD_INS2_YEND_MASK	0x00FFF000	/**< Vertical end line of the
						  * object Mask */
#define XOSD_INS2_YEND_SHIFT	12		/**< Bit shift number of
						  * Vertical end line of the
						  * object */
#define XOSD_INS2_YSTART_MASK	0x00000FFF	/**< Vertical start line of the
						  * Object Mask */
/*@}*/

/** @name GC Instruction word 3 definition
 *  @{
 */
#define XOSD_INS3_COL_MASK	0x0000000F	/**< Color Index for
						  *  Box/Text Mask */
/*@}*/

/** @name GC Instruction Operation Code definition (used in Instruction word 0)
 *  @{
 */
#define XOSD_INS_OPCODE_END	0x0	/**< End of instruction list */
#define XOSD_INS_OPCODE_NOP	0x8	/**< NOP */
#define XOSD_INS_OPCODE_BOX	0xA	/**< Box */
#define XOSD_INS_OPCODE_LINE	0xC	/**< Line */
#define XOSD_INS_OPCODE_TXT	0xE	/**< Text */
#define XOSD_INS_OPCODE_BOXTXT	0xF	/**< Box Text */
/*@}*/

/** @name GC color size
 *  @{
 */
#define XOSD_COLOR_ENTRY_SIZE	4	/**< Size of each color entry
					  *  in bytes */
/*@}*/

/** @name GC font unit size
 *  @{
 */
#define XOSD_FONT_BIT_TO_BYTE	8	/**< Ratio to convert font size
					  *  to byte */
/*@}*/

/** @name Layer priority
 *  @{
 */
#define XOSD_LAYER_PRIORITY_0	0	/**< Priority 0 --- Lowest */
#define XOSD_LAYER_PRIORITY_1	1	/**< Priority 1 */
#define XOSD_LAYER_PRIORITY_2	2	/**< Priority 2 */
#define XOSD_LAYER_PRIORITY_3	3	/**< Priority 3 */
#define XOSD_LAYER_PRIORITY_4	4	/**< Priority 4 */
#define XOSD_LAYER_PRIORITY_5	5	/**< Priority 5 */
#define XOSD_LAYER_PRIORITY_6	6	/**< Priority 6 */
#define XOSD_LAYER_PRIORITY_7	7	/**< Priority 7 --- Highest */
/*@}*/

/** @name Output Encoding Register bit definition
 *  @{
 */
#define XOSD_OPENC_VID_FORMAT_MASK	0x0000000F	/**< OSD Output Video
							  * Format Mask
							  *  0: YUV 4:2:2
							  *  1: YUV 4:4:4
							  *  2: RGB
							  *  3: YUV 4:2:0 */
#define XOSD_OPENC_NBITS_MASK		0x00000030	/**< Vertical Height of
							  *  OSD Output Mask */
/*@}*/

/** @name Error Register bit definition
 *  @{
 */
#define XOSD_ERR_LAYER7_SOF_LATE_MASK	0x80000000	/**< OSD Layer 7 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER7_SOF_EARLY_MASK	0x40000000	/**< OSD Layer 7 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER7_EOL_LATE_MASK	0x20000000	/**< OSD Layer 7 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER7_EOL_EARLY_MASK	0x10000000	/**< OSD Layer 7 error
							  *  for EOL early */

#define XOSD_ERR_LAYER6_SOF_LATE_MASK	0x08000000	/**< OSD Layer 6 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER6_SOF_EARLY_MASK	0x04000000	/**< OSD Layer 6 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER6_EOL_LATE_MASK	0x02000000	/**< OSD Layer 6 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER6_EOL_EARLY_MASK	0x01000000	/**< OSD Layer 6 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_LAYER5_SOF_LATE_MASK	0x00800000	/**< OSD Layer 5 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER5_SOF_EARLY_MASK	0x00400000	/**< OSD Layer 5 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER5_EOL_LATE_MASK	0x00200000	/**< OSD Layer 5 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER5_EOL_EARLY_MASK	0x00100000	/**< OSD Layer 5 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_LAYER4_SOF_LATE_MASK	0x00080000	/**< OSD Layer 4 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER4_SOF_EARLY_MASK	0x00040000	/**< OSD Layer 4 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER4_EOL_LATE_MASK	0x00020000	/**< OSD Layer 4 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER4_EOL_EARLY_MASK	0x00010000	/**< OSD Layer 4 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_LAYER3_SOF_LATE_MASK	0x00008000	/**< OSD Layer 3 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER3_SOF_EARLY_MASK	0x00004000	/**< OSD Layer 3 error
							  * for SOF early
							  * Mask */
#define XOSD_ERR_LAYER3_EOL_LATE_MASK	0x00002000	/**< OSD Layer 3 error
							  * for EOL late
							  * Mask */
#define XOSD_ERR_LAYER3_EOL_EARLY_MASK	0x00001000	/**< OSD Layer 3 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_LAYER2_SOF_LATE_MASK	0x00000800	/**< OSD Layer 2 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER2_SOF_EARLY_MASK	0x00000400	/**< OSD Layer 2 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER2_EOL_LATE_MASK	0x00000200	/**< OSD Layer 2 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER2_EOL_EARLY_MASK	0x00000100	/**< OSD Layer 2 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_LAYER1_SOF_LATE_MASK	0x00000080	/**< OSD Layer 1 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER1_SOF_EARLY_MASK	0x00000040	/**< OSD Layer 1 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER1_EOL_LATE_MASK	0x00000020	/**< OSD Layer 1 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER1_EOL_EARLY_MASK	0x00000010	/**< OSD Layer 1 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_LAYER0_SOF_LATE_MASK	0x00000008	/**< OSD Layer 0 error
							  *  for SOF late
							  *  Mask */
#define XOSD_ERR_LAYER0_SOF_EARLY_MASK	0x00000004	/**< OSD Layer 0 error
							  *  for SOF early
							  *  Mask */
#define XOSD_ERR_LAYER0_EOL_LATE_MASK	0x00000002	/**< OSD Layer 0 error
							  *  for EOL late
							  *  Mask */
#define XOSD_ERR_LAYER0_EOL_EARLY_MASK	0x00000001	/**< OSD Layer 0 error
							  *  for EOL early
							  *  Mask */

#define XOSD_ERR_ALL_ERR_MASK		0xFFFFFFFF	/**< OSD Layer 0 to
							  *  Layer 7 error for
							  *  EOL late or early
							  *  early or SOF late
							  *  or early Mask */
/*@}*/

/** @name Control Register bit definition
 *  @{
 */
#define XOSD_CTL_EN_MASK	0x00000001	/**< Enable / SW Enable Mask */
#define XOSD_CTL_RUE_MASK	0x00000002	/**< Register Update Enable
						  *  Mask */
#define XOSD_CTL_FSYNC_MASK	0x40000000	/**< Frame Sync Reset Mask */
#define XOSD_CTL_SW_RST_MASK	0x80000000	/**< Core Reset Mask */
/*@}*/

/** @name Constants. It is defined as constants to use instead magic numbers.
 *  @{
 */
#define XOSD_DATA_8		8
#define XOSD_DATA_2		2
/*@}*/

/** @name Compatibility Macros
 *  @{
 */
#define XOSD_CTL		XOSD_CTL_OFFSET
#define XOSD_SS			XOSD_ACTIVE_SIZE_OFFSET
#define XOSD_VER		XOSD_VER_OFFSET
#define XOSD_OPENC		XOSD_OPENC_OFFSET
#define XOSD_BC0		XOSD_BC0_OFFSET
#define XOSD_BC1		XOSD_BC1_OFFSET
#define XOSD_BC2		XOSD_BC2_OFFSET

#define XOSD_L0C		XOSD_L0C_OFFSET
#define XOSD_L0P		XOSD_L0P_OFFSET
#define XOSD_L0S		XOSD_L0S_OFFSET

#define XOSD_L1C		XOSD_L1C_OFFSET
#define XOSD_L1P		XOSD_L1P_OFFSET
#define XOSD_L1S		XOSD_L1S_OFFSET

#define XOSD_L2C		XOSD_L2C_OFFSET
#define XOSD_L2P		XOSD_L2P_OFFSET
#define XOSD_L2S		XOSD_L2S_OFFSET

#define XOSD_L3C		XOSD_L3C_OFFSET
#define XOSD_L3P		XOSD_L3P_OFFSET
#define XOSD_L3S		XOSD_L3S_OFFSET

#define XOSD_L4C		XOSD_L4C_OFFSET
#define XOSD_L4P		XOSD_L4P_OFFSET
#define XOSD_L4S		XOSD_L4S_OFFSET

#define XOSD_L5C		XOSD_L5C_OFFSET
#define XOSD_L5P		XOSD_L5P_OFFSET
#define XOSD_L5S		XOSD_L5S_OFFSET

#define XOSD_L6C		XOSD_L6C_OFFSET
#define XOSD_L6P		XOSD_L6P_OFFSET
#define XOSD_L6S		XOSD_L6S_OFFSET

#define XOSD_L7C		XOSD_L7C_OFFSET
#define XOSD_L7P		XOSD_L7P_OFFSET
#define XOSD_L7S		XOSD_L7S_OFFSET

#define XOSD_GCWBA		XOSD_GCWBA_OFFSET
#define XOSD_GCABA		XOSD_GCABA_OFFSET
#define XOSD_GCD		XOSD_GCD_OFFSET

#define XOSD_RST		XOSD_RST_OFFSET

#define XOSD_ISR		XOSD_ISR_OFFSET
#define XOSD_IER		XOSD_IER_OFFSET

#define XOSD_SS_YSIZE_MASK	XOSD_ACTSIZE_NUM_LINE_MASK
#define XOSD_SS_XSIZE_MASK	XOSD_ACTSIZE_NUM_PIXEL_MASK
#define XOSD_SS_YSIZE_SHIFT	XOSD_ACTSIZE_NUM_LINE_SHIFT

#define XOSD_In32	XOsd_In32
#define XOSD_Out32	XOsd_Out32

#define XOSD_ReadReg	XOsd_ReadReg
#define XOSD_WriteReg	XOsd_WriteReg
/*@}*/

/***************** Macros (Inline Functions) Definitions *********************/

/** @name Core register I/O APIs
 *  @{
 */

#define XOsd_In32	Xil_In32
#define XOsd_Out32	Xil_Out32

/*****************************************************************************/
/**
*
* This function reads the given register.
*
* @param	BaseAddress is the base address of the OSD core.
* @param	RegOffset is the register offset of the OSD core.
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XOsd_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XOsd_ReadReg(BaseAddress, RegOffset) \
	XOsd_In32((BaseAddress) + (u32)(RegOffset))

/*****************************************************************************/
/**
*
* This function writes the given register.
*
* @param	BaseAddress is the base address of the OSD core.
* @param	RegOffset is the register offset of the OSD core.
* @param	Data is the 32-bit value to write into the core register.
*
* @return	None.
*
* @note		C-style signature:
*		void XOsd_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XOsd_WriteReg(BaseAddress, RegOffset, Data) \
	XOsd_Out32((BaseAddress) + (u32)(RegOffset), (Data))
/*@}*/

/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
