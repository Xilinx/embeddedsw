/******************************************************************************
* Copyright (C) 2014 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xqspipsu.h
 * @addtogroup qspipsu Overview
 * @{
 * @details
 *
 * This section explains the implementation the functions required to use the
 * QSPIPSU hardware to perform a transfer. These are accessible to the user
 * via xqspipsu.h.
 *
 * Generic QSPI interface allows for communication to any QSPI slave device.
 * GQSPI contains a GENFIFO into which the bus transfers required are to be
 * pushed with appropriate configuration. The controller provides TX and RX
 * FIFO's and a DMA to be used for RX transfers. The controller executes each
 * GENFIFO entry noting the configuration and places data on the bus as required
 *
 * The different options in GENFIFO are as follows:
 * - IMM_DATA : Can be one byte of data to be transmitted, number of clocks or
 *              number of bytes in transfer.
 * - DATA_XFER : Indicates that data/clocks need to be transmitted or received.
 * - EXPONENT : e when 2^e bytes are involved in transfer.
 * - SPI_MODE : SPI/Dual SPI/Quad SPI
 * - CS : Lower or Upper CS or Both
 * - Bus : Lower or Upper Bus or Both
 * - TX : When selected, controller transmits data in IMM or fetches number of
 *        bytes mentioned form TX FIFO. If not selected, dummies are pumped.
 * - RX : When selected, controller receives and fills the RX FIFO/allows RX DMA
 *        of requested number of bytes. If not selected, RX data is discarded.
 * - Stripe : Byte stripe over lower and upper bus or not.
 * - Poll : Polls response to match for to a set value (used along with POLL_CFG
 *          registers) and then proceeds to next GENFIFO entry.
 *          This feature is not currently used in the driver.
 *
 * GENFIFO has manual and auto start options.
 * All DMA requests need a 4-byte aligned destination address buffer and
 * size of transfer should also be a multiple of 4.
 * This driver supports DMA RX and IO RX.
 *
 * <b>Initialization & Configuration</b>
 *
 * This driver uses the GQSPI controller with RX DMA. It supports both
 * interrupt and polled transfers. Manual start of GENFIFO is used.
 * XQspiPsu_CfgInitialize() initializes the instance variables.
 * Additional setting can be done using SetOptions/ClearOptions functions
 * and SelectSlave function.
 *
 * <b>Transfer</b>
 *
 * Polled or Interrupt transfers can be done. The transfer function needs the
 * message(s) to be transmitted in the form of an array of type XQspiPsu_Msg.
 * This is supposed to contain the byte count and any TX/RX buffers as required.
 * Flags can be used indicate further information such as whether the message
 * should be striped. The transfer functions form and write GENFIFO entries,
 * check the status of the transfer and report back to the application
 * when done.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------.
 * 1.0   hk  08/21/14 First release
 *       sk  03/13/15 Added IO mode support.
 *       hk  03/18/15 Switch to I/O mode before clearing RX FIFO.
 *                    Clear and disable DMA interrupts/status in abort.
 *                    Use DMA DONE bit instead of BUSY as recommended.
 *       sk  04/24/15 Modified the code according to MISRAC-2012.
 *       sk  06/17/15 Removed NULL checks for Rx/Tx buffers. As
 *                    writing/reading from 0x0 location is permitted.
 * 1.1   sk  04/12/16 Added debug message prints.
 * 1.2 nsk 07/01/16 Added LQSPI support
 *                  Modified XQspiPsu_Select() macro in xqspipsu.h
 *                  Added XQspiPsu_GetLqspiConfigReg() in xqspipsu.h
 *                  Added required macros in xqspipsu_hw.h
 *                  Modified XQspiPsu_SetOptions() to support
 *                  LQSPI options and updated OptionsTable in
 *                  xqspipsu_options.c
 *       rk  07/15/16 Added support for TapDelays at different frequencies.
 *     nsk 08/05/16 Added example support PollData and PollTimeout
 *                  Added  XQSPIPSU_MSG_FLAG_POLL macro in xqspipsu.h
 *                  Added XQspiPsu_Create_PollConfigData and
 *                  XQspiPsu_PollData() functions in xqspipsu.c
 * 1.3 nsk 09/16/16 Update PollData and Polltimeout support for dual parallel
 *                  configuration. Updated XQspiPsu_PollData() and
 *                  XQspiPsu_Create_PollConfigData() functions in xqspipsu.c
 *                    and also modified the polldata example
 *       ms  03/17/17 Added readme.txt file in examples folder for doxygen
 *                    generation.
 *       ms  04/05/17 Modified Comment lines in functions of qspipsu
 *                    examples to recognize it as documentation block
 *                    and modified filename tag to include them in
 *                    doxygen examples.
 * 1.4 tjs 05/26/17 Added support for accessing upper DDR (0x800000000)
 *                  while booting images from QSPI
 * 1.5 tjs 08/08/17 Added index.html file for importing examples
 *                  from system.mss
 * 1.5 nsk 08/14/17 Added CCI support
 * 1.5 tjs 09/14/17 Modified the checks for 4 byte addressing and commands.
 * 1.6 tjs 10/16/17 Flow for accessing flash is made similar to u-boot
 *                  and linux For CR-984966
 * 1.6   tjs 11/02/17 Resolved the compilation errors for ICCARM. CR-988625
 * 1.7   tjs 11/16/17 Removed the unsupported 4 Byte write and sector erase
 *                    commands.
 * 1.7 tjs 12/01/17 Added support for MT25QL02G Flash from Micron. CR-990642
 * 1.7 tjs 12/19/17 Added support for S25FL064L from Spansion. CR-990724
 * 1.7 tjs 01/11/18 Added support for MX66L1G45G flash from Macronix CR-992367
 * 1.7 tjs 01/16/18 Removed the check for DMA MSB to be written. (CR#992560)
 * 1.7 tjs 01/17/18 Added support to toggle the WP pin of flash. (PR#2448)
 *                    Added XQspiPsu_SetWP() in xqspipsu_options.c
 *                    Added XQspiPsu_WriteProtectToggle() in xqspipsu.c and
 *                    also added write protect example.
 * 1.7 tjs 03/14/18 Added support in EL1 NS mode (CR#974882)
 * 1.7 tjs 26/03/18 In dual parallel mode enable both CS when issuing Write
 *                     enable command. CR-998478
 * 1.8 tjs 05/02/18 Added support for IS25LP064 and IS25WP064.
 * 1.8 tjs 06/26/18 Added an example for accessing 64bit dma within
 *                  32 bit application. CR#1004701
 * 1.8 tjs 06/26/18 Removed checkpatch warnings
 * 1.8 tjs 07/09/19 Fixed cppcheck, doxygen and gcc warnings.
 * 1.8 tjs 07/18/18 Setup64BRxDma() should be called only if the RxAddress is
 *                  greater than 32 bit address space. (CR#1006862)
 * 1.8 tjs 07/18/18 Added support for the low density ISSI flash parts.
 * 1.8 tjs 09/06/18 Fixed the code in XQspiPsu_GenFifoEntryData() for data
 *                  transfer length up to 255 for reducing the extra loop.
 * 1.9 tjs 11/22/17 Added the check for A72 and R5 processors (CR-987075)
 * 1.9 tjs 04/17/18 Updated register addresses as per the latest revision
 *		    of versal (CR#999610)
 * 1.9  aru 01/17/19 Fixed the violations for  MISRAC-2012
 *                  in safety mode .Done changes such as added U suffix,
 *                  Declared pointer param as const.
 * 1.9  nsk 02/01/19 Clear DMA_DST_ADDR_MSB register on 32bit machine, if the
 *		     address is of only 32bit (CR#1020031)
 * 1.9  nsk 02/01/19 Added QSPI idling support
 *
 * 1.9 akm 03/08/19 Set recommended clock and data tap delay values for 40MHZ,
 *                  100MHZ and 150MHZ frequencies(CR#1023187)
 * 1.9  nsk 03/27/19 Update 64bit dma support
 *		     (CR#1018102).
 * 1.9  akm 04/03/19 Fixed data alignment warnings on IAR compiler.
 * 1.9  akm 04/03/19 Fixed compilation error in XQspiPsu_LqspiRead()
 *                     function on IAR compiler.
 * 1.10 sk  08/20/19 Fixed issues in poll timeout feature.
 * 1.10 akm 08/22/19 Set recommended tap delay values for 37.5MHZ, 100MHZ and
 *		     150MHZ frequencies in Versal.
 * 1.10 akm 09/05/19 Added Multi Die Erase and Muti Die Read support.
 * 1.11 akm 11/07/19 Removed LQSPI register access in Versal.
 * 1.11	akm 11/15/19 Fixed Coverity deadcode warning in
 * 				XQspipsu_Calculate_Tapdelay().
 * 1.11 akm 02/19/20 Added XQspiPsu_StartDmaTransfer() and XQspiPsu_CheckDmaDone()
 * 		     APIs for non-blocking transfer.
 * 1.11 sd  01/02/20 Added clocking support
 * 1.11 akm 03/09/20 Reorganize the source code, enable qspi controller and
 *		     interrupts in XQspiPsu_CfgInitialize() API.
 * 1.11 akm 03/26/20 Fixed issue by updating XQspiPsu_CfgInitialize to return
 *		     XST_DEVICE_IS_STARTED instead of asserting, when the
 *		     instance is already configured(CR#1058525).
 * 1.12	akm 09/02/20 Updated the Makefile to support parallel make execution.
 * 1.13 akm 01/04/21 Fix MISRA-C violations.
 * 1.13 sne 04/23/21 Fixed doxygen warnings.
 * 1.14 akm 06/24/21 Allow enough time for the controller to reset the FIFOs.
 * 1.14 akm 08/12/21 Perform Dcache invalidate at the end of the DMA transfer.
 * 1.17 akm 10/31/22 Add support for Winbond flash w25q02nw.
 * 1.17 akm 12/16/22 Add timeout in QSPIPSU driver operation.
 * 1.17 akm 01/02/23 Use Xil_WaitForEvent() API for register bit polling.
 * 1.18 sb  06/07/23 Added support for system device-tree flow.
 * 1.18 sb  06/19/23 Add memory barrier instruction and convert IsBusy varible
 *                   to volatile.
 * 1.18 ht  07/18/23 Fixed GCC warnings.
 * 1.18 sb  08/01/23 Added support for Feed back clock
 *
 * </pre>
 *
 ******************************************************************************/

