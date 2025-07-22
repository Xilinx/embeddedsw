/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xdphy_hw.h
* @addtogroup xdphy Overview
* @{
*
* Hardware definition file. It defines the register interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 vsa 07/07/15 Initial release
* 1.1 sss 08/17/16 Added 64 bit support
* 1.2 vsa 03/02/17 Add support for HS_SETTLE register
* </pre>
*
*****************************************************************************/

#ifndef XDPHY_HW_H_
#define XDPHY_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* Register offset definitions. Register accesses are 32-bit.
 */
/** @name Device registers
 *  Register sets of MIPI DPHY
 *  @{
 */
#define XDPHY_CTRL_REG_OFFSET 		0x00000000  /**< Control Register */
#define XDPHY_HSEXIT_IDELAY_REG_OFFSET 	0x00000004  /**< IDelay Tap per lane for
						      *  Rx Register*/
#define XDPHY_INIT_REG_OFFSET 		0x00000008  /**< Initialization Timer
						      *  Register */
#define XDPHY_WAKEUP_REG_OFFSET 	0x0000000C  /**< Wakeup Timer for ULPS
						      *  exit Register */
#define XDPHY_HSTIMEOUT_REG_OFFSET 	0x00000010  /**< Watchdog timeout in HS
						      *  mode Register */
#define XDPHY_ESCTIMEOUT_REG_OFFSET 	0x00000014  /**< Goto Stop state on
						      *  timeout timer
						      *  Register */
#define XDPHY_CLSTATUS_REG_OFFSET   	0x00000018  /**< Clk lane PHY error
						      *  Status Register */
#define XDPHY_DL0STATUS_REG_OFFSET 	0x0000001C  /**< Data lane 0 PHY error
						      *  Status Register */
#define XDPHY_DL1STATUS_REG_OFFSET 	0x00000020  /**< Data lane 1 PHY error
						      *  Status Register */
#define XDPHY_DL2STATUS_REG_OFFSET 	0x00000024  /**< Data lane 2 PHY error
						      *  Status Register */
#define XDPHY_DL3STATUS_REG_OFFSET 	0x00000028  /**< Data lane 3 PHY error
						      *  Status Register */
#define XDPHY_HSSETTLE_REG_OFFSET	0x00000030  /**< HS Settle Register L0*/
#define XDPHY_IDELAY58_REG_OFFSET 	0x00000034  /**< IDelay Tap per lane for
						      *  Rx 4-7 Register*/
#define XDPHY_HSSETTLE1_REG_OFFSET	0x00000048  /**< HS Settle Register L1*/
#define XDPHY_HSSETTLE2_REG_OFFSET	0x0000004C  /**< HS Settle Register L2*/
#define XDPHY_HSSETTLE3_REG_OFFSET	0x00000050  /**< HS Settle Register L3*/

#define XDPHY_HSSETTLE4_REG_OFFSET	0x00000054  /**< HS Settle Register L4*/
#define XDPHY_HSSETTLE5_REG_OFFSET	0x00000058  /**< HS Settle Register L5*/
#define XDPHY_HSSETTLE6_REG_OFFSET	0x0000005C  /**< HS Settle Register L6*/
#define XDPHY_HSSETTLE7_REG_OFFSET	0x00000060  /**< HS Settle Register L7*/

#define XDPHY_DL4STATUS_REG_OFFSET 	0x00000064  /**< Data lane 0 PHY error
						      *  Status Register */
#define XDPHY_DL5STATUS_REG_OFFSET 	0x00000068  /**< Data lane 1 PHY error
						      *  Status Register */
#define XDPHY_DL6STATUS_REG_OFFSET 	0x0000006C  /**< Data lane 2 PHY error
						      *  Status Register */
#define XDPHY_DL7STATUS_REG_OFFSET 	0x00000070  /**< Data lane 3 PHY error
						      *  Status Register */
/*@}*/

/** @name Bitmasks and offsets of XDPHY_CTRL_REG_OFFSET register
 *
 * This register is used for the enabling/disabling and resetting the DPHY
 * @{
 */
#define XDPHY_CTRL_REG_SOFTRESET_MASK 	0x00000001 /**< Soft Reset */
#define XDPHY_CTRL_REG_DPHYEN_MASK 	0x00000002 /**< Enable/Disable
						     *  controller */

#define XDPHY_CTRL_REG_SOFTRESET_OFFSET 0 /**< Bit offset for Soft Reset */
#define XDPHY_CTRL_REG_DPHYEN_OFFSET 1 /**< Bit offset for DPHY Enable */
/*@}*/

/** @name Bitmasks and offsets of XDPHY_HSEXIT_IDELAY_REG_OFFSET register
 *
 * This register in RX mode acts like IDELAY.
 * In IDELAY mode, it is used to calibrate input delay per lane
 * @{
 */
#define XDPHY_HSEXIT_IDELAY_REG_TAP_MASK 0x1F1F1F1F  /**< used to set the IDELAY
						      *  TAP value for all lanes*/
/*@}*/

/** @name Bitmasks and offsets of XDPHY_INIT_REG_OFFSET register
 *
 * This register is used for lane Initialization. Recommended to use 1ms or
 * longer in for TX mode and 200us-500us for RX mode
 * @{
 */
#define XDPHY_INIT_REG_VAL_MASK 0xFFFFFFFF /**< Init Timer value in ns */

#define XDPHY_INIT_REG_VAL_OFFSET 0 /**< Bit offset for Init Timer */
/*@}*/

/** @name Bitmask and offset of XDPHY_WAKEUP_REG_OFFSET register
 *
 * Wakeup time delay for ULPS exit.
 * @{
 */
#define XDPHY_WAKEUP_REG_VAL_MASK 	0xFFFFFFFF /**< Wakeup timer value */
#define XDPHY_WAKEUP_REG_VAL_OFFSET 	0 /**< Bit offset for Wakeup value */
/*@}*/

/** @name Bitmask and offset of XDPHY_HSTIMEOUT_REG_OFFSET register
 *
 * This register is used to program watchdog timer in high speed mode.
 * Default value is 65541. Valid range 1000-65541.
 *
 * @{
 */
#define XDPHY_HSTIMEOUT_REG_TIMEOUT_MASK 0xFFFFFFFF /**< HS_T/RX_TIMEOUT
								Received */

#define XDPHY_HSTIMEOUT_REG_TIMEOUT_OFFSET 0 /**< Bit offset for Timeout */
/*@}*/

/** @name Bitmask and offset of XDPHY_ESCTIMEOUT_REG_OFFSET register
 *
 * This register contains Rx Data Lanes timeout for watchdog timer in
 * escape mode.
 * @{
 */
#define XDPHY_ESCTIMEOUT_REG_VAL_MASK 0xFFFFFFFF /**< Escape Timout Value */
#define XDPHY_ESCTIMEOUT_REG_VAL_OFFSET 0 /**< Bit offset for Escape Timeout */
/*@}*/

/** @name Bitmask and offset of XDPHY_CLSTATUS_REG_OFFSET register
 *
 * This register contains the clock lane status and state machine control.
 * @{
 */
#define XDPHY_CLSTATUS_REG_ERRCTRL_MASK 0x00000020 /**< Clock lane control
						     *  error. Only for RX */
#define XDPHY_CLSTATUS_REG_STOPSTATE_MASK 0x00000010 /**< Clock lane stop
						       *  state */
#define XDPHY_CLSTATUS_REG_INITDONE_MASK 0x00000008 /**< Initialization done
						      *  bit */
#define XDPHY_CLSTATUS_REG_ULPS_MASK 0x00000004 /**< Set in ULPS mode */
#define XDPHY_CLSTATUS_REG_MODE_MASK 0x00000003 /**< Low, High, Esc mode */

#define XDPHY_CLSTATUS_ALLMASK 	 (XDPHY_CLSTATUS_REG_ERRCTRL_MASK |\
					XDPHY_CLSTATUS_REG_STOPSTATE_MASK |\
					XDPHY_CLSTATUS_REG_INITDONE_MASK |\
					XDPHY_CLSTATUS_REG_ULPS_MASK |\
					XDPHY_CLSTATUS_REG_MODE_MASK)	/**< Bitmask combining
							  * all clock lane status flags: error control,
							  * stop state, initialization done, ULPS
							  * mode, and mode bits */

