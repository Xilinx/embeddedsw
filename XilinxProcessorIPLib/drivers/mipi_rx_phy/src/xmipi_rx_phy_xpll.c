/******************************************************************************
* Copyright 2022-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xmipi_rx_phy_xpll.c
* @addtogroup mipi_rx_phy Overview
* @{
*
* This file implements the X5PLL configuration for dynamic line rate support
* in the MIPI RX PHY driver. It contains the PLL register look-up tables
* and the programming sequence used to reconfigure the PLL at runtime for
* a requested line rate.
*
* <pre>
* MODIFICATION HISTORY:
* Ver Who Date     Changes
* --- --- -------- ------------------------------------------------------------
* 1.0 pg 16/02/24 Initial release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdebug.h"
#include "xstatus.h"
#include "xmipi_rx_phy.h"

/************************** Constant Definitions *****************************/

/** @name PLL Register Offsets for Dynamic Line Rate Configuration
 * @{
 */
#define XPLL_REG_PCSR_MASK_OFFSET	0x00000000  /**< PLL PCSR Mask Register */
#define XPLL_REG_PCSR_CONTROL_OFFSET	0x00000004  /**< PLL PCSR Control Register */
#define XPLL_REG_PCSR_LOCK_OFFSET	0x0000000C  /**< PLL PCSR Lock Register */
#define XPLL_REG_2_M_OFFSET		0x00000030  /**< PLL Config Register 2 */
#define XPLL_REG_3_M_OFFSET		0x00000034  /**< PLL Config Register 3 */
#define XPLL_REG_4_O1_OFFSET	0x00000038  /**< PLL Output 1 Config Register 4 */
#define XPLL_REG_5_O1_OFFSET	0x0000003C  /**< PLL Output 1 Config Register 5 */
#define XPLL_REG_6_O0_OFFSET	0x00000040  /**< PLL Output 0 Config Register 6 */
#define XPLL_REG_7_O0_OFFSET	0x00000044  /**< PLL Output 0 Config Register 7 */
#define XPLL_REG_22_D_OFFSET	0x00000080  /**< PLL Divider Config Register 22 */
#define XPLL_REG_23_D_OFFSET	0x00000084  /**< PLL Divider Config Register 23 */
/*@}*/

/** @name PLL Register Values
 * @{
 */
#define XMIPI_RX_PHY_PLL_UNLOCK_VAL		0xF9E8D7C6  /**< PLL Unlock Value */
#define XMIPI_RX_PHY_PLL_LOCK_VAL		0x00000000  /**< PLL Lock Value */
#define XMIPI_RX_PHY_PLL_MASK_VAL		0x00001200  /**< PLL Mask Value
							      *  (APB + FABRIC_EN) */
#define XMIPI_RX_PHY_PLL_CONTROL_CONFIG_VAL	0x00001000  /**< PLL Control
							      *  Config Value */
#define XMIPI_RX_PHY_PLL_CONTROL_LOCK_VAL	0x00000200  /**< PLL Control
							      *  Lock Value */
#define XMIPI_RX_PHY_PLL_EN_ODISABLE_VAL	0x0000003C  /**< PLL Enable Output Disable Value */
#define XMIPI_RX_PHY_PLL_DIS_ODISABLE		0x00000000  /**< PLL Disable Output Disable Value */
#define XMIPI_RX_PHY_PLL_EN_INITSTATE_VAL		0x00000040  /**< PLL Enable Init State Value */
#define XMIPI_RX_PHY_PLL_DIS_INITSTATE	0x00000000  /**< PLL Disable Init State Value */
/*@}*/

/** @name Line Rate Bucket Definitions
 * @{
 */
/* XMIPI_RX_PHY_LINERATE_MIN and XMIPI_RX_PHY_LINERATE_MAX are
 * defined in xmipi_rx_phy.h
 */
#define XMIPI_RX_PHY_LINERATE_NUM_BUCKETS	6U    /**< Number of line rate
						       *  buckets */
/*@}*/

/**************************** Type Definitions *******************************/

#define XMIPI_RX_PHY_PLL_NUM_REGS	8U	/**< Number of PLL registers
						  *  programmed per bucket */

/**
* The dynamic line rate bucket structure for mipi_rx_phy
*
* Each bucket defines a line rate range (lower and upper bound in Mbps)
* and the corresponding raw PLL register values to program the X5PLL
* for that range.
*
*/
typedef struct {
	u32 LineRateLb;					/**< Lower bound in Mbps */
	u32 LineRateUb;					/**< Upper bound in Mbps */
	u32 PllVals[XMIPI_RX_PHY_PLL_NUM_REGS];	/**< Raw PLL register values */
} XMipi_Rx_Phy_Dynamic_LineRate;

/************************** Macros Definitions *****************************/

/****************************************************************************/
/**
*
* This function writes a value to a X5PLL register space.
* A 32 bit write is performed. If the component is implemented in a smaller
* width, only the least significant data is written.
*
* @param	BaseAddress is the base address of the X5PLL instance.
* @param	RegOffset is the register offset of the register to be written.
* @param	Data is the 32-bit value to write into the register.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
static inline void XPll_WriteReg(UINTPTR BaseAddress, u32 RegOffset, u32 Data)
{
	Xil_Out32(BaseAddress + (u32)RegOffset, (u32)Data);
}

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/**
 * PLL register index to offset mapping (used in programming loop):
 */