#ifndef XQSPIPSU_H_		/**< prevent circular inclusions */
#define XQSPIPSU_H_		/**< by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xstatus.h"
#include "xqspipsu_hw.h"
#include "xil_cache.h"
#include "xil_mem.h"
#include "xil_util.h"
#if defined  (XCLOCKING)
#include "xil_clocking.h"
#endif

/**************************** Type Definitions *******************************/
/**
 * The handler data type allows the user to define a callback function to
 * handle the asynchronous processing for the QSPIPSU device.  The application
 * using this driver is expected to define a handler of this type to support
 * interrupt driven mode.  The handler executes in an interrupt context, so
 * only minimal processing should be performed.
 *
 * @param	CallBackRef is the callback reference passed in by the upper
 *		layer when setting the callback functions, and passed back to
 *		the upper layer when the callback is invoked. Its type is
 *		not important to the driver, so it is a void pointer.
 * @param	StatusEvent holds one or more status events that have occurred.
 *		See the XQspiPsu_SetStatusHandler() for details on the status
 *		events that can be passed in the callback.
 * @param	ByteCount indicates how many bytes of data were successfully
 *		transferred.  This may be less than the number of bytes
 *		requested if the status event indicates an error.
 */
typedef void (*XQspiPsu_StatusHandler) (const void *CallBackRef, u32 StatusEvent,
					u32 ByteCount);

