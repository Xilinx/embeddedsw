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
* @file xmbox.h
* @addtogroup mbox_v4_0
* @{
* @details
*
* The Xilinx mailbox driver. This driver supports the Xilinx
* Mailbox device. More detailed description of the driver operation can
* be found in the xmbox.c file.
*
* - The Xilinx Mailbox is intended to be used as a bi-directional communication
*   core between a pair of processors. The mailbox API shall allow software
*   to send messages in a FIFO fashion, where the receiver is intended to be
*   software on another processor.
* - The mailbox implementation provides for a way to "send" and "receive" in
*   an atomic fashion using seperate FIFOs in the mailbox core.
* - The API itself does not stop the use case where there is more than one
*   sender on the transmit side and more than one receiver on the receiving side
*   (just like TCP/IP sockets). However, unless there is a protocol implemented
*   in the messages being transferred, it is typically good practice to assign
*   just one transmit processor and one receiver processor to a single mailbox.
* - The API provides for both blocking and non-blocking semantics on the send
*   and receive operations.
*
* <b>Initialization & Configuration</b>
*
* The XMbox_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
*   - XMbox_LookupConfig(DeviceId) - Use the device identifier to find the
*     static configuration structure defined in XMbox_g.c. This is setup by
*     the tools. For some operating systems the config structure will be
*     initialized by the software and this call is not needed. This function
*     returns the CfgPtr argument used by the CfgInitialize function described
*     below.
*
*   - XMbox_CfgInitialize(InstancePtr, CfgPtr, EffectiveAddress) - Uses a
*     configuration structure provided by the caller. If running in a system
*     with address translation, the provided virtual memory base address
*     replaces the physical address present in the configuration structure.
*     The EffectiveAddress argument is required regardless of operating system
*     environment, i.e. in standalone, CfgPtr->BaseAddress is recommended, not
*     the xparameters.h #define.
*
*
* <b>Interrupts</b>
*
* The Mailbox hardware has two interrupt outputs, one for each interface.
*
* For an interface that uses Direct FSL the associated interrupt pin shall be
* high while there is data in the FIFO for this interface.
*
* For a PLB interface there shall be 3 associated interrupt sources that can be
* controlled through dedicated registers. Each of these sources shall be
* associated with a specific bit position in the related register. The sources
* shall be: Send Threshold Interrupt (STI), Receive Threshold Interrupt (RTI)
* and FIFO Error (ERR). RTI is set when the number of entries in the receive
* FIFO become greater than the RIT value (rising edge on RTA). STI is set when
* the number of entries in the send FIFO becomes equal or less than the SIT
* value (rising edge on STA). RTI and STI are only set when their respective
* conditions goes from false to true, not continuously when the condition is
* fulfilled.
*
* The Mailbox driver does not have an interrupt service routine. It is the
* responsibility of the caller of Mailbox functions to manage the interrupt
* including connecting to the interrupt and enabling/disabling the interrupt.
* The user can create a handler to service the interrupts generated by the
* Mailbox IP.
*
* Using the Blocking version of the Read function is not recommended since
* the processor will hang until the requested length is received, which might
* be quite a long time.
*
* @note
*
* This driver is intended to be RTOS and processor independent. It works with
* physical addresses only. Any needs for dynamic memory management, threads
* or thread mutual exclusion, virtual memory, or cache control must be
* satisfied by the layer above this driver.
*
* Possible Optimization technique:
* If the interface for the hardware is only expected to be the memory mapped
* or the FSL interface for the lifetime of the project, it is reasonable to
* remove the other, unused, leg through the functions which allow access to
* the other interface method, i.e. if FSL is the only available interface the
* memory mapped clause in the if statements can be removed improving the
* performance some due to the lack of the test and branch.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a va            First release
* 1.00a ecm  06/01/07 Cleanup, new coding standard, check into XCS
* 			Converted to new XPS hardware.
* 1.01a ecm  08/19/08 Fixed CRs 466320, 466322, 476535, 476242, 476243
*			new rev
* 2.00a hm   04/09/09 Added support for mailbox v2.0, which has interrupts
*		      Fixed CR 502464, which removed extra
*		      definitions that are not associated with
*		      the interface.
*		      Fixed the canonical definition so that each
*		      interface is considered as a device instance.
* 3.01a sdm  05/06/10 New driver to support AXI version of the core and
*		      cleaned up for coding guidelines.
* 3.02a bss  08/18/12 Updated tcl script to support Zynq system and AXI stream
* 		      interface for CR 672073 and CR 655224 respectively.
*		      Added XMbox_GetStatus API for CR 676187
* 3.03a bss   01/30/13 Updated driver tcl to fix CR#687103 and CR#688715
* 3.04a bss   05/13/13 Updated test app tcl to display message that Peripheral
*		       tests will not be run if there is only one Processor
*		       in design CR#715626
* 4.0   adk  19/12/13 Updated as per the New Tcl API's
* 4.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XMbox_CfgInitialize API.
*       ms   01/23/17 Modified xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
*       ms   04/05/17 Added tabspace for return statements in functions of
*                     mbox examples for proper documentation while
*                     generating doxygen and modified filename tag to
*                     include them in doxygen examples.
* 4.2   ms   04/18/17 Modified tcl file to add suffix U for all macros
*                     definitions of mbox in xparameters.h
*       ms   08/07/17 Fixed compilation warnings in xmbox_sinit.c
* 4.3   sa   04/20/17 Support for FIFO reset using hardware control register.
*       sd   07/26/17 Modified tcl file to prevent false unconnected flagging.
*
*</pre>
*
******************************************************************************/

#ifndef XMBOX_H			/* prevent circular inclusions */
#define XMBOX_H			/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xmbox_hw.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID of device */
	UINTPTR BaseAddress;	/**< Register base address */
	u8 UseFSL;		/**< use the FSL for the interface. */
	u8 SendID;		/**< FSL link for the write i/f mailbox. */
	u8 RecvID;		/**< FSL link for the read i/f mailbox. */

} XMbox_Config;

