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
* @file xmbox.c
* @addtogroup mbox_v4_3
* @{
*
* Contains required functions for the XMbox driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 1.01a ecm  08/19/08 Fixed the receive routine, FSL leg, was using SendID
*			instead of the correct RecvID.
* 			Fixed CRs 466320, 466322, 476535, 476242, 476243
*					  new rev
* 2.00a hm   04/09/09 Added support for mailbox v2.0, which has interrupts;
*			Fixed CR 502464, which removed extra
*			definitions that are not associated with
*			the interface.
*			Fixed the canonical definition so that each
*			interface is considered as a device instance.
* 3.00a hbm  10/19/09   Migrated to HAL phase 1 to use xil_io.
*			Removed _m from the function names.
*			Renamed _mIsEmpty to _IsEmptyHw and _mIsFull
*			to _IsFullHw.
* 3.02a bss  08/18/12   Added XMbox_GetStatus API for CR 676187
* 4.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XMbox_CfgInitialize API.
* 4.3   sa   04/20/17 Support for FIFO reset using hardware control register.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <string.h>
#include "xmbox.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* Initializes a specific mailbox.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
* @param	CfgPtr is the device configuration structure containing
*		required HW build data.
* @param	EffectiveAddr is the Physical address of the hardware in a
*		Virtual Memory operating system environment. It is the Base
*		Address in a stand alone environment.
*
* @return
*		- XST_SUCCESS if initialization was successful
*
* @note		None.
*
******************************************************************************/
int XMbox_CfgInitialize(XMbox *InstancePtr, XMbox_Config *ConfigPtr,
			UINTPTR EffectiveAddress)
{

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/*
	 * Clear instance memory and make copy of configuration
	 */
	memset(InstancePtr, 0, sizeof(XMbox));
	memcpy(&InstancePtr->Config, ConfigPtr, sizeof(XMbox_Config));

	InstancePtr->Config.BaseAddress = EffectiveAddress;
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Reads requested bytes from the mailbox referenced by InstancePtr,into the
* buffer pointed to by the provided pointer. The number of bytes must be a
* multiple of 4 (bytes). If not, the call will fail in an assert.
*
* This function is non blocking.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
* @param	BufferPtr is the buffer to read the mailbox contents into,
*		aligned to a word boundary.
* @param	RequestedBytes is the number of bytes of data requested.
* @param	BytesRecvdPtr is the memory that is updated with the number of
*		bytes of data actually read.
*
* @return
*		- XST_SUCCESS on success.
*		- XST_NO_DATA ifthere was no data in the mailbox.
*
* On success, the number of bytes read is returned through the pointer.  The
* call may return with fewer bytes placed in the buffer than requested  (not
* including zero). This is not necessarily an error condition and indicates
* the amount of data that was currently available in the mailbox.
*
* @note		None.
*
******************************************************************************/
int XMbox_Read(XMbox *InstancePtr, u32 *BufferPtr, u32 RequestedBytes,
			u32 *BytesRecvdPtr)
{
	u32 NumBytes;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(!((u32) BufferPtr & 0x3));
	Xil_AssertNonvoid(RequestedBytes != 0);
	Xil_AssertNonvoid((RequestedBytes %4) == 0);
	Xil_AssertNonvoid(BytesRecvdPtr != NULL);

	NumBytes = 0;

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */
		if (XMbox_IsEmptyHw(InstancePtr->Config.BaseAddress))
			return XST_NO_DATA;

		/*
		 * Read the Mailbox until empty or the length requested is
		 * satisfied
		 */
		do {
			*BufferPtr++ =
				XMbox_ReadMBox(InstancePtr->Config.BaseAddress);
			NumBytes += 4;
		} while ((NumBytes != RequestedBytes) &&
			 !(XMbox_IsEmptyHw(InstancePtr->Config.BaseAddress)));

		*BytesRecvdPtr = NumBytes;
	} else {

		/* FSL based Access */
		if (XMbox_FSLIsEmpty(InstancePtr->Config.RecvID))
			return XST_NO_DATA;

		/*
		 * Read the Mailbox until empty or the length requested is
		 * satisfied
		 */
		do {
			*BufferPtr++ =
				XMbox_FSLReadMBox(InstancePtr->Config.RecvID);
			NumBytes += 4;
		} while ((NumBytes != RequestedBytes) &&
			 !(XMbox_FSLIsEmpty(InstancePtr->Config.RecvID)));

		*BytesRecvdPtr = NumBytes;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Reads requested bytes from the mailbox referenced by InstancePtr,into the
* buffer pointed to by the provided pointer. The number of bytes must be a
* multiple of 4 (bytes). If not, the call will fail in an assert.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
* @param	BufferPtr is the buffer to read the mailbox contents into,
*		aligned to a word boundary.
* @param	RequestedBytes is the number of bytes of data requested.
*
* @return	None.
*
* @note		The call blocks until the number of bytes requested are
*		available.
*
******************************************************************************/
void XMbox_ReadBlocking(XMbox *InstancePtr, u32 *BufferPtr,
			u32 RequestedBytes)
{
	u32 NumBytes;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(!((u32) BufferPtr & 0x3));
	Xil_AssertVoid(RequestedBytes != 0);
	Xil_AssertVoid((RequestedBytes % 4) == 0);

	NumBytes = 0;

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */
		/* Block while the mailbox FIFO has at-least some data */

		do {
			while(XMbox_IsEmptyHw(InstancePtr->Config.BaseAddress));

			/*
			 * Read the Mailbox until empty or the length
			 * requested is satisfied
			 */
			*BufferPtr++ =
				XMbox_ReadMBox(InstancePtr->Config.BaseAddress);
			NumBytes += 4;
		} while (NumBytes != RequestedBytes);
	} else {

		/* FSL based Access */
		/* Block while the mailbox FIFO has at-least some data */

		do {
			while (XMbox_FSLIsEmpty(InstancePtr->Config.RecvID));

			/*
			 * Read the Mailbox until empty or the length requested
			 * is satisfied
			 */

			*BufferPtr++ =
				XMbox_FSLReadMBox(InstancePtr->Config.RecvID);
			NumBytes += 4;
		} while (NumBytes != RequestedBytes);
	}
}

/*****************************************************************************/
/**
* Writes the requested bytes from the buffer pointed to by the provided
* pointer into the mailbox referenced by InstancePtr.The number of bytes must
* be a multiple of 4 (bytes). If not, the call will fail in an assert.
*
* This function is non blocking.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
* @param	BufferPtr is the source data buffer, aligned to a word
*		boundary.
* @param	RequestedBytes is the number of bytes requested to be written.
* @param	BytesRecvdPtr points to memory which is updated with the actual
*		number of bytes written, return value.
* @return
*
*		- XST_SUCCESS on success.
*		- XST_FIFO_NO_ROOM if the fifo was full.

* On success, the number of bytes successfully written into the destination
* mailbox is returned in the provided pointer. The call may  return with
* zero. This is not necessarily an error condition and indicates that the
* mailbox is currently full.
*
* @note		The provided buffer pointed to by BufferPtr must be aligned to a
*		word boundary.
*
******************************************************************************/
int XMbox_Write(XMbox *InstancePtr, u32 *BufferPtr, u32 RequestedBytes,
		u32 *BytesSentPtr)
{
	u32 NumBytes;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(!((u32) BufferPtr & 0x3));
	Xil_AssertNonvoid(RequestedBytes != 0);
	Xil_AssertNonvoid((RequestedBytes %4) == 0);
	Xil_AssertNonvoid(BytesSentPtr != NULL);

	NumBytes = 0;

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */

		if (XMbox_IsFullHw(InstancePtr->Config.BaseAddress)) {
			return XST_FIFO_NO_ROOM;
		}

		/*
		 * Write to the Mailbox until full or the length requested is
		 * satisfied.
		 */

		do {
			XMbox_WriteMBox(InstancePtr->Config.BaseAddress,
					*BufferPtr++);
			NumBytes += 4;
		} while ((NumBytes != RequestedBytes) &&
			 !(XMbox_IsFullHw(InstancePtr->Config.BaseAddress)));

		*BytesSentPtr = NumBytes;
	} else {

		/* FSL based Access */
		if (XMbox_FSLIsFull(InstancePtr->Config.SendID)) {
			return XST_FIFO_NO_ROOM;
		}

		/*
		 * Write to the Mailbox until full or the length requested is
		 * satisfied.
		 */
		do {
			XMbox_FSLWriteMBox(InstancePtr->Config.SendID,
					    *BufferPtr++);
			NumBytes += 4;
		} while ((NumBytes != RequestedBytes) &&
			 !(XMbox_FSLIsFull(InstancePtr->Config.SendID)));

		*BytesSentPtr = NumBytes;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* Writes the requested bytes from the buffer pointed to by the provided
* pointer into the mailbox referenced by InstancePtr. The number of bytes must
* be a multiple of 4 (bytes). If not, the call will fail in an assert.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
* @param	BufferPtr is the source data buffer, aligned to a word boundary.
* @param	RequestedBytes is the number of bytes requested to be written.
*
* @return	None.
*
* @note		The call blocks until the number of bytes requested are written.
*		The provided buffer pointed to by BufferPtr must be aligned to a
*		word boundary.
*
******************************************************************************/
void XMbox_WriteBlocking(XMbox *InstancePtr, u32 *BufferPtr, u32 RequestedBytes)
{
	u32 NumBytes;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(!((u32) BufferPtr & 0x3));
	Xil_AssertVoid(RequestedBytes != 0);
	Xil_AssertVoid((RequestedBytes %4) == 0);

	NumBytes = 0;

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */
		/* Block while the mailbox FIFO becomes free to transfer
		 * at-least one word
		 */
		do {
			while (XMbox_IsFullHw(InstancePtr->Config.BaseAddress));

			XMbox_WriteMBox(InstancePtr->Config.BaseAddress,
					 *BufferPtr++);
			NumBytes += 4;
		} while (NumBytes != RequestedBytes);
	} else {

		/* FSL based Access */
		/* Block while the mailbox FIFO becomes free to transfer
		 * at-least one word
		 */
		do {
			while (XMbox_FSLIsFull(InstancePtr->Config.SendID));

			XMbox_FSLWriteMBox(InstancePtr->Config.SendID,
					    *BufferPtr++);
			NumBytes += 4;
		} while (NumBytes != RequestedBytes);
	}
}

/*****************************************************************************/
/**
*
* Checks to see if there is data available to be read.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
*
* @return
*		- FALSE if there is data to be read.
*		- TRUE is there no data to be read.
*
* @note		None.
*
******************************************************************************/
u32 XMbox_IsEmpty(XMbox *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */
		return (XMbox_IsEmptyHw(InstancePtr->Config.BaseAddress));
	} else {
		/* FSL based Access */
		return (XMbox_FSLIsEmpty(InstancePtr->Config.RecvID));
	}
}

/*****************************************************************************/
/**
*
* Checks to see if there is room in the write FIFO.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
*
* @return
*		- FALSE if there is room in write FIFO.
*		- TRUE if there is room in write FIFO.
*
* @note		None.
*
******************************************************************************/
u32 XMbox_IsFull(XMbox *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */
		return (XMbox_IsFullHw(InstancePtr->Config.BaseAddress));
	} else {
		/* FSL based Access */
		return (XMbox_FSLIsFull(InstancePtr->Config.SendID));
	}
}

