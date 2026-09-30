/******************************************************************************
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#ifndef AMD_XSPI_H
#define AMD_XSPI_H

/*****************************************************************************/
/**
*
* @file amd_xspi.h
* @addtogroup amd_xspi Overview
* @{
* @brief Defines the software API for the AXI XSPI controller driver.
*
* @details
*
* This file contains the software API definition for the AXI XSPI controller
* driver. The AXI XSPI is a Serial Peripheral Interface (SPI) controller
* that operates in AXI-Lite mode for register-based data transfers.
*
* <b>Features</b>
* - AXI-Lite mode operation (register-based transfers)
* - Multiple operating modes:
*   - Mode 0: Standard SPI half-duplex (single data line)
*   - Mode 1: Standard SPI (io0 = MOSI, io1 = MISO)
*   - Mode 2: Dual SPI (2 data lines)
*   - Mode 3: Quad SPI (4 data lines)
*   - Mode 4: xSPI Profile 1.0 (Octal)
*   - Mode 5: xSPI Profile 2.0 (HyperBus)
* - Configurable SPI clock modes (CPOL/CPHA: 0, 1, 2, 3). Modes 4 and 5
*   support clock mode 0 only, and AmdXspi_SetClkMode() rejects the rest.
* - Multiple slave select support (up to 8 slaves)
* - TX and RX FIFOs with occupancy monitoring
* - FIFO ECC errors reported through the error interrupt
* - Interrupt-driven or polled operation
*
* <b>Initialization</b>
* The driver is initialized using two-step initialization:
* 1. AmdXspi_LookupConfig() to get the configuration
* 2. AmdXspi_CfgInitialize() to initialize the driver instance
*
* <b>Transfer Operations</b>
* This driver supports AXI-Lite mode transfers only. The message based API is
* the primary interface. A message describes one command/address phase and one
* data phase inside a single chip-select window. That is what an opcode based
* flash and a HyperBus device both need.
* - AmdXspi_PolledTransfer() to run a message queue to completion
* - AmdXspi_InterruptTransfer() to run one in the background
*
* A message always carries a command/address phase, which the instruction
* field cannot encode as zero bytes. For a device that needs none, such as a
* plain SPI part in modes 0 to 3, AmdXspi_Transfer() moves raw bytes instead.
* Pass NULL for either buffer to make the transfer one-directional.
*
* <b>Interrupts</b>
* Interrupts are used by AmdXspi_InterruptTransfer() only; the byte oriented
* interface is polled. A single callback reports the outcome:
* - AmdXspi_SetStatusHandler() to register it
* - AmdXspi_InterruptHandler() as the main ISR
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   akm  07/29/26 Initial release
* 1.0   vn   09/30/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "amd_xspi_hw.h"
#include "xstatus.h"
#include "xil_assert.h"

/************************** Constant Definitions *****************************/

/** @name Message Flags for AmdXspi_Msg (message-based transfer API)
 * @{
 */
#define AMD_XSPI_MSG_FLAG_TX		0x1U /**< Message has transmit data. */
#define AMD_XSPI_MSG_FLAG_RX		0x2U /**< Message has receive data. */
/** @} */

/**************************** Type Definitions *******************************/

/**
*
* @brief Defines the callback function type for interrupt handlers.
*
* @param[in]   CallBackRef Callback reference passed during handler setup.
* @param[in]   StatusEvent Status/event mask that triggered the callback.
* @param[in]   ByteCount Number of bytes transferred (if applicable).
*
 */
typedef void (*AmdXspi_StatusHandler)(void *CallBackRef, u32 StatusEvent,
				       u32 ByteCount);

/**
 * @brief Stores the configuration of an AXI XSPI device.
 *
 * Fields are populated from the system device tree by the BSP generator.
 */
typedef struct {
	char *Name;		/**< Compatible string of the device. */
	UINTPTR BaseAddress;	/**< Base address of the device. */
	u8 SpiMode;		/**< SPI Mode (C_SPI_MODE: 0-5). */
	u32 FifoDepth;		/**< Depth of TX/RX FIFOs (C_FIFO_DEPTH). */
	u8 NumSlaves;		/**< Number of slave devices supported
				 * (C_NUM_SS_BITS). */
	u32 SpiClkFreq;		/**< External SPI Clock Frequency in MHz
				 * (C_EXT_SPI_CLK_FREQ). */
	/*
	 * SpiClkFreq is the clock into the core, not the bus clock. SCK is
	 * SpiClkFreq divided by SckRatio; use AmdXspi_GetSckFreqMHz() rather
	 * than dividing at each call site.
	 */
	u32 SckRatio;		/**< SPI clock division ratio (C_SCK_RATIO). */
	/*
	 * C_SPI_MEMORY, C_TGT_MEM_SIZE, C_TGT_LATENCY and C_TGT_BURST_LENGTH
	 * are deliberately absent. A Lite mode design leaves them at defaults
	 * that need not describe the part fitted to it, so memory type,
	 * geometry, latency and burst length are discovered from the device at
	 * runtime instead.
	 */
	u32 IntrId;		/**< Interrupt ID. */
	UINTPTR IntrParent;	/**< Interrupt parent. */
} AmdXspi_Config;

