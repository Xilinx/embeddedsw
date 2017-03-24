/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xfsbl_misc_drivers.h
*
* This is the header file which contains declarations for wrapper functions
* for WDT, CSUDMA drivers
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   10/21/13 Initial release
* 2.0   bv   12/02/16 Made compliance to MISRAC 2012 guidelines
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_MISC_DRIVERS_H
#define XFSBL_MISC_DRIVERS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"

/************************** Constant Definitions *****************************/

#ifdef XFSBL_WDT_PRESENT
#define XFSBL_WDT_DEVICE_ID			(XPAR_XWDTPS_0_DEVICE_ID)
#define XFSBL_WDT_EXPIRE_TIME			(100U)
#define XFSBL_WDT_CRV_SHIFT			(12U)
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
#ifdef XFSBL_WDT_PRESENT
u32 XFsbl_InitWdt(void);
void XFsbl_StopWdt(void);
void XFsbl_RestartWdt(void);
#endif

/************************** Variable Definitions *****************************/

typedef struct {
	u32 DeviceBaseAddress; /**< Flash device base address */
	u32 (*DeviceInit) (u32 DeviceFlags);
		/**< Function pointer for Device initialization code */
	u32 (*DeviceCopy) (u32 SrcAddress, PTRSIZE DestAddress, u32 Length);
		/**< Function pointer for device copy */
	u32 (*DeviceRelease) ();
		/**< Function pointer for device release */
} XFsblPs_DeviceOps;


/**
 * SD driver functions
 */
#if (defined(XFSBL_SD_0) || defined(XFSBL_SD_1))
u32 XFsbl_SdInit(u32 DeviceFlags);
u32 XFsbl_SdCopy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length);
u32 XFsbl_SdRelease(void );
#endif

/**
 * NAND driver functions
 */
#ifdef XFSBL_NAND
u32 XFsbl_NandInit(u32 DeviceFlags);
u32 XFsbl_NandCopy(u32 SrcAddress, PTRSIZE DestAddress, u32 Length);
u32 XFsbl_NandRelease(void );
#endif

/**
 * PMU firmware initialization
 */
u32 XFsbl_PmInit(void);

#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_MISC_DRIVERS_H */