/**
 * This typedef contains configuration information for a flash message.
 */
typedef struct {
	u8 *TxBfrPtr;	/**< Tx Buffer pointer */
	u8 *RxBfrPtr;	/**< Rx Buffer pointer */
	u32 ByteCount;	/**< Byte Count */
	u32 BusWidth;	/**< Bus Width */
	u32 Flags;	/**< Flags */
	u8 PollData;	/**< Poll Data */
	u32 PollTimeout;/**< Poll Timeout */
	u8 PollStatusCmd; /**< Poll Status command */
	u8 PollBusMask;	  /**< Poll Bus mask */
	u64 RxAddr64bit;  /**< 64 bit Rx address */
	u8 Xfer64bit;	  /**< 64 bit Tx address */
} XQspiPsu_Msg;

/**
 * This typedef contains configuration information for the device.
 */
typedef struct {
#ifndef SDT
	u16 DeviceId;		/**< Unique ID  of device */
#else
	char *Name;
#endif
	UINTPTR BaseAddress;	/**< Base address of the device */
	u32 InputClockHz;	/**< Input clock frequency */
	u8  ConnectionMode;	/**< Single, Stacked and Parallel mode */
	u8  BusWidth;		/**< Bus width available on board */
	u8 IsCacheCoherent;	/**< Describes whether Cache Coherent or not */
#ifdef SDT
	u16 IntrId;             /** Bits[11:0] Interrupt-id Bits[15:12]
                                * trigger type and level flags */
	UINTPTR IntrParent;     /** Bit[0] Interrupt parent type Bit[64/32:1]
                                * Parent base address */
#endif
#if defined  (XCLOCKING) || defined (SDT)
	u32 RefClk;		/**< Input clocks */
#endif
	u8 IsFbClock;		/**< Describes whether Feed Back clock or not */
} XQspiPsu_Config;