/*****************************************************************************/
/**
*
* Resets the mailbox FIFOs by emptying the READ FIFO and making sure the
* Error Status is zero.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
*
* @return
*		- XST_SUCCESS on success.
*		- XST_FAILURE if there are any outstanding errors.
*
* @note		Data from read FIFO is thrown away.
*
******************************************************************************/
int XMbox_Flush(XMbox *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);

	if (InstancePtr->Config.UseFSL == 0) {
		/* For memory mapped IO */
		do {
			(void)XMbox_ReadMBox(InstancePtr->Config.BaseAddress);
		} while (!(XMbox_IsEmptyHw(InstancePtr->Config.BaseAddress)));

	} else {
		/* FSL based Access */
		do {
			(void) XMbox_FSLReadMBox(InstancePtr->Config.RecvID);
		} while (!(XMbox_FSLIsEmpty(InstancePtr->Config.RecvID)));
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* Resets the mailbox FIFOs by clearing the READ and WRITE FIFOs using the
* hardware control register for memory mapped IO.
*
* @param	InstancePtr is a pointer to the XMbox instance to be worked on.
*
* @return	None.
*
* @note		Use XMbox_Flush instead for FSL based access.
*
******************************************************************************/
void XMbox_ResetFifos(XMbox *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.UseFSL == 0);

	/* For memory mapped IO:
	 *
	 * Write to the control register to reset both send and
	 * receive FIFOs, these bits are self-clearing such that
	 * there's no need to clear them.
	 */
	XMbox_WriteReg(InstancePtr->Config.BaseAddress,
		       XMB_CTRL_REG_OFFSET,
		       XMB_CTRL_RESET_SEND_FIFO |
		       XMB_CTRL_RESET_RECV_FIFO);
}