#define XDPHY_CLSTATUS_REG_ERRCTRL_OFFSET 5 /**< Bit offset for Control Error
					      *  on Clock*/
#define XDPHY_CLSTATUS_REG_STOPSTATE_OFFSET 4 /**< Bit offset for Stop State on
						*  Clock */
#define XDPHY_CLSTATUS_REG_INITDONE_OFFSET 3 /**< Bit offset for Initialization
					       *  Done */
#define XDPHY_CLSTATUS_REG_ULPS_OFFSET 2 /**< Bit offset for ULPS */
#define XDPHY_CLSTATUS_REG_MODE_OFFSET 0 /**< Bit offset for Mode bits */
/*@}*/

/** @name Bitmasks and offsets of XDPHY_DLxSTATUS_REG_OFFSET register
 *
 * This register contains the data lanes status
 * @{
 */
#define XDPHY_DLXSTATUS_REG_PACKETCOUNT_MASK 0xFFFF0000 /**< Packet Count */
#define XDPHY_DLXSTATUS_REG_CALIB_STATUS_MASK 0x00000100 /**< Calib status */
#define XDPHY_DLXSTATUS_REG_CALIB_COMPLETE_MASK 0x00000080 /**< Calib complete */
#define XDPHY_DLXSTATUS_REG_STOP_MASK 0x00000040 /**< Stop State on data lane */
#define XDPHY_DLXSTATUS_REG_ESCABRT_MASK 0x00000020 /**< Set on Data Lane Esc
						      *  timeout occurs */