static const u32 XMipi_Rx_Phy_PllRegOffsets[XMIPI_RX_PHY_PLL_NUM_REGS] = {
	XPLL_REG_2_M_OFFSET,
	XPLL_REG_3_M_OFFSET,
	XPLL_REG_4_O1_OFFSET,
	XPLL_REG_5_O1_OFFSET,
	XPLL_REG_6_O0_OFFSET,
	XPLL_REG_7_O0_OFFSET,
	XPLL_REG_22_D_OFFSET,
	XPLL_REG_23_D_OFFSET,
};

/**
 * Line rate bucket look-up table.
 * PllVals order: REG_2_M, REG_3_M, REG_4_O1, REG_5_O1,
 *               REG_6_O0, REG_7_O0, REG_22_D, REG_23_D
 *
 */
static const XMipi_Rx_Phy_Dynamic_LineRate
		XMipi_Rx_Phy_Dynamic_LineRateCfg[XMIPI_RX_PHY_LINERATE_NUM_BUCKETS] = {
	{  400,  540, { 0x9600, 0x1010, 0x1A00, 0x2020, 0x1A00, 0x0E0E, 0x0000, 0x0000 } },		/**< Bucket 1 */
	{  541, 1080, { 0x7600, 0x0808, 0x1A00, 0x1010, 0x1A00, 0x0707, 0x0000, 0x0000 } },		/**< Bucket 2 */
	{ 1081, 2160, { 0x5600, 0x0808, 0x1A00, 0x0808, 0x1B00, 0x0303, 0x0000, 0x0000 } },		/**< Bucket 3 */
	{ 2161, 2250, { 0x3700, 0x0505, 0x1A00, 0x0404, 0xBB00, 0x0101, 0x0000, 0x0000 } },		/**< Bucket 4 */
	{ 2251, 4320, { 0x3600, 0x0808, 0x1A00, 0x0404, 0x1B00, 0x0303, 0x0000, 0x0000 } },		/**< Bucket 5 */
	{ 4321, 4500, { 0x1700, 0x0505, 0x1A00, 0x0202, 0xBB00, 0x0101, 0x0000, 0x0000 } },		/**< Bucket 6 */
};

/****************************************************************************/
/**
* This function programs the X5PLL registers for a given line rate. It
* looks up the matching bucket from the line rate table, then performs the
* PLL unlock -> configure -> re-lock sequence.
*
* @param	PllBaseAddr is the base address of the X5PLL register space.
* @param	LineRate is the desired line rate in Mbps (400-4500 Mbps).
*
* @return
*		- XST_SUCCESS if PLL programming is successful.
*		- XST_FAILURE if no matching line rate bucket is found.
*
* @note		None.
*
*****************************************************************************/
u32 XMipi_Rx_Phy_XPllConfig(UINTPTR PllBaseAddr, u32 LineRate)
{
	u32 Index;
	const XMipi_Rx_Phy_Dynamic_LineRate *Bucket = NULL;

	/* Verify arguments */
	Xil_AssertNonvoid(PllBaseAddr != 0U);

	/* Search for the matching bucket based on the requested line rate */
	for (Index = 0U; Index < XMIPI_RX_PHY_LINERATE_NUM_BUCKETS; Index++) {
		if ((LineRate >= XMipi_Rx_Phy_Dynamic_LineRateCfg[Index].LineRateLb) &&
		    (LineRate <= XMipi_Rx_Phy_Dynamic_LineRateCfg[Index].LineRateUb)) {
			Bucket = &XMipi_Rx_Phy_Dynamic_LineRateCfg[Index];
			break;
		}
	}

	if (Bucket == NULL) {
		return XST_FAILURE;
	}

	/* Fabric Access: Unlock PLL */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_MASK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_CONTROL_LOCK_VAL);

	/* APB Access: Enter configuration mode */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_MASK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_CONTROL_CONFIG_VAL);

	/* Enable Output Disable */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_EN_ODISABLE_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_EN_ODISABLE_VAL);

	/* Enable Init State */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_EN_INITSTATE_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_EN_INITSTATE_VAL);


	/* Program all PLL registers from the bucket */
	for (Index = 0U; Index < XMIPI_RX_PHY_PLL_NUM_REGS; Index++) {
		XPll_WriteReg(PllBaseAddr,
				XMipi_Rx_Phy_PllRegOffsets[Index],
				Bucket->PllVals[Index]);
	}

	/* Disable Init State */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_EN_INITSTATE_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_DIS_INITSTATE);

	/* Disable Output Disable */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_EN_ODISABLE_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_DIS_ODISABLE);

	/* Fabric Access: Re-lock PLL */
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_UNLOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_MASK_OFFSET,
			XMIPI_RX_PHY_PLL_MASK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_CONTROL_OFFSET,
			XMIPI_RX_PHY_PLL_CONTROL_LOCK_VAL);
	XPll_WriteReg(PllBaseAddr, XPLL_REG_PCSR_LOCK_OFFSET,
			XMIPI_RX_PHY_PLL_LOCK_VAL);

	return XST_SUCCESS;
}

/** @} */
