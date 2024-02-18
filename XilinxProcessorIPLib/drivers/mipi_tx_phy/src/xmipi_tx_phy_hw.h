/******************************************************************************
* Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xmipi_tx_phy_hw.h
* @addtogroup mipi_tx_phy Overview
* @{
*
* Hardware definition file. It defines the register interface.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 pg 16/02/24 Initial release
* </pre>
*
*****************************************************************************/

#ifndef XMIPI_TX_PHY_HW_H_
#define XMIPI_TX_PHY_HW_H_		/**< Prevent circular inclusions
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
 *  Register sets of MIPI_TX_PHY
 *  @{
 */
#define XMIPI_TX_PHY_CTRL_REG_OFFSET 		0x00000000  /**< Control Register */
#define XMIPI_TX_PHY_VERSION_REG_OFFSET 	0x00000004  /**< Core Version Register */
#define XMIPI_TX_PHY_INIT_TIMER_REG_OFFSET 	0x00000008  /**< Initialization Timer
						      *  Register */
#define XMIPI_TX_PHY_HSTIMEOUT_REG_OFFSET 	0x00000010  /**< Watchdog timeout in HS
						      *  mode Register */
#define XMIPI_TX_PHY_ESCTIMEOUT_REG_OFFSET 	0x00000014  /**< Goto Stop state on
						      *  timeout timer Register */
#define XMIPI_TX_PHY_CLSTATUS_REG_OFFSET   	0x00000018  /**< Clk lane PHY error
						      *  Status Register */
#define XMIPI_TX_PHY_DL0STATUS_REG_OFFSET 	0x0000001C  /**< Data lane 0 PHY error
						      *  Status Register */
#define XMIPI_TX_PHY_DL1STATUS_REG_OFFSET 	0x00000020  /**< Data lane 1 PHY error
						      *  Status Register */
#define XMIPI_TX_PHY_DL2STATUS_REG_OFFSET 	0x00000024  /**< Data lane 2 PHY error
						      *  Status Register */
#define XMIPI_TX_PHY_DL3STATUS_REG_OFFSET 	0x00000028  /**< Data lane 3 PHY error
						      *  Status Register */
#define XMIPI_TX_PHY_PROG_SEQ_CTRL_OFFSET 	0x00000038  /**< Prog Seq Control Register */
#define XMIPI_TX_PHY_PROG_SEQ_DATA0_OFFSET 	0x0000003C  /**< Prog Seq Data Register 0 */
#define XMIPI_TX_PHY_PROG_SEQ_DATA1_OFFSET 	0x00000040  /**< Prog Seq Data Register 1 */

/*@}*/

/** @name Bitmasks and offsets of XMIPI_TX_PHY_CTRL_REG_OFFSET register
 *
 * This register is used for the enabling/disabling and resetting the MIPI_TX_PHY
 * @{
 */
#define XMIPI_TX_PHY_CTRL_REG_SOFTRESET_MASK 	0x00000001 /**< Soft Reset */
#define XMIPI_TX_PHY_CTRL_REG_PHYEN_MASK 	0x00000002 /**< Enable/Disable
						     *  controller */

#define XMIPI_TX_PHY_CTRL_REG_SOFTRESET_OFFSET 0 /**< Bit offset for Soft Reset */
#define XMIPI_TX_PHY_CTRL_REG_PHYEN_OFFSET 1 /**< Bit offset for PHY Enable */

#define XMIPI_TX_PHY_PROG_SEQ_EN_MASK 0x1	/**< Enable/Disable Prog Seq */

/*@}*/

/** @name Bitmasks and offsets of XMIPI_TX_PHY_HSEXIT_IDELAY_REG_OFFSET register
 *
 * This register in RX mode acts like IDELAY.
 * In IDELAY mode, it is used to calibrate input delay per lane
 * @{
 */
/*@}*/

/** @name Bitmasks and offsets of XMIPI_TX_PHY_INIT_REG_OFFSET register
 *
 * This register is used for lane Initialization. Recommended to use 1ms or
 * longer in for TX mode and 200us-500us for RX mode
 * @{
 */
#define XMIPI_TX_PHY_INIT_REG_VAL_MASK 0xFFFFFFFF /**< Init Timer value in ns */

#define XMIPI_TX_PHY_INIT_REG_VAL_OFFSET 0 /**< Bit offset for Init Timer */
/*@}*/

/** @name Bitmask and offset of XMIPI_TX_PHY_HSTIMEOUT_REG_OFFSET register
 *
 * This register is used to program watchdog timer in high speed mode.
 * Default value is 65541. Valid range 1000-65541.
 *
 * @{
 */
#define XMIPI_TX_PHY_HSTIMEOUT_REG_TIMEOUT_MASK 0xFFFFFFFF /**< HS_TX_TIMEOUT
								Received */

#define XMIPI_TX_PHY_HSTIMEOUT_REG_TIMEOUT_OFFSET 0 /**< Bit offset for Timeout */
/*@}*/

/** @name Bitmask and offset of XMIPI_TX_PHY_ESCTIMEOUT_REG_OFFSET register
 *
 * This register contains Rx Data Lanes timeout for watchdog timer in
 * escape mode.
 * @{
 */
#define XMIPI_TX_PHY_ESCTIMEOUT_REG_VAL_MASK 0xFFFFFFFF /**< Escape Timout Value */
#define XMIPI_TX_PHY_ESCTIMEOUT_REG_VAL_OFFSET 0 /**< Bit offset for Escape Timeout */
/*@}*/

/** @name Bitmask and offset of XMIPI_TX_PHY_CLSTATUS_REG_OFFSET register
 *
 * This register contains the clock lane status and state machine control.
 * @{
 */
#define XMIPI_TX_PHY_CLSTATUS_REG_ERRCTRL_MASK 0x00000020 /**< Clock lane control
						     *  error. Only for RX */
#define XMIPI_TX_PHY_CLSTATUS_REG_STOPSTATE_MASK 0x00000010 /**< Clock lane stop
						       *  state */
#define XMIPI_TX_PHY_CLSTATUS_REG_INITDONE_MASK 0x00000008 /**< Initialization done
						      *  bit */
#define XMIPI_TX_PHY_CLSTATUS_REG_ULPS_MASK 0x00000004 /**< Set in ULPS mode */
#define XMIPI_TX_PHY_CLSTATUS_REG_MODE_MASK 0x00000003 /**< Low, High, Esc mode */

#define XMIPI_TX_PHY_CLSTATUS_ALLMASK 	 (XMIPI_TX_PHY_CLSTATUS_REG_ERRCTRL_MASK |\
					XMIPI_TX_PHY_CLSTATUS_REG_STOPSTATE_MASK |\
					XMIPI_TX_PHY_CLSTATUS_REG_INITDONE_MASK |\
					XMIPI_TX_PHY_CLSTATUS_REG_ULPS_MASK |\
					XMIPI_TX_PHY_CLSTATUS_REG_MODE_MASK)