/*****************************************************************************/
/**
* Sets the interrupt enable register for this mailbox. This function can only
* be used for Non-FSL interface. If not, the function will fail in an assert.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Mask is a logical OR of XMB_IX_* constants found in xmbox_hw.h.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XMbox_SetInterruptEnable(XMbox *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.UseFSL == 0);

	if (InstancePtr->Config.UseFSL == 0)
		XMbox_WriteReg(InstancePtr->Config.BaseAddress,
			       XMB_IE_REG_OFFSET,
			       Mask);
}

/*****************************************************************************/
/**
* Retrieves the interrupt enable for the mailbox. AND the result of this
* function with XMB_IX_* to determine which interrupts of this mailbox
* are enabled. This function can only be used for Non-FSL interface. If not,
* the function will fail in an assert.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
*
* @return	Mask of interrupt bits made up of XMB_IX_* constants found
*		in xmbox_hw.h.
*
* @note		None.
*
*
******************************************************************************/
u32 XMbox_GetInterruptEnable(XMbox *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.UseFSL == 0);

	return XMbox_ReadReg(InstancePtr->Config.BaseAddress,
				XMB_IE_REG_OFFSET);
}

/*****************************************************************************/
/**
* Retrieve the interrupt status for the mailbox. AND the results of this
* function with XMB_IX_* to determine which interrupts are currently pending
* to the processor. This function can only be used for Non-FSL interface.
* If not, the function will fail in an assert.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
*
* @return	Mask of interrupt bits made up of XMB_IX_* constants found
*		in xmbox_hw.h.
*
******************************************************************************/
u32 XMbox_GetInterruptStatus(XMbox *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.UseFSL == 0);

	return XMbox_ReadReg(InstancePtr->Config.BaseAddress,
				XMB_IS_REG_OFFSET);
}

