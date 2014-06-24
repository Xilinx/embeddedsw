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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xosd_hw.h
*
* This header file contains identifiers and register-level driver functions (or
* macros) that can be used to access the Xilinx MVI Video On-Screen-Display
* (OSD) device.
*
* For more information about the operation of this device, see the hardware
* specification and documentation in the higher level driver xosd.h source
* code file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	 Date	  Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a xd   08/01/08 First release
* 2.00a cm   06/12/12 14.1/14.2 release with address map updated to match
*                     Video over AXI4-Stream Specification
* 2.00a cjm  12/18/12 Converted from xio.h to xil_io.h, translating
*                     basic types, MB cache functions, exceptions and
*                     assertions to xil_io format. 
* </pre>
*
******************************************************************************/

#ifndef XOSD_HW_H		 /* prevent circular inclusions */
#define XOSD_HW_H		 /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Offsets
 *  @{
 */
#define XOSD_CTL	0x000	 /**< Control */
#define XOSD_SS		0x020	 /**< Screen Size */
#define XOSD_BC0	0x100	 /**< Background Color Channel 0 */
#define XOSD_BC1	0x104	 /**< Background Color Channel 1 */
#define XOSD_BC2	0x108	 /**< Background Color Channel 2 */

#define XOSD_L0C	0x110	 /**< Layer 0 Control */
#define XOSD_L0P	0x114	 /**< Layer 0 Position */
#define XOSD_L0S	0x118	 /**< Layer 0 Size */

#define XOSD_L1C	0x120	 /**< Layer 1 Control */
#define XOSD_L1P	0x124	 /**< Layer 1 Position */
#define XOSD_L1S	0x128	 /**< Layer 1 Size */

#define XOSD_L2C	0x130	 /**< Layer 2 Control */
#define XOSD_L2P	0x134	 /**< Layer 2 Position */
#define XOSD_L2S	0x138	 /**< Layer 2 Size */

#define XOSD_L3C	0x140	 /**< Layer 3 Control */
#define XOSD_L3P	0x144	 /**< Layer 3 Position */
#define XOSD_L3S	0x148	 /**< Layer 3 Size */

#define XOSD_L4C	0x150	 /**< Layer 4 Control */
#define XOSD_L4P	0x154	 /**< Layer 4 Position */
#define XOSD_L4S	0x158	 /**< Layer 4 Size */

#define XOSD_L5C	0x160	 /**< Layer 5 Control */
#define XOSD_L5P	0x164	 /**< Layer 5 Position */
#define XOSD_L5S	0x168	 /**< Layer 5 Size */

#define XOSD_L6C	0x170	 /**< Layer 6 Control */
#define XOSD_L6P	0x174	 /**< Layer 6 Position */
#define XOSD_L6S	0x178	 /**< Layer 6 Size */

#define XOSD_L7C	0x180	 /**< Layer 7 Control */
#define XOSD_L7P	0x184	 /**< Layer 7 Position */
#define XOSD_L7S	0x188	 /**< Layer 7 Size */

#define XOSD_GCWBA	0x190	 /**< GPU Write Bank Address */
#define XOSD_GCABA	0x194	 /**< GPU Active Bank Address */
#define XOSD_GCD	0x198	 /**< GPU Data */

#define XOSD_VER	0x010	 /**< Version Register */
#define XOSD_RST	0x000	 /**< Software Reset */

#define XOSD_GIER	0x010	 /**< Global Interrupt Enable Register */
#define XOSD_ISR	0x004	 /**< Interrupt Status Register */
#define XOSD_IER	0x00c	 /**< Interrupt Enable Register */
/*@}*/

/** @name Memory footprint of layers
 *  @{
 */
#define XOSD_LAYER_SIZE	0x10
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

/** @name OSD Control Register bit definition
 *  @{
 */
#define XOSD_CTL_VBP_MASK	0x00000020 /**< Vertical Blank Polarity */
#define XOSD_CTL_HBP_MASK	0x00000010 /**< Horizontal Blank Polarity */
#define XOSD_CTL_RUE_MASK	0x00000002 /**< OSD Register Update Enable */
#define XOSD_CTL_EN_MASK	0x00000001 /**< OSD Enable */
/*@}*/

/** @name OSD Screen Size Register bit definition
 *  @{
 */
#define XOSD_SS_YSIZE_MASK   0x0FFF0000 /**< Vertical Height of OSD Output */
#define XOSD_SS_YSIZE_SHIFT  16	        /**< Bit shift of XOSD_SS_YSIZE_MASK */
#define XOSD_SS_XSIZE_MASK   0x00000FFF /**< Horizontal Width of OSD Output */
/*@}*/

/** @name OSD Background Color Channel 0
 *  @{
 */
#define XOSD_BC0_YG_MASK	0x00000FFF /**< Y (luma) or Green */
/*@}*/

/** @name OSD Background Color Channel 1
 *  @{
 */
#define XOSD_BC1_UCBB_MASK	0x00000FFF /**< U (Cb) or Blue */
/*@}*/

/** @name OSD Background Color Channel 2
 *  @{
 */
#define XOSD_BC2_VCRR_MASK	0x00000FFF /**< V(Cr) or Red */
/*@}*/

/** @name Maximum number of the layers
 *  @{
 */
#define XOSD_MAX_NUM_OF_LAYERS	8	   /**< The max number of layers */
/*@}*/

/** @name OSD Layer Control (Layer 0 through (XOSD_MAX_NUM_OF_LAYERS - 1))
 *  @{
 */
#define XOSD_LXC_ALPHA_MASK	0x0FFF0000 /**< Global Alpha Value */
#define XOSD_LXC_ALPHA_SHIFT	16	   /**< Bit shift number of Global
					     *  Alpha Value */
#define XOSD_LXC_PRIORITY_MASK	0x00000700 /**< Layer Priority */
#define XOSD_LXC_PRIORITY_SHIFT 8	   /**< Bit shift number of Layer
					     *  Priority */
#define XOSD_LXC_GALPHAEN_MASK	0x00000002 /**< Global Alpha Enable */
#define XOSD_LXC_EN_MASK	0x00000001 /**< Layer Enable */
/*@}*/

/** @name OSD Layer Position (Layer 0 through (XOSD_MAX_NUM_OF_LAYERS - 1))
 *  @{
 */
#define XOSD_LXP_YSTART_MASK	0x0FFF0000 /**< Vertical start line of origin
					     *  of layer */
#define XOSD_LXP_YSTART_SHIFT	16	   /**< Bit shift of vertical start
					     *  line of origin of layer */
#define XOSD_LXP_XSTART_MASK	0x00000FFF /**< Horizontal start pixel of
					     *  origin of layer */
/*@}*/

/** @name OSD Layer Size (Layer 0 through (XOSD_MAX_NUM_OF_LAYERS - 1))
 *  @{
 */
#define XOSD_LXS_YSIZE_MASK	0x0FFF0000 /**< Vertical size of layer */
#define XOSD_LXS_YSIZE_SHIFT	16	   /**< Bit shift number of vertical
					     *  size of layer */
#define XOSD_LXS_XSIZE_MASK	0x00000FFF /**< Horizontal size of layer */
/*@}*/

/** @name OSD Graphics Controller Write Bank Address
 *  @{
 */
#define XOSD_GCWBA_GCNUM_MASK	0x00000700 /**< Graphics Controller Number */
#define XOSD_GCWBA_GCNUM_SHIFT	8	   /**< Bit shift of Graphics
					     *  Controller Number */
#define XOSD_GCWBA_BANK_MASK	0x00000007 /**< Controls which bank to write
					     *  GPU instructions and Color
					     *  LUT data into. */

#define XOSD_GCWBA_INS0		0x00000000 /** Instruction RAM 0 */
#define XOSD_GCWBA_INS1		0x00000001 /** Instruction RAM 1 */
#define XOSD_GCWBA_COL0		0x00000002 /** Color LUT RAM 0 */
#define XOSD_GCWBA_COL1		0x00000003 /** Color LUT RAM 1 */
#define XOSD_GCWBA_TXT0		0x00000004 /** Text RAM 0 */
#define XOSD_GCWBA_TXT1		0x00000005 /** Text RAM 1 */
#define XOSD_GCWBA_CHR0		0x00000006 /** Character Set RAM 0 */
#define XOSD_GCWBA_CHR1		0x00000007 /** Character Set RAM 1 */
/*@}*/

/** @name OSD Graphics Controller Active Bank Address
 *  @{
 */
#define XOSD_GCABA_CHR_MASK	0xFF000000 /**< Set the active Character Bank*/
#define XOSD_GCABA_CHR_SHIFT	24	   /**< Bit shift of active Character
					     *  Bank */
#define XOSD_GCABA_TXT_MASK	0x00FF0000 /**< Set the active Text Bank */
#define XOSD_GCABA_TXT_SHIFT	16	   /**< Bit shift of active Text Bank*/
#define XOSD_GCABA_COL_MASK	0x0000FF00 /**< Set the active Color Table
					     *  Bank */
#define XOSD_GCABA_COL_SHIFT	8	   /**< Bit shift of active Color Table
					     *  Bank */
#define XOSD_GCABA_INS_MASK	0x000000FF /**< Set the active instruction Bank
					     */
/*@}*/

/** @name Version Register bit definition
 *  @{
 */
#define XOSD_VER_MAJOR_MASK	0xFF000000	/**< Major Version*/
#define XOSD_VER_MAJOR_SHIFT	24		/**< Major Version Bit Shift*/
#define XOSD_VER_MINOR_MASK	0x00FF0000	/**< Minor Version */
#define XOSD_VER_MINOR_SHIFT	16		/**< Minor Version Bit Shift*/
#define XOSD_VER_REV_MASK	0x0000F000	/**< Revision Version */
#define XOSD_VER_REV_SHIFT	12		/**< Revision Bit Shift*/
/*@}*/

/** @name OSD Software Reset
 *  @{
 */
#define XOSD_RST_RESET		0x80000000 /**< Software Reset */
/*@}*/

/** @name Global Interrupt Enable Register bit definition
 *  @{
 */
#define XOSD_GIER_GIE_MASK	0x80000000 /**< Global interrupt enable */
/*@}*/

/** @name Interrupt Status/Enable Register bit definition
 *  @{
 */
#define XOSD_IXR_GAO_MASK	0xFF000000 /**< Graphics Controller Instruction
					     *  Overflow */
#define XOSD_IXR_GIE_MASK	0x00FF0000 /**< Graphics Controller Instruction
					     *  Error */
#define XOSD_IXR_OOE_MASK	0x00000010 /**< OSD Output Overflow Error */
#define XOSD_IXR_IUE_MASK	0x00ff0000 /**< OSD Input Underflow Error */
#define XOSD_IXR_VBIE_MASK	0x00000004 /**< Vertical Blank Interval End */
#define XOSD_IXR_VBIS_MASK	0x00000002 /**< Vertical Blank Interval Start*/
#define XOSD_IXR_FE_MASK	0x00000008 /**< OSD did not complete
					     *  processing frame before next
					     *  Vblank */
#define XOSD_IXR_FD_MASK	0x00000001 /**< OSD completed processing
					     *  Frame */
#define XOSD_IXR_ALLIERR_MASK (XOSD_IXR_GAO_MASK | \
				XOSD_IXR_GIE_MASK | \
				XOSD_IXR_OOE_MASK | \
				XOSD_IXR_IUE_MASK | \
				XOSD_IXR_FE_MASK) /**< Mask for all error
						    *  interrupts */

