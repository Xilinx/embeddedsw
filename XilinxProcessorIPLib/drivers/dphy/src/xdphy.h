/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
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
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdphy.h
*
* @addtogroup dphy_v1_0
* @{
* @details
*
* This file contains the implementation of the MIPI DPHY Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
* <b>MIPI DPHY Overview</b>
*
* The DPHY currently supports the MIPI?Alliance Specification
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
* <b>Core Features</b>
*
* The GUI in IPI allows for the following configurations
*	- Lanes ( 1 to 4 )
*	- Line Rate (80 - 1500 Mbps)
*	- Data Flow direction (Tx or Rx)
*	- Escape Clock (10 - 20 Mhz)
*	- LPX period (50 - 100 ns)
*	- Enable register interface
*	- HS Timeout in Bytes (1000 - 65541)
*	- Escape Timeout in ns (800 - 25600)
*
* <b>Software Initialization & Configuration</b>
*
* By default, the DPHY core is initialized and ready.
*
* The application needs to do following steps in order for preparing the
* MIPI DPHY core to be ready.
*
* - Call XDphy_LookupConfig using a device ID to find the core
*   configuration.
* - Call XDphy_CfgInitialize to initialize the device and the driver
*   instance associated with it.
* - Individual parameters can be configured by sending values with
*   appropriate handles.
*
* <b>Interrupts</b>
*
* There are no interrupts from the DPHY.
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
* <b>Building the driver</b>
*
* The DPHY driver is composed of source files and doesn't depend on any other
* drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/08/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
*     ms  01/23/17 Modified xil_printf statement in main function for all
*                  examples to ensure that "Successfully ran" and "Failed"
*                  strings are available in all examples. This is a fix
*                  for CR-965028.
* 1.2 vsa 03/02/17 Add support for HS_SETTLE register
*     ms  03/17/17 Added readme.txt file in examples folder for doxygen
*                  generation.
*     ms  04/05/17 Modified Comment lines in functions of dphy
*                  examples to recognize it as documentation block
*                  for doxygen generation of examples.
* </pre>
*
******************************************************************************/

#ifndef XDPHY_H_
#define XDPHY_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
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
#define XDPHY_MAX_LANES_V10	4 /**< V1.0 supports 4 Lanes */
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
#define XDPHY_HANDLE_HSSETTLE	10 /**< Handle for HS SETTLE */
#define XDPHY_HANDLE_DLANE4	11 /**< Handle for Data Lane 4 */
#define XDPHY_HANDLE_DLANE5	12 /**< Handle for Data Lane 5 */
#define XDPHY_HANDLE_DLANE6	13 /**< Handle for Data Lane 6 */
#define XDPHY_HANDLE_DLANE7	14 /**< Handle for Data Lane 7 */
#define XDPHY_HANDLE_HSSETTLE1	15 /**< Handle for HS SETTLE L1 */
#define XDPHY_HANDLE_HSSETTLE2	16 /**< Handle for HS SETTLE L2 */
#define XDPHY_HANDLE_HSSETTLE3	17 /**< Handle for HS SETTLE L3 */
#define XDPHY_HANDLE_HSSETTLE4	18 /**< Handle for HS SETTLE */
#define XDPHY_HANDLE_HSSETTLE5	19 /**< Handle for HS SETTLE L1 */
#define XDPHY_HANDLE_HSSETTLE6	20 /**< Handle for HS SETTLE L2 */
#define XDPHY_HANDLE_HSSETTLE7	21 /**< Handle for HS SETTLE L3 */
#define XDPHY_HANDLE_MAX 	21 /**< Upper Bound for XDPHY_HANDLE */
/*@}*/

/**************************** Macros Definitions *****************************/
/** @name DPHY HSTIMEOUT range
 * @{
*/
#define XDPHY_HS_TIMEOUT_MIN_VALUE	10000UL
#define XDPHY_HS_TIMEOUT_MAX_VALUE	65541UL
/*@}*/

/** @name DPHY HSSETTLE range
 * @{
*/
#define XDPHY_HS_SETTLE_MAX_VALUE	0x1FF
/*@}*/

/** @name DPHY Flags to Enable or Disable core
 * @{
*/
#define XDPHY_ENABLE_FLAG 	1
#define XDPHY_DISABLE_FLAG 	0
/*@}*/

/************************* Bit field operations ****************************/

/****************************************************************************/
/**
*
* This inline function is used to set bit in a DPHY register space
*
* @param 	BaseAddress is a base address of IP
* @param 	RegisterOffset is offset where the register is present
* @param 	BitMask of bit field to be set
*
* @return 	None
*
* @note 	None
*
****************************************************************************/
static inline void XDphy_BitSet(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask)
{
	XDphy_WriteReg(BaseAddress, RegisterOffset,
	(XDphy_ReadReg(BaseAddress, RegisterOffset) | BitMask));
}

/****************************************************************************/
/**
*
* This inline function is used to reset bit in a DPHY register space
*
* @param 	BaseAddress is a base address of IP
* @param 	RegisterOffset is offset where the register is present
* @param 	BitMask of bit field to be reset
*
* @return 	None
*
* @note 	None
*
****************************************************************************/

static inline void XDphy_BitReset(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask)
{
	XDphy_WriteReg(BaseAddress, RegisterOffset,
	(XDphy_ReadReg(BaseAddress, RegisterOffset) & ~ BitMask));
}


/****************************************************************************/
/**
*
* This function is used to get the value of bitfield from DPHY register space
*
* @param 	BaseAddress is a base address of IP
* @param 	RegisterOffset is offset where the register is present
* @param 	BitMask of bit field whose value needs to be obtained
* @param 	BitShift is offset of bit field
*
* @return 	Bit Field Value in u32 format
*
* @note 	None
*
****************************************************************************/
static inline u32 XDphy_GetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
						u32 BitMask, u32 BitShift)
{
	return((XDphy_ReadReg(BaseAddress, RegisterOffset)
		 & BitMask) >> BitShift);
}