/**
 * @brief Describes a transfer message for the message-based transfer API.
 *
 * The message-based transfer API is AmdXspi_PolledTransfer() and
 * AmdXspi_InterruptTransfer().
 *
 * Each message describes one Lite-mode transaction: command/address followed
 * by a single data phase (transmit OR receive) within one chip-select window.
 */
typedef struct {
	u8 *CmdAddrBufPtr;	/**< Command/address bytes (for example,
				 * HyperBus CA). */
	u32 CmdAddrBytes;	/**< Number of command/address bytes (1-6). */
	u8 *TxBfrPtr;		/**< Transmit data buffer (NULL if none). */
	u8 *RxBfrPtr;		/**< Receive data buffer (NULL if none). */
	u32 ByteCount;		/**< Number of data bytes to transfer. */
	u8 DummyCycles;		/**< Dummy/latency cycles (0-63). */
	u8 FrameFormat;		/**< Frame format (0-3). */
	u8 IoFormat;		/**< IO format (for Dual/Quad modes). */
	u32 Flags;		/**< AMD_XSPI_MSG_FLAG_TX /
				 * AMD_XSPI_MSG_FLAG_RX. */
} AmdXspi_Msg;

/**
 * @brief Stores the AXI XSPI driver instance data.
 */
typedef struct {
	AmdXspi_Config Config;		/**< Configuration data. */
	u32 IsReady;			/**< Device is initialized and ready. */

	/* Message-based transfer API state */
	AmdXspi_Msg *Msg;		/**< Active message queue (NULL if
					 * none). */
	u32 NumMsg;			/**< Number of messages in the queue. */
	u32 MsgCnt;			/**< Index of message being
					 * processed. */
	u32 MsgRxCnt;			/**< Bytes drained so far for this
					 * message. */
	u32 MsgTxCnt;			/**< Bytes pushed so far for this
					 * message. */
	volatile u32 IsBusy;		/**< A message-based transfer is
					 * active. */
	AmdXspi_StatusHandler StatusHandler;
					/**< Status callback. */
	void *StatusRef;		/**< Status callback reference. */
} AmdXspi;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
*
* @brief Gets the SPI bus clock (SCK) frequency in MHz.
*
* Config.SpiClkFreq is the clock supplied to the core, which is not what
* appears on SCK: the core divides it by C_SCK_RATIO. Use this rather than
* Config.SpiClkFreq wherever bus timing matters, such as working out how long
* chip select stays low against a device limit.
*
* Returns 0 if the divider is unavailable, which is the case on a BSP
* generated before this parameter was described in the driver metadata.
* A 0 means unknown, not zero-frequency, so treat it as "cannot compute".
*
* @param[in]   InstancePtr Pointer to the AmdXspi instance.
*
* @return      SCK frequency in MHz, or 0 if it cannot be determined.
*
* @note        C-style signature:
*              u32 AmdXspi_GetSckFreqMHz(AmdXspi *InstancePtr)
*
******************************************************************************/
#define AmdXspi_GetSckFreqMHz(InstancePtr) \
	(((InstancePtr)->Config.SckRatio == 0U) ? 0U : \
	 ((InstancePtr)->Config.SpiClkFreq / (InstancePtr)->Config.SckRatio))

/************************** Function Prototypes ******************************/

/* Initialization functions (amd_xspi.c) */
s32 AmdXspi_CfgInitialize(AmdXspi *InstancePtr, AmdXspi_Config *ConfigPtr,
			   UINTPTR EffectiveAddr);

/* Configuration functions (amd_xspi_options.c) */
s32 AmdXspi_SetClkMode(AmdXspi *InstancePtr, u8 ClkMode);
s32 AmdXspi_SetSlaveSelect(AmdXspi *InstancePtr, u8 SlaveNum);

/* Message-based transfer API (amd_xspi.c) */
s32 AmdXspi_PolledTransfer(AmdXspi *InstancePtr, AmdXspi_Msg *Msg, u32 NumMsg);
s32 AmdXspi_InterruptTransfer(AmdXspi *InstancePtr, AmdXspi_Msg *Msg, u32 NumMsg);
void AmdXspi_Abort(AmdXspi *InstancePtr);

/* Register level functions (amd_xspi_hw.c) */
s32 AmdXspi_Transfer(AmdXspi *InstancePtr, u8 *SendBufPtr, u8 *RecvBufPtr,
		      u32 ByteCount);

/* Lookup function (amd_xspi_sinit.c) */
AmdXspi_Config *AmdXspi_LookupConfig(UINTPTR BaseAddress);

/* Interrupt functions (amd_xspi.c) */
void AmdXspi_InterruptHandler(void *InstancePtr);
void AmdXspi_SetStatusHandler(AmdXspi *InstancePtr, void *CallBackRef,
			    AmdXspi_StatusHandler FuncPtr);

/* Interrupt register functions (amd_xspi_hw.c) */
void AmdXspi_IntrEnable(AmdXspi *InstancePtr, u32 IntrMask);
void AmdXspi_IntrDisable(AmdXspi *InstancePtr, u32 IntrMask);

/************************** Variable Definitions *****************************/

/**
 * @brief Holds the configuration entry for each AXI XSPI instance.
 */
extern AmdXspi_Config AmdXspi_ConfigTable[];

#ifdef __cplusplus
}
#endif


/** @} */
#endif /* AMD_XSPI_H */