#define XMIPI_TX_PHY_CLSTATUS_REG_ERRCTRL_OFFSET 5 /**< Bit offset for Control Error
					      *  on Clock*/
#define XMIPI_TX_PHY_CLSTATUS_REG_STOPSTATE_OFFSET 4 /**< Bit offset for Stop State on
						*  Clock */
#define XMIPI_TX_PHY_CLSTATUS_REG_INITDONE_OFFSET 3 /**< Bit offset for Initialization
					       *  Done */
#define XMIPI_TX_PHY_CLSTATUS_REG_ULPS_OFFSET 2 /**< Bit offset for ULPS */
#define XMIPI_TX_PHY_CLSTATUS_REG_MODE_OFFSET 0 /**< Bit offset for Mode bits */
/*@}*/

/** @name Bitmasks and offsets of XMIPI_TX_PHY_DLxSTATUS_REG_OFFSET register
 *
 * This register contains the data lanes status
 * @{
 */
#define XMIPI_TX_PHY_DLXSTATUS_REG_PACKETCOUNT_MASK 0xFFFF0000 /**< Packet Count */
#define XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_STATUS_MASK 0x00000100 /**< Calib status */
#define XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_COMPLETE_MASK 0x00000080 /**< Calib complete */
#define XMIPI_TX_PHY_DLXSTATUS_REG_STOP_MASK 0x00000040 /**< Stop State on data lane */
#define XMIPI_TX_PHY_DLXSTATUS_REG_ESCABRT_MASK 0x00000020 /**< Set on Data Lane Esc
						      *  timeout occurs */