#define XDPHY_DLXSTATUS_REG_HSABRT_MASK 0x00000010 /**< Set on Data Lane
						     *  HS timeout */
#define XDPHY_DLXSTATUS_REG_INITDONE_MASK 0x00000008 /**< Set after
						       * initialization */
#define XDPHY_DLXSTATUS_REG_ULPS_MASK 0x00000004 /**< Set when DPHY in ULPS
						   *  mode */
#define XDPHY_DLXSTATUS_REG_MODE_MASK 0x00000003 /**< Control Mode (Esc, Low,
						   *  High) of Data Lane */

#define XDPHY_DLXSTATUS_ALLMASK	(XDPHY_DLXSTATUS_REG_MODE_MASK |\
					XDPHY_DLXSTATUS_REG_ULPS_MASK |\
					XDPHY_DLXSTATUS_REG_INITDONE_MASK |\
					XDPHY_DLXSTATUS_REG_HSABRT_MASK |\
					XDPHY_DLXSTATUS_REG_ESCABRT_MASK |\
					XDPHY_DLXSTATUS_REG_STOP_MASK |\
					XDPHY_DLXSTATUS_REG_CALIB_STATUS_MASK |\
					XDPHY_DLXSTATUS_REG_CALIB_COMPLETE_MASK)	/**< Bitmask
						  * combining all data lane status flags: mode, ULPS,
						  * init done, HS abort, escape abort, stop state,
						  * calibration status and complete */

#define XDPHY_DLXSTATUS_REG_PACKCOUNT_OFFSET 16 /**<Bit offset packet count*/
#define XDPHY_DLXSTATUS_REG_CALIB_STATUS_OFFSET 8 /**<Bit offset calib status*/
#define XDPHY_DLXSTATUS_REG_CALIB_COMPLETE_OFFSET 7 /**<Bit offset Calib complete*/
#define XDPHY_DLXSTATUS_REG_STOP_OFFSET 6 /**< Bit offset for Stop State */
#define XDPHY_DLXSTATUS_REG_ESCABRT_OFFSET 5 /**< Bit offset for Escape Abort */
#define XDPHY_DLXSTATUS_REG_HSABRT_OFFSET 4 /**< Bit offset for High Speed
							Abort */
#define XDPHY_DLXSTATUS_REG_INITDONE_OFFSET 3 /**< Bit offset for
							Initialization done */
#define XDPHY_DLXSTATUS_REG_ULPS_OFFSET 2 /**< Bit offset for ULPS */
#define XDPHY_DLXSTATUS_REG_MODE_OFFSET 0 /**< Bit offset for Modes */
/*@}*/

/** @name Bitmask and offset of XDPHY_HSSETTLE_REG_OFFSET register
 *
 * This register is used to program the HS SETTLE register.
 * Default value is 135 + 10UI.
 *
 * @{
 */
#define XDPHY_HSSETTLE_REG_TIMEOUT_MASK	0x1FF	/**< HS_SETTLE value */
#define XDPHY_HSSETTLE_REG_TIMEOUT_OFFSET 0 /**< Bit offset for HS_SETTLE */
/*@}*/

#define DL_LANE_OFFSET	4	/**< Offset value for data lane registers */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/**
*
* This function reads a value from a DPHY register space.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XCsiSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		None.
*
******************************************************************************/
static inline u32 XDphy_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return (Xil_In32(BaseAddress + (u32)RegOffset));
}

/*****************************************************************************/
/**
*
* This function writes a value to a DPHY register space
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XCsiSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void XDphy_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
