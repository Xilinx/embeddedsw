/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
* @file xprc_hw.h
* @addtogroup prc_v1_0
* @{
*
* This header file contains identifiers and register-level driver functions
* (or macros) that can be used to access the PRC.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver  Who    Date        Changes
* --- ---- ------------  ------------------------------------------------
* 1.0  ms   07/18/2016   First release
* 1.1  ms   08/01/17     Defined "XPRC_SR_DECOMPRESS_BAD_FORMAT_ERROR" and
*                        "XPRC_SR_DECOMPRESS_BAD_SIZE_ERROR" status error
*                        macros and modified the value of macro
*                        "XPRC_SR_BS_COMPATIBLE_ERROR".
*
* </pre>
*
******************************************************************************/

#ifndef XPRC_HW_H_ /* Prevent circular inclusions */
#define XPRC_HW_H_ /* by using protection macros  */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/** @name Register Bank Numbers
 * @{
 */
#define XPRC_VSM_GENERAL_REG_BANK	(0)	/**< General registers */
#define XPRC_VSM_TRIGGER_REG_BANK	(1)	/**< Trigger to Rm register */
#define XPRC_VSM_RM_REG_BANK		(2)	/**< Rm Information register */
#define XPRC_VSM_BS_REG_BANK		(3)	/**< Bs Information register */
/*@}*/

/** @name Bank Identifier
 * @{
 */
#define XPRC_DEFAULT_BANKID		(0)	/**< Default Bank Identifier */
/*@}*/

/** @name The Table Identifiers within a bank for each Register type
 * @{
 */
#define XPRC_STATUS_REG_TABLE_ID	(0)	/**< Status register
						  *  Table ID */
#define XPRC_CONTROL_REG_TABLE_ID	(0)	/**< Control register
						  *  Table ID */
#define XPRC_SW_TRIGGER_REG_TABLE_ID	(1)	/**< Sw Trigger register
						  *  Table ID */
#define XPRC_TRIGGER_REG_TABLE_ID	(0)	/**< Trigger register
						  *  Table ID */
#define XPRC_RM_BS_INDEX_REG_TABLE_ID	(0)	/**< RmBs Index register
						  *  Table ID */
#define XPRC_RM_CONTROL_REG_TABLE_ID	(1)	/**< Rm Control register
						  *  Table ID */
#define XPRC_BS_ID_REG_TABLE_ID		(0)	/**< Bs Identifier register
						  *  Table ID */
#define XPRC_BS_ADDRESS_REG_TABLE_ID	(1)	/**< Bs Address register
						  *  Table ID */
#define XPRC_BS_SIZE_REG_TABLE_ID	(2)	/**< Bs Size register Table
						  *  ID */
#define XPRC_DEFAULT_TABLEID		(0)	/**< Default Table Identifier
						  *  within the Bank */
/*@}*/

/** @name These are not addresses. They identify the type of register to be
 *  accessed.
 * @{
 */
#define XPRC_STATUS_REG			(0)	/**< Status register */
#define XPRC_CONTROL_REG		(1)	/**< Control register */
#define XPRC_SW_TRIGGER_REG		(2)	/**< Sw Trigger register */
#define XPRC_TRIGGER_REG		(3)	/**< Trigger register */
#define XPRC_RM_BS_INDEX_REG		(4)	/**< RmBs Index register */
#define XPRC_RM_CONTROL_REG		(5)	/**< Rm Control register */
#define XPRC_BS_ID_REG			(6)	/**< Bs Identifier register */
#define XPRC_BS_ADDRESS_REG		(7)	/**< Bs Address register */
#define XPRC_BS_SIZE_REG		(8)	/**< Bs Size register */
/*@}*/

/** @name Control Register commands
 * @{
 */
#define XPRC_CR_SHUTDOWN_CMD		(0)	/**< Shutdown Command */
#define XPRC_CR_RESTART_NO_STATUS_CMD	(1)	/**< Restart With No Status
						  *  Command */
#define XPRC_CR_RESTART_WITH_STATUS_CMD	(2)	/**< Restart With Status
						  *  Command */
#define XPRC_CR_OK_TO_PROCEED_CMD	(3)	/**< Proceed Command */
#define XPRC_CR_USER_CTRL_CMD		(4)	/**< User Control Command */
/*@}*/

/** @name Control Register field positions
 * @{
 */

/** @name The Command field
 * @{
 */
#define XPRC_CR_CMD_FIELD_LSB		(0)	/**< Command Field LSB */
#define XPRC_CR_CMD_FIELD_WIDTH		(8)	/**< Command Field Width */
#define XPRC_CR_CMD_FIELD_MSB \
	(XPRC_CR_CMD_FIELD_LSB + XPRC_CR_CMD_FIELD_WIDTH-1)
						/**< Command Field MSB */
/*@}*/

/** @name The Byte field
 * @{
 */
#define XPRC_CR_BYTE_FIELD_LSB		(XPRC_CR_CMD_FIELD_MSB+1)
						/**< Byte Field LSB */
#define XPRC_CR_BYTE_FIELD_WIDTH	(8)	/**< Byte Field Width */
#define XPRC_CR_BYTE_FIELD_MSB \
	(XPRC_CR_BYTE_FIELD_LSB + XPRC_CR_BYTE_FIELD_WIDTH-1)
						/**< Byte Field MSB */
/*@}*/

/** @name The Halfword field
 * @{
 */
#define XPRC_CR_HALFWORD_FIELD_LSB	(XPRC_CR_BYTE_FIELD_MSB+1)
						/**< Halfword Field LSB */
#define XPRC_CR_HALFWORD_FIELD_WIDTH	(16)	/**< Halfword Field Width */
#define XPRC_CR_HALFWORD_FIELD_MSB \
	(XPRC_CR_HALFWORD_FIELD_LSB + XPRC_CR_HALFWORD_FIELD_WIDTH-1)
						/**< Halfword Field MSB */
/*@}*/

/** @name Bit positions for User Control command options within the byte field
 * @{
 */
#define XPRC_CR_USER_CONTROL_RM_SHUTDOWN_REQ_BIT	(0)
						/**< Rm Shutdown Required */
#define XPRC_CR_USER_CONTROL_RM_DECOUPLE_BIT		(1)
						/**< Rm Decouple */
#define XPRC_CR_USER_CONTROL_SW_SHUTDOWN_REQ_BIT	(2)
						/**< Sw_Shutdown Required */
#define XPRC_CR_USER_CONTROL_SW_STARTUP_REQ_BIT		(3)
						/**< Sw Startup Required */
#define XPRC_CR_USER_CONTROL_RM_RESET_BIT		(4)
						/**< Rm Reset */
/*@}*/

/** @name Status Register Value
 * @{
 */
#define XPRC_SR_SHUTDOWN_MASK		(0x80)	/**< Shutdown State Mask */
/*@}*/

/** @name State is stored in bits 2:0, Mask = 111 = 0x7
 * @{
 */
#define XPRC_SR_STATE_MASK		(0x7)	/**< Vsm State Mask */
#define XPRC_SR_STATE_EMPTY		(0)	/**< Empty State Of VSM */
#define XPRC_SR_STATE_HW_SHUTDOWN	(1)	/**< Hardware Shutdown State
						  *  of VSM */
#define XPRC_SR_STATE_SW_SHUTDOWN	(2)	/**< Software Shutdown State
						  *  of VSM */
#define XPRC_SR_STATE_RM_UNLOAD		(3)	/**< Reconfigurable Module
						  *  Unload State Of VSM */
#define XPRC_SR_STATE_RM_LOAD		(4)	/**< Reconfigurable Module
						  *  Load State Of VSM */
#define XPRC_SR_STATE_SW_STARTUP	(5)	/**< Software Startup State
						  *  of VSM */
#define XPRC_SR_STATE_RM_RESET		(6)	/**< Reset State of VSM */
#define XPRC_SR_STATE_FULL		(7)	/**< Full State Of VSM */

#define XPRC_SR_ERROR_SHIFT		(3)	/**< Error Shift */
/*@}*/

/** @name Errors are stored in bits 6:3, Mask = 1111000 = 0x78
 * @{
 */
#define XPRC_SR_ERROR_MASK		(0x78)	/**< Error Codes Mask */
#define XPRC_SR_UNKNOWN_ERROR		(15)	/**< Unknown Error */
#define XPRC_SR_BS_COMPATIBLE_ERROR	(14)	/**< Bitstream Compatible
						  *  Error */
#define XPRC_SR_DECOMPRESS_BAD_FORMAT_ERROR	(8) /**< Bad compression
						  *  format error */
#define XPRC_SR_DECOMPRESS_BAD_SIZE_ERROR	(7) /**< Bad compression
						  *  size error */
#define XPRC_SR_FETCH_AND_CP_LOST_ERROR	(6)	/**< Fetch and Lost Error */
#define XPRC_SR_FETCH_AND_BS_ERROR	(5)	/**< Fetch and Bitstream
						  *  Error */
#define XPRC_SR_FETCH_ERROR		(4)	/**< Fetch Error */
#define XPRC_SR_CP_LOST_ERROR		(3)	/**< CP Lost Error */
#define XPRC_SR_BS_ERROR		(2)	/**< Bitstream Error */
#define XPRC_SR_BAD_CONFIG_ERROR	(1)	/**< Bad Configuration Error */
#define XPRC_SR_NO_ERROR		(0)	/**< No Error */

#define XPRC_SR_RMID_SHIFT		(8)	/**< Rm Identifier Shift */
/*@}*/

/** @name RM_ID is stored in bits 23:8,
 *  Mask = 111111111111111100000000 = 0xFFFF00
 * @{
 */
#define XPRC_SR_RMID_MASK	(0xFFFF00)	/**< Rm Identifier Mask */

#define XPRC_SR_BSID_SHIFT	(24)		/**< Bs Identifier Shift */
/*@}*/

/** @name BS_ID is stored in bits 31:24,
 *  Mask = 11111111000000000000000000000000
 * @{
 */
#define XPRC_SR_BSID_MASK	(0xFF000000)	/**< Bs Identifier Mask */
/*@}*/

/** @name Software Trigger Register Values
 *  This is the largest ID possible. The bits used are will be sized in
 *  hardware to match the number of triggers actually allocated.
 * @{
 */
#define XPRC_SW_TRIGGER_ID_MASK		(0x7FFFFFFF)	/**< Sw Trigger Id
							  *  Mask */
#define XPRC_SW_TRIGGER_PENDING_MASK	(0x80000000)	/**< Sw Trigger Pending
							  *  Mask */

#define XPRC_RM_BS_INDEX_SHIFT		(0)		/**< Rm Bs Index
							  *  Shift */
#define XPRC_RM_BS_INDEX_MASK		(0x0000FFFF)	/**< Rm Bs Index
							  *  Mask */

#define XPRC_RM_CLEARING_BS_INDEX_SHIFT	(16)		/**< Clearing Bs Index
							  *  Shift */
#define XPRC_RM_CLEARING_BS_INDEX_MASK  (0xFFFF0000)	/**< Clearing Bs Index
							  *  Mask */
/*@}*/

/** @name RM Control Register constants
 * @{
 */
#define XPRC_RM_CR_SHUTDOWN_REQUIRED_SHIFT	(0)	/**< Shutdown Required
							  *  Shift */
#define XPRC_RM_CR_STARTUP_REQUIRED_SHIFT	(2)	/**< Startup Required
							  *  Shift */
#define XPRC_RM_CR_RESET_REQUIRED_SHIFT		(3)	/**< Reset Required
							  *   Shift */
#define XPRC_RM_CR_RESET_DURATION_SHIFT		(5)	/**< Reset Duration
							  *  Shift */
/*@}*/

/** @name Shutdown required stored in bits 0:1, 0000000000011 = 0x3
 * @{
 */
#define XPRC_RM_CR_SHUTDOWN_REQUIRED_MASK	(0x3)	/**< Shutdown
							  *  Required Mask */
/*@}*/

/** @name Startup required stored in bit 2, 0000000000100 = 0x4
 * @{
 */
#define XPRC_RM_CR_STARTUP_REQUIRED_MASK	(0x4)	/**< Startup
							  *  Required Mask */
/*@}*/

/** @name Reset required stored in bits 3:4, 0000000011000 = 0x18
 * @{
 */
#define XPRC_RM_CR_RESET_REQUIRED_MASK		(0x18)	/**< Reset Required
							  *  Mask */
/*@}*/

/** @name Reset Duration stored in bits 5:12, 1111111100000 = 0x1FE0
 * @{
 */
#define XPRC_RM_CR_RESET_DURATION_MASK		(0x1FE0)	/**< Reset Duration
							  *  Mask */

#define XPRC_RM_CR_NO_SHUTDOWN_REQUIRED		(0)	/**< No Shutdown
							  *  Required */
#define XPRC_RM_CR_HW_SHUTDOWN_REQUIRED		(1)	/**< Hardware Only
							  *  Shutdown
							  *  Required */
#define XPRC_RM_CR_HW_SW_SHUTDOWN_REQUIRED	(2)	/**< Hardware Shutdown
							  *  and then Software
							  *  Shutdown
							  *  Required */
#define XPRC_RM_CR_SW_HW_SHUTDOWN_REQUIRED	(3)	/**< Software Shutdown
							  *  and then Hardware
							  *  Shutdown
							  *  Required */

#define XPRC_RM_CR_STARTUP_NOT_REQUIRED		(0)	/**< Startup Not
							  *  Required */
#define XPRC_RM_CR_SW_STARTUP_REQUIRED		(1)	/**< Software
							  *  Only Startup
							  *  Required */

#define XPRC_RM_CR_NO_RESET_REQUIRED		(0)	/**< No Reset
							  *  Required */
#define XPRC_RM_CR_LOW_RESET_REQUIRED		(2)	/**< Low Reset
							  *  Required */
#define XPRC_RM_CR_HIGH_RESET_REQUIRED		(3)	/**< High Reset
							  *  Required */
/*@}*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XPrc_In32	Xil_In32
#define XPrc_Out32	Xil_Out32

/*****************************************************************************/
/**
*
* This macro writes a value to a PRC register. A 32 bit write is performed.
*
* @param	Address is the address of the register to write to.
* @param	Data is the data written to the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XPRC_WriteReg(u32 Address, u32 Data)
*
******************************************************************************/
#define XPrc_WriteReg(Address, Data)	XPrc_Out32((Address), (u32)(Data))

/*****************************************************************************/
/**
*
* This macro reads a value from a PRC register. A 32 bit read is performed.
*
* @param	Address is the address of the register to read from.
*
* @return	Data read from the register.
*
* @note		C-style signature:
*		u32 XPrc_ReadReg(u32 Address)
*
******************************************************************************/
#define XPrc_ReadReg(Address)		XPrc_In32((Address))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* End of protection macro */
/** @} */
