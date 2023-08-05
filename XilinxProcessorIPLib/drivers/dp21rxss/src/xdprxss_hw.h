/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_hw.h
* @addtogroup dprxss Overview
* @{
*
* This header file contains identifiers and register-level core functions (or
* macros) that can be used to access the Xilinx DisplayPort Receiver Subsystem.
*
* For more information about the operation of this core see the hardware
* specification and documentation in the higher level driver xdprxss.h file.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Added Timer Counter reset value macro.
* </pre>
*
******************************************************************************/
#ifndef XDPRXSS_HW_H_
#define XDPRXSS_HW_H_		/**< Prevent circular inclusions
				  *  by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xil_io.h"

/************************** Constant Definitions *****************************/

/* 0x09C: OVER_LINK_BW_SET */
#define XDPRXSS_LINK_BW_SET_162GBPS XDP_RX_OVER_LINK_BW_SET_162GBPS /**< 1.62
								      *  Gbps
								      * link
								      * rate.
								      */
#define XDPRXSS_LINK_BW_SET_270GBPS XDP_RX_OVER_LINK_BW_SET_270GBPS /**< 2.70
								      *  Gbps
								      * link
								      * rate.
								      */
#define XDPRXSS_LINK_BW_SET_540GBPS XDP_RX_OVER_LINK_BW_SET_540GBPS /**< 5.40
								      *  Gbps
								      * link
								      * rate.
								      */
#define XDPRXSS_LINK_BW_SET_810GBPS XDP_RX_OVER_LINK_BW_SET_810GBPS /**< 8.10
								      *  Gbps
								      * link
								      * rate.
								      */

/* 0x0A0: OVER_LANE_COUNT_SET */
#define XDPRXSS_LANE_COUNT_SET_1 XDP_RX_OVER_LANE_COUNT_SET_1	/**< Lane count
								  *  of 1. */
#define XDPRXSS_LANE_COUNT_SET_2 XDP_RX_OVER_LANE_COUNT_SET_2	/**< Lane count
								  *  of 2. */
#define XDPRXSS_LANE_COUNT_SET_4 XDP_RX_OVER_LANE_COUNT_SET_4	/**< Lane count
								  *  of 4. */

#define XDPRXSS_RX_PHY_CONFIG	XDP_RX_PHY_CONFIG	/**< PHY reset and
							  *  config */
#define XDPRXSS_PHY_POWER_DOWN	XDP_RX_PHY_POWER_DOWN	/**< PHY power down */

#define XDPRXSS_MSA_HRES	XDP_RX_MSA_HRES	/**< Number of active pixels
						  *  per line (the horizontal
						  *  resolution). */
#define XDPRXSS_MSA_VRES	XDP_RX_MSA_VHEIGHT	/**< Number of active
							  *  lines (the
							  * vertical
							  * resolution). */

/* Link bandwidth and lane count setting as exposed in the RX DPCD */
#define XDPRXSS_DPCD_LINK_BW_SET	XDP_RX_DPCD_LINK_BW_SET
#define XDPRXSS_DPCD_LANE_COUNT_SET	XDP_RX_DPCD_LANE_COUNT_SET

/* Link training status for lanes 0, lane 1, lane 2 and lane 3 as exposed in
 * the RX DPCD
 */
#define XDPRXSS_DPCD_LANE01_STATUS	XDP_RX_DPCD_LANE01_STATUS
#define XDPRXSS_DPCD_LANE23_STATUS	XDP_RX_DPCD_LANE23_STATUS

/* Vertical blank interrupt mask */
#define XDPRXSS_INTR_VBLANK_MASK	XDP_RX_INTERRUPT_MASK_VBLANK_MASK

#define XDPRXSS_NUM_STREAMS		4	/**< Maximum number of
						  *  streams supported */

#define XDPRXSS_MAX_NPORTS		XDP_MAX_NPORTS	/**< Maximum number of
							  *  RX ports */
#define XDPRXSS_GUID_NBYTES		XDP_GUID_NBYTES	/**< Number of bytes
							  *  for GUID */

#define XDPRXSS_TMRCTR_RST_VAL		100000000	/**< Timer Counter
							  *  reset value */

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

/** @name Register access macro definition
* @{
*/
#define XDpRxSs_In32		Xil_In32	/**< Input Operations */
#define XDpRxSs_Out32		Xil_Out32	/**< Output Operations */

/*****************************************************************************/
/**
*
* This macro reads a value from a DisplayPort Receiver Subsystem register.
* A 32 bit read is performed. If the component is implemented in a smaller
* width, only the least significant data is read from the register. The most
* significant data will be read as 0.
*
* @param	BaseAddress is the base address of the XDpRxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file).
*
* @return	The 32-bit value of the register.
*
* @note		C-style signature:
*		u32 XDpRxSs_ReadReg(UINTPTR BaseAddress, u32 RegOffset)
*
******************************************************************************/
#define XDpRxSs_ReadReg(BaseAddress, RegOffset) \
	XDpRxSs_In32((BaseAddress) + ((u32)RegOffset))

/*****************************************************************************/
/**
*
* This macro writes a value to a DisplayPort Receiver Subsystem register.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the XDpRxSs core instance.
* @param	RegOffset is the register offset of the register (defined at
*		the top of this file) to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		C-style signature:
*		void XDpRxSs_WriteReg(UINTPTR BaseAddress, u32 RegOffset,
*		u32 Data)
*
******************************************************************************/
#define XDpRxSs_WriteReg(BaseAddress, RegOffset, Data) \
	XDpRxSs_Out32((BaseAddress) + ((u32)RegOffset), (u32)(Data))
/*@}*/

/************************** Function Prototypes ******************************/


/************************** Variable Declarations ****************************/


#ifdef __cplusplus
}
#endif

#endif /* end of protection macro */
/** @} */
