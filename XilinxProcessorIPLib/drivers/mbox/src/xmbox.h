/******************************************************************************
*
* (c) Copyright 2007-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xmbox.h
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
* 1.01a ecm  08/19/08 Fixed CR’s 466320, 466322, 476535, 476242, 476243
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
	u32 BaseAddress;	/**< Register base address */
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
			u32 EffectiveAddress);
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