#define XOSD_IXR_ALLINTR_MASK  (XOSD_IXR_VBIE_MASK | \
				XOSD_IXR_VBIS_MASK | \
				XOSD_IXR_FD_MASK | \
				XOSD_IXR_ALLIERR_MASK) /**< Mask for all
							 *  interrupts */
/*@}*/

/** @name Layer Types
 *  @{
 */
#define XOSD_LAYER_TYPE_DISABLE 0	/**< Layer is disabled	  */
#define XOSD_LAYER_TYPE_GPU	1	/**< Layer's type is GPU  */
#define XOSD_LAYER_TYPE_VFBC	2	/**< Layer's type is VFBC */
/*@}*/

/** @name Supported Instruction numbers given an instruction memory size
 *  @{
 */
#define XOSD_INS_MEM_SIZE_TO_NUM 1	/**< Conversion to the number of
					  *  instructions from the
					  *  instruction memory size */
/*@}*/

/** @name GC Instruction word offset definition
 *  @{
 */
#define XOSD_INS0	0	/**< Instruction word 0 offset */
#define XOSD_INS1	1	/**< Instruction word 1 offset */
#define XOSD_INS2	2	/**< Instruction word 2 offset */
#define XOSD_INS3	3	/**< Instruction word 3 offset */
#define XOSD_INS_SIZE	4	/**< Size of an instruction in words */
/*@}*/

