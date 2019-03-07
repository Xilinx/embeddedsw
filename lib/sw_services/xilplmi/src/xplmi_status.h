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
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
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
 * XXXX - PLM/LOADER/XPLMI error codes as defined in xplmi_status.h
 * YYYY - Libraries / Drivers error code as defined in respective modules
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
	XPLM_SUCCESS = 0x0, /* 0x0U */
	XPLM_FAILURE, /* 0x1U */
	XPLMI_TASK_INPROGRESS,

	XPLMI_ERR_DMA_LOOKUP = 0x100,
	XPLMI_ERR_DMA_CFG,
	XPLMI_ERR_DMA_SELFTEST,

	XPLMI_ERR_IOMOD_INIT,
	XPLMI_ERR_IOMOD_START,
	XPLMI_ERR_IOMOD_CONNECT,

	XPLMI_ERR_MODULE_MAX,
	XPLMI_ERR_CMD_APIID,
	XPLMI_ERR_CMD_HANDLER_NULL,
	XPLMI_ERR_CMD_HANDLER,
	XPLMI_ERR_RESUME_HANDLER,

	XPLMI_ERR_CDO_HDR_ID,
	XPLMI_ERR_CDO_CHECKSUM,

	/** Status codes used in PLM */
	XPLM_ERR_TASK_CREATE = 0x200,
	XPLM_ERR_PM_MOD,
	XPLM_ERR_LPD_MOD,
	XPLM_ERR_EXCEPTION,

	/** Status codes used in XLOADER */
	XLOADER_UNSUPPORTED_BOOT_MODE = 0x300,
	XLOADER_ERR_BOOTHDR,
	XLOADER_ERR_IMGHDR_TBL,
	XLOADER_ERR_IMGHDR,
	XLOADER_ERR_PRTNHDR,
	XLOADER_ERR_WAKEUP_A72_0,
	XLOADER_ERR_WAKEUP_A72_1,
	XLOADER_ERR_WAKEUP_R5_0,
	XLOADER_ERR_WAKEUP_R5_1,
	XLOADER_ERR_WAKEUP_R5_L,
	XLOADER_ERR_PL_NOT_PWRUP,
	XLOADER_ERR_UNSUPPORTED_OSPI,
	XLOADER_ERR_UNSUPPORTED_OSPI_SIZE,
	XLOADER_ERR_OSPI_INIT,
	XLOADER_ERR_OSPI_CFG,
	XLOADER_ERR_OSPI_SEL_FLASH,
	XLOADER_ERR_OSPI_READID,
	XLOADER_ERR_OSPI_READ,
	XLOADER_ERR_OSPI_4BMODE,

	XLOADER_ERR_QSPI_READ_ID,
	XLOADER_ERR_UNSUPPORTED_QSPI,
	XLOADER_ERR_QSPI_INIT,
	XLOADER_ERR_QSPI_MANUAL_START,
	XLOADER_ERR_QSPI_PRESCALER_CLK,
	XLOADER_ERR_QSPI_CONNECTION,
	XLOADER_ERR_QSPI_READ,
	XLOADER_ERR_QSPI_LENGTH,

	XLOADER_ERR_SD_INIT,
	XLOADER_ERR_SD_F_OPEN,
	XLOADER_ERR_SD_F_LSEEK,
	XLOADER_ERR_SD_F_READ,

	XLOADER_ERR_IMG_ID_NOT_FOUND,
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