/**
 * The XQspiPsu driver instance data. The user is required to allocate a
 * variable of this type for every QSPIPSU device in the system. A pointer
 * to a variable of this type is then passed to the driver API functions.
 */
typedef struct {
	XQspiPsu_Config Config;	 /**< Configuration structure */
	u32 IsReady;		 /**< Device is initialized and ready */

	u8 *SendBufferPtr;	 /**< Buffer to send (state) */
	u8 *RecvBufferPtr;	 /**< Buffer to receive (state) */
	u64 RecvBuffer;		 /**< Buffer Address to receive (state) */
	u8 *GenFifoBufferPtr;	 /**< Gen FIFO entries */
	s32 TxBytes;	 /**< Number of bytes to transfer (state) */
	s32 RxBytes;	 /**< Number of bytes left to transfer(state) */
	s32 GenFifoEntries;	 /**< Number of Gen FIFO entries remaining */
	volatile u32 IsBusy;		 /**< A transfer is in progress (state) */
	u32 ReadMode;		 /**< DMA or IO mode */
	u32 GenFifoCS;		/**< Gen FIFO chip selection */
	u32 GenFifoBus;		/**< Gen FIFO bus */
	s32 NumMsg;		/**< Number of messages */
	s32 MsgCnt;		/**< Message Count */
	s32 IsUnaligned;	/**< Unaligned information */
	u8 IsManualstart;	/**< Manual start information */
	XQspiPsu_Msg *Msg;	/**< Message */
	XQspiPsu_StatusHandler StatusHandler;	/**< Status Handler */
	void *StatusRef;	/**< Callback reference for status handler */
} XQspiPsu;

/***************** Macros (Inline Functions) Definitions *********************/

/**
 * Definitions for Intel, STM, Winbond and Spansion Serial Flash Device
 * geometry.
 */
#define BYTES256_PER_PAGE	256U		/**< 256 Bytes per Page */
#define BYTES512_PER_PAGE	512U		/**< 512 Bytes per Page */
#define BYTES1024_PER_PAGE	1024U		/**< 1024 Bytes per Page */
#define PAGES16_PER_SECTOR	16U		/**< 16 Pages per Sector */
#define PAGES128_PER_SECTOR	128U		/**< 128 Pages per Sector */
#define PAGES256_PER_SECTOR	256U		/**< 256 Pages per Sector */
#define PAGES512_PER_SECTOR	512U		/**< 512 Pages per Sector */
#define PAGES1024_PER_SECTOR	1024U		/**< 1024 Pages per Sector */
#define NUM_OF_SECTORS2		2U		/**< 2 Sectors */
#define NUM_OF_SECTORS4		4U		/**< 4 Sectors */
#define NUM_OF_SECTORS8		8U		/**< 8 Sector */
#define NUM_OF_SECTORS16	16U		/**< 16 Sectors */
#define NUM_OF_SECTORS32	32U		/**< 32 Sectors */
#define NUM_OF_SECTORS64	64U		/**< 64 Sectors */
#define NUM_OF_SECTORS128	128U		/**< 128 Sectors */
#define NUM_OF_SECTORS256	256U		/**< 256 Sectors */
#define NUM_OF_SECTORS512	512U		/**< 512 Sectors */
#define NUM_OF_SECTORS1024	1024U		/**< 1024 Sectors */
#define NUM_OF_SECTORS2048	2048U		/**< 2048 Sectors */
#define NUM_OF_SECTORS4096	4096U		/**< 4096 Sectors */
#define NUM_OF_SECTORS8192	8192U		/**< 8192 Sectors */
#define SECTOR_SIZE_64K		0X10000U	/**< 64K Sector */
#define SECTOR_SIZE_128K	0X20000U	/**< 128K Sector */
#define SECTOR_SIZE_256K	0X40000U	/**< 256K Sector */
#define SECTOR_SIZE_512K	0X80000U	/**< 512K Sector */


