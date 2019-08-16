/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xospipsv.h
* @addtogroup ospipsv_v1_1
* @{
* @details
*
* This is the header file for the implementation of OSPIPSV driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------.
* 1.0   nsk  02/19/18 First release
*       sk   01/09/19 Added interrupt mode support.
*                     Remove STIG/DMA mode selection by the user, driver will
*                     take care of operating in DMA/STIG based on command.
*                     Added support for unaligned byte count read.
*       sk   02/04/19 Added support for SDR+PHY and DDR+PHY modes.
*       sk   02/07/19 Added OSPI Idling sequence.
* 1.0   akm 03/29/19 Fixed data alignment issues on IAR compiler.
* 1.1   sk   07/22/19 Added RX Tuning algorithm for SDR and DDR modes.
*       sk   08/08/19 Added flash device reset support.
*       sk   08/16/19 Set Read Delay Fld to 0x1 for Non-Phy mode.
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
#include "xil_mem.h"
#if defined (__aarch64__)
#include "xil_smc.h"
#endif

/**************************** Type Definitions *******************************/
/**
 * The handler data type allows the user to define a callback function to
 * handle the asynchronous processing for the OSPIPSV device.  The application
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
 */
typedef void (*XOspiPsv_StatusHandler) (void *CallBackRef, u32 StatusEvent);

/**
 * This typedef contains configuration information for a flash message.
 */
typedef struct {
	u8 *TxBfrPtr;	/**< Write buffer pointer */
	u8 *RxBfrPtr;	/**< Read buffer pointer */
	u32 ByteCount;	/**< Number of bytes to read or write */
	u32 Flags;		/**< Used to indicate the Msg is for TX or RX */
	u8 Opcode;		/**< Opcode/Command */
	u32 Addr;		/**< Device Address */
	u8 Addrsize;	/**< Size of address in bytes */
	u8 Addrvalid;	/**< 1 if Address is required for opcode, 0 otherwise */
	u8 Dummy;		/**< Number of dummy cycles for opcode */
	u8 Proto;		/**< Indicate number of Cmd-Addr-Data lines */
	u8 IsDDROpCode;	/**< 1 if opcode is DDR command, 0 otherwise */
} XOspiPsv_Msg;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	u32 BaseAddress;	/**< Base address of the device */
	u32 InputClockHz;	/**< Input clock frequency */
	u8 IsCacheCoherent;		/**< If OSPI is Cache Coherent or not */
} XOspiPsv_Config;

/**
 * The XOspiPsv driver instance data. The user is required to allocate a
 * variable of this type for every OSPIPSV device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XOspiPsv_Config Config;	 /**< Configuration structure */
	u32 IsReady;		 /**< Device is initialized and ready */
	u8 *SendBufferPtr;	 /**< Buffer to send (state) */
	u8 *RecvBufferPtr;	 /**< Buffer to receive (state) */
	u32 TxBytes;	 /**< Number of bytes to transfer (state) */
	u32 RxBytes;	 /**< Number of bytes left to transfer(state) */
	u32 IsBusy;		 /**< A transfer is in progress (state) */
	u32 OpMode;		 /**< Operating Mode DAC or INDAC */
	u32 SdrDdrMode;	/**< Edge mode can be SDR or DDR */
	u8 ChipSelect;
	XOspiPsv_Msg *Msg;
	XOspiPsv_StatusHandler StatusHandler;
	void *StatusRef;  	 /**< Callback reference for status handler */
	u8 IsUnaligned;		/* Flag used to indicate bytecnt is aligned or not */
	u32 DeviceIdData;	/* Contains Device Id Data information */
	u8 Extra_DummyCycle;
#ifdef __ICCARM__
#pragma pack(push, 8)
	u8 UnalignReadBuffer[4];	/**< Buffer used to read the unaligned bytes in DMA */
#pragma pack(pop)
#else
	u8 UnalignReadBuffer[4] __attribute__ ((aligned(64)));
#endif
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


#define XOspiPsv_ReadReg(BaseAddress, RegOffset) Xil_In32((BaseAddress) + (u32)(RegOffset))
#define XOspiPsv_WriteReg(BaseAddress, RegOffset, RegisterValue) \
		Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(RegisterValue))

#define XOSPIPSV_IDAC_MODE		0x0U
#define XOSPIPSV_DAC_MODE		0x1U

#define XOSPIPSV_MSG_FLAG_RX	0x2U
#define XOSPIPSV_MSG_FLAG_TX	0x4U

#define XOSPIPSV_READ_1_1_1	0U
#define XOSPIPSV_READ_1_1_8	3U
#define XOSPIPSV_READ_1_8_8	4U
#define XOSPIPSV_READ_8_8_8	5U
#define XOSPIPSV_READ_8_0_8	6U

#define XOSPIPSV_WRITE_1_1_1	0U
#define XOSPIPSV_WRITE_1_1_8	3U
#define XOSPIPSV_WRITE_1_8_8	4U
#define XOSPIPSV_WRITE_8_8_8	5U
#define XOSPIPSV_WRITE_8_0_0	6U
#define XOSPIPSV_WRITE_8_8_0	7U