/**
 * The XMbox driver instance data. The user is required to allocate a
 * variable of this type for every mbox device in the system. A
 * pointer to a variable of this type is then passed to the driver API
 * functions.
 */
typedef struct {
	XMbox_Config Config;	/**< Configuration data, includes base address
				  */
	u32 IsReady;		/**< Device is initialized and ready */
} XMbox;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/*
 * Required functions, in file xmbox.c
 */
int XMbox_CfgInitialize(XMbox *InstancePtr, XMbox_Config *ConfigPtr,
			UINTPTR EffectiveAddress);
int XMbox_Read(XMbox *InstancePtr, u32 *BufferPtr, u32 RequestedBytes,
			u32 *BytesRecvdPtr);
void XMbox_ReadBlocking(XMbox *InstancePtr, u32 *BufferPtr,
			u32 RequestedBytes);
int XMbox_Write(XMbox *InstancePtr, u32 *BufferPtr, u32 RequestedBytes,
		u32 *BytesSentPtr);
void XMbox_WriteBlocking(XMbox *InstancePtr, u32 *BufferPtr,
			 u32 RequestedBytes);
u32 XMbox_IsEmpty(XMbox *InstancePtr);
u32 XMbox_IsFull(XMbox *InstancePtr);
int XMbox_Flush(XMbox *InstancePtr);
void XMbox_ResetFifos(XMbox *InstancePtr);
void XMbox_SetInterruptEnable(XMbox *InstancePtr, u32 Mask);
u32 XMbox_GetInterruptEnable(XMbox *InstancePtr);
u32 XMbox_GetInterruptStatus(XMbox *InstancePtr);
void XMbox_ClearInterrupt(XMbox *InstancePtr, u32 Mask);
u32 XMbox_GetStatus(XMbox *InstancePtr);
void XMbox_SetSendThreshold(XMbox *InstancePtr, u32 Value);
void XMbox_SetReceiveThreshold(XMbox *InstancePtr, u32 Value);

/*
 * Static initialization function, in file xmbox_sinit.c
 */
XMbox_Config *XMbox_LookupConfig(u16 DeviceId);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