#define XQSPIPSU_READMODE_DMA	0x0U	/**< DMA read mode */
#define XQSPIPSU_READMODE_IO	0x1U	/**< IO read mode */

#define XQSPIPSU_SELECT_FLASH_CS_LOWER	0x1U	/**< Select lower flash */
#define XQSPIPSU_SELECT_FLASH_CS_UPPER	0x2U	/**< Select upper flash */
#define XQSPIPSU_SELECT_FLASH_CS_BOTH	0x3U	/**< Select both flash */

#define XQSPIPSU_SELECT_FLASH_BUS_LOWER	0x1U	/**< Select lower bus flash */
#define XQSPIPSU_SELECT_FLASH_BUS_UPPER	0x2U	/**< Select upper bus flash */
#define XQSPIPSU_SELECT_FLASH_BUS_BOTH	0x3U	/**< Select both bus flash */

#define XQSPIPSU_SELECT_MODE_SPI	0x1U	/**< Select SPI mode */
#define XQSPIPSU_SELECT_MODE_DUALSPI	0x2U	/**< Select dual SPI mode */
#define XQSPIPSU_SELECT_MODE_QUADSPI	0x4U	/**< Select quad SPI mode */

#define XQSPIPSU_GENFIFO_CS_SETUP	0x05U	/**< Chip select setup in GENFIO */
#define XQSPIPSU_GENFIFO_CS_HOLD	0x04U	/**< Chip select hold in GENFIFO */

#define XQSPIPSU_CLK_ACTIVE_LOW_OPTION	0x2U	/**< Clk Active low option */
#define XQSPIPSU_CLK_PHASE_1_OPTION	0x4U	/**< Clk phase 1 option */
#define XQSPIPSU_MANUAL_START_OPTION	0x8U	/**< Manual start option */
#if !defined (versal)
#define XQSPIPSU_LQSPI_MODE_OPTION	0x20U	/**< LQSPI mode option */

#define XQSPIPSU_LQSPI_LESS_THEN_SIXTEENMB	1U /**< LQSPI less Than 16 MB */
#endif

#define XQSPIPSU_GENFIFO_EXP_START	0x100U /**< Genfifo start */

#define XQSPIPSU_DMA_BYTES_MAX		0x10000000U /**< DMA bytes max */

#define XQSPIPSU_CLK_PRESCALE_2		0x00U	/**< Clock prescale 2 */
#define XQSPIPSU_CLK_PRESCALE_4		0x01U	/**< Clock prescale 4 */
#define XQSPIPSU_CLK_PRESCALE_8		0x02U	/**< Clock prescale 8 */
#define XQSPIPSU_CLK_PRESCALE_16	0x03U	/**< Clock prescale 16 */
#define XQSPIPSU_CLK_PRESCALE_32	0x04U	/**< Clock prescale 32 */
#define XQSPIPSU_CLK_PRESCALE_64	0x05U	/**< Clock prescale 64 */
#define XQSPIPSU_CLK_PRESCALE_128	0x06U	/**< Clock prescale 128 */
#define XQSPIPSU_CLK_PRESCALE_256	0x07U	/**< Clock prescale 256 */
#define XQSPIPSU_CR_PRESC_MAXIMUM	7U	/**< Prescale max */