#define XOSPIPSV_SELECT_FLASH_CS0	0
#define XOSPIPSV_SELECT_FLASH_CS1	1

#define XOSPIPSV_CLK_PRESCALE_2		0U
#define XOSPIPSV_CLK_PRESCALE_4		1U
#define XOSPIPSV_CLK_PRESCALE_6		2U
#define XOSPIPSV_CLK_PRESCALE_8		3U
#define XOSPIPSV_CLK_PRESCALE_10	4U
#define XOSPIPSV_CLK_PRESCALE_12	5U
#define XOSPIPSV_CLK_PRESCALE_14	6U
#define XOSPIPSV_CLK_PRESCALE_16	7U
#define XOSPIPSV_CLK_PRESCALE_18	8U
#define XOSPIPSV_CLK_PRESCALE_20	9U
#define XOSPIPSV_CLK_PRESCALE_22	10U
#define XOSPIPSV_CLK_PRESCALE_24	11U
#define XOSPIPSV_CLK_PRESCALE_26	12U
#define XOSPIPSV_CLK_PRESCALE_28	13U
#define XOSPIPSV_CLK_PRESCALE_30	14U
#define XOSPIPSV_CLK_PRESCALE_32	15U
#define XOSPIPSV_CR_PRESC_MAXIMUM	15U

#define XOSPIPSV_IOMODE_BYTECNT	8U

/* Temporary macro for fsbl and can be removed after fsbl update */
#define XOSPIPSV_CLK_PRESCALE_15	XOSPIPSV_CLK_PRESCALE_32

#define XOSPIPSV_NO_SLAVE_SELCT_VALUE	0xFU
#define XOSPIPSV_DISABLE_DAC_VALUE		0x0U
#define XOSPIPSV_SPI_DISABLE_VALUE		0x0U
#define XOSPIPSV_CONFIG_INIT_VALUE		(((u32)XOSPIPSV_CLK_PRESCALE_2 << \
				(u32)XOSPIPSV_CONFIG_REG_MSTR_BAUD_DIV_FLD_SHIFT) | \
			((u32)XOSPIPSV_NO_SLAVE_SELCT_VALUE << \
					(u32)XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_SHIFT) | \
			((u32)XOSPIPSV_DISABLE_DAC_VALUE << \
					(u32)XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_SHIFT) | \
					(u32)XOSPIPSV_SPI_DISABLE_VALUE)
#define XOSPIPSV_POLL_CNT_FLD_PHY	0x3U
#define XOSPIPSV_POLL_CNT_FLD_NON_PHY	0x1U
#define XOSPIPSV_MIN_PHY_FREQ	50000000
#define XOSPIPSV_DDR_STATS_REG_DUMMY	0x8U

#define XOSPIPSV_EDGE_MODE_SDR_PHY			0x0U
#define XOSPIPSV_EDGE_MODE_SDR_NON_PHY		0x1U
#define XOSPIPSV_EDGE_MODE_DDR_PHY			0x2U

#define XOSPIPSV_REMAP_ADDR_VAL		0x40000000U
#define XOSPIPSV_SDR_TX_VAL			0x5U
#define XOSPIPSV_DDR_TX_VAL			0x0U

#define XOSPIPSV_HWPIN_RESET	0x0U
#define XOSPIPSV_INBAND_RESET	0x1U

#define XOSPIPSV_NON_PHY_RD_DLY		0x1

/* Initialization and reset */
XOspiPsv_Config *XOspiPsv_LookupConfig(u16 DeviceId);
u32 XOspiPsv_CfgInitialize(XOspiPsv *InstancePtr, const XOspiPsv_Config *ConfigPtr);
void XOspiPsv_Reset(const XOspiPsv *InstancePtr);
/* Configuration functions */
u32 XOspiPsv_SetClkPrescaler(XOspiPsv *InstancePtr, u8 Prescaler);
u32 XOspiPsv_SelectFlash(XOspiPsv *InstancePtr, u8 chip_select);
u32 XOspiPsv_SetOptions(XOspiPsv *InstancePtr, u32 Options);
u32 XOspiPsv_GetOptions(const XOspiPsv *InstancePtr);
u32 XOspiPsv_PollTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_IntrTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_IntrHandler(XOspiPsv *InstancePtr);
void XOspiPsv_SetStatusHandler(XOspiPsv *InstancePtr, void *CallBackRef,
				XOspiPsv_StatusHandler FuncPointer);
u32 XOspiPsv_SetSdrDdrMode(XOspiPsv *InstancePtr, u32 Mode);
void XOspiPsv_ConfigureAutoPolling(XOspiPsv *InstancePtr, u32 FlashMode);
void XOspiPsv_Idle(const XOspiPsv *InstancePtr);
u32 XOspiPsv_DeviceReset(u8 Type);
#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_H_ */
/** @} */
