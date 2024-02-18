/******************************************************************************
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmipi_rx_phy.h
*
* @addtogroup mipi_rx_phy Overview
* @{
* @details
*
* This file contains the implementation of the MIPI mipi_rx_phy Controller driver.
* User documentation for the driver functions is contained in this file in the
* form of comment blocks at the front of each function.
*
* <b>MIPI mipi_rx_phy Overview</b>
*
* The mipi_rx_phy currently supports the MIPI?Alliance Specification
* for mipi_rx_phy Version 1.0.
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
* By default, the mipi_rx_phy core is initialized and ready.
*
* The application needs to do following steps in order for preparing the
* MIPI mipi_rx_phy core to be ready.
*
* - Call XMipi_Rx_Phy_LookupConfig using a device ID to find the core
*   configuration.
* - Call XMipi_Rx_Phy_CfgInitialize to initialize the device and the driver
*   instance associated with it.
* - Individual parameters can be configured by sending values with
*   appropriate handles.
*
* <b>Interrupts</b>
*
* There are no interrupts from the mipi_rx_phy.
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
* The mipi_rx_phy driver is composed of source files and doesn't depend on any other
* drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 pg 16/02/24 Initial release
* </pre>
*
******************************************************************************/

#ifndef XMIPI_RX_PHY_H_
#define XMIPI_RX_PHY_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xmipi_rx_phy_hw.h"

/************************** Constant Definitions *****************************/

/** @name mipi_rx_phy Modes
 * @{
*/
#define XMIPI_RX_PHY_MODE_MIN 		0 /**< Lower limit for Mode */
#define XMIPI_RX_PHY_LOW_POWER_MODE 	0 /**< Lane in Low Power Mode */
#define XMIPI_RX_PHY_HIGH_POWER_MODE	1 /**< Lane in High Power Mode */
#define XMIPI_RX_PHY_ESCAPE_MODE	2 /**< Lane in Escape Mode */
#define XMIPI_RX_PHY_MODE_MAX 		2 /**< Upper Limit for mode */
#define XMIPI_RX_PHY_MAX_LANES_V10	4 /**< V1.0 supports 4 Lanes */
/*@}*/

/** @name mipi_rx_phy Info Handles
 * @{
*/
#define XMIPI_RX_PHY_HANDLE_MIN 	0 /**< Lower Bound for XMIPI_RX_PHY_HANDLE */
#define XMIPI_RX_PHY_HANDLE_IDELAY	0 /**< Handle for IDELAY Reg */
#define XMIPI_RX_PHY_HANDLE_INIT_TIMER	1 /**< Handle for Initialization Timer */
#define XMIPI_RX_PHY_HANDLE_WAKEUP	2 /**< Handle for Wakeup timer */
#define XMIPI_RX_PHY_HANDLE_HSTIMEOUT	3 /**< Handle for HS Timeout */
#define XMIPI_RX_PHY_HANDLE_ESCTIMEOUT	4 /**< Handle for Escape Timeout */
#define XMIPI_RX_PHY_HANDLE_CLKLANE	5 /**< Handle for Clock Lane */
#define XMIPI_RX_PHY_HANDLE_DLANE0	6 /**< Handle for Data Lane 0 */
#define XMIPI_RX_PHY_HANDLE_DLANE1	7 /**< Handle for Data Lane 1 */
#define XMIPI_RX_PHY_HANDLE_DLANE2	8 /**< Handle for Data Lane 2 */
#define XMIPI_RX_PHY_HANDLE_DLANE3	9 /**< Handle for Data Lane 3 */
#define XMIPI_RX_PHY_HANDLE_HSSETTLE0	10 /**< Handle for HS SETTLE L0 */
#define XMIPI_RX_PHY_HANDLE_HSSETTLE1	11 /**< Handle for HS SETTLE L1 */
#define XMIPI_RX_PHY_HANDLE_HSSETTLE2	12 /**< Handle for HS SETTLE L2 */
#define XMIPI_RX_PHY_HANDLE_HSSETTLE3	13 /**< Handle for HS SETTLE L3 */
#define XMIPI_RX_PHY_HANDLE_MAX 	14 /**< Upper Bound for XMIPI_RX_PHY_HANDLE */
/*@}*/

/**************************** Macros Definitions *****************************/
/** @name mipi_rx_phy HSTIMEOUT range
 * @{
*/
#define XMIPI_RX_PHY_HS_TIMEOUT_MIN_VALUE	10000UL
#define XMIPI_RX_PHY_HS_TIMEOUT_MAX_VALUE	65541UL
/*@}*/

/** @name mipi_rx_phy HSSETTLE range
 * @{
*/
#define XMIPI_RX_PHY_HS_SETTLE_MAX_VALUE	0x1FF
/*@}*/

/** @name mipi_rx_phy Flags to Enable or Disable core
 * @{
*/
#define XMIPI_RX_PHY_ENABLE_FLAG 	1
#define XMIPI_RX_PHY_DISABLE_FLAG 	0
/*@}*/

/************************* Bit field operations ****************************/