#define XQSPIPSU_CONNECTION_MODE_SINGLE		0U /**< Single mode connection */
#define XQSPIPSU_CONNECTION_MODE_STACKED	1U /**< Stacked mode connection */
#define XQSPIPSU_CONNECTION_MODE_PARALLEL	2U /**< Parallel mode connection */

/*QSPI Frequencies*/
#define XQSPIPSU_FREQ_37_5MHZ 37500000U	/**< Frequency 375 Mhz */
#define XQSPIPSU_FREQ_40MHZ 40000000U	/**< Frequency 40 Mhz */
#define XQSPIPSU_FREQ_100MHZ 100000000U	/**< Frequency 100 Mhz */
#define XQSPIPSU_FREQ_150MHZ 150000000U	/**< Frequency 150 Mhz */

/* Add more flags as required */
#define XQSPIPSU_MSG_FLAG_STRIPE	0x1U /**< Stripe Msg flag */
#define XQSPIPSU_MSG_FLAG_RX		0x2U /**< Rx Msg flag */
#define XQSPIPSU_MSG_FLAG_TX		0x4U /**< Tx Msg flag */
#define XQSPIPSU_MSG_FLAG_POLL		0x8U /**< POLL Msg flag */

#define XQSPIPSU_RXADDR_OVER_32BIT	0x100000000U /**< Rx address over 32 bit */

#define XQSPIPSU_SET_WP		1 /**< GQSPI configuration to toggle WP of flash */

/**
 * select QSPI controller
 */
#define XQspiPsu_Select(InstancePtr, Mask)	\
	XQspiPsu_Out32(((InstancePtr)->Config.BaseAddress) + \
		       XQSPIPSU_SEL_OFFSET, (Mask))

/**
 * Enable QSPI Controller
 */
#define XQspiPsu_Enable(InstancePtr)	\
	XQspiPsu_Out32(((InstancePtr)->Config.BaseAddress) + \
		       XQSPIPSU_EN_OFFSET, XQSPIPSU_EN_MASK)

/**
 * Disable QSPI controller  */
#define XQspiPsu_Disable(InstancePtr)	\
	XQspiPsu_Out32(((InstancePtr)->Config.BaseAddress) + \
		       XQSPIPSU_EN_OFFSET, 0x0U)

/**
 * Read Configuration register of LQSPI Controller
 */
#if !defined (versal)
#define XQspiPsu_GetLqspiConfigReg(InstancePtr)	\
	XQspiPsu_In32((XQSPIPS_BASEADDR) + \
		      XQSPIPSU_LQSPI_CR_OFFSET)
#endif

/*****************************************************************************/
/**
 *
 * This function enables the manual start option
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XQspiPsu_ManualStartEnable(XQspiPsu *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_ManualStartEnable\r\n");
#endif

	if (InstancePtr->IsManualstart == (u8)TRUE) {
#ifdef DEBUG
		xil_printf("\nManual Start\r\n");
#endif
		XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET,
				  XQspiPsu_ReadReg(InstancePtr->Config.BaseAddress, XQSPIPSU_CFG_OFFSET) |
				  XQSPIPSU_CFG_START_GEN_FIFO_MASK);
	}
}
/*****************************************************************************/
/**
 *
 * This function writes the GENFIFO entry to assert CS.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XQspiPsu_GenFifoEntryCSAssert(const XQspiPsu *InstancePtr)
{
	u32 GenFifoEntry;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_GenFifoEntryCSAssert\r\n");
#endif

	GenFifoEntry = 0x0U;
	GenFifoEntry |= (XQSPIPSU_GENFIFO_MODE_SPI | InstancePtr->GenFifoCS |
			 InstancePtr->GenFifoBus | XQSPIPSU_GENFIFO_CS_SETUP);
#ifdef DEBUG
	xil_printf("\nFifoEntry=%08x\r\n", GenFifoEntry);
#endif
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			  XQSPIPSU_GEN_FIFO_OFFSET, GenFifoEntry);
}

/*****************************************************************************/
/**
 *
 * This function writes the GENFIFO entry to de-assert CS.
 *
 * @param	InstancePtr is a pointer to the XQspiPsu instance.
 *
 * @return	None
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void XQspiPsu_GenFifoEntryCSDeAssert(const XQspiPsu *InstancePtr)
{
	u32 GenFifoEntry;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
#ifdef DEBUG
	xil_printf("\nXQspiPsu_GenFifoEntryCSDeAssert\r\n");
#endif

	GenFifoEntry = 0x0U;
	GenFifoEntry |= (XQSPIPSU_GENFIFO_MODE_SPI | InstancePtr->GenFifoBus |
			 XQSPIPSU_GENFIFO_CS_HOLD);
#ifdef DEBUG
	xil_printf("\nFifoEntry=%08x\r\n", GenFifoEntry);
#endif
	XQspiPsu_WriteReg(InstancePtr->Config.BaseAddress,
			  XQSPIPSU_GEN_FIFO_OFFSET, GenFifoEntry);
}

/*****************************************************************************/
/**
 *
 * This is a stub for the status callback. The stub is here in case the upper
 * layers forget to set the handler.
 *
 * @param	CallBackRef is a pointer to the upper layer callback reference
 * @param	StatusEvent is the event that just occurred.
 * @param	ByteCount is the number of bytes transferred up until the event
 *		occurred.
 *
 * @return	None.
 *
 * @note	None.
 *
 ******************************************************************************/