/** @name GC Instruction word 0 definition
 *  @{
 */
#define XOSD_INS0_OPCODE_MASK	0xF0000000 /**< Operation Code (OpCode) */
#define XOSD_INS0_OPCODE_SHIFT	28	   /**< Bit shift number of OpCode */
#define XOSD_INS0_GCNUM_MASK	0x07000000 /**< Graphics controller number
					     *  (GC#) */
#define XOSD_INS0_GCNUM_SHIFT	24	   /**< Bit shift number of GC# */
#define XOSD_INS0_XEND_MASK	0x00FFF000 /**< Horizontal end pixel of the
					     *  object */
#define XOSD_INS0_XEND_SHIFT	12	   /**< Bit shift number of Horizontal
					     *  end pixel of the object */
#define XOSD_INS0_XSTART_MASK	0x00000FFF /**< Horizontal start pixel of the
					     *  Object */
/*@}*/

/** @name GC Instruction word 1 definition
 *  @{
 */
#define XOSD_INS1_TXTINDEX_MASK	0x0000000F /**< String Index */
/*@}*/

/** @name GC Instruction word 2 definition
 *  @{
 */
#define XOSD_INS2_OBJSIZE_MASK	0xFF000000 /**< Object Size */
#define XOSD_INS2_OBJSIZE_SHIFT	24	   /**< Bit shift number of Object
					     *  Size */
#define XOSD_INS2_YEND_MASK	0x00FFF000 /**< Vertical end line of the
					     *  object */
#define XOSD_INS2_YEND_SHIFT	12	   /**< Bit shift number of Vertical
					     *  end line of the object */
#define XOSD_INS2_YSTART_MASK	0x00000FFF /**< Vertical start line of the
					     *  Object */
/*@}*/

/** @name GC Instruction word 3 definition
 *  @{
 */
#define XOSD_INS3_COL_MASK	0x0000000F /**< Color Index for Box/Text */
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
#define XOSD_COLOR_ENTRY_SIZE	4	/**< Size of each color entry in
					  *  bytes */
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

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Device register I/O APIs
 *  @{
 */

#define XOSD_In32	Xil_In32
#define XOSD_Out32	Xil_Out32

/*****************************************************************************/
/**
*
* Read the given register.
*
* @param	BaseAddress is the base address of the device
* @param	RegOffset is the register offset to be read
*
* @return	The 32-bit value of the register
*
* @note
* C-style signature:
*	 u32 XOSD_ReadReg(u32 BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XOSD_ReadReg(BaseAddress, RegOffset) \
	XOSD_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
*
* Write the given register.
*
* @param	BaseAddress is the base address of the device
* @param	RegOffset is the register offset to be written
* @param	Data is the 32-bit value to write to the register
*
* @return	None.
*
* @note
* C-style signature:
*	 void XOSD_WriteReg(u32 BaseAddress, u32 RegOffset, u32 Data)
*
******************************************************************************/
#define XOSD_WriteReg(BaseAddress, RegOffset, Data) \
	XOSD_Out32((BaseAddress) + (RegOffset), (Data))
/*@}*/

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