/****************************************************************************/
/**
*
* This inline function is used to set bit in a mipi_rx_phy register space
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
static inline void XMipi_Rx_Phy_BitSet(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask)
{
	XMipi_Rx_Phy_WriteReg(BaseAddress, RegisterOffset,
	(XMipi_Rx_Phy_ReadReg(BaseAddress, RegisterOffset) | BitMask));
}

/****************************************************************************/
/**
*
* This inline function is used to reset bit in a mipi_rx_phy register space
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

static inline void XMipi_Rx_Phy_BitReset(UINTPTR BaseAddress, u32 RegisterOffset,
					u32 BitMask)
{
	XMipi_Rx_Phy_WriteReg(BaseAddress, RegisterOffset,
	(XMipi_Rx_Phy_ReadReg(BaseAddress, RegisterOffset) & ~ BitMask));
}


/****************************************************************************/
/**
*
* This function is used to get the value of bitfield from mipi_rx_phy register space
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
static inline u32 XMipi_Rx_Phy_GetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
						u32 BitMask, u32 BitShift)
{
	return((XMipi_Rx_Phy_ReadReg(BaseAddress, RegisterOffset)
		 & BitMask) >> BitShift);
}

/****************************************************************************/
/**
*
* This function is used to set the value of bitfield from mipi_rx_phy register space
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
static inline void XMipi_Rx_Phy_SetBitField(UINTPTR BaseAddress, u32 RegisterOffset,
				u32 BitMask, u32 BitShift, u32 Value)
{
	XMipi_Rx_Phy_WriteReg(BaseAddress, RegisterOffset,
		((XMipi_Rx_Phy_ReadReg(BaseAddress, RegisterOffset) &
		 ~ BitMask) | (Value << BitShift)));
}

/**************************** Type Definitions *******************************/

/**
* The configuration structure for mipi_rx_phy
*
* This structure passes the hardware building information to the driver
*
*/
typedef struct {
    char *Name;
	UINTPTR BaseAddr; /**< Base address of mipi_rx_phy */
	u32 IsDphy; /**< CPHY or DPHY Mode */
	u32 IsRegisterPresent; /**< Is register access allowed */
	u32 MaxLanesPresent; /**< Number of Lanes. Range 1 - 4 */
	u32 EscClkPeriod; /**< Escape Clock Peroid */
	u32 EscTimeout; /**< Escape Timeout */
	u32 HSLineRate; /**< High Speed Line Rate */
	u32 HSTimeOut; /**< Max Frame Length  */
	u32 StableClkPeriod;
	u32 Wakeup; /**< Time to exit ULPS mode */
	u32 EnableTimeOutRegs;	/**< Enable HS and Esc Timeout Regs */
	u32 HSSettle;
} XMipi_Rx_Phy_Config;

/**
* The Xmipi_rx_phy Controller driver instance data.
* The user is required to allocate a variable of this type for every Xmipi_rx_phy
* device in the system. A pointer to a variable of this type is then passed
* to the driver API functions.
*/
typedef struct {
	XMipi_Rx_Phy_Config Config; /**< Hardware Configuration */
	u32 IsReady; /**< Driver is ready */
} XMipi_Rx_Phy;

/************************** Function Prototypes ******************************/

/* Initialization function in xmipi_rx_phy_sinit.c */
XMipi_Rx_Phy_Config *XMipi_Rx_Phy_LookupConfig(UINTPTR BaseAddress);

/* Initialization and control functions xmipi_rx_phy.c */
u32 XMipi_Rx_Phy_CfgInitialize(XMipi_Rx_Phy *InstancePtr, XMipi_Rx_Phy_Config *Config,
			UINTPTR EffectiveAddr);
u32 XMipi_Rx_Phy_Configure(XMipi_Rx_Phy *InstancePtr, u8 Handle, u32 Value);
u32 XMipi_Rx_Phy_GetInfo(XMipi_Rx_Phy *InstancePtr, u8 Handle);
void XMipi_Rx_Phy_Reset(XMipi_Rx_Phy *InstancePtr);
void XMipi_Rx_Phy_ClearDataLane(XMipi_Rx_Phy *InstancePtr, u8 DataLane, u32 Mask);
u32 XMipi_Rx_Phy_GetClkLaneStatus(XMipi_Rx_Phy *InstancePtr);
u32 XMipi_Rx_Phy_GetClkLaneMode(XMipi_Rx_Phy *InstancePtr);
u32 XMipi_Rx_Phy_GetDataLaneStatus(XMipi_Rx_Phy *InstancePtr, u8 DataLane);
u16 XMipi_Rx_Phy_GetPacketCount(XMipi_Rx_Phy *InstancePtr, u8 DataLane);
u8 XMipi_Rx_Phy_GetDLCalibStatus(XMipi_Rx_Phy *InstancePtr, u8 DataLane);
u32 XMipi_Rx_Phy_GetDataLaneMode(XMipi_Rx_Phy *InstancePtr, u8 DataLane);
void XMipi_Rx_Phy_Activate(XMipi_Rx_Phy *InstancePtr, u8 Flag);
u8 XMipi_Rx_Phy_GetRegIntfcPresent(XMipi_Rx_Phy *InstancePtr);

/* Self test function in xcsiss_selftest.c */
u32 XMipi_Rx_Phy_SelfTest(XMipi_Rx_Phy *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
