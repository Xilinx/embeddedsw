/******************************************************************************
* Copyright (C) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef XOSPIPSV_H_
#define XOSPIPSV_H_

/*****************************************************************************/
/**
*
* @file xospipsv.h
* @addtogroup ospipsv Overview
* @{
* @brief Contains the OSPIPSV driver types, macros and function prototypes.
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
* 1.8   sk   11/11/22 Enable Master DLL mode by default for Versal Net.
*       sk   11/29/22 Added support for Indirect Non-Dma write.
* 1.9   sb   06/06/23 Added support for system device-tree flow.
* 1.10	akm  01/31/24 Use OSPI controller reset for resetting flash device.
* 1.10	akm  02/06/24 Increase the delay after device reset.
* 1.10	akm  02/01/24 Update OSPI TX tap delay element to 0x26 for DDR mode.
* 1.11	akm  05/15/24 Added support for x2/x4 operations.
* 1.11  ng  08/20/24 Add spartanup device support
* 1.13  sb  06/26/25 Set the required configurations for SpartanUp in
*                    SDR-PHY & DDR-PHY mode.
* 1.14  vlt 12/15/25 Update Doxygen comments to include SDT flow details.
* 	    abh 11/10/25 Fixed MISRA-C violations
* 1.15  sb  08/11/26 Add support for pattern-based RX PHY tuning.
* 1.15  tb  09/30/26 Align Doxygen comments with AMD documentation standards
*
* </pre>
*
******************************************************************************/

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
 * @brief Specifies the status callback handler type for the OSPIPSV device.
 *
 * The handler data type allows the user to define a callback function to
 * handle the asynchronous processing for the OSPIPSV device. The application
 * using this driver is expected to define a handler of this type to support
 * interrupt driven mode. The handler executes in an interrupt context, so
 * only minimal processing should be performed.
 *
 * @param[in]	CallBackRef Callback reference passed in by the upper
 *		layer when setting the callback functions, and passed back to
 *		the upper layer when the callback is invoked. Its type is
 *		not important to the driver, so it is a void pointer.
 * @param[in] 	StatusEvent Holds one or more status events that have occurred.
 *		See the XOspiPsv_SetStatusHandler() for details on the status
 *		events that can be passed in the callback.
 */
typedef void (*XOspiPsv_StatusHandler) (void *CallBackRef, u32 StatusEvent);

/**
 * @brief Holds the configuration information for a flash message.
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
 * @brief Holds the RX tuning related information.
 *
 * The RX tuning algorithm compares the data read at every RX tap against a
 * reference pattern which is expected to be pre-programmed in the flash. The
 * application owns the pattern and fills this structure in the driver instance
 * before switching to a PHY mode using XOspiPsv_SetSdrDdrMode(). It supplies
 * the reference data, the location of the pattern in flash, the read
 * instruction used to fetch it and the buffer it is read into.
 */
typedef struct {
	u8 Opcode;	/**< Read opcode used to fetch the pattern */
	u8 Dummy;	/**< Number of dummy cycles for the read opcode */
	u8 Proto;	/**< Indicate number of Cmd-Addr-Data lines */
	u8 Addrsize;	/**< Size of the pattern address in bytes */
	u32 PatternAddr;	/**< Flash address the pattern is programmed at */
	u32 PatternSize;	/**< Number of pattern bytes to read and compare,
			* has to be a multiple of four */
	const u8 *Pattern;	/**< Reference pattern used for comparison */
	u8 *ReadBfrPtr;	/**< Application buffer the pattern is read into.
			* It is filled using DMA, so it has to be cache line
			* aligned and at least PatternSize bytes long */
} XOspiPsv_RxTuning;

/**
 * @brief Holds the configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID  of device */
#else
	char *Name;             /**< Name of the device */
#endif
	UINTPTR BaseAddress;	/**< Base address of the device */
	u32 InputClockHz;	/**< Input clock frequency */
	u8 IsCacheCoherent;		/**< If OSPI is Cache Coherent or not */
	u8 ConnectionMode;	/**< OSPI connection mode */
#ifdef SDT
	u16 IntrId;             /**< Bits[11:0] Interrupt-id Bits[15:12]
	                        * trigger type and level flags */
	UINTPTR IntrParent;     /**< Bit[0] Interrupt parent type Bit[64/32:1]
	                        * Parent base address */
#endif
	u8  BusWidth;		/**< Bus width available on board */
} XOspiPsv_Config;