/*****************************************************************************/
/**
* Clears pending interrupts with the provided mask. This function should be
* called after the software has serviced the interrupts that are pending.
* This function clears the corresponding bits of the Interrupt Status
* Register. This function can only be used for Non-FSL interface. If not, the
* function will fail in an assert.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Mask is a logical OR of XMB_IX_* constants found in
*		xmbox_hw.h.
*
* @note		None.
*
******************************************************************************/
void XMbox_ClearInterrupt(XMbox *InstancePtr, u32 Mask)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.UseFSL == 0);

	if (InstancePtr->Config.UseFSL == 0) {
		XMbox_WriteReg(InstancePtr->Config.BaseAddress,
				XMB_IS_REG_OFFSET,
				Mask);
	}
}

/*****************************************************************************/
/**
* Sets the Send Interrupt Threshold. This function can only be used for
* Non-FSL interface. If not, the function will fail in an assert.
*
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Value is a value to set for the SIT. Only lower
*		Log2(FIFO Depth) bits are used.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XMbox_SetSendThreshold(XMbox *InstancePtr, u32 Value)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.UseFSL == 0);

	if (InstancePtr->Config.UseFSL == 0) {
		XMbox_WriteReg(InstancePtr->Config.BaseAddress,
				XMB_SIT_REG_OFFSET,
				Value);
	}
}

/*****************************************************************************/
/**
* Set the Receive Interrupt Threshold. This function can only be used for
* Non-FSL interface. If not, the function will fail in an assert.
* @param	InstancePtr is a pointer to the instance to be worked on.
* @param	Value is a value to set for the RIT. Only lower
*		Log2(FIFO Depth) bits are used.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XMbox_SetReceiveThreshold(XMbox *InstancePtr, u32 Value)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->Config.UseFSL == 0);

	if (InstancePtr->Config.UseFSL == 0) {
		XMbox_WriteReg(InstancePtr->Config.BaseAddress,
				XMB_RIT_REG_OFFSET,
				Value);
	}
}

/*****************************************************************************/
/**
* Returns Status register contents. This function can only be used for
* Non-FSL interface. If not, the function will fail in an assert.
* @param	InstancePtr is a pointer to the instance to be worked on.
*
* @return	Value returns Status Register contents.
*
* @note		None.
*
******************************************************************************/
u32 XMbox_GetStatus(XMbox *InstancePtr)
{
	u32 Value;
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Config.UseFSL == 0);


	Value = XMbox_ReadReg(InstancePtr->Config.BaseAddress,
				XMB_STATUS_REG_OFFSET);
	return Value;

}
/** @} */