static inline void StubStatusHandler(const void *CallBackRef, u32 StatusEvent,
				     u32 ByteCount)
{
	(const void) CallBackRef;
	(void) StatusEvent;
	(void) ByteCount;

	Xil_AssertVoidAlways();
}
/************************** Function Prototypes ******************************/

/* Initialization and reset */
#ifndef SDT
XQspiPsu_Config *XQspiPsu_LookupConfig(u16 DeviceId);
#else
XQspiPsu_Config *XQspiPsu_LookupConfig(u32 BaseAddress);
#endif
s32 XQspiPsu_CfgInitialize(XQspiPsu *InstancePtr,
			   const XQspiPsu_Config *ConfigPtr,
			   UINTPTR EffectiveAddr);
void XQspiPsu_Reset(XQspiPsu *InstancePtr);
void XQspiPsu_Abort(XQspiPsu *InstancePtr);

/* Transfer functions and handlers */
s32 XQspiPsu_PolledTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			    u32 NumMsg);
s32 XQspiPsu_InterruptTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			       u32 NumMsg);
s32 XQspiPsu_InterruptHandler(XQspiPsu *InstancePtr);
void XQspiPsu_SetStatusHandler(XQspiPsu *InstancePtr, void *CallBackRef,
			       XQspiPsu_StatusHandler FuncPointer);

/* Non blocking Transfer functions */
s32 XQspiPsu_StartDmaTransfer(XQspiPsu *InstancePtr, XQspiPsu_Msg *Msg,
			      u32 NumMsg);
s32 XQspiPsu_CheckDmaDone(XQspiPsu *InstancePtr);

/* Configuration functions */
s32 XQspiPsu_SetClkPrescaler(const XQspiPsu *InstancePtr, u8 Prescaler);
void XQspiPsu_SelectFlash(XQspiPsu *InstancePtr, u8 FlashCS, u8 FlashBus);
s32 XQspiPsu_SetOptions(XQspiPsu *InstancePtr, u32 Options);
s32 XQspiPsu_ClearOptions(XQspiPsu *InstancePtr, u32 Options);
u32 XQspiPsu_GetOptions(const XQspiPsu *InstancePtr);
s32 XQspiPsu_SetReadMode(XQspiPsu *InstancePtr, u32 Mode);
void XQspiPsu_SetWP(const XQspiPsu *InstancePtr, u8 Value);
void XQspiPsu_WriteProtectToggle(const XQspiPsu *InstancePtr, u32 Toggle);
void XQspiPsu_Idle(const XQspiPsu *InstancePtr);

/************************** Variable Prototypes ******************************/

/**
 * This table contains configuration information for each QSPIPSU device
 * in the system.
 */
#ifndef SDT
extern XQspiPsu_Config XQspiPsu_ConfigTable[XPAR_XQSPIPSU_NUM_INSTANCES];
#else
extern XQspiPsu_Config XQspiPsu_ConfigTable[];
#endif

#ifdef __cplusplus
}
#endif


#endif /* XQSPIPSU_H_ */
/** @} */
