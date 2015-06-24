/******************************************************************************
*
* Copyright (C) 2007 - 2014 Xilinx, Inc.  All rights reserved.
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
* @file xilflash_properties.h
*
* This file contains various data common to flash devices most of which can be
* derived from the CFI query.
*
* @note
*
* There is no implementation file with this component.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  10/25/07 First release
* 1.00a mta  10/25/07 Updated to flash library
* 1.01a ksu  04/10/08 Added support for AMD CFI Interface
* 1.02a ksu  06/16/09 Changed size of DeviceID in XFlashPartID structure
* 2.00a ktn  12/04/09 Updated to use the HAL processor APIs/macros
* 3.01a srt  03/02/12 Added support for Micron G18 Flash device to fix
*		      CRs 648372, 648282.
* 3.02a srt  05/30/12 Changed Implementation for Micron G18 Flash, which
*		      fixes the CR 662317.
*		      CR 662317 Description - Xilinx Platform Flash on ML605
*		      fails to work.
* </pre>
*
***************************************************************************/

#ifndef XFLASH_PROPERTIES_H	/* prevent circular inclusions */
#define XFLASH_PROPERTIES_H	/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xstatus.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**
 * Flash timing
 */
typedef struct {
	u16 WriteSingle_Us;	/**< Time to program a single word unit
				  *   Units are in microseconds */
	u16 WriteBuffer_Us;	/**< Time to program the contents of the
				  *  write buffer. Units are in microseconds
				  *  If the part does not support write
				  *  buffers, then this value should be
				  *  zero */
	u16 EraseBlock_Ms;	/**< Time to erase a single block
				  *  Units are in milliseconds */
	u16 EraseChip_Ms;	/**< Time to perform a chip erase
				  *  Units are in milliseconds */
} XFlashTiming;

/**
 * Flash identification
 */
typedef struct {
	u8 ManufacturerID;	/**< Manufacturer of parts */
	u16 DeviceID;		/**< Part number of manufacturer */
	u16 CommandSet;		/**< Command algorithm used by part. Choices
				  *  are defined in XFL_CMDSET constants */
} XFlashPartID;

/**
 * Programming parameters
 */
typedef struct {
	u32 WriteBufferSize;	 	/**< Number of bytes that can be
					  * programmed at once */
	u32 WriteBufferAlignmentMask;	/**< Alignment of the write buffer */
	u32 EraseQueueSize;		/**< Number of erase blocks that can be
					  *  queued up at once */
} XFlashProgCap;

/**
 * Consolidated parameters
 */
typedef struct {
	XFlashPartID PartID;		/**< Uniquely identifies the part */
	XFlashTiming TimeTypical;	/**< Typical timing data */
	XFlashTiming TimeMax;		/**< Worst case timing data */
	XFlashProgCap ProgCap;		/**< Programming capabilities */
} XFlashProperties;

/**
 * Flash Specific Command Set
 */
typedef struct {
	u32 WriteBufferCommand;
	u32 ProgramCommand;
} XFlashCommandSet;

/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
