/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file xdphy.h
*
* @mainpage dphy_v1_0
*
* This file contains the implementation of the MIPI DPHY Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
* The DPHY currently supports the MIPI�Alliance Specification
* for DPHY Version 1.1.
*
* It is capable of synchronous transfer at high speed mode at 80-1500 Mbps
* It has one clock lane and up to 4 data lanes. These lanes are unidirectional.
* It can do asynchronous transfer at upto 10 Mbps in low power mode. The clock
* lane can be in low power mode or high speed mode whereas the data lanes
* can be in Low power, High power or Escape mode.
*
* The programmable parameters like IDelay, Wakeup, HS Timeout, Esc Timeout are
* present and various status like Stop state, Error detected, ULPS state,etc
* are available through the status register
*
* <b> Examples </b>
*
* There is currently no example code provided.
*
* <b>RTOS Independence</b>
*
* This driver is intended to be RTOS and processor independent.  It works with
* physical addresses only.  Any needs for dynamic memory management, threads or
* thread mutual exclusion, virtual memory, or cache control must be satisfied
* by the layer above this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a vs   07/08/15 First release
* </pre>
*
******************************************************************************/

#ifndef XDPHY_H_   /* prevent circular inclusions */
#define XDPHY_H_

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

#include "xdphy_hw.h"

/************************** Constant Definitions *****************************/

/** @name DPHY Modes
 * @{
*/
#define XDPHY_MODE_MIN 		0 /**< Lower limit for Mode */
#define XDPHY_LOW_POWER_MODE 	0 /**< Lane in Low Power Mode */
#define XDPHY_HIGH_POWER_MODE	1 /**< Lane in High Power Mode */
#define XDPHY_ESCAPE_MODE	2 /**< Lane in Escape Mode */
#define XDPHY_MODE_MAX 		2 /**< Upper Limit for mode */
/*@}*/

/** @name DPHY Info Handles
 * @{
*/
#define XDPHY_HANDLE_MIN 	0 /**< Lower Bound for XDPHY_HANDLE */
#define XDPHY_HANDLE_IDELAY	0 /**< Handle for IDELAY Reg */
#define XDPHY_HANDLE_INIT_TIMER	1 /**< Handle for Initialization Timer */
#define XDPHY_HANDLE_WAKEUP	2 /**< Handle for Wakeup timer */
#define XDPHY_HANDLE_HSTIMEOUT	3 /**< Handle for HS Timeout */
#define XDPHY_HANDLE_ESCTIMEOUT	4 /**< Handle for Escape Timeout */
#define XDPHY_HANDLE_CLKLANE	5 /**< Handle for Clock Lane */
#define XDPHY_HANDLE_DLANE0	6 /**< Handle for Data Lane 0 */
#define XDPHY_HANDLE_DLANE1	7 /**< Handle for Data Lane 1 */
#define XDPHY_HANDLE_DLANE2	8 /**< Handle for Data Lane 2 */
#define XDPHY_HANDLE_DLANE3	9 /**< Handle for Data Lane 3 */
#define XDPHY_HANDLE_MAX 	9 /**< Upper Bound for XDPHY_HANDLE */
/*@}*/
/**************************** Macros Definitions *****************************/
/** @name DPHY HSTIMEOUT range
 * @{
*/
#define XDPHY_HS_TIMEOUT_MIN_VALUE	10000UL
#define XDPHY_HS_TIMEOUT_MAX_VALUE	65541UL
/*@}*/

/** @name DPHY Flags to Enable or Disable core
 * @{
*/
#define XDPHY_ENABLE_FLAG 	1
#define XDPHY_DISABLE_FLAG 	0
/*@}*/

