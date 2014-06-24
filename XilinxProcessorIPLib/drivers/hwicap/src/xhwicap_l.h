/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xhwicap_l.h
*
* This header file contains identifiers and basic driver functions (or
* macros) that can be used to access the device. Other driver functions
* are defined in xhwicap.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 2.00a sv   09/28/07 First release for the FIFO mode
* 3.00a sv   11/28/08 Added Abort bit definition in the Control Register
*		      Removed XHI_WFO_MAX_VACANCY and XHI_RFO_MAX_OCCUPANCY
*		      definitions.
* 3.01a sv   10/21/09 Corrected the IDCODE definitions for some of the
*                     V5 FX parts.
* 4.00a hvm  11/30/09 Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  02/04/10 Added S6 Support
* 5.03a hvm  15/4/11  Updated with V6 CXT device definitions.
* 6.00a hvm  08/01/11 Updated with K7 device Ids.
* 7.00a bss  03/14/12 Added EOS mask and Hang mask CR CR 637538
*		      Added Virtex 7 and Zynq Idcodes - CR 647140, CR 643295
*
* 8.00a bss  06/20/12 Deleted Hang mask definition as per CR 656162
* 9.0   bss  02/20/14 Added Kintex 8 and Virtex72000T device Idcodes.
*
* </pre>
*
*****************************************************************************/
#ifndef XHWICAP_L_H_ /* prevent circular inclusions */
#define XHWICAP_L_H_ /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include <xil_types.h>
#include <xil_assert.h>
#include "xil_io.h"
#include "xhwicap_family.h"
/************************** Constant Definitions ****************************/

/******************************************************************/
/** @name Register Map
 *
 * Register offsets for the XHwIcap device.
 * @{
 */
#define XHI_GIER_OFFSET		0x1C  /**< Device Global Interrupt Enable Reg */
#define XHI_IPISR_OFFSET	0x20  /**< Interrupt Status Register */
#define XHI_IPIER_OFFSET	0x28  /**< Interrupt Enable Register */
#define XHI_WF_OFFSET		0x100 /**< Write FIFO */
#define XHI_RF_OFFSET		0x104 /**< Read FIFO */
#define XHI_SZ_OFFSET		0x108 /**< Size Register */
#define XHI_CR_OFFSET		0x10C /**< Control Register */
#define XHI_SR_OFFSET		0x110 /**< Status Register */
#define XHI_WFV_OFFSET		0x114 /**< Write FIFO Vacancy Register */
#define XHI_RFO_OFFSET		0x118 /**< Read FIFO Occupancy Register */

/* @} */


/** @name Device Global Interrupt Enable Register (GIER) bit definitions
 *
 * @{
 */
#define XHI_GIER_GIE_MASK      0x80000000 /**< Global Interrupt enable Mask */

/* @} */

/** @name HwIcap Device Interrupt Status/Enable Registers
 *
 * <b> Interrupt Status Register (IPISR) </b>
 *
 * This register holds the interrupt status flags for the device. These
 * bits are toggle on write.
 *
 * <b> Interrupt Enable Register (IPIER) </b>
 *
 * This register is used to enable interrupt sources for the device.
 * Writing a '1' to a bit in this register enables the corresponding Interrupt.
 * Writing a '0' to a bit in this register disables the corresponding Interrupt.
 *
 * IPISR/IPIER registers have the same bit definitions and are only defined
 * once.
 * @{
 */
#define XHI_IPIXR_RFULL_MASK	0x00000008 /**< Read FIFO Full */
#define XHI_IPIXR_WEMPTY_MASK	0x00000004 /**< Write FIFO Empty */
#define XHI_IPIXR_RDP_MASK	0x00000002 /**< Read FIFO half full */
#define XHI_IPIXR_WRP_MASK	0x00000001 /**< Write FIFO half full */
#define XHI_IPIXR_ALL_MASK	0x0000000F /**< Mask of all interrupts */

/* @} */

/** @name Control Register (CR)
 *
 * @{
 */
