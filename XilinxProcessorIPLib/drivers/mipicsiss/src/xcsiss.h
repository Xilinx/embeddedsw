/******************************************************************************
 *
 * Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file xcsiss.h
*
* @mainpage mipicsiss_v1_0
*
* This is main header file of the Xilinx MIPI CSI Rx Subsystem driver
*
* <b>MIPI CSI Rx Subsystem Overview</b>
*
* MIPI CSI Subsystem is collection of IP cores to control, receive and
* translate data received from a MIPI CSI Transmitter. The MIPI CSI2 Rx
* Subsystem is a plug-in solution for interfacing with MIPI CSI based image
* sensors and rest of the video pipeline. It hides all the complexities of
* programming the underlying cores from end user.
*
* <b>Subsystem Features</b>
*
* MIPI CSI Rx Subsystem supports following features
* 	- Support for 1 to 4 PPI Lanes.
* 	- Line rates ranging from 80 to 1500 Mbps.
* 	- Different data type support(RAW,RGB,YUV).
* 	- AXI IIC support for CCI Interface.
* 	- Using existing AXI IIC for CCI interface support for better
* 	  understanding & compatibility with other IIC’s (if any) used
* 	  in the system
*	- Filtering of packets based on Virtual channel ID.
*	- Single,Dual,Quad pixel support at output interface compliant
*	  to UG934 format.
*
* <b>Subsystem Configurations</b>
*
* The GUI in IPI allows for the following configurations
* 	- Lanes ( 1 to 4 )
* 	- Pixel Format (All RAW and RGB, only YUV422 8bit)
* 	- Virtual Channel (to filter or allow all from interlaced streams)
* 	- Number of Pixels per clock (1, 2, 4)
* 	- DPHY with/without Register interface
* 	- Line Rate
* 	- Buffer Depth
* 	- Embedded Non Image data (if needed)
* 	- Add IIC to subsystem (if required)
*
* The IIC can be added if the system doesn't contain an IIC or if a dedicated
* IIC is to be used for MIPI CSI Rx Subsystem. In order to reduce resource
* usage, the DPHY can be configured to be without register interface with
* fixed functions. Static configuration parameters are stored in xcsiss_g.c
* file that gets generated when compiling the board support package (BSP).
* A table is defined where each entry contains configuration information
* for the instances of the subsystem in the design. This information includes
* the elected configuration, sub-cores used and their device ID, base addresses
* of memory mapped devices and address range available for subsystem
* frame/field buffers.
*
* The subsystem driver itself always includes the full software stack
* irrespective of the configuration selected. Generic API's are provided to
* interact with the subsystem and/or with the included sub -cores.
* At run-time the subsystem will query the static configuration and configures
* itself for supported use cases
*
* <b>Subsystem Driver Description</b>
*  The subsystem driver provides an abstraction on top of the CSI and DPHY
*  drivers.
*
*  The IIC instance (if present) is shared with application
*  and can be controlled using the AXI IIC driver.
*
* <b>Pre-Requisite's</b>
*
* <b>Subsystem Driver Usage</b>
*
*
* <b>Memory Requirement</b>
*
*
* <b>Interrupt Service</b>
*
*
* <b> Virtual Memory </b>
*
* This driver supports Virtual Memory. The RTOS is responsible for calculating
* the correct device base address in Virtual Memory space.
*
* <b> Threads </b>
*
* This driver is not thread safe. Any needs for threads or thread mutual
* exclusion must be satisfied by the layer above this driver.
*

* <b>Asserts</b>
*
* Asserts are used within all Xilinx drivers to enforce constraints on argument
* values. Asserts can be turned off on a system-wide basis by defining, at
* compile time, the NDEBUG identifier.  By default, asserts are turned on and
* it is recommended that application developers leave asserts on during
* development.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  vs    07/25/15   Initial Release

* </pre>
*
******************************************************************************/


#ifndef XCSISS_H_   /* prevent circular inclusions */
#define XCSISS_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#ifdef __MICROBLAZE__
#include "xenv.h"
#else
#include "xil_types.h"
#include "xil_cache.h"
#endif

#include "xcsi.h"
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
#include "xdphy.h"
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
#include "xiic.h"
#endif

/**
*
* Callback type which acts as a wrapper on top of CSI Callback.
*
* @param	CallBackRef is a callback reference passed in by the upper
*		layer when setting the callback functions, and passed back to
*		the upper layer when the callback is invoked.
* @param	Mask is a bit mask indicating the cause of the event. For
*		current core version, this parameter is "OR" of 0 or more
*		XCSI_ISR_*_MASK constants defined in xcsi_hw.h.
*
* @return	None.
*
* @note		None.
*
 *****************************************************************************/
typedef void (*XCsiSs_CallBack)(void *CallBackRef, u32 Mask);

/**
 * Sub-Core Configuration Table
 */

typedef struct {
	u32 IsPresent; /**< Flag to indicate if sub-core is present in the design*/
	u32 DeviceId;  /**< Device ID of the sub-core */
	u32 AddrOffset;/**< sub-core offset from subsystem base address */
} SubCore;

