/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xospipsv.h
* @addtogroup Overview
* @{
* @details
*
* This section contains information about the driver structures, user
* API prototypes and all the defines required for the user to interact
* with the driver. OspiPsv driver supports the OSPI controller in Versal
* SoC platform.
*
* <b>Initialization & Configuration</b>
*
* The XOspiPsv_Config structure is used by the driver to configure. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed by
* various operating systems, the driver instance can be initialized in the
* following way:
*
*    - XOspiPsv_LookupConfig(u16 DeviceId) - Use the device identifier to find
*      the static configuration structure defined in xospipsv_g.c.
*
*    - XOspiPsv_CfgInitialize(InstancePtr, ConfigPtr) - Uses a
*      configuration structure provided by the caller. This will initialize
*      the XOspiPsv driver structure.
*
* <b>Supported features</b>
*
* This driver supports following features
*    - SDR-NON-PHY mode.
*    - SDR-PHY mode.
*    - DDR-PHY mode.
*    - INDAC mode of operations.
*    - DAC mode of operations.
*    - Polled and Interrupt mode transfers.
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
* 1.2   sk   02/03/20 Added APIs for non-blocking transfer support.
*       sk   02/20/20 Reorganize the source code, enable the interrupts
*                     by default and updated XOspiPsv_DeviceReset() API with
*                     masked data writes.
*       sk   02/20/20 Make XOspiPsv_SetDllDelay() API as user API.
*       sk   02/20/20 Added support for DLL Master mode.
* 1.3   sk   04/09/20 Added support for 64-bit address read from 32-bit proc.
*       sk   05/27/20 Added support for reading C_OSPI_MODE param.
*       sk  08/19/20 Reduced the usleep delay while checking transfer done.
*       sk   10/06/20 Clear the ISR for polled mode transfers.
* 1.4   sk   02/18/21 Added support for Dual byte opcode.
*       sk   02/18/21 Updated RX Tuning algorithm for Master DLL mode.
*       sk   04/08/21 Fixed doxygen warnings in all source files.
*       sk   05/07/21 Fixed MISRAC violations.
* 1.5   sk   08/17/21 Added DCache invalidate after non-blocking DMA read.
*       sk   08/30/21 Limit RX maximum number of taps to 127.
* 1.6   sk   11/29/21 Configure OSPI MUX while setting the DMA mode.
*            11/29/21 Fix compilation warnings reported with "-Wundef" flag.
*       sk   02/07/22 Replaced driver version in addtogroup with Overview.
*       sk   02/07/22 Added driver details to Overview section.
*       sk   02/07/22 Restructured the XOspiPsv_ExecuteRxTuning() API to meet
*                     safety guidelines for CCM metric.
*
* </pre>
*
******************************************************************************/
#ifndef XOSPIPSV_H_		/**< prevent circular inclusions */
#define XOSPIPSV_H_		/**< by using protection macros */

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
	u64 RxAddr64bit; /**< Provide 64-bit read address for 32-bit platform */
	u8 Xfer64bit; /**< Set to 1 when reading from 64-bit addr otherwise 0 */
	u8 ExtendedOpcode; /**< Extended opcode in dual-byte opcode mode */
} XOspiPsv_Msg;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
	u16 DeviceId;		/**< Unique ID  of device */
	UINTPTR BaseAddress;	/**< Base address of the device */
	u32 InputClockHz;	/**< Input clock frequency */
	u8 IsCacheCoherent;		/**< If OSPI is Cache Coherent or not */
	u8 ConnectionMode;	/**< OSPI connection mode */
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
	u8 ChipSelect;	/**< Chip select information */
	XOspiPsv_Msg *Msg;	/**< Pointer to the Flash Message structure */
	XOspiPsv_StatusHandler StatusHandler;	/**< Status/Callback handler */
	void *StatusRef;  	 /**< Callback reference for status handler */
	u8 IsUnaligned;		/**< Flag used to indicate bytecnt is aligned or not */
	u32 DeviceIdData;	/**< Contains Device Id Data information */
	u8 Extra_DummyCycle;	/**< Contains extra dummy cycle data */
	u8 DllMode;		/**< DLL mode */
	u8 DualByteOpcodeEn;	/**< Flag to indicate Dual Byte Opcode */
#ifdef __ICCARM__
#pragma pack(push, 8)
	u8 UnalignReadBuffer[4];	/**< Buffer used to read the unaligned bytes in DMA */
#pragma pack(pop)
#else
	/**< Buffer used to read the unaligned bytes in DMA */
	u8 UnalignReadBuffer[4] __attribute__ ((aligned(64))); /**< Read Buffer */
#endif
} XOspiPsv;

/************************** Variable Definitions *****************************/
extern XOspiPsv_Config XOspiPsv_ConfigTable[];

/***************** Macros (Inline Functions) Definitions *********************/
/**
 * @name Configuration options
 * @{
 */
/**
 * User configuration options.
 */
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
/** @} */

/**
 * @name Low-level memory read/write
 * @{
 */