#define XHI_CR_SW_ABORT_MASK	0x00000010 /**< Abort current ICAP Read/Write */
#define XHI_CR_SW_RESET_MASK	0x00000008 /**< SW Reset Mask */
#define XHI_CR_FIFO_CLR_MASK	0x00000004 /**< FIFO Clear Mask */
#define XHI_CR_READ_MASK	0x00000002 /**< Read from ICAP to FIFO */
#define XHI_CR_WRITE_MASK	0x00000001 /**< Write from FIFO to ICAP */

/* @} */


/** @name Status Register (SR)
 *
 * @{
 */
#define XHI_SR_CFGERR_N_MASK	0x00000100 /**< Config Error Mask */
#define XHI_SR_DALIGN_MASK	0x00000080 /**< Data Alignment Mask */
#define XHI_SR_RIP_MASK		0x00000040 /**< Read back Mask */
#define XHI_SR_IN_ABORT_N_MASK	0x00000020 /**< Select Map Abort Mask */
#define XHI_SR_DONE_MASK	0x00000001 /**< Done bit Mask */
#define XHI_SR_EOS_MASK 	0x00000004 /**< EOS bit Mask */

/* @} */

/** @name IDCODE's for the Virtex4 Devices.
 *
 * @{
 */
#define XHI_XC4VLX15	0x01658093
#define XHI_XC4VLX25	0x0167C093
#define XHI_XC4VLX40	0x016A4093
#define XHI_XC4VLX60	0x016B4093
#define XHI_XC4VLX80	0x016D8093
#define XHI_XC4VLX100	0x01700093
#define XHI_XC4VLX160	0x01718093
#define XHI_XC4VLX200	0x01734093

#define XHI_XC4VSX25	0x02068093
#define XHI_XC4VSX35	0x02088093
#define XHI_XC4VSX55	0x020B0093

#define XHI_XC4VFX12	0x01E58093
#define XHI_XC4VFX20	0x01E64093
#define XHI_XC4VFX40	0x01E8C093
#define XHI_XC4VFX60	0x01EB4093
#define XHI_XC4VFX100	0x01EE4093
#define XHI_XC4VFX140	0x01F14093

#define XHI_V4_NUM_DEVICES	17

/* @} */

/** @name IDCODE's for the Virtex5 Devices.
 *
 * @{
 */
#define XHI_XC5VLX30	0x0286E093
#define XHI_XC5VLX50	0x02896093
#define XHI_XC5VLX85	0x028AE093
#define XHI_XC5VLX110	0x028D6093
#define XHI_XC5VLX220	0x0290C093
#define XHI_XC5VLX330	0x0295C093

#define XHI_XC5VLX30T	0x02A6E093
#define XHI_XC5VLX50T	0x02A96093
#define XHI_XC5VLX85T	0x02AAE093
#define XHI_XC5VLX110T	0x02AD6093
#define XHI_XC5VLX220T	0x02B0C093
#define XHI_XC5VLX330T	0x02B5C093

#define XHI_XC5VSX35T	0x02E72093
#define XHI_XC5VSX50T	0x02E9A093
#define XHI_XC5VSX95T	0x02ECE093

#define XHI_XC5VFX30T	0x03276093
#define XHI_XC5VFX70T	0x032C6093
#define XHI_XC5VFX100T	0x032D8093
#define XHI_XC5VFX130T	0x03300093
#define XHI_XC5VFX200T	0x03334093

#define XHI_V5_NUM_DEVICES	20
/* @} */

/** @name IDCODE's for the Virtex6 Devices.
 *
 * @{
 */

#define XHI_XC6VHX250T	0x042A2093
#define XHI_XC6VHX255T	0x042A4093
#define XHI_XC6VHX380T	0x042A8093
#define XHI_XC6VHX565T	0x042AC093

#define XHI_XC6VLX75T	0x04244093
#define XHI_XC6VLX130T	0x0424A093
#define XHI_XC6VLX195T	0x0424C093
#define XHI_XC6VLX240T 	0x04250093
#define XHI_XC6VLX365T	0x04252093
#define XHI_XC6VLX550T	0x04256093
#define XHI_XC6VLX760	0x0423A093
#define XHI_XC6VSX315T	0x04286093
#define XHI_XC6VSX475T	0x04288093
#define XHI_XC6VCX75T	0x042C4093
#define XHI_XC6VCX130T	0x042CA093
#define XHI_XC6VCX195T	0x042CC093
#define XHI_XC6VCX240T 	0x042D0093