/**
 * MIPI CSI Rx Subsystem configuration structure.
 * Each subsystem device should have a configuration structure associated
 * that defines the MAX supported sub-cores within subsystem
 */

typedef struct {
	u32 DeviceId;		/**< DeviceId is the unique ID  of the device */
	u32 BaseAddr;		/**< BaseAddress is the physical base address of the
				     subsystem address range */
	u32 HighAddr;		/**< HighAddress is the physical MAX address of the
				     subsystem address range */
	u32 IsIICPresent;	/**< Flag for IIC presence in subsystem */
	u32 LanesPresent;	/**< Number of PPI Lanes in the design */
	u32 PixelCount;		/**< Number of Pixels per clock 1,2,4 */
	u32 PixelFormat;	/**< The pixel format selected from all RGB, RAW
				     and YUV422 8bit options */
	u32 VCNo;		/**< Number of Virtual Channels supported by system.
				     This can range from 1 - 4 to ALL */
	u32 CsiBuffDepth;	/**< Line buffer Depth set */
	u32 IsEmbNonImgPresent; /**< Flag for presence of Embedded Non Image data */
	u32 IsDPHYRegIntfcPresent; /**< Flag for DPHY register interface presence */
	u32 DphyLineRate;	/**< DPHY Line Rate ranging from 80-1500 Mbps */
	SubCore IicInfo;	/**< Sub-core instance configuration */
	SubCore CsiInfo;	/**< Sub-core instance configuration */
	SubCore DphyInfo;	/**< Sub-core instance configuration */
} XCsiSs_Config;

/**
* User input structure
*/
typedef struct {
	u32 IntrRequest;	/**< Interrupts subscribed for in CSI */
	u8 Lanes;		/**< Active Lanes to be used in CSI */
} XCsiSs_UsrOpt;

/**
 * The XCsiSs driver instance data. The user is required to allocate a variable
 * of this type for every XCsiSs device in the system. A pointer to a variable
 * of this type is then passed to the driver API functions.
 */
typedef struct {
	XCsiSs_Config Config;	/**< Hardware configuration */
	u32 IsReady;		/**< Device and the driver instance are
                                     initialized */
	XCsi  *CsiPtr;		/**< handle to sub-core driver instance */
#if (XPAR_XDPHY_NUM_INSTANCES > 0)
	XDphy *DphyPtr;		/**< handle to sub-core driver instance */
#endif
#if (XPAR_XIIC_NUM_INSTANCES > 0)
	XIic  *IicPtr;		/**< handle to sub-core driver instance */
#endif
	XCsiSs_UsrOpt UsrOpt; 	/**< User options registered here */
	XCsi_ClkLaneInfo ClkInfo;
	XCsi_DataLaneInfo DLInfo[XCSI_MAX_LANES];
	XCsi_SPktData SpktData;
} XCsiSs;

typedef enum {
	XCSISS_RESET_SUBCORE_IIC,
	XCSISS_RESET_SUBCORE_CSI,
	XCSISS_RESET_SUBCORE_DPHY,
	XCSISS_RESET_SUBCORE_ALL
} XCsiSs_ResetType;

/************************** Function Prototypes ******************************/
XCsiSs_Config* XCsiSs_LookupConfig(u32 DeviceId);

int XCsiSs_CfgInitialize(XCsiSs *InstancePtr,
                           XCsiSs_Config *CfgPtr,
                           u32 EffectiveAddr);

#if (XPAR_XIIC_NUM_INSTANCES > 0)
/* return NULL if not preset else ptr to IIC instance inside Subsystem*/
XIic* XCsiSs_GetIicInstance(XCsiSs *InstancePtr);
#endif

/* Configure lanes  and interrupts in one go*/
u32 XCsiSs_Configure(XCsiSs *InstancePtr, XCsiSs_UsrOpt *UsrOpt);

XCsiSs_UsrOpt * XCsiSs_GetUsrOpt(XCsiSs *InstancePtr);

void XCsiSs_Activate(XCsiSs *InstancePtr, u8 Flag);

/* reset CSI and DPHY only as IIC is in application's domain */
u32 XCsiSs_Reset(XCsiSs *InstancePtr, XCsiSs_ResetType Type);

/* Reports which cores are present in subsys */
void XCsiSs_ReportCoreInfo(XCsiSs *InstancePtr);

u32 XCsiSs_SelfTest(XCsiSs *InstancePtr);

void XCsiSs_GetLaneInfo(XCsiSs *InstancePtr);

void XCsiSs_GetShortPacket(XCsiSs *InstancePtr);

/* Interrupt functions in xdptxss_intr.c */
void XCsiSs_IntrHandler(void *InstancePtr);

u32 XCsiSs_SetCallBack(XCsiSs *InstancePtr, u32 HandlerType,
			void *CallBackFunc, void *CallBackRef);

/************************** Variable Declarations ****************************/

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