/**
 * @brief Holds the XOspiPsv driver instance data.
 *
 * The user is required to allocate a
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
	u8 DualByteOpcodeEn;	/**< Flag to indicate Dual Byte Opcode
				* 0 - DualByte opcode is disabled
				* 1 - DualByte opcode is enabled with inverted opcode
				* 2 - DualByte opcode is enabled with same opcode */
#ifdef __ICCARM__
#pragma pack(push, 8)
	u8 UnalignReadBuffer[4];	/**< Buffer used to read the unaligned bytes in DMA */
#pragma pack(pop)
#else
	/** Buffer used to read the unaligned bytes in DMA */
	u8 UnalignReadBuffer[4] __attribute__ ((aligned(64))); /**< Read Buffer */
#endif
	XOspiPsv_RxTuning RxTuning;	/**< RX tuning configuration */
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
#define XOSPIPSV_IDAC_EN_OPTION	0x2U	/**< INDAC (DMA) mode enable option. */
#define XOSPIPSV_STIG_EN_OPTION	0x4U	/**< STIG mode enable option. */
#define XOSPIPSV_CLK_POL_OPTION	0x8U	/**< Clock polarity option. */
#define XOSPIPSV_CLK_PHASE_OPTION	0x10U	/**< Clock phase option. */
#define XOSPIPSV_PHY_EN_OPTION	0x20U	/**< PHY mode enable option. */
#define XOSPIPSV_LEGIP_EN_OPTION	0x40U	/**< Legacy IP mode option. */
#define XOSPIPSV_DTR_EN_OPTION	0x80U	/**< DTR protocol enable option. */
#define XOSPIPSV_CRC_EN_OPTION	0x100U	/**< CRC enable option. */
#define XOSPIPSV_DB_OP_EN_OPTION	0x200U	/**< Dual-byte opcode option. */
/** @} */

/**
 * @name Low-level memory read/write
 * @{
 */
/****************************************************************************/
/**
*
* @brief Reads the given OSPIPSV register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	RegOffset Offset of the register.
*
* @return	The 32-bit value of the register.
*
* @note
* C-style signature
*	u32 XOspiPsv_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XOspiPsv_ReadReg(BaseAddress, RegOffset) Xil_In32((BaseAddress) + (u32)(RegOffset))

/****************************************************************************/
/**
*
* @brief Writes a value to the given OSPIPSV register.
*
* @param[in]	BaseAddress Base address of the device.
* @param[in]	RegOffset Offset of the register.
* @param[in]	RegisterValue 32-bit value to write to the register.
*
* @note
* C-style signature
*	void XOspiPsv_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
*		u32 RegisterValue)
*
******************************************************************************/
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
#define XOSPIPSV_DAC_MODE		0x1U	/**< Direct access mode. */
/** @} */

/**
 * @name Transfer type selection
 * @{
 */
/**
 * Macros to select RX and TX transfer flags.
 */
#define XOSPIPSV_MSG_FLAG_RX	0x2U
#define XOSPIPSV_MSG_FLAG_TX	0x4U	/**< TX transfer flag. */
/** @} */

/**
 * @name Prototype(Bus width: Cmd_Addr_Data) selection
 * @{
 */
/**
 * Macros to select Read and Write prototype.
 */
#define XOSPIPSV_READ_1_1_1	0U
#define XOSPIPSV_READ_1_0_1	1U	/**< 1-0-1 read protocol. */
#define XOSPIPSV_READ_1_1_2	2U	/**< 1-1-2 read protocol. */
#define XOSPIPSV_READ_1_1_8	3U	/**< 1-1-8 read protocol. */
#define XOSPIPSV_READ_1_8_8	4U	/**< 1-8-8 read protocol. */
#define XOSPIPSV_READ_8_8_8	5U	/**< 8-8-8 read protocol. */
#define XOSPIPSV_READ_8_0_0	6U	/**< 8-0-0 read protocol. */
#define XOSPIPSV_READ_8_8_0	7U	/**< 8-8-0 read protocol. */
#define XOSPIPSV_READ_8_0_8	8U	/**< 8-0-8 read protocol. */
#define XOSPIPSV_READ_1_1_4	9U	/**< 1-1-4 read protocol. */
#define XOSPIPSV_READ_4_4_4	10U	/**< 4-4-4 read protocol. */

