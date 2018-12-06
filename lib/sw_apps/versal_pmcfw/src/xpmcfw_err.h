/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
* @file xpmcfw_error.h
*
* This is the header file which contains error codes for the PMCFW.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPMCFW_ERR_H
#define XPMCFW_ERR_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/
#include "xstatus.h"

/************************** Constant Definitions *****************************/
/**
 * @name PMCFW error codes description
 *
 * XXYY - Error code format
 * 8 bit error code for PMC FW, 8 bit error code for Drivers
 * XX - PMC FW error code
 * YY - Drivers / PSU Init error code
 */

#define XPMCFW_SUCCESS					(XST_SUCCESS)
#define XPMCFW_FAILURE					(XST_FAILURE)
#define XPMCFW_SUCCESS_NOT_PRTN_OWNER			(0x100)
#define XPMCFW_ERR_UNSUPPORTED_BOOT_MODE		(0x200)
#define XPMCFW_JTAG_SUCCESS				(0x300)
#define XPMCFW_POST_BOOT				(0x7FFF)

#define XPMCFW_ERR_QSPI_READ_ID				(0x1000)
#define XPMCFW_ERR_UNSUPPORTED_QSPI			(0x1100)
#define XPMCFW_ERR_QSPI_INIT				(0x1200)
#define XPMCFW_ERR_QSPI_LENGTH				(0x1300)
#define XPMCFW_ERR_QSPI_READ				(0x1400)
#define XPMCFW_ERR_QSPI_CONNECTION			(0x1500)
#define XPMCFW_ERR_QSPI_MANUAL_START			(0x1600)
#define XPMCFW_ERR_QSPI_PRESCALER_CLK			(0x1700)
#define XPMCFW_ERR_PMC_CDO_INIT				(0x1800)
#define XPMCFW_ERR_EXCEPTION				(0x1900)
#define XPMCFW_ERR_DDR_TEST				(0x1A00)
#define XPMCFW_ERR_SD_INIT				(0x1B00)
#define XPMCFW_ERR_SD_F_OPEN				(0x1C00)
#define XPMCFW_ERR_SD_F_LSEEK				(0x1D00)
#define XPMCFW_ERR_SD_F_READ				(0x1E00)
#define XPMCFW_ERR_CFI_LOAD				(0x1F00)
#define XPMCFW_ERR_SCAN_CLEAR_LPD			(0x2100)
#define XPMCFW_ERR_DMA_LOOKUP				(0x2200)
#define XPMCFW_ERR_DMA_CFG				(0x2300)
#define XPMCFW_ERR_DMA_SELFTEST				(0x2400)
#define XPMCFW_ERR_CFU_LOOKUP				(0x2500)
#define XPMCFW_ERR_CFU_CFG				(0x2600)
#define XPMCFW_ERR_CFU_SELFTEST				(0x2700)
#define XPMCFW_ERR_CFRAME_LOOKUP			(0x2800)
#define XPMCFW_ERR_CFRAME_CFG				(0x2900)
#define XPMCFW_ERR_CFRAME_SELFTEST			(0x2A00)
#define XPMCFW_ERR_IOMOD_INIT				(0x2B00)
#define XPMCFW_ERR_IOMOD_START				(0x2C00)
#define XPMCFW_ERR_IOMOD_CONNECT			(0x2D00)
#define XPMCFW_ERR_SSIT_BOOT				(0x2E00)
#define XPMCFW_ERR_PL_NOT_PWRUP				(0x2F00)
#define XPMCFW_ERR_SECURE_ISNOT_EN			(0x3000)
#define XPMCFW_ERR_OSPI_INIT				(0x4000)
#define XPMCFW_ERR_OSPI_CFG					(0x4100)
#define XPMCFW_ERR_UNSUPPORTED_OSPI			(0x4200)
#define XPMCFW_ERR_UNSUPPORTED_OSPI_SIZE	(0x4300)
#define XPMCFW_ERR_AUTHDEC_NOTALLOW			(0x4400)
#define XPMCFW_ERR_MASK				(0xFF00U)
#define XPMCFW_ERR_MODULE_MASK			(0xFFU)
#define XPMCFW_UPDATE_ERR(PmcFwErr, ModuleErr)		\
		((PmcFwErr&XPMCFW_ERR_MASK) + \
		 (ModuleErr&XPMCFW_ERR_MODULE_MASK))

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/


#ifdef __cplusplus
}
#endif

#endif  /* XPMCFW_ERR_H */
