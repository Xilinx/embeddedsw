/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* @file xospipsv.h
* @addtogroup ospips_v1_0
* @{
* @details
*
* This is the header file for the implementation of OSPIPS driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   nsk  02/19/18 First release
*
* </pre>
*
******************************************************************************/
#ifndef XOSPIPSV_H_		/* prevent circular inclusions */
#define XOSPIPSV_H_		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xospipsv_hw.h"
#include "xil_cache.h"

/**************************** Type Definitions *******************************/
/**
 * The handler data type allows the user to define a callback function to
 * handle the asynchronous processing for the OSPIPS device.  The application
 * using this driver is expected to define a handler of this type to support
 * interrupt driven mode.  The handler executes in an interrupt context, so
 * only minimal processing should be performed.
 *
 * @param	CallBackRef is the callback reference passed in by the upper
 *		layer when setting the callback functions, and passed back to
 *		the upper layer when the callback is invoked. Its type is
 *		not important to the driver, so it is a void pointer.
 * @param 	StatusEvent holds one or more status events that have occurred.
 *		See the XOspiPsv_SetStatusHandler() for details on the status
 *		events that can be passed in the callback.
 * @param	ByteCount indicates how many bytes of data were successfully
 *		transferred.  This may be less than the number of bytes
 *		requested if the status event indicates an error.
 */
typedef void (*XOspiPsv_StatusHandler) (void *CallBackRef, u32 StatusEvent,
					u32 ByteCount);

/**
 * This typedef contains configuration information for a flash message.
 */
typedef struct {
	u8 *TxBfrPtr;
	u8 *RxBfrPtr;
	u32 ByteCount;
	u32 Flags;
	u8 Opcode;
	u32 Addr;
	u8 Addrsize;
	u8 Addrvalid;
	u8 Dummy;
	u8 Proto;
} XOspiPsv_Msg;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	u32 BaseAddress;	/**< Base address of the device */
	u32 InputClockHz;	/**< Input clock frequency */
	u8  ConnectionMode; /**< Single, Stacked and Parallel mode */
} XOspiPsv_Config;

/**
 * The XOspiPsv driver instance data. The user is required to allocate a
 * variable of this type for every OSPIPS device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XOspiPsv_Config Config;	 /**< Configuration structure */
	u32 IsReady;		 /**< Device is initialized and ready */

	u8 *SendBufferPtr;	 /**< Buffer to send (state) */
	u8 *RecvBufferPtr;	 /**< Buffer to receive (state) */
	s32 TxBytes;	 /**< Number of bytes to transfer (state) */
	s32 RxBytes;	 /**< Number of bytes left to transfer(state) */
	u32 IsBusy;		 /**< A transfer is in progress (state) */
	u32 OpMode;		 /**< Operating Mode DMA, Linear or IO mode */
	u8 ChipSelect;
	XOspiPsv_Msg *Msg;
	XOspiPsv_StatusHandler StatusHandler;
	void *StatusRef;  	 /**< Callback reference for status handler */
} XOspiPsv;

/***************** Macros (Inline Functions) Definitions *********************/
#define XOSPIPSV_DAC_EN_OPTION	0x1U
#define XOSPIPSV_IDAC_EN_OPTION	0x2U
#define XOSPIPSV_STIG_EN_OPTION	0x4U
#define XOSPIPSV_CLK_POL_OPTION	0x8U
#define XOSPIPSV_CLK_PHASE_OPTION	0x10U
#define XOSPIPSV_PHY_EN_OPTION	0x20U
#define XOSPIPSV_LEGIP_EN_OPTION	0x40U
#define XOSPIPSV_DTR_EN_OPTION	0x80U
#define XOSPIPSV_CRC_EN_OPTION	0x100U
#define XOSPIPSV_DB_OP_EN_OPTION	0x200U
#define XOSPIPSV_IO_EN_OPTION	0x400U


#define XOspiPsv_ReadReg(BaseAddress, RegOffset) Xil_In32((BaseAddress) + (u32)(RegOffset))
#define XOspiPsv_WriteReg(BaseAddress, RegOffset, RegisterValue) \
		Xil_Out32((BaseAddress) + \
	(u32)(RegOffset), (u32)(RegisterValue)) ; \
	//xil_printf("Offset 0x%08x anv val 0x%08x\n\r", (BaseAddress + RegOffset), RegisterValue); \

#define XOSPIPSV_READMODE_DMA	0x0U //Review change READMODE
#define XOSPIPSV_READMODE_IO		0x1U
#define XOSPIPSV_READMODE_DAC	0x2U
#define XOSPIPSV_MSG_FLAG_RX	0x2U
#define XOSPIPSV_MSG_FLAG_TX	0x4U

#define XOSPIPSV_READ_1_1_1	0U
#define XOSPIPSV_READ_1_1_2	1U
#define XOSPIPSV_READ_1_1_4	2U
#define XOSPIPSV_READ_1_1_8	3U
#define XOSPIPSV_READ_1_8_8	4U

#define XOSPIPSV_WRITE_1_1_1	0U
#define XOSPIPSV_WRITE_1_1_2	1U
#define XOSPIPSV_WRITE_1_1_4	2U
#define XOSPIPSV_WRITE_1_1_8	3U
#define XOSPIPSV_WRITE_1_8_8	4U

#define XOSPIPSV_SELECT_FLASH_CS0	0
#define XOSPIPSV_SELECT_FLASH_CS1	1

#define XOSPIPSV_CLK_PRESCALE_1		1U
#define XOSPIPSV_CLK_PRESCALE_2		2U
#define XOSPIPSV_CLK_PRESCALE_3		3U
#define XOSPIPSV_CLK_PRESCALE_4		4U
#define XOSPIPSV_CLK_PRESCALE_5		5U
#define XOSPIPSV_CLK_PRESCALE_6		6U
#define XOSPIPSV_CLK_PRESCALE_7		7U
#define XOSPIPSV_CLK_PRESCALE_8		8U
#define XOSPIPSV_CLK_PRESCALE_9		9U
#define XOSPIPSV_CLK_PRESCALE_10		10U
#define XOSPIPSV_CLK_PRESCALE_11		11U
#define XOSPIPSV_CLK_PRESCALE_12		12U
#define XOSPIPSV_CLK_PRESCALE_13		13U
#define XOSPIPSV_CLK_PRESCALE_14		14U
#define XOSPIPSV_CLK_PRESCALE_15		15U
#define XOSPIPSV_CR_PRESC_MAXIMUM	15U

#define XOSPIPSV_IOMODE_BYTECNT	8U

/* Initialization and reset */
XOspiPsv_Config *XOspiPsv_LookupConfig(u16 DeviceId);
s32 XOspiPsv_CfgInitialize(XOspiPsv *InstancePtr, XOspiPsv_Config *ConfigPtr);
void XOspiPsv_Reset(XOspiPsv *InstancePtr);
/* Configuration functions */
s32 XOspiPsv_SetClkPrescaler(XOspiPsv *InstancePtr, u8 Prescaler);
s32 XOspiPsv_SelectFlash(XOspiPsv *InstancePtr, u8 FlashCS);
s32 XOspiPsv_SetOptions(XOspiPsv *InstancePtr, u32 Options);
u32 XOspiPsv_GetOptions(XOspiPsv *InstancePtr);
u32 XOspiPsv_PollTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_H_ */
/** @} */
