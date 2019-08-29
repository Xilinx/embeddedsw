/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
#define XPLMI_SUCCESS					(XST_SUCCESS)
#define XPLMI_CORRECTABLE_ERROR_MASK			(1U<<31U)
#define XPLMI_UNCORRECTABLE_ERROR_MASK			(1U<<30U)

#define XPLMI_STATUS_MASK				(0xFFFF0000U)
#define XPLMI_STATUS_MODULE_MASK			(0xFFFFU)
#define XPLMI_UPDATE_STATUS(PlmiStatus, ModuleStatus)		\
		(((PlmiStatus<<16U) & XPLMI_STATUS_MASK) + \
		 (ModuleStatus & XPLMI_STATUS_MODULE_MASK))

/**
 * Status for PLM functions
 */
enum {
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

	/** Status codes used in PLM */
	XPLM_ERR_TASK_CREATE = 0x200,	/**< 0x200 - Error when task create
					  fails. This can happen when max
					  tasks are created */
	XPLM_ERR_PM_MOD,		/**< 0x201 - Error initializing
					  the PM Module */
	XPLM_ERR_LPD_MOD,		/**< 0x202 - Error initializing
					  the LPD modules */
	XPLM_ERR_EXCEPTION,		/**< 0x203 - Expection has occurred
					  duing PLM initialization. EAR and
					  ESR are printed on UART console
					  if enabled */

	/** Status codes used in XLOADER */
	XLOADER_UNSUPPORTED_BOOT_MODE = 0x300, /**< 0x300 - Error for
					 unsupported bootmode. It occurs if
					 invalid boot mode is selected or
					 selected boot mode peripheral is
					 not selected in CIPS */
	XLOADER_ERR_IMGHDR_TBL = 0x302,	/**< 0x302 - Multiple conditions can
					  give this error.
					  -If Image header table has wrong
					  checksum or invalid IDCODE
					  - If PLM is unable to read
					  image header table
					  - If secure settings do not
					  match to efuse settings  */
	XLOADER_ERR_IMGHDR,		/**< 0x303 - Error if image header
					  checksum fails */
	XLOADER_ERR_PRTNHDR,		/**< 0x304 - Error if partition header
					  checksum fails */
	XLOADER_ERR_WAKEUP_A72_0,	/**< 0x305 - Error waking up the A72-0
					  during handoff. Check the PLM minor
					  code for PM error code. */
	XLOADER_ERR_WAKEUP_A72_1,	/**< 0x306 - Error waking up the A72-0
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
	XLOADER_ERR_OSPI_SEL_FLASH,	/**< 0x310 - Error when OSPI driver is
					  unable to select flash. Check minor
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
	XLOADER_ERR_UNSUPPORTED_QSPI,	/**< 0x315 - Error when QSPI flash is
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
	XLOADER_ERR_IDCODE,			/**< 0X326 - Error caused due to
						mismatch in IDCODEs */
};

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XPlmi_ErrMgr(int Status);
void XPlmi_DumpRegisters();

/************************** Variable Definitions *****************************/

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_STATUS_H */
