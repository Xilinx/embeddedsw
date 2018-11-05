/******************************************************************************
*
* Copyright (C) 2007 - 2017 Xilinx, Inc.  All rights reserved.
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
* @file xmbox_hw.h
* @addtogroup mbox_v4_3
* @{
*
* This header file contains identifiers and driver macros that can be used to
* access the device. The user should refer to the hardware device specification
* for more details of the device operation.
* The driver functions/APIs are defined in xmbox.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- --------   -----------------------------------------------
* 1.00a va              First release
* 1.00a ecm  06/01/07   Cleanup, new coding standard, check into XCS
* 1.00a ecm  08/19/08   Fixed CRs 466320, 466322, 476535, 476242, 476243
*			new rev
* 2.00a hm   04/09/09   Added support for mailbox v2.0, which has interrupts.
*			Fixed CR 502464, which removed extra
*			definitions that are not associated with
*			the interface.
*			Fixed the canonical definition so that each
*			interface is considered as a device instance.
* 3.00a hbm  10/19/09   Migrated to HAL phase 1 to use xil_io.
*			Removed _m from the function names.
*			Renamed _mIsEmpty to _IsEmptyHw and _mIsFull
*			to _IsFullHw.
* 4.3   sa   04/20/17   Added CTRL register definitions.
* </pre>
*
******************************************************************************/

#ifndef XMBOX_HW_H		/* prevent circular inclusions */
#define XMBOX_HW_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

#ifdef __MICROBLAZE__
#include "fsl.h"		/* MicroBlaze Specific */
#endif

/************************** Constant Definitions *****************************/

/** @name Register Offset Definitions
 * Register offsets within a mbox.
 * @{
 */

#define XMB_WRITE_REG_OFFSET	0x00	/**< Mbox write register */
#define XMB_READ_REG_OFFSET	0x08	/**< Mbox read register */
#define XMB_STATUS_REG_OFFSET	0x10	/**< Mbox status reg  */
#define XMB_ERROR_REG_OFFSET	0x14	/**< Mbox Error reg  */
#define XMB_SIT_REG_OFFSET	0x18	/**< Mbox send interrupt threshold
					     register */
#define XMB_RIT_REG_OFFSET	0x1C	/**< Mbox receive interrupt threshold
					     register */
#define XMB_IS_REG_OFFSET	0x20	/**< Mbox interrupt status register */
#define XMB_IE_REG_OFFSET	0x24	/**< Mbox interrupt enable register */
#define XMB_IP_REG_OFFSET	0x28	/**< Mbox interrupt pending register */
#define XMB_CTRL_REG_OFFSET	0x2C	/**< Mbox control register */

/*@}*/

/**
 * @name Status register bit definitions
 * These status bits are used to poll the FIFOs
 * @{
 */

#define XMB_STATUS_FIFO_EMPTY	0x00000001 /**< Receive FIFO is Empty */
#define XMB_STATUS_FIFO_FULL	0x00000002 /**< Send FIFO is Full */
#define XMB_STATUS_STA		0x00000004 /**< Send FIFO Threshold Status */
#define XMB_STATUS_RTA		0x00000008 /**< Receive FIFO Threshold Status */

/* @} */

/**
 * @name Interrupt Registers(s) bits definitions.
 * The IS, IE, and IP registers all have the same bit definition.
 * @{
 */

#define XMB_IX_STA		0x01 /**< Send Threshold Active, when the
					number of Send FIFO entries is less
					than and equal to Send Interrupt
					Threshold */
#define XMB_IX_RTA		0x02 /**< Receive Threshold Active, when the
					number of Receive FIFO entries is
					greater than Receive Interrupt
					Threshold */
#define XMB_IX_ERR		0x04 /**< Mailbox Error, when read on empty or
					write on full */

/* @} */

/**
 * @name Error bits definition.
 * @{
 */

#define XMB_ERROR_FIFO_EMPTY	0x00000001 /**< Receive FIFO is Empty */
#define XMB_ERROR_FIFO_FULL	0x00000002 /**< Send FIFO is Full */

/* @} */

/**
 * @name Control register bits definition.
 * @{
 */

#define XMB_CTRL_RESET_SEND_FIFO       	0x00000001 /**< Clear Send FIFO */
#define XMB_CTRL_RESET_RECV_FIFO       	0x00000002 /**< Clear Receive FIFO */

/* @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* The following macros are for the memory mapped access method */

/*****************************************************************************/
/**
* Read one of the mbox registers.
*
* @param	BaseAddress contains the base address of the mbox device.
* @param	RegOffset contains the offset from the 1st register of the mbox
*		to select the specific register of the mbox.
*
* @return	The value read from the register, a 32 bit value.
*
* @note		C-style signature:
*		u32 XMbox_ReadReg(u32 BaseAddress, unsigned RegOffset);
*
******************************************************************************/
#define XMbox_ReadReg(BaseAddress, RegOffset)			\
	Xil_In32((BaseAddress) + (RegOffset))

/*****************************************************************************/
/**
* Write a specified value to a register of a mbox.
*
* @param	BaseAddress is the base address of the mbox device.
* @param	RegOffset contain the offset from the 1st register of the
*		mbox to select the specific register of the mbox.
* @param	ValueToWrite is the 32 bit value to be written to the register.
*
* @return	None
*
* @note		C-style signature:
* void XMbox_WriteReg(u32 BaseAddress, unsigned RegOffset, u32 ValueToWrite);
*
******************************************************************************/
#define XMbox_WriteReg(BaseAddress, RegOffset, ValueToWrite)	\
	Xil_Out32(((BaseAddress) + (RegOffset)), (ValueToWrite))


