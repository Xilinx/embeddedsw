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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
	XPLMI_ERR_DMA_CFG, /* 0x101 */
	XPLMI_ERR_DMA_SELFTEST, /* 0x102 */

	XPLMI_ERR_IOMOD_INIT, /* 0x103 */
	XPLMI_ERR_IOMOD_START, /* 0x104 */
	XPLMI_ERR_IOMOD_CONNECT, /* 0x105 */

	XPLMI_ERR_MODULE_MAX, /* 0x106 */
	XPLMI_ERR_CMD_APIID, /* 0x107 */
	XPLMI_ERR_CMD_HANDLER_NULL, /* 0x108 */
	XPLMI_ERR_CMD_HANDLER, /* 0x109 */
	XPLMI_ERR_RESUME_HANDLER, /* 0x10A */

	XPLMI_ERR_CDO_HDR_ID, /* 0x10B */
	XPLMI_ERR_CDO_CHECKSUM, /* 0x10C */

	/** Status codes used in PLM */
	XPLM_ERR_TASK_CREATE = 0x200,
	XPLM_ERR_PM_MOD, /* 0x201 */
	XPLM_ERR_LPD_MOD, /* 0x202 */
	XPLM_ERR_EXCEPTION, /* 0x203 */

	/** Status codes used in XLOADER */
	XLOADER_UNSUPPORTED_BOOT_MODE = 0x300,
	XLOADER_ERR_BOOTHDR, /* 0x301 */
	XLOADER_ERR_IMGHDR_TBL, /* 0x302 */
	XLOADER_ERR_IMGHDR, /* 0x303 */
	XLOADER_ERR_PRTNHDR, /* 0x304 */
	XLOADER_ERR_WAKEUP_A72_0, /* 0x305 */
	XLOADER_ERR_WAKEUP_A72_1, /* 0x306 */
	XLOADER_ERR_WAKEUP_R5_0, /* 0x307 */
	XLOADER_ERR_WAKEUP_R5_1, /* 0x308 */
	XLOADER_ERR_WAKEUP_R5_L, /* 0x309 */
	XLOADER_ERR_PL_NOT_PWRUP, /* 0x30A */
	XLOADER_ERR_UNSUPPORTED_OSPI, /* 0x30B */
	XLOADER_ERR_UNSUPPORTED_OSPI_SIZE, /* 0x30C */
	XLOADER_ERR_OSPI_INIT, /* 0x30D */
	XLOADER_ERR_OSPI_CFG, /* 0x30E */
	XLOADER_ERR_OSPI_SEL_FLASH, /* 0x30F */
	XLOADER_ERR_OSPI_READID, /* 0x310 */
	XLOADER_ERR_OSPI_READ, /* 0x311 */
	XLOADER_ERR_OSPI_4BMODE, /* 0x312 */

	XLOADER_ERR_QSPI_READ_ID, /* 0x313 */
	XLOADER_ERR_UNSUPPORTED_QSPI, /* 0x314 */
	XLOADER_ERR_QSPI_INIT, /* 0x315 */
	XLOADER_ERR_QSPI_MANUAL_START, /* 0x316 */
	XLOADER_ERR_QSPI_PRESCALER_CLK, /* 0x317 */
	XLOADER_ERR_QSPI_CONNECTION, /* 0x318 */
	XLOADER_ERR_QSPI_READ, /* 0x319 */
	XLOADER_ERR_QSPI_LENGTH, /* 0x31A */

	XLOADER_ERR_SD_INIT, /* 0x31B */
	XLOADER_ERR_SD_F_OPEN, /* 0x31C */
	XLOADER_ERR_SD_F_LSEEK, /* 0x31D */
	XLOADER_ERR_SD_F_READ, /* 0x31E */

	XLOADER_ERR_IMG_ID_NOT_FOUND, /* 0x31F */
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