#define XHI_V6_NUM_DEVICES 	17

/* @} */

/** @name IDCODE's for the Spartan6 Devices.
 *
 * @{
 */

#define XHI_XC6SLX4	0x04000093
#define XHI_XC6SLX9	0x04001093
#define XHI_XC6SLX16	0x04002093
#define XHI_XC6SLX25	0x04004093
#define XHI_XC6SLX25T	0x04024093
#define XHI_XC6SLX45	0x04008093
#define XHI_XC6SLX45T	0x04028093
#define XHI_XC6SLX75	0x0400E093
#define XHI_XC6SLX75T	0x0402E093
#define XHI_XC6SLX100	0x04011093
#define XHI_XC6SLX100T	0x04031093
#define XHI_XC6SLX150	0x0401D093
#define XHI_XC6SLX150T	0x0403D093

#define XHI_S6_NUM_DEVICES	13

/* @} */

/** @name IDCODE's for the Kintex7 Devices.
 *
 * @{
 */

#define XHI_XC7K30T	0x03642093
#define XHI_XC7K70T	0x03647093
#define XHI_XC7K160T	0x0364C093
#define XHI_XC7K325T	0x03651093
#define XHI_XC7K410T	0x03656093
#define XHI_XC7K235T	0x0365B093
#define XHI_XC7K125T	0x0365C093
#define XHI_XC7K290T	0x0365D093
#define XHI_XC7K355T	0x03747093
#define XHI_XC7K420T	0x0374C093
#define XHI_XC7K480T	0x03751093

#define XHI_K7_NUM_DEVICES      11

/* @} */


/** @name IDCODE's for the Virtex7 Devices.
 *
 * @{
 */

#define XHI_XC7VX80T		0x03680093
#define XHI_XC7VX82T		0x03681093
#define XHI_XC7VX330T		0x03667093
#define XHI_XC7VX415T		0x03682093
#define XHI_XC7V450T		0x0366C093
#define XHI_XC7VX485T		0x03687093
#define XHI_XC7VX550T		0x03692093
#define XHI_XC7V585T		0x03671093
#define XHI_XC7VX690T		0x03691093
#define XHI_XC7VX980T		0x03696093
#define XHI_XC7VX2000T_SLR0	0x036B3093
#define XHI_XC7VX2000T_SLR1	0x036A2093
#define XHI_XC7VX2000T_SLR2	0x036A4093
#define XHI_XC7VX2000T_SLR3	0x036A6093


#define XHI_V7_NUM_DEVICES      14

/* @} */

/** @name IDCODE's for the Artix7 Devices.
 *
 * @{
 */

#define XHI_XC7A15		0x03627093
#define XHI_XC7A30T		0x0362D093
#define XHI_XC7A50T		0x0362C093
#define XHI_XC7A100T		0x03631093
#define XHI_XC7A200T		0x03636093
#define XHI_XC7A350T		0x0363B093

#define XHI_A7_NUM_DEVICES      6

/* @} */

/** @name IDCODE's for the Zynq Devices.
 *
 * @{
 */

#define XHI_XC7Z010		0x03722093
#define XHI_XC7Z020		0x03727093
#define XHI_XC7Z030		0x0372C093
#define XHI_XC7Z045		0x03731093

#define XHI_ZYNQ_NUM_DEVICES      4

/* @} */

/** @name IDCODE's for the Kintex8 Devices.
 *
 * @{
 */
#define XHI_XCKU040T	0x03822093

#define XHI_K8_NUM_DEVICES      1

/* @} */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

#define XHwIcap_In32 Xil_In32

#define XHwIcap_Out32 Xil_Out32

/****************************************************************************/
/**
*
* Read from the specified HwIcap device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XHwIcap_ReadReg(u32 BaseAddress, u32 RegOffset);
*
******************************************************************************/
#define XHwIcap_ReadReg(BaseAddress, RegOffset) \
	XHwIcap_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
*
* Write to the specified HwIcap device register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the 1st register of the
*		device to select the specific register.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XHwIcap_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 RegisterValue);
******************************************************************************/
#define XHwIcap_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	XHwIcap_Out32((BaseAddress) + (RegOffset), (RegisterValue))

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif         /* end of protection macro */