/****************************************************************************/
/**
*
* This function is used to set the value of bitfield from DPHY register space
*
* @param 	BaseAddress is a base address of IP
* @param 	RegisterOffset is offset where the register is present
* @param 	BitMask of bit field whose value needs to be updated
* @param 	BitShift is offset of bit field
* @param 	Value to be set. Passed in u32 format.
*
* @return 	None
*
* @note 	None
****************************************************************************/
static inline void XDphy_SetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
				u32 BitMask, u32 BitShift, u32 Value)
{
	XDphy_WriteReg(BaseAddress, RegisterOffset,
		((XDphy_ReadReg(BaseAddress, RegisterOffset) &
		 ~ BitMask) | (Value << BitShift)));
}

/**************************** Type Definitions *******************************/

/**
* The configuration structure for DPHY
*
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
	u32 DeviceId; /**< Device Id */
	UINTPTR BaseAddr; /**< Base address of DPHY */

	u32 IsRx; /**< TX or RX Mode */
	u32 IsRegisterPresent; /**< Is register access allowed */
	u32 MaxLanesPresent; /**< Number of Lanes. Range 1 - 4 */
	u32 EscClkPeriod; /**< Escape Clock Peroid */
	u32 EscTimeout; /**< Escape Timeout */
	u32 HSLineRate; /**< High Speed Line Rate */
	u32 HSTimeOut; /**< Max Frame Length  */
	u32 LPXPeriod;
	u32 StableClkPeriod;
	u32 TxPllClkinPeriod;
	u32 Wakeup; /**< Time to exit ULPS mode */
	u32 EnableTimeOutRegs;	/**< Enable HS and Esc Timeout Regs */
	u32 HSSettle;
} XDphy_Config;

/**
* The XDphy Controller driver instance data.
* The user is required to allocate a variable of this type for every XDphy
* device in the system. A pointer to a variable of this type is then passed
* to the driver API functions.
*/
typedef struct {
	XDphy_Config Config; /**< Hardware Configuration */
	u32 IsReady; /**< Driver is ready */
} XDphy;

/************************** Function Prototypes ******************************/

/* Initialization function in xdphy_sinit.c */
XDphy_Config *XDphy_LookupConfig(u32 DeviceId);

/* Initialization and control functions xdphy.c */
u32 XDphy_CfgInitialize(XDphy *InstancePtr, XDphy_Config *Config,
			UINTPTR EffectiveAddr);
u32 XDphy_Configure(XDphy *InstancePtr, u8 Handle, u32 Value);
u32 XDphy_GetInfo(XDphy *InstancePtr, u8 Handle);
void XDphy_Reset(XDphy *InstancePtr);
void XDphy_ClearDataLane(XDphy *InstancePtr, u8 DataLane, u32 Mask);
u32 XDphy_GetClkLaneStatus(XDphy *InstancePtr);
u32 XDphy_GetClkLaneMode(XDphy *InstancePtr);
u32 XDphy_GetDataLaneStatus(XDphy *InstancePtr, u8 DataLane);
u16 XDphy_GetPacketCount(XDphy *InstancePtr, u8 DataLane);
u8 XDphy_GetDLCalibStatus(XDphy *InstancePtr, u8 DataLane);
u32 XDphy_GetDataLaneMode(XDphy *InstancePtr, u8 DataLane);
void XDphy_Activate(XDphy *InstancePtr, u8 Flag);
u8 XDphy_GetRegIntfcPresent(XDphy *InstancePtr);

/* Self test function in xcsiss_selftest.c */
u32 XDphy_SelfTest(XDphy *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
