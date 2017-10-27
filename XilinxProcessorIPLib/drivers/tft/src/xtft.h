/******************************************************************************
*
* Copyright (C) 2008 - 2015 Xilinx, Inc.  All rights reserved.
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
/***************************************************************************/
/**
*
* @file xtft.h
* @addtogroup tft_v5_0
* @{
* @details
*
* This header file contains the definitions and declarations for the
* high level driver to access the Xilinx TFT Controller Device.
*
* The device has the capability of displaying data onto a 640*480 VGA TFT
* screen. It can take up to 256K colors. There is no interrupt mode.
*
* The functions XTft_Setpixel and XTft_Getpixel are provided in the driver
* to write to and read from the individual pixels, the color values.
*
* These are generally stored in the assigned 2MB Video Memory which is
* configurable.
*
* Video Memory stores each pixel value in 32bits. Out of this 2MB memory which
* can hold 1024 pixels per line and 512 lines per frame data, only 640 pixels
* per line and 480 lines per frame are used.
*
* Each base color Red, Green, Blue is encoded using 6 bits which sums up to
* 18bits which is stored in the Dual port BRAM.
*
*
* <b>Initialization & Configuration</b>
*
* The XTft_Config structure is used by the driver to configure itself. This
* configuration structure is typically created by the tool-chain based on HW
* build properties.
*
* To support multiple runtime loading and initialization strategies employed
* by various operating systems, the driver instance can be initialized as
* follows:
*
*   - XTft_CfgInitialize(InstancePtr, CfgPtr, BaseAddress) - Uses a
*	configuration structure provided by the caller. If running in a system
*	with address translation, the provided virtual memory base address
*	replaces the physical address present in the configuration structure.
*
* <b>Interrupts</b>
*
* The TFT device supports a single interrupt which is generated for a Vsync
* pulse.
*
* This driver does not provide a Interrupt Service Routine (ISR) for the device.
* It is the responsibility of the application to provide one if needed.
*
* <b>RTOS Independence</b>
*
* This driver is intended to be RTOS and processor independent. It works
* with physical addresses only. Any needs for dynamic memory management,
* threads or thread mutual exclusion, virtual memory, or cache control must
* be satisfied by the layer above this driver.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
* 2.00a	 ktn   07/06/09	 Added XTft_IntrEnable(), XTft_IntrDisable()and,
*			 XTft_GetVsyncStatus() functions to access newly added
*			 Interrupt Enable and Status Register.
* 3.00a  ktn   10/22/09  Updated driver to use the HAL APIs/macros.
*		         Removed the macros XTft_mSetPixel and XTft_mGetPixel.
* 3.00a  bss   01/16/12  Updated driver to remove warnings from asserts.
* 3.01a  sg    05/30/12  Corrected the brace error introduced in
*			 XTft_GetPixel while changing it from macro to
*			 function for CR 647750.
* 3.02a  bss   11/30/12  CR 690338 - Corrected the brace error introduced in
*			 XTft_GetPixel for CR 647750.
* 4.00a  bss   01/25/13	 Added support for AXI TFT controller, this driver
*			 can only be used for AXI TFT controller
* 	 		 XTft_WriteReg and XTft_ReadReg functions are updated
*			 Removed all functionality associated with DCR access
*			 PlbAccess and DcrBaseAddr are removed from the
*			 XTft_Config config structure
* 4.01a  bss   11/01/13	 Modified driver tcl to retrieve C_BASEADDR/C_HIGHADDR
*			 CR#757359.
* 5.0   adk  19/12/13 Updated as per the New Tcl API's
* 6.0    sd  19/08/15  Updated the BaseAddress and VideoMemBaseAddr
*		         variables in XTft_Config to be UINTPTR to support
*			 64 bit addresses. Added AddrWidth to the
*			 XTft_Config structure which reflects the value of
*			 C_M_AXI_ADDR_WIDTH.
*			 Updated to tcl add the C_M_AXI_ADDR_WIDTH  parameter.
*			 Added XTFT_AR_LSB_OFFSET and XTFT_AR_MSB_OFFSET
*			 definitions to the xtft_hw.h file, these offsets
*			 are valid only when the Address Width is greater
*			 than 32 bits.
*       ms   01/23/17 Added xil_printf statement in main function for all
*                     examples to ensure that "Successfully ran" and "Failed"
*                     strings are available in all examples. This is a fix
*                     for CR-965028.
*       ms   03/17/17 Added readme.txt file in examples folder for doxygen
*                     generation.
* 6.1   ms   04/18/17 Modified tcl file to add suffix U for all macros
*                     definitions of tft in xparameters.h
*</pre>
*
****************************************************************************/
#ifndef XTFT_H /* prevent circular inclusions */
#define XTFT_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *******************************/
#include "xstatus.h"
#include "xtft_hw.h"