/*****************************************************************************/
/**
* Write the mbox write register.
*
* @param	BaseAddress contains the base address of the mbox device.
* @param	ValueToWrite contains the value to be written.
*
* @return	None.
*
* @note		C-style signature:
* 		u32 XMbox_WriteMBox(u32 BaseAddress, u32 ValueToWrite);
*
******************************************************************************/
#define XMbox_WriteMBox(BaseAddress, ValueToWrite)		\
	XMbox_WriteReg (BaseAddress, XMB_WRITE_REG_OFFSET, ValueToWrite)

/*****************************************************************************/
/**
* Read the mbox read FIFO.
*
* @param	BaseAddress contains the base address of the mbox device.
*
* @return	The value read from the register, a 32 bit value.
*
* @note		C-style signature:
* 		u32 XMbox_ReadMBox(u32 BaseAddress);
*
******************************************************************************/
#define XMbox_ReadMBox(BaseAddress)				\
	XMbox_ReadReg (BaseAddress, XMB_READ_REG_OFFSET)

/*****************************************************************************/
/**
* Checks if the Read FIFO is Empty.
*
* @param	BaseAddress contains the base address of the mbox device.
*
* @return
*		- TRUE if the FIFO is empty.
*		- FALSE if there is data to be read from the FIFO.
*
* @note		C-style signature:
*		u32 XMbox_IsEmptyHw(u32 BaseAddress);
*
******************************************************************************/
#define XMbox_IsEmptyHw(BaseAddress)				 \
((XMbox_ReadReg (BaseAddress, XMB_STATUS_REG_OFFSET) & XMB_STATUS_FIFO_EMPTY))

/*****************************************************************************/
/**
* Checks if there is room in the Write FIFO.
*
* @param	BaseAddress contains the base address of the mbox device.
*
* @return
*		- FALSE if there is room in FIFO,
*		- TRUE if there is no room in the FIFO
*
* @note		C-style signature:
*		u32 XMbox_IsFullHw(u32 BaseAddress);
*
******************************************************************************/
#define XMbox_IsFullHw(BaseAddress)				\
((XMbox_ReadReg (BaseAddress, XMB_STATUS_REG_OFFSET) & XMB_STATUS_FIFO_FULL))

/*
 * The following macros are for the MicroBlaze specific Fast Simplex Link (FSL)
 * access method
 */

#ifdef __MICROBLAZE__
/*****************************************************************************/
/**
* Read the mbox FIFO using FSL
*
* @param	ID contains the link to the mbox device.
*
* @return	The value read from the FIFO, a 32 bit value.
*
* @note		C-style signature:
*		u32 XMbox_FSLReadReg(u32 ID)
*
******************************************************************************/
#define XMbox_FSLReadMBox(ID)		\
({ 					\
u32 rVal;				\
getdfslx(rVal,(ID),FSL_DEFAULT);	\
rVal;					\
})

/*****************************************************************************/
/**
* Write a specified value to mbox FIFO using FSL
*
* @param	ID contains the link to the mbox device.
* @param	ValueToWrite is the 32 bit value to be written to the register.
*
* @return	None
*
* @note		C-style signature:
*		void XMbox_FSLWriteReg(u32 ID, u32 ValueToWrite);
*
******************************************************************************/
#define XMbox_FSLWriteMBox(ID, ValueToWrite)			\
	putdfslx((ValueToWrite),(ID), FSL_DEFAULT)

/*****************************************************************************/
/**
* Checks if the Read FIFO is Empty.
*
* @param	ID contains the link to the mbox device.
*
* @return
*		- TRUE if the Read FIFO is empty.
*		- FALSE if the Read FIFO is not empty and has data to be read.
*
* @note		C-style signature:
*		u32 XMbox_FSLIsEmpty(u32 ID)
*
******************************************************************************/
#define XMbox_FSLIsEmpty(ID)		\
({					\
u32 temp, Empty;			\
tgetdfslx(temp,(ID), FSL_NONBLOCKING);	\
fsl_isinvalid(Empty);			\
Empty;					\
})

/*****************************************************************************/
/**
* Checks if there is room in the Write FIFO.
*
* @param	ID contains the link to the mbox device.
*
* @return
*		- TRUE if there is no room in the FIFO (It is Full)
*		- FALSE if there is room in FIFO
*
* @note		C-style signature:
*		u32 XMbox_FSLIsFull(u32 ID)
*
******************************************************************************/
#define XMbox_FSLIsFull(ID) 		\
({					\
u32 Full;				\
tputdfslx((ID), FSL_NONBLOCKING);	\
fsl_isinvalid(Full);			\
Full;					\
})

#else

/* these definitions allow the PPC version to compile, empty calls */

#define XMbox_FSLReadMBox(ID)			0
#define XMbox_FSLWriteMBox(ID, ValueToWrite)
#define XMbox_FSLIsEmpty(ID) 			0
#define XMbox_FSLIsFull(ID)			0

#endif /* __MICROBLAZE__ */

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