/**
 * Low level macros to read/write memory.
 */
#define XOspiPsv_ReadReg(BaseAddress, RegOffset) Xil_In32((BaseAddress) + (u32)(RegOffset))
#define XOspiPsv_WriteReg(BaseAddress, RegOffset, RegisterValue) \
		Xil_Out32((BaseAddress) + (u32)(RegOffset), (u32)(RegisterValue))
/** @} */

/**
 * @name Modes of operation selection
 * @{
 */
/**
 * Macros to select INDAC and DAC modes.
 */
#define XOSPIPSV_IDAC_MODE		0x0U
#define XOSPIPSV_DAC_MODE		0x1U
/** @} */

/**
 * @name Transfer type selection
 * @{
 */
/**
 * Macros to select RX and TX transfer flags.
 */
#define XOSPIPSV_MSG_FLAG_RX	0x2U
#define XOSPIPSV_MSG_FLAG_TX	0x4U
/** @} */

/**
 * @name Prototype(Bus width: Cmd_Addr_Data) selection
 * @{
 */
/**
 * Macros to select Read and Write prototype.
 */
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
/** @} */

/**
 * @name Chip Select selection
 * @{
 */
/**
 * Macros to select CS0 and CS1.
 */
#define XOSPIPSV_SELECT_FLASH_CS0	0
#define XOSPIPSV_SELECT_FLASH_CS1	1
/** @} */

/**
 * @name Prescaler selection
 * @{
 */
/**
 * Macros to select different baud rate divisors.
 */
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
/** @} */

/**
 * @name Default configuration selection
 * @{
 */
/**
 * Macros to select default OSPI configuration.
 */
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
#define XOSPIPSV_REMAP_ADDR_VAL		0x40000000U
#define XOSPIPSV_NON_PHY_RD_DLY		0x1U
/** @} */

/**
 * @name Edge mode selection
 * @{
 */
/**
 * Macros to select different edge modes like
 * SDR+NON-PHY, SDR+PHY and DDR+PHY.
 */
#define XOSPIPSV_EDGE_MODE_SDR_PHY			0x0U
#define XOSPIPSV_EDGE_MODE_SDR_NON_PHY		0x1U
#define XOSPIPSV_EDGE_MODE_DDR_PHY			0x2U
/** @} */

/**
 * @name DLL delay selection
 * @{
 */
/**
 * Macros to select TX DLL delays.
 */
#define XOSPIPSV_SDR_TX_VAL			0x5U
#define XOSPIPSV_DDR_TX_VAL			0x0U
#define XOSPIPSV_DDR_TX_VAL_MASTER		0x1EU
#define XOSPIPSV_SDR_TX_VAL_MASTER		0x3CU
#define XOSPIPSV_DLL_MAX_TAPS			0x7FU
/** @} */

/**
 * @name OSPI Device reset selection
 * @{
 */
/**
 * Macros to select HWPIN and INBAND resets.
 */
#define XOSPIPSV_HWPIN_RESET	0x0U
#define XOSPIPSV_INBAND_RESET	0x1U
/** @} */

/**
 * @name DLL mode selection
 * @{
 */
/**
 * Macros to select DLL BYPASS and MASTER modes.
 */
#define XOSPIPSV_DLL_BYPASS_MODE	0x0U
#define XOSPIPSV_DLL_MASTER_MODE	0x1U
/** @} */

/**
 * @name Connection mode selection
 * @{
 */
/**
 * Macros to select SINGLE and STACKED connection modes.
 */
#define XOSPIPSV_CONNECTION_MODE_SINGLE		0x0U
#define XOSPIPSV_CONNECTION_MODE_STACKED	0x1U
/** @} */

/**
 * @name Dual Byte opcode selection
 * @{
 */
/**
 * Macros to enable/disable Dual Byte opcode.
 */
#define XOSPIPSV_DUAL_BYTE_OP_DISABLE	0x0U
#define XOSPIPSV_DUAL_BYTE_OP_ENABLE	0x1U
/** @} */

/**< Macro used for more than 32-bit address */
#define XOSPIPSV_RXADDR_OVER_32BIT	0x100000000U

/* Initialization and reset */
XOspiPsv_Config *XOspiPsv_LookupConfig(u16 DeviceId);
u32 XOspiPsv_CfgInitialize(XOspiPsv *InstancePtr, const XOspiPsv_Config *ConfigPtr);
void XOspiPsv_Reset(XOspiPsv *InstancePtr);
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
void XOspiPsv_ConfigureAutoPolling(const XOspiPsv *InstancePtr, u32 FlashMode);
void XOspiPsv_Idle(const XOspiPsv *InstancePtr);
u32 XOspiPsv_DeviceReset(u8 Type);
u32 XOspiPsv_StartDmaTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_CheckDmaDone(XOspiPsv *InstancePtr);
u32 XOspiPsv_SetDllDelay(XOspiPsv *InstancePtr);
u32 XOspiPsv_ConfigDualByteOpcode(XOspiPsv *InstancePtr, u8 Enable);
#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_H_ */
/** @} */