#define XOSPIPSV_WRITE_1_1_1	0U	/**< 1-1-1 write protocol. */
#define XOSPIPSV_WRITE_1_0_1	1U	/**< 1-0-1 write protocol. */
#define XOSPIPSV_WRITE_1_1_2	2U	/**< 1-1-2 write protocol. */
#define XOSPIPSV_WRITE_1_1_8	3U	/**< 1-1-8 write protocol. */
#define XOSPIPSV_WRITE_1_8_8	4U	/**< 1-8-8 write protocol. */
#define XOSPIPSV_WRITE_8_8_8	5U	/**< 8-8-8 write protocol. */
#define XOSPIPSV_WRITE_8_0_0	6U	/**< 8-0-0 write protocol. */
#define XOSPIPSV_WRITE_8_8_0	7U	/**< 8-8-0 write protocol. */
#define XOSPIPSV_WRITE_8_0_8	8U	/**< 8-0-8 write protocol. */
#define XOSPIPSV_WRITE_1_1_4	9U	/**< 1-1-4 write protocol. */
#define XOSPIPSV_WRITE_4_4_4	10U	/**< 4-4-4 write protocol. */
/** @} */

/**
 * @name Chip Select selection
 * @{
 */
/**
 * Macros to select CS0 and CS1.
 */
#define XOSPIPSV_SELECT_FLASH_CS0	0U
#define XOSPIPSV_SELECT_FLASH_CS1	1U	/**< Select flash on CS1. */
/** @} */

/**
 * @name Prescaler selection
 * @{
 */
/**
 * Macros to select different baud rate divisors.
 */
#define XOSPIPSV_CLK_PRESCALE_2		0U
#define XOSPIPSV_CLK_PRESCALE_4		1U	/**< Baud rate divisor 4. */
#define XOSPIPSV_CLK_PRESCALE_6		2U	/**< Baud rate divisor 6. */
#define XOSPIPSV_CLK_PRESCALE_8		3U	/**< Baud rate divisor 8. */
#define XOSPIPSV_CLK_PRESCALE_10	4U	/**< Baud rate divisor 10. */
#define XOSPIPSV_CLK_PRESCALE_12	5U	/**< Baud rate divisor 12. */
#define XOSPIPSV_CLK_PRESCALE_14	6U	/**< Baud rate divisor 14. */
#define XOSPIPSV_CLK_PRESCALE_16	7U	/**< Baud rate divisor 16. */
#define XOSPIPSV_CLK_PRESCALE_18	8U	/**< Baud rate divisor 18. */
#define XOSPIPSV_CLK_PRESCALE_20	9U	/**< Baud rate divisor 20. */
#define XOSPIPSV_CLK_PRESCALE_22	10U	/**< Baud rate divisor 22. */
#define XOSPIPSV_CLK_PRESCALE_24	11U	/**< Baud rate divisor 24. */
#define XOSPIPSV_CLK_PRESCALE_26	12U	/**< Baud rate divisor 26. */
#define XOSPIPSV_CLK_PRESCALE_28	13U	/**< Baud rate divisor 28. */
#define XOSPIPSV_CLK_PRESCALE_30	14U	/**< Baud rate divisor 30. */
#define XOSPIPSV_CLK_PRESCALE_32	15U	/**< Baud rate divisor 32. */
#define XOSPIPSV_CR_PRESC_MAXIMUM	15U	/**< Maximum prescaler value. */
/** @} */

/**
 * @name Default configuration selection
 * @{
 */
/**
 * Macros to select default OSPI configuration.
 */
#define XOSPIPSV_NO_SLAVE_SELCT_VALUE	0xFU
/** DAC disable value used in the initial configuration. */
#define XOSPIPSV_DISABLE_DAC_VALUE		0x0U
/** SPI disable value used in the initial configuration. */
#define XOSPIPSV_SPI_DISABLE_VALUE		0x0U
/** Initial value of the configuration register. */
#define XOSPIPSV_CONFIG_INIT_VALUE		(((u32)XOSPIPSV_CLK_PRESCALE_2 << \
		(u32)XOSPIPSV_CONFIG_REG_MSTR_BAUD_DIV_FLD_SHIFT) | \
		((u32)XOSPIPSV_NO_SLAVE_SELCT_VALUE << \
		 (u32)XOSPIPSV_CONFIG_REG_PERIPH_CS_LINES_FLD_SHIFT) | \
		((u32)XOSPIPSV_DISABLE_DAC_VALUE << \
		 (u32)XOSPIPSV_CONFIG_REG_ENB_DIR_ACC_CTLR_FLD_SHIFT) | \
		(u32)XOSPIPSV_SPI_DISABLE_VALUE)
