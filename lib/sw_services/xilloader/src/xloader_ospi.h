/******************************************************************************
* Copyright (C) 2018 - 2019 Xilinx, Inc. All rights reserved.
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
* @file xloader_ospi.h
*
* This is the header file which contains ospi declarations for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2018 Initial release
* 1.01  bsv  09/15/2019 Added Read Config and Write Config registers
* </pre>
*
* @note
*
******************************************************************************/
#ifndef XLOADER_OSPI_H
#define XLOADER_OSPI_H

#ifdef __cplusplus
extern "C" {
#endif


/***************************** Include Files *********************************/
#include "xplmi_hw.h"

#ifdef XLOADER_OSPI
#include "xparameters.h"	/* SDK generated parameters */
#include "xospipsv.h"		/* OSPIPSV device driver */
#include "xplmi_status.h"	/* PLMI error codes */

/************************** Constant Definitions *****************************/

/**
 * Flash connection type as defined in PCW
 */
#define FLASH_SIZE_16MB				(0x1000000U)
#define MICRON_ID				(0x20U)
#define FLASH_SIZE_ID_512M			(0x20U)
#define FLASH_SIZE_512M				(0x4000000U)
#define FLASH_SIZE_1G				(0x8000000U)
#define FLASH_SIZE_2G                   	(0x10000000U)
#define READ_CMD_OCTAL_4B    			(0x7CU)
#define READ_ID					(0x9FU)
#define MICRON_INDEX_START			(0x0U)
#define WRITE_DISABLE_CMD			(0x4U)
#define OSPI_WRITE_ENABLE_CMD		(0x6U)
#define ENTER_4B_ADDR_MODE      	(0xB7U)
#define EXIT_4B_ADDR_MODE       	(0xE9U)
#define READ_FLAG_STATUS_CMD		(0x70U)
#define WRITE_CONFIG_REG			(0x81U)
#define READ_CONFIG_REG				(0x85U)
/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0x5B
 * Byte 2 is second byte of Device ID describes flash size:
 * 512Mbit : 0x1A
 */
#define	MICRON_OCTAL_ID_BYTE0		(0x2CU)
#define MICRON_OCTAL_ID_BYTE2_512	(0x1AU)
#define MICRON_OCTAL_ID_BYTE2_1G	(0x1BU)
#define MICRON_OCTAL_ID_BYTE2_2G	(0x1CU)
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int XLoader_OspiInit(u32 DeviceFlags);
XStatus XLoader_OspiCopy(u32 SrcAddr, u64 DestAddress, u32 Length, u32 Flags);
int XLoader_OspiRelease(void );
int XLoader_FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, u32 Enable);
int XLoader_FlashSetDDRMode(XOspiPsv *OspiPsvPtr);
/************************** Variable Definitions *****************************/


#endif /* end of XLOADER_OSPI */

#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_QSPI_H */