/************************** Constant Definitions ***************************/
/**
 * As the first bitmap available in the Character Bitmap array is "space"
 * whose value is 32 in the ASCII character table, this offset enables us to
 * move to this first character in the array. To achieve this we subtract
 * this offset from the char value actually received to the XTft_WriteChar
 * function. Similarly to move to any other character this offset must be
 * subtracted.
 */
#define XTFT_ASCIICHAR_OFFSET 32

/**
 * The default color is white for foreground and black for background.
 * These values can range from 0 to 0xFFFFFFFF as each color is ranging
 * from 0 to 3F. The default value for column and row is 0.
 */
#define XTFT_DEF_FGCOLOR 0x00FFFFFF	/**< Foreground Color - White */
#define XTFT_DEF_BGCOLOR 0x0		/**< Background Color - Black */
#define XTFT_DEF_COLVAL  0x0		/**< Default Column Value */
#define XTFT_DEF_ROWVAL	 0x0		/**< Default Row Value */

/**************************** Type Definitions *****************************/

/**
 * @struct XTft_Config
 *
 * This structure holds the Device base address, video memory base address
 * and Unique identifier of the device.
 */
typedef struct {

	u16 DeviceId;			/**< Unique ID of device */
	UINTPTR BaseAddress;		/**< Base address of device */
	UINTPTR VideoMemBaseAddr;	/**< Video Memory Base address */
	u32 AddrWidth;			/**< Address Width */
} XTft_Config;


/**
 * This structure is the base for whole of the operations that are to be
 * performed  on the TFT screen. With this we will get a handle to the driver
 * through which we access different members  like  base address, deviceID
 * and using them we navigate, fill colors etc.
 */
typedef struct {

	XTft_Config TftConfig;	/**< Instance of Config Structure */
	u32 IsReady;		/**< Status of Instance */
	u32 ColVal;		/**< Column position */
	u32 RowVal;		/**< Row position */
	u32 FgColor;		/**< Foreground Color */
	u32 BgColor;		/**< Background Color */

} XTft;

/***************** Macros (Inline Functions) Definitions *******************/

/************************** Function Prototypes ****************************/

/*
 * Initialization function in xtft_sinit.c.
 */
XTft_Config *XTft_LookupConfig(u16 DeviceId);

/*
 * Functions for basic driver operations in xtft.c.
 */
int XTft_CfgInitialize(XTft *InstancePtr, XTft_Config *ConfigPtr,
			 UINTPTR EffectiveAddr);

void XTft_SetPos(XTft *InstancePtr, u32 ColVal, u32 RowVal);
void XTft_SetPosChar(XTft *InstancePtr, u32 ColVal, u32 RowVal);
void XTft_SetColor(XTft *InstancePtr, u32 FgColor, u32 BgColor);
void XTft_SetPixel(XTft *InstancePtr, u32 ColVal, u32 RowVal, u32 PixelVal);
void XTft_GetPixel(XTft *InstancePtr, u32 ColVal, u32 RowVal, u32* PixelVal);

void XTft_Write(XTft *InstancePtr, u8 CharValue);

void XTft_Scroll(XTft *InstancePtr);
void XTft_ClearScreen(XTft *InstancePtr);
void XTft_FillScreen(XTft* InstancePtr, u32 ColStartVal,
			 u32 RowStartVal,u32 ColEndVal, u32 RowEndVal,
			 u32 PixelVal);

void XTft_EnableDisplay(XTft *InstancePtr);
void XTft_DisableDisplay(XTft *InstancePtr);
void XTft_ScanReverse(XTft* InstancePtr);
void XTft_ScanNormal(XTft* InstancePtr);
void XTft_SetFrameBaseAddr(XTft *InstancePtr, UINTPTR NewFrameBaseAddr);
void XTft_WriteReg(XTft* InstancePtr, u32 RegOffset, u32 Data);
u32 XTft_ReadReg(XTft* InstancePtr, u32 RegOffset);
void XTft_IntrEnable(XTft* InstancePtr);
void XTft_IntrDisable(XTft* InstancePtr);
int XTft_GetVsyncStatus(XTft* InstancePtr);

/************************** Variable Definitions ***************************/

/************************** Function Definitions ***************************/

#ifdef __cplusplus
}
#endif

#endif /* XTFT_H */

/** @} */