#define XMIPI_TX_PHY_DLXSTATUS_REG_HSABRT_MASK 0x00000010 /**< Set on Data Lane
						     *  HS timeout */
#define XMIPI_TX_PHY_DLXSTATUS_REG_INITDONE_MASK 0x00000008 /**< Set after
						       * initialization */
#define XMIPI_TX_PHY_DLXSTATUS_REG_ULPS_MASK 0x00000004 /**< Set when MIPI_TX_PHY in ULPS
						   *  mode */
#define XMIPI_TX_PHY_DLXSTATUS_REG_MODE_MASK 0x00000003 /**< Control Mode (Esc, Low,
						   *  High) of Data Lane */

#define XMIPI_TX_PHY_DLXSTATUS_ALLMASK	(XMIPI_TX_PHY_DLXSTATUS_REG_MODE_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_ULPS_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_INITDONE_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_HSABRT_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_ESCABRT_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_STOP_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_STATUS_MASK |\
					XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_COMPLETE_MASK)

#define XMIPI_TX_PHY_DLXSTATUS_REG_PACKCOUNT_OFFSET 16 /**<Bit offset packet count*/
#define XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_STATUS_OFFSET 8 /**<Bit offset calib status*/
#define XMIPI_TX_PHY_DLXSTATUS_REG_CALIB_COMPLETE_OFFSET 7 /**<Bit offset Calib complete*/
#define XMIPI_TX_PHY_DLXSTATUS_REG_STOP_OFFSET 6 /**< Bit offset for Stop State */
#define XMIPI_TX_PHY_DLXSTATUS_REG_ESCABRT_OFFSET 5 /**< Bit offset for Escape Abort */
#define XMIPI_TX_PHY_DLXSTATUS_REG_HSABRT_OFFSET 4 /**< Bit offset for High Speed
							Abort */
#define XMIPI_TX_PHY_DLXSTATUS_REG_INITDONE_OFFSET 3 /**< Bit offset for
							Initialization done */
#define XMIPI_TX_PHY_DLXSTATUS_REG_ULPS_OFFSET 2 /**< Bit offset for ULPS */
#define XMIPI_TX_PHY_DLXSTATUS_REG_MODE_OFFSET 0 /**< Bit offset for Modes */
/*@}*/

/** @name Bitmask and offset of XMIPI_TX_PHY_HSSETTLE_REG_OFFSET register
 *
 * This register is used to program the HS SETTLE register.
 * Default value is 135 + 10UI.
 *
 * @{
 */
#define XMIPI_TX_PHY_HSSETTLE_REG_TIMEOUT_MASK	0x1FF	/**< HS_SETTLE value */
#define XMIPI_TX_PHY_HSSETTLE_REG_TIMEOUT_OFFSET 0 /**< Bit offset for HS_SETTLE */
/*@}*/

#define DL_LANE_OFFSET	4

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/**
*
* This function reads a value from a MIPI_TX_PHY register space.
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
static inline u32 XMipi_Tx_Phy_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
{
	return (Xil_In32(BaseAddress + (u32)RegOffset));
}

/*****************************************************************************/
/**
*
* This function writes a value to a MIPI_TX_PHY register space
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
static inline void XMipi_Tx_Phy_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
