/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_status.h
*
* This is the header file which contains status codes for the PLM, PLMI
* and loader.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/13/2018 Initial release
* 1.01  bsv  06/11/2019 Added TCM power up code to Xilloader to fix issue in
*						R5-1 split mode functionality
*       bsv  06/17/2019 Added support for CFI and CFU error handling
*       bsv  06/26/2019 Added secondary boot support
*       vnsl 07/30/2019 Added code to load secure headers
*       ma   08/01/2019 Added LPD init code
*       ma   08/24/2019 Added SSIT commands
*       kc   08/28/2019 Added descriptions to PLM error codes
*       scs  08/29/2019 Added API to validate extended ID Code
* 1.02  bsv  10/31/2019 Added USB secondary boot mode support
*       kc   12/17/2019 Add deferred error mechanism for mask poll
*       bsv  02/12/2020 Added support for SD/eMMC raw boot mode
*       bsv  02/13/2020 XilPlmi generic commands should not be supported
* 						via IPI
*       ma   02/18/2020 Added event logging code
*       bsv  02/23/2020 Added multi partition support for SD/eMMC FS boot modes
*       kc   02/27/2020 Added SEM support for partial reconfiguration
*       bsv  02/28/2020 Added support for delay handoff
*       har  02/18/2020 Added major error code for Security
*       bsv  04/04/2020 Code clean up
*       kc   04/23/2020 Added interrupt support for SEU events
* 1.03  bsv  06/27/2020 Add dual stacked mode support
*       bsv  07/01/2020 Unmount file system after loading PDIs
*       skd  07/14/2020 Added a macro for DMA transfer error
*       kal  07/21/2020 Added a macro for Data copy failure for Security
*       bsv  07/29/2020 Added error codes related to delay load
*       kpt  07/30/2020 Added error code for Meta header length overflow
*       kc   08/04/2020 Added error code NPLL lock status for master SLR
*       har  08/11/2020 Added error code for authenticated JTAG
*       td   08/19/2020 Fixed MISRA C violations Rule 10.3
*       rama 08/21/2020 Added error code for STL
*       bm   09/21/2020 Added error codes for DFx Compatibility Check
*       bm   09/24/2020 Added error code for FuncID mismatch
*       bsv  10/13/2020 Code clean up
*       kpt  10/19/2020 Added error code for glitch detection
*       td   10/19/2020 MISRA C Fixes
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_STATUS_H
#define XPLMI_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xstatus.h"

/************************** Constant Definitions *****************************/
/**
 * @name PLM error codes description
 *
 * 0xXXXXYYYY - Error code format
 * XXXX - Major error code - PLM/LOADER/XPLMI error codes
 *		as defined in xplmi_status.h
 * YYYY - Minor error code - Libraries / Drivers error code
 *		as defined in respective modules
 */
#define XPLMI_CORRECTABLE_ERROR_MASK			(1U << 31U)
#define XPLMI_UNCORRECTABLE_ERROR_MASK			(1U << 30U)

#define XPLMI_STATUS_MASK				(0xFFFF0000U)
#define XPLMI_STATUS_MODULE_MASK			(0xFFFFU)
#define XPLMI_ERR_CDO_CMD_MASK				(0x1FFFU)
#define XPLMI_STATUS_SHIFT				(16U)
#define XPLMI_ERR_CODE_MASK				(0x7FFFFFFFU)

/**
 * Status for PLM functions
 */