/** Write completion poll count field value in PHY mode. */
#define XOSPIPSV_POLL_CNT_FLD_PHY	0x3U
/** Write completion poll count field value in non-PHY mode. */
#define XOSPIPSV_POLL_CNT_FLD_NON_PHY	0x1U
/** Minimum PHY mode frequency in Hz. */
#define XOSPIPSV_MIN_PHY_FREQ	50000000
/** Additional status polling dummy cycles in DDR mode. */
#define XOSPIPSV_DDR_STATS_REG_DUMMY	0x8U
/** Remap address register value used in DAC mode. */
#define XOSPIPSV_REMAP_ADDR_VAL		0x40000000U
/** Read data capture delay in SDR non-PHY mode. */
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
/** SDR non-PHY edge mode. */
#define XOSPIPSV_EDGE_MODE_SDR_NON_PHY		0x1U
/** DDR PHY edge mode. */
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
#define XOSPIPSV_DDR_TX_VAL			0x0U	/**< DDR TX delay. */
#if defined (VERSAL_NET)
/** TX DLL delay in DDR mode with DLL master mode. */
#define XOSPIPSV_DDR_TX_VAL_MASTER		0x26U
#else
/** TX DLL delay in DDR mode with DLL master mode. */
#define XOSPIPSV_DDR_TX_VAL_MASTER		0x1EU
#endif
#if defined (SPARTANUP)
/** TX DLL delay in SDR mode with DLL master mode. */
#define XOSPIPSV_SDR_TX_VAL_MASTER		0x40U
#else
/** TX DLL delay in SDR mode with DLL master mode. */
#define XOSPIPSV_SDR_TX_VAL_MASTER		0x3CU
#endif
/** Maximum number of DLL taps. */
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
#define XOSPIPSV_INBAND_RESET	0x1U	/**< Inband reset. */
/** @} */

/**
 * @name DLL mode selection
 * @{
 */
/**
 * Macros to select DLL BYPASS and MASTER modes.
 */
#define XOSPIPSV_DLL_BYPASS_MODE	0x0U
#define XOSPIPSV_DLL_MASTER_MODE	0x1U	/**< DLL master mode. */
/** @} */

/**
 * @name Connection mode selection
 * @{
 */
/**
 * Macros to select SINGLE and STACKED connection modes.
 */
#define XOSPIPSV_CONNECTION_MODE_SINGLE		0x0U
#define XOSPIPSV_CONNECTION_MODE_STACKED	0x1U	/**< Stacked mode. */
/** @} */

/**
 * @name Dual Byte opcode selection
 * @{
 */
/**
 * Macros to enable/disable Dual Byte opcode.
 */
#define XOSPIPSV_DUAL_BYTE_OP_DISABLE	0x0U
/** Dual byte opcode enabled with inverted opcode. */
#define XOSPIPSV_DUAL_BYTE_OP_ENABLE	0x1U
/** Dual byte opcode enabled with same opcode. */
#define XOSPIPSV_DUAL_BYTE_OP_SAME	0x2U
/** @} */

/** Macro used for more than 32-bit address */
#define XOSPIPSV_RXADDR_OVER_32BIT	0x100000000U

/**
 * Minimum frequency for Tap grain selection.
 */
#define XOSPIPSV_TAP_GRAN_SEL_MIN_FREQ	120000000U

/* Initialization and reset */
#ifndef SDT
XOspiPsv_Config *XOspiPsv_LookupConfig(u16 DeviceId);
#else
XOspiPsv_Config *XOspiPsv_LookupConfig(UINTPTR BaseAddress);
#endif
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
u32 XOspiPsv_DeviceResetViaOspi(const XOspiPsv *InstancePtr, u8 Type);
u32 XOspiPsv_StartDmaTransfer(XOspiPsv *InstancePtr, XOspiPsv_Msg *Msg);
u32 XOspiPsv_CheckDmaDone(XOspiPsv *InstancePtr);
u32 XOspiPsv_SetDllDelay(XOspiPsv *InstancePtr);
u32 XOspiPsv_ConfigDualByteOpcode(XOspiPsv *InstancePtr, u8 Enable);
#ifdef __cplusplus
}
#endif

#endif /* XOSPIPSV_H_ */
/** @} */