/************************* Bit field operations ****************************/
/* Setting and resetting bits */
/****************************************************************************/
/**
*
* XDPHY_BIT_SET is used to set bit in a register.
*
* @param 	BaseAddress is a base address of IP
*
* @param 	RegisterOffset is offset where the register is present
*
* @param 	BitName is the part of the bitname before _MASK
*
* @return 	None
*
* @note 	None
*
****************************************************************************/
#define XDPHY_BIT_SET(BaseAddress, RegisterOffset, BitName) \
	do \
	{ \
		XDphy_WriteReg((BaseAddress), (RegisterOffset), \
			(XDphy_ReadReg((BaseAddress), (RegisterOffset)) | \
				(BitName##_MASK))); \
	} \
	while (0);

/****************************************************************************/
/**
*
* XDPHY_BIT_RESET is used to reset bit in a register.
*
* @param 	BaseAddress is a base address of IP
*
* @param 	RegisterOffset is offset where the register is present
*
* @param 	BitName is the part of the bitname before _MASK
*
* @return 	None
*
* @note 	None
*
****************************************************************************/
#define XDPHY_BIT_RESET(BaseAddress, RegisterOffset, BitName) \
	do \
	{ \
		XDphy_WriteReg((BaseAddress),(RegisterOffset), \
			(XDphy_ReadReg((BaseAddress),(RegisterOffset)) &\
				~(BitName##_MASK))); \
	} \
	while (0);

/* Get a bitfield/s value. To be used for reading */
/****************************************************************************/
/**
*
* XDPHY_GET_BITFIELD_VALUE is used to get the value of bitfield from register.
*
* @param 	BaseAddress is a base address of IP
*
* @param 	RegisterOffset is offset where the register is present
*
* @param 	BitName is the part of the bitname before _MASK or _OFFSET
*
* @return 	Bit Field Value in u32 format
*
* @note 	C-style signature:
*		u32 XDPHY_GET_BITFIELD_VALUE(u32 BaseAddress,
*					     u32 RegisterOffset,
*					     u32 BitMask,
*					     u32 BitOffset)
*		The bit mask and bit offset are generated from the common part
*		of the bit name by appending _MASK and _OFFSET
*
****************************************************************************/
#define XDPHY_GET_BITFIELD_VALUE(BaseAddress, RegisterOffset, BitName) \
		((XDphy_ReadReg((BaseAddress), (RegisterOffset)) & \
			(BitName##_MASK)) >> (BitName##_OFFSET))

/****************************************************************************/
/**
*
* XDPHY_SET_BITFIELD_VALUE is used to set the value of bitfield from register.
*
* @param 	BaseAddress is a base address of IP
*
* @param 	RegisterOffset is offset where the register is present
*
* @param 	BitName is the part of the bitname before _MASK or _OFFSET
*
* @param 	Value is to be set. Passed in u32 format.
*
* @return 	None
*
* @note 	C-style signature:
*		u32 XDPHY_SET_BITFIELD_VALUE(u32 BaseAddress,
*					     u32 RegisterOffset,
*					     u32 BitMask,
*					     u32 BitOffset,
*					     u32 Value)
*
*		The bit mask and bit offset are generated from the common part
*		of the bit name by appending _MASK and _OFFSET
*
****************************************************************************/
#define XDPHY_SET_BITFIELD_VALUE(BaseAddress, RegisterOffset, BitName, Value) \
		XDphy_WriteReg((BaseAddress), (RegisterOffset), \
			((XDphy_ReadReg((BaseAddress), (RegisterOffset)) & \
			  ~(BitName##_MASK)) | ((Value) << (BitName##_OFFSET))))

/**************************** Type Definitions *******************************/

/**
* The configuration structure for DPHY
*
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
	u32 DeviceId; /**< Device Id */
	u32 BaseAddr; /**< Base address of DPHY */

	u32 IsRx; /**< TX or RX Mode */
	u32 IsRegisterPresent; /**< Is register access allowed */
	u32 MaxLanesPresent; /**< Number of Lanes. Range 1 - 4 */
	u32 EscClkPeriod; /**< Escape Clock Peroid */
	u32 EscTimeout; /**< Escape Timeout */
	u32 HSLineRate; /**< High Speed Line Rate */
	u32 HSTimeOut; /**< Max Frame Length  */
	u32 InitTime; /**< Lane initialization time */
	u32 LPXPeriod;
	u32 StableClkPeriod;
	u32 TxPllClkinPeriod;
	u32 Wakeup; /**< Time to exit ULPS mode */

} XDphy_Config;

/**
* The XDphy Controller driver instance data.
* An instance must be allocated for each Dphy in use.
*/
typedef struct {
	XDphy_Config Config; /**< Hardware Configuration */

	u32 IsReady; /**< Driver is ready */
} XDphy;

/************************** Function Prototypes ******************************/

XDphy_Config *XDphy_LookupConfig(u32 DeviceId);

u32 XDphy_CfgInitialize(XDphy *InstancePtr, XDphy_Config *Config,
			u32 EffectiveAddr);

u32 XDphy_Configure(XDphy *InstancePtr, u8 Handle, u32 Value);

u32 XDphy_GetInfo(XDphy *InstancePtr, u8 Handle);

void XDphy_Reset(XDphy *InstancePtr);

void XDphy_ClearDataLane(XDphy *InstancePtr, u8 DataLane, u32 Mask);

u32 XDphy_GetClkLaneStatus(XDphy *InstancePtr);

u32 XDphy_GetClkLaneMode(XDphy *InstancePtr);

u32 XDphy_GetDataLaneStatus(XDphy *InstancePtr, u8 DataLane);

u32 XDphy_GetDataLaneMode(XDphy *InstancePtr, u8 DataLane);

void XDphy_Activate(XDphy *InstancePtr, u8 Flag);

u32 XDphy_SelfTest(XDphy *InstancePtr);

u8 XDphy_GetRegIntfcPresent(XDphy *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