typedef enum {
	/** Status codes used in PLMI */
	XPLM_SUCCESS = 0x0,		/**< 0x0 - Success */
	XPLM_FAILURE,			/**< 0x1 - Used internally
					  for small functions */
	XPLMI_TASK_INPROGRESS,		/**< 0x2 - Used internally
					  to indicate task is in progress */

	XPLMI_ERR_DMA_LOOKUP = 0x100,	/**< 0x100 - Error when DMA driver
					  lookup fails. */
	XPLMI_ERR_DMA_CFG,		/**< 0x101 - Error when DMA driver
					  config fails. */
	XPLMI_ERR_DMA_SELFTEST,		/**< 0x102 - Error when DMA Self test
					  fails. It occurs when DMA is in reset
					  and PLM tries to initialize it */

	XPLMI_ERR_IOMOD_INIT,		/**< 0x103 - Error when IOModule
					  driver look up fails */
	XPLMI_ERR_IOMOD_START,		/**< 0x104 - Error when IOModule
					  driver startup fails */
	XPLMI_ERR_IOMOD_CONNECT,	/**< 0x105 - Error when IOModule
					  driver connection fails */

	XPLMI_ERR_MODULE_MAX,		/**< 0x106 - Error when PLMI module
					  is not registered. Can occur when
					  invalid CDO CMD is processed
					  by Xilplmi */
	XPLMI_ERR_CMD_APIID,		/**< 0x107 - Error when valid module
					  and unregistered CMD ID is
					  processed by xilplmi */
	XPLMI_ERR_CMD_HANDLER_NULL,	/**< 0x108 - Error when no command
					 handler is registered by module
					  for CDO CMD */
	XPLMI_ERR_CMD_HANDLER,		/**< 0x109 - Error returned by the
					  CDO CMD handler. For error returned
					  by the CMD, check the PLM minor code*/
	XPLMI_ERR_RESUME_HANDLER,	/**< 0x10A - Error returned by the CDO
					  CMD resume handler. For error returned
					  by the CMD, check the PLM minor code*/

	XPLMI_ERR_CDO_HDR_ID,		/**< 0x10B - Error when valid CDO header
					  ID is not present in CDO header. Can
					  happen when different partition
					  type is processed as CDO */
	XPLMI_ERR_CDO_CHECKSUM,		/**< 0x10C - Error when CDO header
					  checksum is wrong. Can happen when
					  CDO header is corrupted */

	XPLMI_ERR_UART_DEV_PM_REQ,	/**< 0x10D - Error when libPM request
					  device for UART fails. PM error code
					  is present in PLM minor code */
	XPLMI_ERR_UART_LOOKUP,		/**< 0x10E - Error when UART driver
					  lookup fails. */
	XPLMI_ERR_UART_CFG,		/**< 0x10F - Error when UART driver
					  config fails. */

	XPLMI_ERR_SSIT_MASTER_SYNC,	/**< 0x110 - Error when SSIT slave
					  sync fails with master */
	XPLMI_ERR_SSIT_SLAVE_SYNC,	/**< 0x111 - Error when SSIT master
					 times out waiting for slaves sync
					 point */
	XPLMI_ERR_INVALID_LOG_LEVEL, /**< 0x112 - Error when invalid log level
					 is received in Logging command. */
	XPLMI_ERR_INVALID_LOG_BUF_ADDR, /**< 0x113 - Error when invalid log buffer
					 address is received in Logging command. */
	XPLMI_ERR_INVALID_LOG_BUF_LEN, /**< 0x114 - Error when invalid log buffer
						 length is received in Logging command. */
	XPLMI_ERR_IPI_CMD, /**< 0x115 - Error when command execution through
							IPI is not supported */
	XPLMI_ERR_REGISTER_IOMOD_HANDLER, /**< 0x116 - Error when registering
						IoModule Handler */
	XPLMI_ERR_WDT_PERIODICITY, /**< 0x117 - Invalid Periodicity parameter for
				     SetWDT command */
	XPLMI_ERR_WDT_NODE_ID, /**< 0x118 - Invalid Node ID parameter for SetWDT
				 command */
	XPLMI_ERR_WDT_LPD_NOT_INITIALIZED, /**< 0x119 LPD MIO is used for WDT
					     but LPD is not initialized */
	XPLMI_ERR_INVALID_INTR_ID_DISABLE, /**< 0x11A Invalid Interrupt ID used
					     to disable interrupt */
	XPLMI_ERR_INVALID_INTR_ID_CLEAR,   /**< 0x11B Invalid Interrupt ID used
					     to clear interrupt */
	XPLMI_ERR_INVALID_INTR_ID_REGISTER, /**< 0x11C Invalid Interrupt ID used
					     to register interrupt handler */
	XPLMI_ERR_DMA_XFER_WAIT,	    /**< 0x11D Dma transfer wait failed */
	XPLMI_ERR_NON_BLOCK_DMA_WAIT_SRC,   /**< 0x11E Non Block Dma transfer wait
						failed in Src channel*/
	XPLMI_ERR_NON_BLOCK_DMA_WAIT_DEST,  /**< 0x11F Non Block Dma transfer wait
						failed in Dest channel WaitForDone */
	XPLMI_ERR_NON_BLOCK_SRC_DMA_WAIT,   /**< 0x120 Non Block Src Dma transfer
						wait failed */
	XPLMI_ERR_NON_BLOCK_DEST_DMA_WAIT,  /**< 0x121 Non Block Dest Dma transfer
						wait failed */
	XPLMI_ERR_DMA_XFER_WAIT_SRC,	    /**< 0x122 Dma Xfer failed in Src Channel
						wait for done */
	XPLMI_ERR_DMA_XFER_WAIT_DEST,	    /**< 0x123 Dma Xfer failed in Dest Channel
						wait for done */
	XPLMI_ERR_UART_MEMSET,		    /**< 0x124 Memset of UartPsv Instance failed */
	XPLMI_ERR_MEMCPY_COPY_CMD,		/**< 0x125 Error during memcpy of
						CdoCopyCmd */
	XPLMI_ERR_MEMCPY_CMD_EXEC,		/**< 0x126 Error during memcpy of
						CdoCmdExecute */
	XPLMI_ERR_MEMCPY_IMAGE_INFO,	/**< 0x127 Error during memcpy of
						XLoader_ImageInfo */
	XPLMI_ERR_UART_PSV_SET_BAUD_RATE,	/**<0x128 Error during setting
						XUartPsv_SetBaudRate to XPLMI_UART_BAUD_RATE */
	XPLMI_ERR_IPI_WRITE, /**<0x129 Error during XPlmi_IpiWrite */

	/** Status codes used in PLM */
	XPLM_ERR_TASK_CREATE = 0x200,	/**< 0x200 - Error when task create
					  fails. This can happen when max
					  tasks are created */
	XPLM_ERR_PM_MOD,		/**< 0x201 - Error initializing
					  the PM Module */
	XPLM_ERR_LPD_MOD,		/**< 0x202 - Error initializing
					  the LPD modules */
	XPLM_ERR_EXCEPTION,		/**< 0x203 - Exception has occurred
					  during PLM initialization. EAR and
					  ESR are printed on UART console
					  if enabled */
	XPLM_ERR_NPLL_LOCK,		/**< 0x204 - Unable to lock NOC PLL
					  for master SLR devices */
	XPLM_ERR_STL_MOD,		/**< 0x205 - Error initializing
					  the STL Module */

	/** Status codes used in XLOADER */
	XLOADER_UNSUPPORTED_BOOT_MODE = 0x300, /**< 0x300 - Error for
					 unsupported bootmode. It occurs if
					 invalid boot mode is selected or
					 selected boot mode peripheral is
					 not selected in CIPS */
	XLOADER_ERR_IMGHDR_TBL = 0x302,	/**< 0x302 - Multiple conditions can
					  give this error.
					  - If PLM is unable to read
					  image header table */
	XLOADER_ERR_IMGHDR,		/**< 0x303 - Error if image header
					  checksum fails */
	XLOADER_ERR_PRTNHDR,		/**< 0x304 - Error if partition header
					  checksum fails */
	XLOADER_ERR_WAKEUP_A72_0,	/**< 0x305 - Error waking up the A72-0
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_A72_1,	/**< 0x306 - Error waking up the A72-1
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_R5_0,	/**< 0x307 - Error waking up the R5-0
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_R5_1,	/**< 0x308 - Error waking up the R5-1
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_R5_L,	/**< 0x309 - Error waking up the R5-L
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_PSM,		/**< 0x30A - Error waking up the PSM
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_PL_NOT_PWRUP,	/**< 0x30B - Error powering up the PL */
	XLOADER_ERR_UNSUPPORTED_OSPI,	/**< 0x30C - Error for unsupported OSPI
					  flash */
	XLOADER_ERR_UNSUPPORTED_OSPI_SIZE, /**< 0x30D - Error for unsupported
					     OSPI flash size */
	XLOADER_ERR_OSPI_INIT,		/**< 0x30E - Error when OSPI driver
					  lookup fails. This happens when OSPI
					  is not selected in CIPS */
	XLOADER_ERR_OSPI_CFG,		/**< 0x30F - Error when OSPI driver
					  CFG fails */
	XLOADER_ERR_OSPI_SEL_FLASH_CS0,	/**< 0x310 - Error when OSPI driver is
					  unable to select flash CS0. Check minor
					  code for OSPI driver error code */
	XLOADER_ERR_OSPI_READID,	/**< 0x311 - Error when OSPI ReadID
					  fails */
	XLOADER_ERR_OSPI_READ,		/**< 0x312 - Error when OSPI driver read
					  fails. Check minor code for OSPI
					  driver error code */
	XLOADER_ERR_OSPI_4BMODE,	/**< 0x313 - Error when OSPI is unable
					  to enter/exit 4B mode */

	XLOADER_ERR_QSPI_READ_ID,	/**< 0x314 - Error when QSPI read
					  fails */
	XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_ID,	/**< 0x315 - Error when QSPI flash ID is
							not supported */
	XLOADER_ERR_QSPI_INIT,		/**< 0x316 - Error when QSPI driver look
					  up or cfg fails */
	XLOADER_ERR_QSPI_MANUAL_START,	/**< 0x317 - Error when QSPI driver
					  manual start fails */
	XLOADER_ERR_QSPI_PRESCALER_CLK, /**< 0x318 - Error when QSPI driver
					  Prescalar setting fails */
	XLOADER_ERR_QSPI_CONNECTION,	/**< 0x319 - Error when invalid QSPI
					  connection listed other than Single,
					  Dual, stacked*/
	XLOADER_ERR_QSPI_READ,		/**< 0x31A - Error when QSPI driver
					  Read fails */
	XLOADER_ERR_QSPI_LENGTH,	/**< 0x31B - Error when QSPI read length
					  is greater than flash size */

	XLOADER_ERR_SD_INIT,		/**< 0x31C - Error when SD mount fails*/
	XLOADER_ERR_SD_F_OPEN,		/**< 0x31D - Error when SD file open
					  fails. This can happen when file is
					  not present or read from SD fails.
					  File system error code is present in
					  PLM minor code */
	XLOADER_ERR_SD_F_LSEEK,		/**< 0x31E - Error when f_seek fails
					  while reading from SD card */
	XLOADER_ERR_SD_F_READ,		/**< 0x31F - Error while reading from
					  SD card */

	XLOADER_ERR_IMG_ID_NOT_FOUND,	/**< 0x320 - Error when Image ID is
					  not found in subsystem while reloading
					  image */
	XLOADER_ERR_TCM_ADDR_OUTOF_RANGE, /**< 0x321 - Error while loading to
					    TCM and if address is out of range*/
	XLOADER_ERR_CFRAME_LOOKUP,	/**< 0x322 - Error when CFRAME driver
					  look up fails */
	XLOADER_ERR_CFRAME_CFG,		/**< 0x323 - Error when CFRAME driver
					  CFG fails */
	XLOADER_ERR_UNSUPPORTED_SEC_BOOT_MODE, /**< 0x324 - Error for
					 unsupported secondary boot mode */
	XLOADER_ERR_SECURE_METAHDR,	/**< 0x325 - Error when meta header
					  secure validations fail. */
	XLOADER_ERR_GEN_IDCODE,		/**< 0X326 - Error caused due to
						mismatch in IDCODEs */
	XLOADER_ERR_USB_LOOKUP,		/**< 0x327 - Error when USB lookup fails*/
	XLOADER_ERR_USB_CFG,		/**< 0x328 - Error when USB cfg initialize fails */
	XLOADER_ERR_USB_START,		/**< 0x329 - Error when USB fails to start */
	XLOADER_ERR_DFU_DWNLD,	/**< 0x32A - Error when pdi fails to download */
	XLOADER_ERR_DEFERRED_CDO_PROCESS, /**< 0x32B - Error occurred while
					  processing CDO but error is deferred
					  till whole CDO processing is completed */
	XLOADER_ERR_SD_LOOKUP,		/**< 0x32C - Error when SD look up fails */
	XLOADER_ERR_SD_CFG,		/**< 0x32D - Error when SD config fails */
	XLOADER_ERR_SD_CARD_INIT,	/**< 0x32E - Error when SD card init fails */
	XLOADER_ERR_MMC_PART_CONFIG, /**< 0x32F - Error when MMC part
						configuration fails */
	XLOADER_ERR_SEM_STOP_SCAN,	/**< 0x330 - Error while stopping the
					  SEM Scan */
	XLOADER_ERR_SEM_CFR_INIT,	/**< 0x331 - Error while starting the
					  SEM Scan */
	XLOADER_ERR_DELAY_ATTRB,	/**< 0x332 - Error when both delay handoff
					  and delay load are set for the same image */
	XLOADER_ERR_NUM_HANDOFF_CPUS,	/**< 0x333 - Error when number of CPUs
						exceed max count */
	XLOADER_ERR_OSPI_CONN_MODE, /**< 0x334 - Error when OSPI mode is not
						supported */
	XLOADER_ERR_OSPI_SEL_FLASH_CS1,	/**< 0x335 - Error when OSPI driver is
					  unable to select flash CS1. Check minor
					  code for OSPI driver error code */
	XLOADER_ERR_OSPI_SDR_NON_PHY, /**< 0x336 - Error when OSPI driver is unable
						to set the controller to SDR NON PHY mode */
	XLOADER_ERR_OSPI_COPY_OVERFLOW, /**< 0x337 - Error when source address in
						OSPI copy exceeds flash size */
	XLOADER_ERR_SD_F_CLOSE,		/**< 0x338 - Error on closure of file in SD
							filesystem modes */
	XLOADER_ERR_SD_UMOUNT,		/**< 0X339 - Error on unmounting filesystem */
	XLOADER_ERR_DMA_XFER,		/**< 0x33A DMA Transfer failed */
	XLOADER_ERR_DMA_XFER_SD_RAW,		/**< 0x33B DMA Transfer failed in SD Raw*/
	XLOADER_ERR_CONFIG_SUBSYSTEM,		/**< 0X33C - Error while configuring subsystem */
	XLOADER_ERR_COPY_TO_MEM,		/**< 0x33D - Error on copying image to DDR with
						the copy to memory attribute enabled */
	XLOADER_ERR_DELAY_LOAD,		/**< 0x33E - When the image has delay load
						attribute set and the boot source is SMAP, SBI,
						PCIE or JTAG, the image is copied to PMC RAM to
						free it from the SBI buffers. Errors occurred
						during such copies to PMC RAM is denoted using
						this error code. */
	XLOADER_ERR_ADD_TASK_SCHEDULER,	/**<0x33F - Error while adding task to
						the scheduler */
	XLOADER_ERR_SD_MAX_BOOT_FILES_LIMIT,	/**< 0x340 - Error code returned when search
							for bootable file crosses max limit */
	XLOADER_ERR_UNSUPPORTED_QSPI_FLASH_SIZE, /**< 0x341 - Error when QSPI flash Size is
							not supported */
	XLOADER_ERR_PM_DEV_PSM_PROC,		/**< 0x342 - Failed in XPM Request Device for
							PM_DEV_PSM_PROC */
	XLOADER_ERR_PM_DEV_IOCTL_RPU0_SPLIT,	/**<0x343 - Failed in XPM Device Ioctl for
							RPU0_0 in SPLIT mode */
	XLOADER_ERR_PM_DEV_IOCTL_RPU1_SPLIT,	/**< 0x344 - Failed in XPM Device Ioctl for
							RPU0_1 in SPLIT mode */
	XLOADER_ERR_PM_DEV_IOCTL_RPU0_LOCKSTEP, /**< 0x345 - Failed to XPM Device Ioctl for
							RPU0_0 in LOCKSTEP mode */
	XLOADER_ERR_PM_DEV_IOCTL_RPU1_LOCKSTEP, /**< 0x346 - Failed to XPM Device Ioctl for
							RPU0_1 in LOCKSTEP mode */
	XLOADER_ERR_PM_DEV_TCM_0_A,		/**< 0x347 - Failed to XPM Request Device for
							PM_DEV_TCM_0_A */
	XLOADER_ERR_PM_DEV_TCM_0_B,		/**< 0x348 - Failed to XPM Request Device for
							PM_DEV_TCM_0_B */
	XLOADER_ERR_PM_DEV_TCM_1_A,		/**< 0x349 - Failed in XPM Request Device for
							PM_DEV_TCM_1_A */
	XLOADER_ERR_PM_DEV_TCM_1_B,		/***<0x34A - Failed in XPM Request Device for
							PM_DEV_TCM_1_B */
	XLOADER_ERR_PM_DEV_DDR_0,		/**< 0x34B - Failed to XPM Request Device for
							PM_DEV_DDR_0 */
	XLOADER_ERR_PM_DEV_QSPI,		/**< 0x34C - Failed to XPM Request Device for
							PM_DEV_QSPI */
	XLOADER_ERR_PM_DEV_SDIO_0,		/**< 0x34D - Failed to XPM Request Device for
							PM_DEV_SDIO_0 */
	XLOADER_ERR_PM_DEV_SDIO_1,		/**< 0x34E - Failed to XPM Request Device for
							PM_DEV_SDIO_1 */
	XLOADER_ERR_PM_DEV_USB_0,		/**< 0x34F - Failed to XPM Request Device for
							PM_DEV_USB_0 */
	XLOADER_ERR_PM_DEV_OSPI,		/**< 0x350 - Failed to XPM Request Device for
							PM_DEV_OSPI */
	XLOADER_ERR_DEV_NOT_DEFINED,		/**< 0x351 - Device ID of the image to be
							loaded is not defined */
	XLOADER_ERR_PARENT_QUERY_VERIFY,	/**< 0x352 - Failed to Query Parent ID of
							an image while verifying its Image UIDs */
	XLOADER_ERR_INCOMPATIBLE_CHILD_IMAGE,	/**< 0x353 - Error while checking
							compatibility of a image with it's parent */
	XLOADER_ERR_NO_VALID_PARENT_IMG_ENTRY,	/**< 0x354 - Error if No Valid Parent
							Image Entry is found in the ImageInfo Table */
	XLOADER_ERR_INVALIDATE_CHILD_IMG,	/**< 0x355 - Error while Invalidating the
							Child Image Entry */
	XLOADER_ERR_INVALID_PARENT_IMG_ID,	/**< 0x356 - Error when Invalid ParentImgID
							is obtained when queried for parent ImgID */
	XLOADER_ERR_IMAGE_INFO_TBL_OVERFLOW,	/**< 0x357 - Error when ImageInfo Table
							overflows */
	XLOADER_ERR_FUNCTION_ID_MISMATCH,	/**< 0x358 - Error when Function ID given
							while loading Image from DDR is not matching with the ID
							stored in Image Header */
	XLOADER_ERR_MEMSET,	/**< 0x359 - Error during memset */
	XLOADER_DDR_COPY_UNSUPPORTED_PARAMS,	/**< 0x35A - Error when source address,
							destination address or length params passed to
							XLoader_DdrCopy are not word aligned */

	/**< Security Major error codes */
	XLOADER_ERR_INIT_GET_DMA = 0x600,
		/**< 0x600 Failed to get DMA instance at time of initialization */
	XLOADER_ERR_INIT_INVALID_CHECKSUM_TYPE,
		/**< 0x601 only SHA3 checksum is supported */
	XLOADER_ERR_INIT_CHECKSUM_COPY_FAIL,
		/**< 0x602 Failed when copying Checksum from flash device */
	XLOADER_ERR_INIT_AC_COPY_FAIL,
		/**< 0x603 Failed when copying AC from flash device */
	XLOADER_ERR_INIT_CHECKSUM_INVLD_WITH_AUTHDEC,
		/**< 0x604 Failed as checksum was enabled with authentication
				 and encryption enabled */

	XLOADER_ERR_DMA_TRANSFER,
		/**< 0x605 DMA Transfer failed while copying */

	XLOADER_ERR_IHT_AUTH_DISABLED,
		/**< 0x606 Authentication is not enabled for Image Header table */
	XLOADER_ERR_IHT_GET_DMA,
		/**< 0x607 Failed to get DMA instance for IHT authentication */
	XLOADER_ERR_IHT_COPY_FAIL,
		/**< 0x608 Failed when copying IHT AC from flash device */
	XLOADER_ERR_IHT_HASH_CALC_FAIL,
		/**< 0x609 Failed to calculate hash for IHT authentication */
	XLOADER_ERR_IHT_AUTH_FAIL,
		/**< 0x60A Failed to authenticate IHT */

	XLOADER_ERR_HDR_COPY_FAIL,
		/**< 0x60B Failed when copying IH/PH from flash device */
	XLOADER_ERR_HDR_AES_OP_FAIL,
		/**< 0x60C Failed due to AES init or Decrypt init or key selection
		failure */
	XLOADER_ERR_HDR_DEC_FAIL,
		/**< 0x60D Failed to decrypt IH/PH */
	XLOADER_ERR_HDR_AUTH_FAIL,
		/**< 0x60E Failed to authenticate IH/PH */
	XLOADER_ERR_HDR_NOT_SECURE,
		/**< 0x60F Neither authentication nor encryption enabled for IH/PH */
	XLOADER_ERR_HDR_GET_DMA,
		/**< 0x610 Failed to get DMA instance for IH/PH
		authentication/decryption */
	XLOADER_ERR_HDR_HASH_CALC_FAIL,
		/**< 0x611 Failed to calculate hash for IH/PH authentication */
	XLOADER_ERR_HDR_NOT_ENCRYPTED,
		/**< 0x612 IH/PH is not encrypted */
	XLOADER_ERR_HDR_AUTH_DISABLED,
		/**< 0x613 Authentication disabled for IH/PH */

	XLOADER_ERR_SEC_IH_READ_VERIFY_FAIL,
		/**< 0x614 Failed to read IH and verify checksum */
	XLOADER_ERR_SEC_PH_READ_VERIFY_FAIL,
		/**< 0x615 Failed to read PH and verify checksum */

	XLOADER_ERR_PRTN_HASH_CALC_FAIL,
		/**< 0x616 Hash calculation failed for partition authentication */
	XLOADER_ERR_PRTN_AUTH_FAIL,
		/**< 0x617 Partition authentication failed */
	XLOADER_ERR_PRTN_HASH_COMPARE_FAIL,
		/**< 0x618 Partition hash comparison failed */
	XLOADER_ERR_PRTN_DECRYPT_FAIL,
		/**< 0x619 Partition decryption failed */

	XLOADER_ERR_HWROT_EFUSE_AUTH_COMPULSORY,
		/**< 0x61A PPK Programmed but eFuse authentication is disabled */
	XLOADER_ERR_HWROT_BH_AUTH_NOT_ALLOWED,
		/**< 0x61B PPK Programmed and BH authentication is enabled */
	XLOADER_ERR_AUTH_EN_PPK_HASH_ZERO,
		/**< 0x61C PPK not programmed and authentication is enabled */
	XLOADER_ERR_ENCONLY_ENC_COMPULSORY,
		/**< 0x61D Encryption is disabled */
	XLOADER_ERR_KAT_FAILED,
		/**< 0x61E KAT failed */
	XLOADER_ERR_DATA_COPY_FAIL,
		/**< 0x61F Data copy to internal memory failed */
	XLOADER_ERR_METAHDR_LEN_OVERFLOW,
		/**< 0x620 Failed when total size is greater than Metahdr length */

	XLOADER_ERR_AUTH_JTAG_EFUSE_AUTH_COMPULSORY,
		/**< 0x621 Jtag Authentication failed when PPK not programmed */
	XLOADER_ERR_AUTH_JTAG_DISABLED,
		/**< 0x622 Jtag Authentication disable efuse bit is set */
	XLOADER_ERR_AUTH_JTAG_PPK_VERIFY_FAIL,
		/**< 0x623 Jtag Authentication failed when verification of PPK
		hash verification failed */
	XLOADER_ERR_AUTH_JTAG_SIGN_VERIFY_FAIL,
		/**< 0x624 Jtag Authentication failed when verification of
		signature failed*/
	XLOADER_ERR_AUTH_JTAG_EXCEED_ATTEMPTS,
		/**< 0x625 Jtag Authentication failed more than once */
	XLOADER_ERR_AUTH_JTAG_GET_DMA,
		/**< 0x626 Failed to get DMA instance for JTAG authentication */
	XLOADER_ERR_AUTH_JTAG_HASH_CALCULATION_FAIL,
		/**< 0x627 Hash calculation failed before signature verification */
	XLOADER_ERR_AUTH_JTAG_DMA_XFR,
		/**< 0x628 Failed to get Auth Jtag data with DMA Xfr */
	XLOADER_ERR_MEMSET_SECURE_PTR,
		/**< 0x629 Error during memset for SecurePtr */
	XLOADER_ERR_GLITCH_DETECTED,
		/**< 0x62A Error glitch detected */

	XPLMI_ERR_CDO_CMD = 0x2000,
		/**< 0x2XXX, CDO command handler has failed.
		 * [12:8] contains Module ID, [7:0] contains API ID.
		 * Refer Minor code for Handler error code */
} XPlmiStatus_t;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
static inline int XPlmi_UpdateStatus(XPlmiStatus_t PlmiStatus, int ModuleStatus)
{
	u32 UStatus = (u32)PlmiStatus;
	u32 UModuleStatus = (u32)ModuleStatus;

	UStatus = UStatus << XPLMI_STATUS_SHIFT;
	UStatus = UStatus | (UModuleStatus & XPLMI_STATUS_MODULE_MASK);
	UStatus = UStatus & XPLMI_ERR_CODE_MASK;

	return (int)UStatus;
}

/************************** Function Prototypes ******************************/
void XPlmi_ErrMgr(int ErrStatus);

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_STATUS_H */
