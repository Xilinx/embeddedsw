/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 *
 * @file xfsbl_ddr_init.c
 *
 * This is the file which contains initialization code for the DDR. This
 * code is used for all the ZynqMP boards.
 *
 * This code will identify the DDR DIMM part by fetching SPD data from EEPROM
 * of the DIMM on run time and Initialize the same.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.0   mn   07/06/18 Add DDR initialization support for new DDR DIMM part
 *       mn   07/18/18 Move iicps.h inclusion under ZCU102 and ZCU106 macro
 *                     checks
 * 2.0   mn   02/28/19 Add Dynamic DDR initialization support for all DDR DIMMs
 *       mn   03/12/19 Select EEPROM Lower Page for reading SPD data
 *       mn   09/03/19 Fix coverity warnings
 * 3.0   bsv  11/12/19 Added support for ZCU216 board
 *       mn   12/24/19 Enable Address Mirroring based on SPD data
 *       bsv  02/05/20 Added support for ZCU208 board
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"

#ifdef XFSBL_PS_DDR
#ifdef XPAR_DYNAMIC_DDR_ENABLED

#include "xiicps.h"
#include "xfsbl_ddr_init.h"

/************************** Constant Definitions *****************************/

/* Default values for DDRC register fields */
#define XFSBL_DDRC_REG_DEFVAL {					\
	0x0U, 0x0U, 0x0U, 0x4U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x3U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x800000U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x40U, 0x20U, 0x10U, 0x2U, 0x10U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x62U,	\
	0x0U, 0x8CU, 0x0U, 0x0U, 0x0U, 0x0U, 0x10U, 0x1U,	\
	0x0U, 0x0U, 0x0U, 0x0U, 0x30U, 0x5U, 0x0U,		\
	0x0U, 0x2U, 0x4EU, 0x0U, 0x0U, 0x0U, 0xDU, 0x5U,	\
	0x0U, 0x510U, 0x5102U, 0x5103U, 0x0U, 0x4U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x6U, 0x6U, 0xFU, 0xFU, 0x10U, 0x1BU,		\
	0xFU, 0x8U, 0x4U, 0x14U, 0x3U, 0x5U, 0x6U, 0xDU,	\
	0x5U, 0x4U, 0xCU, 0x5U, 0x4U, 0x4U, 0x5U, 0x5U,		\
	0x5U, 0x4U, 0x3U, 0x0U, 0x0U, 0x5U, 0x2U, 0x2U,		\
	0x3U, 0x3U, 0x44U, 0x5U, 0x0U, 0x4U, 0x4U, 0xDU,	\
	0x44U, 0xCU, 0x0U, 0x1CU, 0x2U, 0x6U, 0x10U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x200U, 0x40U, 0x20U,		\
	0x100U, 0x0U, 0x0U, 0x2U, 0x0U, 0x0U, 0x2U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x40U, 0x3U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x2U,	\
	0x2U, 0x0U, 0x0U, 0x1U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x4U, 0x0U, 0x4U, 0x0U, 0x0U, 0x2U, 0x0U, 0x1U,		\
	0x0U, 0x0U, 0x20U, 0x0U, 0x0U, 0x1U, 0x0U, 0x0U,	\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U		\
}

/* Default values for DDR-PHY register fields */
#define XFSBL_PHY_REG_DEFVAL {					\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0xEU, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x5U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x4U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x1D0U, 0x0U, 0x0U, 0x200U, 0x8U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x1U, 0xDU, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x1U, 0xDU, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x2U, 0x1U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x5U, 0x4U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x5U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x5U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x19U,	\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x7U, 0x7U,		\
	0xBU, 0xBU, 0x0U, 0x1U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x1U,		\
	0x0U, 0x0U, 0x0U, 0x1U, 0x1U, 0x1U, 0x1U, 0x1U,		\
	0xFFU, 0x1U, 0x1U, 0x1U, 0x1U, 0x1U, 0x1U, 0x0U,	\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x2U,		\
	0x0U, 0x0U, 0x0U, 0x1U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x1U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U,		\
	0x0U, 0x0U, 0x0U, 0x0U, 0x0U, 0x0U			\
}

/* Column offset Value used for HIF calculation */
#define XFSBL_HIF_COLUMN(XVal)				(100U + (XVal))
/* Row offset Value used for HIF calculation */
#define XFSBL_HIF_ROW(XVal)				(200U + (XVal))
/* Bank offset Value used for HIF calculation */
#define XFSBL_HIF_BANK(XVal)				(300U + (XVal))
/* Bank Group offset Value used for HIF calculation */
#define XFSBL_HIF_BG(XVal)				(400U + (XVal))
/* Rank offset Value used for HIF calculation */
#define XFSBL_HIF_RANK(XVal)				(500U + (XVal))

/* IIC Serial Clock rate */
#define XFSBL_IIC_SCLK_RATE		100000U
/* IIC Mux Address */
#define XFSBL_MUX_ADDR			0x75U
/* SODIMM Slave Address */
#define XFSBL_SODIMM_SLAVE_ADDR		0x51U
/* SODIMM Control Address Low */
#define XFSBL_SODIMM_CONTROL_ADDR_LOW	0x36U
/* SODIMM Control Address High */
#define XFSBL_SODIMM_CONTROL_ADDR_HIGH	0x37U
/* IIC Bus Idle Timeout */
#define XFSBL_IIC_BUS_TIMEOUT		1000000U

#define XFSBL_DDR_TRAINING_TIMEOUT	1000000U

#define XFSBL_DDRC_BASE_ADDR		0xFD070000U

#define XFSBL_DDRPHY_BASE_ADDR		0xFD080000U

#define XFSBL_DBI_INFO			XPAR_PSU_DDRC_0_DDR_DATA_MASK_AND_DBI

#define XFSBL_VIDEOBUF			XPAR_PSU_DDRC_0_VIDEO_BUFFER_SIZE

#define XFSBL_BRCMAPPING		XPAR_PSU_DDRC_0_BRC_MAPPING

#define XFSBL_DDR4ADDRMAPPING		XPAR_PSU_DDRC_0_DDR4_ADDR_MAPPING
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Macro to find the maximum between two values */
#define XFSBL_MAX(Val1, Val2) (((Val1) > (Val2)) ? (Val1) : (Val2))
/* Macro to find the minimum between two values */
#define XFSBL_MIN(Val1, Val2) (((Val1) < (Val2)) ? (Val1) : (Val2))
/* Macro to set particular bits in a register */
#define XFSBL_SETBITS(a, b, c)		((a & ((1U << c) - 1U)) << b)
/* Macro  to poll for register bits to be set with certain value */
#define XFSBL_POLL(a, b, c)		{while ((Xil_In32(a) & (b)) != (c));}
/* Macro  to poll for register bits to be set equal to given mask value */
#define XFSBL_MASK_POLL(a, b)		{while ((Xil_In32(a) & (b)) != (b));}
/* Program the register with given value, shifts and mask */
#define XFSBL_PROG_REG(Addr, mask, shift, Value) {	\
	Xil_Out32((Addr), ((Xil_In32(Addr) &			\
					(~(mask))) | ((Value) << (shift))));			\
}
/* Convert the timing value from SPD to picoseconds */
#define XFSBL_SPD_TO_PS(Mtb, Ftb)	\
	(Mtb * PDimmPtr->MtbPs + (Ftb * (s8)PDimmPtr->Ftb10thPs) / 10)

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function returns log2 of the given value in argument
 *
 * @param	XVal is the value whose log2 value is returned
 *
 * @return	returns log2 of the given value
 *
 *****************************************************************************/
static u32 XFsbl_GetLog2(u32 XVal)
{
	u32 RVal = 0U;

	if (XVal == 0U) {
		goto END;
	}

	while (XVal != 1U) {
		RVal = RVal + 1U;
		XVal = XVal >> 1U;
	}

END:
	return RVal;
}

/*****************************************************************************/
/**
 * This function returns the particular DDR rank
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	returns DDR Rank Size
 *
 *****************************************************************************/
u64 XFsbl_ComputeDdr4RankSize(struct Ddr4SpdEeprom *Ddr4SpdData)
{
	u64 BSize;

	u32 NbitDdramCapBsize;
	u32 NbitPrimaryBusWidth;
	u32 NbitSdramWidth;
	u32 DieCount;
	u8 Package3Ds;

	if ((Ddr4SpdData->DensityBanks & 0xFU) <= 7U) {
		NbitDdramCapBsize = (Ddr4SpdData->DensityBanks & 0xFU) + 28U;
	} else {
		NbitDdramCapBsize = 0U;
	}

	if ((Ddr4SpdData->BusWidth & 0x7U) < 4U) {
		NbitPrimaryBusWidth = (Ddr4SpdData->BusWidth & 0x7U) + 3U;
	} else {
		NbitPrimaryBusWidth = 0U;
	}

	if ((Ddr4SpdData->Organization & 0x7U) < 4U) {
		NbitSdramWidth = (Ddr4SpdData->Organization & 0x7U) + 2U;
	} else {
		NbitSdramWidth = 0U;
	}

	Package3Ds = (Ddr4SpdData->PackageType & 0x3U) == 0x2U;
	if (Package3Ds) {
		DieCount = (Ddr4SpdData->PackageType >> 4U) & 0x7U;
	} else {
		DieCount = 0U;
	}

	BSize = 1ULL << (NbitDdramCapBsize - 3U +
			NbitPrimaryBusWidth - NbitSdramWidth +
			DieCount);

	return BSize;
}

/*****************************************************************************/
/**
 * This function returns the particular DDR rank
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	returns DDR Rank Size
 *
 *****************************************************************************/
u64 XFsbl_ComputeDdr3RankSize(struct Ddr3SpdEeprom *Ddr3SpdData)
{
	u64 BSize;
	u32 NbitDdramCapBsize;
	u32 NbitPrimaryBusWidth;
	u32 NbitSdramWidth;

	if ((Ddr3SpdData->DensityBanks & 0xFU) < 7U) {
		NbitDdramCapBsize = (Ddr3SpdData->DensityBanks & 0xFU) + 28U;
	} else {
		NbitDdramCapBsize = 0U;
	}

	if ((Ddr3SpdData->BusWidth & 0x7U) < 4U) {
		NbitPrimaryBusWidth = (Ddr3SpdData->BusWidth & 0x7U) + 3U;
	} else {
		NbitPrimaryBusWidth = 0U;
	}

	if ((Ddr3SpdData->Organization & 0x7U) < 4U) {
		NbitSdramWidth = (Ddr3SpdData->Organization & 0x7U) + 2U;
	} else {
		NbitSdramWidth = 0U;
	}

	BSize = 1ULL << (NbitDdramCapBsize - 3U
			+ NbitPrimaryBusWidth - NbitSdramWidth);

	return BSize;
}

/*****************************************************************************/
/**
 * This function returns the particular DDR rank
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	returns DDR Rank Size
 *
 *****************************************************************************/
u64 XFsbl_ComputeLpDdrRankSize(struct LpDdrSpdEeprom *LpDdrSpdData)
{
	u64 BSize;

	u32 NbitDdramCapBsize;
	u32 NbitPrimaryBusWidth;
	u32 NbitSdramWidth;
	u32 DieCount;
	u8 Package3Ds;

	if ((LpDdrSpdData->DensityBanks & 0xFU) <= 7U) {
		NbitDdramCapBsize = (LpDdrSpdData->DensityBanks & 0xFU) + 28U;
	} else {
		NbitDdramCapBsize = 0U;
	}

	if ((LpDdrSpdData->BusWidth & 0x7U) < 4U) {
		NbitPrimaryBusWidth = (LpDdrSpdData->BusWidth & 0x7U) + 3U;
	} else {
		NbitPrimaryBusWidth = 0U;
	}

	if ((LpDdrSpdData->Organization & 0x7U) < 4U) {
		NbitSdramWidth = (LpDdrSpdData->Organization & 0x7U) + 2U;
	} else {
		NbitSdramWidth = 0U;
	}

	Package3Ds = (LpDdrSpdData->PackageType & 0x3U) == 0x2U;
	if (Package3Ds) {
		DieCount = (LpDdrSpdData->PackageType >> 4U) & 0x7U;
	} else {
		DieCount = 0U;
	}

	BSize = 1ULL << (NbitDdramCapBsize - 3U +
			NbitPrimaryBusWidth - NbitSdramWidth +
			DieCount);

	return BSize;
}

/*****************************************************************************/
/**
 * This function computes DIMM parameters based upon the SPD information in
 * Ddr4SpdData. Writes the results to the XFsbl_DimmParams structure pointed
 * by PDimmPtr.
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	SpdData is the array containing the SPD data from DIMM EEPROM
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
u32 XFsbl_ComputeDdr4Params(u8 *SpdData, struct DdrcInitData *DdrDataPtr)
{
	struct Ddr4SpdEeprom *Ddr4SpdData = (struct Ddr4SpdEeprom *)SpdData;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 Status = XFSBL_FAILURE;

	memset(PDimmPtr->Mpart, 0U, sizeof(PDimmPtr->Mpart));
	if ((Ddr4SpdData->InfoSizeCrc & 0xFU) > 2U)
		memcpy(PDimmPtr->Mpart, Ddr4SpdData->Mpart, sizeof(PDimmPtr->Mpart) - 1U);

	PDimmPtr->MemType = Ddr4SpdData->MemType;

	PDimmPtr->NRanks = ((Ddr4SpdData->Organization >> 3U) & 0x7U) + 1U;
	PDimmPtr->RankDensity = XFsbl_ComputeDdr4RankSize(Ddr4SpdData);
	PDimmPtr->Capacity = (PDimmPtr->NRanks * PDimmPtr->RankDensity) / (1024U * 1024U);
	PDimmPtr->BusWidth = 1U << (3U + (Ddr4SpdData->BusWidth & 0x7U));
	if ((Ddr4SpdData->BusWidth >> 3U) & 0x3U)
		PDimmPtr->EccBusWidth = 8U;
	else
		PDimmPtr->EccBusWidth = 0U;
	PDimmPtr->DramWidth = 1U << ((Ddr4SpdData->Organization & 0x7U) + 2U);

	PDimmPtr->AddrMirror = 0U;
	PDimmPtr->RDimm = 0U;
	switch (Ddr4SpdData->ModuleType & DDR4_SPD_MODULETYPE_MASK) {
		case DDR4_SPD_MODULETYPE_RDIMM:
		case DDR4_SPD_MODULETYPE_72B_SO_RDIMM:
			PDimmPtr->RDimm = 1U;
			break;

		case DDR4_SPD_MODULETYPE_UDIMM:
		case DDR4_SPD_MODULETYPE_SO_DIMM:
		case DDR4_SPD_MODULETYPE_72B_SO_UDIMM:
			PDimmPtr->UDimm = 1U;
			if (Ddr4SpdData->ModSection.unbuffered.AddrMapping & 0x1U)
				PDimmPtr->AddrMirror = 1U;
			break;

		default:
			/* Do nothing as Status is initialized to XFSBL_FAILURE */
			goto END;
	}

	PDimmPtr->NumRowAddr = ((Ddr4SpdData->Addressing >> 3U) & 0x7U) + 12U;
	PDimmPtr->NumColAddr = (Ddr4SpdData->Addressing & 0x7U) + 9U;
	PDimmPtr->NumBankAddr = ((Ddr4SpdData->DensityBanks >> 4U) & 0x3U) + 2U;
	PDimmPtr->NumBgAddr = (Ddr4SpdData->DensityBanks >> 6U) & 0x3U;
	PDimmPtr->NumRankAddr = XFsbl_GetLog2(PDimmPtr->NRanks);

	PDimmPtr->BurstLength = 8U;
	PDimmPtr->RowDensity = XFsbl_GetLog2(PDimmPtr->RankDensity);

	if ((Ddr4SpdData->Timebases & 0xFU) == 0x0U) {
		PDimmPtr->MtbPs = 125U;
		PDimmPtr->Ftb10thPs = 10U;
	}

	PDimmPtr->TckminXPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TckMin, Ddr4SpdData->FineTckMin);

	PDimmPtr->SpeedBin = (u32)XFsbl_Ceil(2000000.0 / PDimmPtr->TckminXPs);

	PDimmPtr->FreqMhz = 1000000.0 / PDimmPtr->TckminXPs;

	PDimmPtr->ClockPeriod = PDimmPtr->TckminXPs / 1000.0;

	PDimmPtr->TckmaxPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TckMax, Ddr4SpdData->FineTckMax);

	PDimmPtr->CaslatX  = (Ddr4SpdData->CaslatB1 << 7U)	|
		(Ddr4SpdData->CaslatB2 << 15U)	|
		(Ddr4SpdData->CaslatB3 << 23U);

	PDimmPtr->TaaPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TaaMin, Ddr4SpdData->FineTaaMin);

	PDimmPtr->CasLatency = (u32)XFsbl_Ceil(PDimmPtr->TaaPs / 1000.0) + 1U;

	PDimmPtr->CasWriteLatency = XFsbl_Ceil(PDimmPtr->TaaPs / 1000.0);

	PDimmPtr->TRcdPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TrcdMin, Ddr4SpdData->FineTrcdMin);

	PDimmPtr->TRpPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TrpMin, Ddr4SpdData->FineTrpMin);

	PDimmPtr->TRasPs = (((Ddr4SpdData->TrasTrcExt & 0xFU) << 8U) +
			Ddr4SpdData->TrasMinLsb) * PDimmPtr->MtbPs;

	PDimmPtr->TRcPs = XFSBL_SPD_TO_PS((((Ddr4SpdData->TrasTrcExt & 0xF0U) << 4U) +
				Ddr4SpdData->TrcMinLsb), Ddr4SpdData->FineTrcMin);
	PDimmPtr->TRfc1Ps = ((Ddr4SpdData->Trfc1MinMsb << 8U) | (Ddr4SpdData->Trfc1MinLsb)) *
		PDimmPtr->MtbPs;
	PDimmPtr->TRfc2Ps = ((Ddr4SpdData->Trfc2MinMsb << 8U) | (Ddr4SpdData->Trfc2MinLsb)) *
		PDimmPtr->MtbPs;
	PDimmPtr->TRfc4Ps = ((Ddr4SpdData->Trfc4MinMsb << 8U) | (Ddr4SpdData->Trfc4MinLsb)) *
		PDimmPtr->MtbPs;
	PDimmPtr->TFawPs = (((Ddr4SpdData->TfawMsb & 0xFU) << 8U) | Ddr4SpdData->TfawMin) *
		PDimmPtr->MtbPs;

	PDimmPtr->TRrdsPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TrrdsMin, Ddr4SpdData->FineTrrdsMin);
	PDimmPtr->TRrdlPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TrrdlMin, Ddr4SpdData->FineTrrdlMin);
	PDimmPtr->TCcdlPs = XFSBL_SPD_TO_PS(Ddr4SpdData->TccdlMin, Ddr4SpdData->FineTccdlMin);

	PDimmPtr->TRefi = 7800000U;
	Status = XFSBL_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function computes DIMM parameters based upon the SPD information in
 * Dd34SpdData. Writes the results to the XFsbl_DimmParams structure pointed
 * by PDimmPtr.
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	SpdData is the array containing the SPD data from DIMM EEPROM
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
u32 XFsbl_ComputeDdr3Params(u8 *SpdData, struct DdrcInitData *DdrDataPtr)
{
	struct Ddr3SpdEeprom *Ddr3SpdData = (struct Ddr3SpdEeprom *)SpdData;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	u32 MtbPs;
	u32 Ftb10thPs;
	u32 Status = XFSBL_FAILURE;

	memset(PDimmPtr->Mpart, 0U, sizeof(PDimmPtr->Mpart));
	if ((Ddr3SpdData->InfoSizeCrc & 0xFU) > 1U)
		memcpy(PDimmPtr->Mpart, Ddr3SpdData->Mpart, sizeof(PDimmPtr->Mpart) - 1U);

	PDimmPtr->MemType = Ddr3SpdData->MemType;
	PDimmPtr->NRanks = ((Ddr3SpdData->Organization >> 3U) & 0x7U) + 1U;
	PDimmPtr->RankDensity = XFsbl_ComputeDdr3RankSize(Ddr3SpdData);
	PDimmPtr->Capacity = PDimmPtr->NRanks * PDimmPtr->RankDensity;
	PDimmPtr->BusWidth = 1U << (3U + (Ddr3SpdData->BusWidth & 0x7U));
	if ((Ddr3SpdData->BusWidth >> 3U) & 0x3U)
		PDimmPtr->EccBusWidth = 8U;
	else
		PDimmPtr->EccBusWidth = 0U;
	PDimmPtr->DramWidth = 1U << ((Ddr3SpdData->Organization & 0x7U) + 2U);

	PDimmPtr->AddrMirror = 0U;
	PDimmPtr->RDimm = 0U;
	switch (Ddr3SpdData->ModuleType & DDR3_SPD_MODULETYPE_MASK) {
		case DDR3_SPD_MODULETYPE_RDIMM:
		case DDR3_SPD_MODULETYPE_MINI_RDIMM:
		case DDR3_SPD_MODULETYPE_72B_SO_RDIMM:
			PDimmPtr->RDimm = 1U;
			break;

		case DDR3_SPD_MODULETYPE_UDIMM:
		case DDR3_SPD_MODULETYPE_SO_DIMM:
		case DDR3_SPD_MODULETYPE_MICRO_DIMM:
		case DDR3_SPD_MODULETYPE_MINI_UDIMM:
		case DDR3_SPD_MODULETYPE_MINI_CDIMM:
		case DDR3_SPD_MODULETYPE_72B_SO_UDIMM:
		case DDR3_SPD_MODULETYPE_72B_SO_CDIMM:
		case DDR3_SPD_MODULETYPE_LRDIMM:
		case DDR3_SPD_MODULETYPE_16B_SO_DIMM:
		case DDR3_SPD_MODULETYPE_32B_SO_DIMM:
			if (Ddr3SpdData->ModSection.unbuffered.AddrMapping & 0x1U)
				PDimmPtr->AddrMirror = 1U;
			break;

		default:
			/* Do nothing as Status is initialized to XFSBL_FAILURE */
			goto END;
	}

	PDimmPtr->NBanksPerSdramDevice = 8U << ((Ddr3SpdData->DensityBanks >> 4U) & 0x7U);
	PDimmPtr->NumRowAddr = ((Ddr3SpdData->Addressing >> 3U) & 0x7U) + 12U;
	PDimmPtr->NumColAddr = (Ddr3SpdData->Addressing & 0x7U) + 9U;
	PDimmPtr->NumBankAddr = ((Ddr3SpdData->DensityBanks >> 4U) & 0x3U) + 2U;
	PDimmPtr->NumBgAddr = (Ddr3SpdData->DensityBanks >> 6U) & 0x3U;
	PDimmPtr->NumRankAddr = XFsbl_GetLog2(PDimmPtr->NRanks);

	PDimmPtr->BurstLength = 8U;
	PDimmPtr->RowDensity = XFsbl_GetLog2(PDimmPtr->RankDensity);

	MtbPs = (Ddr3SpdData->MtbDividend * 1000U) / Ddr3SpdData->MtbDivisor;
	PDimmPtr->MtbPs = MtbPs;

	Ftb10thPs =
		((Ddr3SpdData->FtbDiv & 0xF0U) >> 4U) * 10U / (Ddr3SpdData->FtbDiv & 0x0fU);
	PDimmPtr->Ftb10thPs = Ftb10thPs;
	PDimmPtr->TckminXPs = Ddr3SpdData->TckMin * MtbPs +
		(Ddr3SpdData->FineTckMin * Ftb10thPs) / 10U;

	PDimmPtr->CaslatX  = ((Ddr3SpdData->CaslatMsb << 8U) | Ddr3SpdData->CaslatLsb) << 4U;

	PDimmPtr->TaaPs = Ddr3SpdData->TaaMin * MtbPs +
		(Ddr3SpdData->FineTaaMin * Ftb10thPs) / 10U;

	PDimmPtr->TwrPs = Ddr3SpdData->TwrMin * MtbPs;

	PDimmPtr->TRcdPs = Ddr3SpdData->TrcdMin * MtbPs +
		(Ddr3SpdData->FineTrcdMin * Ftb10thPs) / 10U;

	PDimmPtr->TRrdPs = Ddr3SpdData->TrrdMin * MtbPs;

	PDimmPtr->TRpPs = Ddr3SpdData->TrpMin * MtbPs +
		(Ddr3SpdData->FineTrpMin * Ftb10thPs) / 10U;

	PDimmPtr->TRasPs = (((Ddr3SpdData->TrasTrcExt & 0xFU) << 8U) | Ddr3SpdData->TrasMinLsb)
		* MtbPs;
	PDimmPtr->TRcPs = (((Ddr3SpdData->TrasTrcExt & 0xF0U) << 4U) | Ddr3SpdData->TrcMinLsb)
		* MtbPs + (Ddr3SpdData->FineTrcMin * Ftb10thPs) / 10U;
	PDimmPtr->TRfcPs = ((Ddr3SpdData->TrfcMinMsb << 8U) | Ddr3SpdData->TrfcMinLsb)
		* MtbPs;
	PDimmPtr->TwtrPs = Ddr3SpdData->TwtrMin * MtbPs;

	PDimmPtr->TrtpPs = Ddr3SpdData->TrtpMin * MtbPs;

	PDimmPtr->TRefi = 7800000U;
	if ((Ddr3SpdData->ThermRefOpt & 0x1U) && !(Ddr3SpdData->ThermRefOpt & 0x2U)) {
		PDimmPtr->TRefi = 3900000U;
	}

	PDimmPtr->TFawPs = (((Ddr3SpdData->TfawMsb & 0xFU) << 8U) | Ddr3SpdData->TfawMin)
		* MtbPs;
	Status = XFSBL_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function computes DIMM parameters based upon the SPD information in
 * LpDdrSpdData. Writes the results to the XFsbl_DimmParams structure pointed
 * by PDimmPtr.
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	SpdData is the array containing the SPD data from DIMM EEPROM
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
u32 XFsbl_ComputeLpDdrParams(u8 *SpdData, struct DdrcInitData *DdrDataPtr)
{
	struct LpDdrSpdEeprom *LpDdrSpdData = (struct LpDdrSpdEeprom *)SpdData;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	memset(PDimmPtr->Mpart, 0U, sizeof(PDimmPtr->Mpart));
	if ((LpDdrSpdData->InfoSizeCrc & 0xFU) > 2U)
		memcpy(PDimmPtr->Mpart, LpDdrSpdData->Mpart, sizeof(PDimmPtr->Mpart) - 1U);

	PDimmPtr->MemType = LpDdrSpdData->MemType;
	PDimmPtr->NRanks = ((LpDdrSpdData->Organization >> 3U) & 0x7U) + 1U;
	PDimmPtr->RankDensity = XFsbl_ComputeLpDdrRankSize(LpDdrSpdData);
	PDimmPtr->Capacity = (PDimmPtr->NRanks * PDimmPtr->RankDensity) / (1024U * 1024U);
	PDimmPtr->BusWidth = 1U << (3U + (LpDdrSpdData->BusWidth & 0x7U));
	if ((LpDdrSpdData->BusWidth >> 3U) & 0x3U)
		PDimmPtr->EccBusWidth = 8U;
	else
		PDimmPtr->EccBusWidth = 0U;
	PDimmPtr->DramWidth = 1U << ((LpDdrSpdData->Organization & 0x7U) + 2U);

	PDimmPtr->AddrMirror = 0U;
	PDimmPtr->RDimm = 0U;

	PDimmPtr->NumRowAddr = ((LpDdrSpdData->Addressing >> 3U) & 0x7U) + 12U;
	PDimmPtr->NumColAddr = (LpDdrSpdData->Addressing & 0x7U) + 9U;
	PDimmPtr->NumBankAddr = ((LpDdrSpdData->DensityBanks >> 4U) & 0x3U) + 2U;
	PDimmPtr->NumBgAddr = (LpDdrSpdData->DensityBanks >> 6U) & 0x3U;
	PDimmPtr->NumRankAddr = XFsbl_GetLog2(PDimmPtr->NRanks);

	PDimmPtr->BurstLength = 8U;
	PDimmPtr->RowDensity = XFsbl_GetLog2(PDimmPtr->RankDensity);

	if ((LpDdrSpdData->Timebases & 0xFU) == 0x0U) {
		PDimmPtr->MtbPs = 125U;
		PDimmPtr->Ftb10thPs = 10U;
	}

	PDimmPtr->TckminXPs = XFSBL_SPD_TO_PS(LpDdrSpdData->TckMin, LpDdrSpdData->FineTckMin);

	PDimmPtr->SpeedBin = (u32)XFsbl_Ceil(2000000.0 / PDimmPtr->TckminXPs);

	PDimmPtr->FreqMhz = 1000000.0 / PDimmPtr->TckminXPs;

	PDimmPtr->ClockPeriod = PDimmPtr->TckminXPs / 1000.0;

	PDimmPtr->TckmaxPs = XFSBL_SPD_TO_PS(LpDdrSpdData->TckMax, LpDdrSpdData->FineTckMax);

	PDimmPtr->CaslatX  = (LpDdrSpdData->CaslatB1 << 7U)	|
		(LpDdrSpdData->CaslatB2 << 15U)	|
		(LpDdrSpdData->CaslatB3 << 23U);

	PDimmPtr->TaaPs = XFSBL_SPD_TO_PS(LpDdrSpdData->TaaMin, LpDdrSpdData->FineTaaMin);

	PDimmPtr->CasLatency = (u32)XFsbl_Ceil(PDimmPtr->TaaPs / 1000.0) + 1U;

	PDimmPtr->CasWriteLatency = XFsbl_Ceil(PDimmPtr->TaaPs / 1000.0);

	PDimmPtr->TRcdPs = XFSBL_SPD_TO_PS(LpDdrSpdData->TrcdMin, LpDdrSpdData->FineTrcdMin);

	PDimmPtr->TrpabPs = XFSBL_SPD_TO_PS(LpDdrSpdData->TrpabMin, LpDdrSpdData->FineTrpabMin);
	PDimmPtr->TrppbPs = XFSBL_SPD_TO_PS(LpDdrSpdData->TrppbMin, LpDdrSpdData->FineTrppbMin);

	PDimmPtr->TRfcAbPs = ((LpDdrSpdData->TrfcabMinMsb << 8U) |
			(LpDdrSpdData->TrfcabMinLsb)) *
		PDimmPtr->MtbPs;
	PDimmPtr->TRfcPbPs = ((LpDdrSpdData->TrfcpbMinMsb << 8U) |
			(LpDdrSpdData->TrfcpbMinLsb)) *
		PDimmPtr->MtbPs;

	if ((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) ||
			((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) &&
			 (PDimmPtr->Capacity != 1024U))) {
		PDimmPtr->TRefi = 3900000U;
	} else {
		PDimmPtr->TRefi = 7800000U;
	}

	return XFSBL_SUCCESS;
}

#if !(defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208))
/*****************************************************************************/
/**
 * This function computes DIMM parameters based upon the SPD information.
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	SpdData is the array containing SPD data got from EEPROM
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
static u32 XFsbl_DdrComputeDimmParameters(u8 *SpdData,
		struct DdrcInitData *DdrDataPtr)
{
	u32 Status = XFSBL_FAILURE;

	switch (SpdData[2U]) {
	case SPD_MEMTYPE_DDR4:
		Status = XFsbl_ComputeDdr4Params(SpdData, DdrDataPtr);
		break;
	case SPD_MEMTYPE_DDR3:
		Status = XFsbl_ComputeDdr3Params(SpdData, DdrDataPtr);
		break;
	case SPD_MEMTYPE_LPDDR3:
	case SPD_MEMTYPE_LPDDR4:
		Status = XFsbl_ComputeLpDdrParams(SpdData, DdrDataPtr);
		break;
	default:
		/* Do nothing as Status is initialized to XFSBL_FAILURE */
		break;
	}

	return Status;
}
#endif

/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Non-DDR4 mode
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcDdr4HifAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 Position = 0U;
	u32 Index;

	/* Define Column positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumColAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Bank Group positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumBgAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_BG(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumBankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRowAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}

#if (XFSBL_VIDEOBUF != 0U)
/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Video mapping mode
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 * @param	VideoBuf is size of Video Buffer used in the design
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddrVideo(struct DdrcInitData *DdrDataPtr,
		u32 *HifAddr, u32 VideoBuf)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 Position=0U;
	u32 Index;
	u32 BufWidth;
	u32 RemainingRow;

	/* Calculate Buffer Width based on Video Buffer Size */
	for (Index = 0U; Index <= 6U; Index++) {
		if (((u32)1U << Index) == VideoBuf) {
			BufWidth = Index + 20U;
			break;
		} else {
			BufWidth = 20U;
		}
	}

    if (PDimmPtr->BusWidth == 16U) {
	BufWidth = BufWidth + 2U;
    }

    if (PDimmPtr->BusWidth == 32U) {
	BufWidth = BufWidth + 1U;
    }

	if (PDimmPtr->MemType != SPD_MEMTYPE_DDR4) {
		/* Define Column positions in HIF Addresses */
		for (Index = 3U; Index < PDimmPtr->NumColAddr; Index++) {
			HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
			Position++;
		}

		/* Define Row positions in HIF Addresses */
		for (Index = 0U; Index < (BufWidth - PDimmPtr->NumColAddr - 3U); Index++) {
			HifAddr[Position] = XFSBL_HIF_ROW(Index);
			Position++;
		}

		/* Define Bank positions in HIF Addresses */
		for (Index = 0U; Index < PDimmPtr->NumBankAddr; Index++) {
			HifAddr[Position] = XFSBL_HIF_BANK(Index);
			Position++;
		}

		/* Define Remaining Row positions in HIF Addresses */
		RemainingRow = PDimmPtr->NumRowAddr - (BufWidth - PDimmPtr->NumColAddr - 3U);
		for (Index = 0U; Index < RemainingRow; Index++) {
			HifAddr[Position] = XFSBL_HIF_ROW(Index) + (BufWidth -
					PDimmPtr->NumColAddr - 4U);
			Position++;
		}

		/* Define Rank positions in HIF Addresses */
		for (Index = 0U; Index < PDimmPtr->NumRankAddr; Index++) {
			HifAddr[Position] = XFSBL_HIF_RANK(Index);
			Position++;
		}
	} else {
		/* First four HIF addresses are fixed for Video Mapping */
		HifAddr[0U] = XFSBL_HIF_COLUMN(0U);
		HifAddr[1U] = XFSBL_HIF_COLUMN(1U);
		HifAddr[2U] = XFSBL_HIF_COLUMN(2U);
		HifAddr[3U] = XFSBL_HIF_BG(0U);

		Position = 4U;
		/* Define Column positions in HIF Addresses */
		for (Index = 3U; Index < PDimmPtr->NumColAddr; Index++) {
			HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
			Position++;
		}

		/* Define Row positions in HIF Addresses */
		for (Index = 0U; Index < (BufWidth - PDimmPtr->NumColAddr - 4U); Index++) {
			HifAddr[Position] = XFSBL_HIF_ROW(Index);
			Position++;
		}

		/* Define Bank positions in HIF Addresses */
		for (Index = 0U; Index < PDimmPtr->NumBankAddr; Index++) {
			HifAddr[Position] = XFSBL_HIF_BANK(Index);
			Position++;
		}

		/* Define Remaining Row positions in HIF Addresses */
		RemainingRow = PDimmPtr->NumRowAddr - (BufWidth - PDimmPtr->NumColAddr - 4U);
		for (Index = 0U; Index < RemainingRow; Index++) {
			HifAddr[Position] = XFSBL_HIF_ROW(Index) + (BufWidth -
					PDimmPtr->NumColAddr - 4U);
			Position++;
		}

		/* Define Rank positions in HIF Addresses */
		for (Index = 0U; Index < PDimmPtr->NumRankAddr; Index++) {
			HifAddr[Position] = XFSBL_HIF_RANK(Index);
			Position++;
		}
	}
}
#endif

#if (XFSBL_BRCMAPPING == 1U)
/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Bank-Row-Column mapping mode
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddrBrcMap(struct DdrcInitData *DdrDataPtr,
		u32 *HifAddr)
{
	u32 Position;
	u32 Index;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* First four HIF addresses are fixed for BRC Mapping */
	HifAddr[0U] = XFSBL_HIF_COLUMN(0U);
	HifAddr[1U] = XFSBL_HIF_COLUMN(1U);
	HifAddr[2U] = XFSBL_HIF_COLUMN(2U);
	HifAddr[3U] = XFSBL_HIF_BG(0U);

	Position = 4U;
	/* Define Column positions in HIF Addresses */
	for (Index = 3U; Index < PDimmPtr->NumColAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRowAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumBankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}
#endif

#if (XFSBL_DDR4ADDRMAPPING == 1U)
/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Address Mapping Enabled mode
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddrMemMap(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Position;
	u32 Index;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* First four HIF addresses are fixed for DDR4 Address Mapped Mode */
	HifAddr[0U] = XFSBL_HIF_COLUMN(0U);
	HifAddr[1U] = XFSBL_HIF_COLUMN(1U);
	HifAddr[2U] = XFSBL_HIF_COLUMN(2U);
	HifAddr[3U] = XFSBL_HIF_BG(0U);

	Position = 4U;
	/* Define Column positions in HIF Addresses */
	for (Index = 3U; Index < PDimmPtr->NumColAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumBankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRowAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}
#endif

/*****************************************************************************/
/**
 * This function calculates the HIF Addresses for Non-DDR4 mode
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcHifAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Position = 0U;
	u32 Index;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* Define Column positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumColAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_COLUMN(Index);
		Position++;
	}

	/* Define Bank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumBankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_BANK(Index);
		Position++;
	}

	/* Define Row positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRowAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_ROW(Index);
		Position++;
	}

	/* Define Rank positions in HIF Addresses */
	for (Index = 0U; Index < PDimmPtr->NumRankAddr; Index++) {
		HifAddr[Position] = XFSBL_HIF_RANK(Index);
		Position++;
	}
}

/*****************************************************************************/
/**
 * This function calculates the Bank Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcBankAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 BankBit;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* Calculate Bank Addresses */
	for (BankBit = 0U; BankBit < XFSBL_MAX_BANKS; BankBit++) {
		if (BankBit < PDimmPtr->NumBankAddr) {
			Index = BankBit + 2U;

			while (HifAddr[Index] != XFSBL_HIF_BANK(BankBit)) {
				Index++;
			}

			DdrDataPtr->AddrMapBankBit[BankBit] = Index -
				(BankBit + 2U);
		} else {
			DdrDataPtr->AddrMapBankBit[BankBit] = 0x1FU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Bank Group Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcBgAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 BgBit;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* Calculate Bank Group Addresses */
	for (BgBit = 0U; BgBit < XFSBL_MAX_BANK_GROUPS; BgBit++) {
		if (BgBit < PDimmPtr->NumBgAddr) {
			Index = BgBit + 2U;

			while (HifAddr[Index] != XFSBL_HIF_BG(BgBit)) {
				Index++;
			}

			if (Index >= (BgBit + 2U)) {
				DdrDataPtr->AddrMapBgBit[BgBit] = Index -
					(BgBit + 2U);
			} else {
				DdrDataPtr->AddrMapBgBit[BgBit] = 0U;
			}
		} else {
			DdrDataPtr->AddrMapBgBit[BgBit] = 0x1FU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Column Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcColAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 ColBit;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* Calculate Column Addresses */
	for (ColBit = 2U; ColBit < XFSBL_MAX_COLUMNS; ColBit++) {
		if (ColBit < PDimmPtr->NumColAddr) {
			Index = ColBit;

			while (HifAddr[Index] != XFSBL_HIF_COLUMN(ColBit)) {
				Index++;
			}

			DdrDataPtr->AddrMapColBit[ColBit] = Index - ColBit;
		} else {
			DdrDataPtr->AddrMapColBit[ColBit] = 0xFU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Row Address Map based on HIF Addresses
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	HifAddr is the pointer to the HIF Addresses Array
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcRowAddr(struct DdrcInitData *DdrDataPtr, u32 *HifAddr)
{
	u32 Index;
	u32 RowBit;
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	/* HIF address bits used as row address bits 2U to 10U */
	DdrDataPtr->AddrMapRowBits2To10 = 15U;

	/* Calculate Row Addresses */
	for (RowBit = 0U; RowBit < XFSBL_MAX_ROWS; RowBit++) {
		if (RowBit < PDimmPtr->NumRowAddr) {
			Index = RowBit + 6U;

			while (HifAddr[Index] != XFSBL_HIF_ROW(RowBit)) {
				Index++;
			}

			DdrDataPtr->AddrMapRowBit[RowBit] = Index -
				(RowBit + 6U);
		} else {
			DdrDataPtr->AddrMapRowBit[RowBit] = 0xFU;
		}
	}
}

/*****************************************************************************/
/**
 * This function calculates the Address Mapping of the DDR
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrCalcAddrMap(struct DdrcInitData *DdrDataPtr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 HifAddr[40U] = {0U};
	u32 RegVal[12U];
	u32 Index;

#if (XFSBL_VIDEOBUF != 0U)
	/* Calculate the HIF Addresses when Video Buffers are Enabled */
	XFsbl_DdrCalcHifAddrVideo(DdrDataPtr, HifAddr, VideoBuf);
#elif (XFSBL_BRCMAPPING == 1U)
	if (PDimmPtr->MemType != SPD_MEMTYPE_DDR4) {
		/*
		 * Calculate the HIF Addresses for Non-DDR4 Mapping
		 */
		XFsbl_DdrCalcHifAddr(DdrDataPtr, HifAddr);
	} else {
		/*
		 * Calculate the HIF Addresses when Bank-Row-Column Mapping is
		 * Enabled
		 */
		XFsbl_DdrCalcHifAddrBrcMap(DdrDataPtr, HifAddr);
	}
#else
	if (PDimmPtr->MemType != SPD_MEMTYPE_DDR4) {
		/*
		 * Calculate the HIF Addresses for Non-DDR4 Mapping
		 */
		XFsbl_DdrCalcHifAddr(DdrDataPtr, HifAddr);

	} else {
#if (XFSBL_DDR4ADDRMAPPING == 1U)
		/*
		 * Calculate the HIF Addresses when DDR4 Address Mapping
		 * is Enabled
		 */
		XFsbl_DdrCalcHifAddrMemMap(DdrDataPtr, HifAddr);
#else
		/*
		 * Calculate the HIF Addresses for default DDR4 Mapping
		 */
		XFsbl_DdrCalcDdr4HifAddr(DdrDataPtr, HifAddr);
#endif
	}
#endif

    if (PDimmPtr->BusWidth <= 32U) {
	for (Index = 0U; Index < 39U; Index++) {
		HifAddr[Index] = HifAddr[Index + 1U];
	}
	HifAddr[39U] = 0U;
    }

    if (PDimmPtr->BusWidth == 16U) {
	for (Index = 0U; Index < 39U; Index++) {
		HifAddr[Index] = HifAddr[Index + 1U];
	}
	HifAddr[39U] = 0U;
    }

	/* Calculate Bank Address Map based on HIF Addresses */
	XFsbl_DdrCalcBankAddr(DdrDataPtr, HifAddr);

	/* Calculate Column Address Map based on HIF Addresses */
	XFsbl_DdrCalcColAddr(DdrDataPtr, HifAddr);

	/* Calculate Row Address Map based on HIF Addresses */
	XFsbl_DdrCalcRowAddr(DdrDataPtr, HifAddr);

	/* Calculate Bank Group Address Map based on HIF Addresses */
	XFsbl_DdrCalcBgAddr(DdrDataPtr, HifAddr);

	/* Rank Width is 0U for the New DIMM */
	DdrDataPtr->AddrMapCsBit0 = 0x1FU;

	/* Address Map Register 0U */
	RegVal[0U] = ((DdrDataPtr->AddrMapCsBit0 & 0x1FU) << 0U);

	/* Address Map Register 1U */
	RegVal[1U]  = ((DdrDataPtr->AddrMapBankBit[2U] &
				0x1FU) << 16U);
	RegVal[1U] |= ((DdrDataPtr->AddrMapBankBit[1U] &
				0x1FU) << 8U);
	RegVal[1U] |= ((DdrDataPtr->AddrMapBankBit[0U] &
				0x1FU) << 0U);

	/* Address Map Register 2U */
	RegVal[2U]  = ((DdrDataPtr->AddrMapColBit[5U] &
				0xFU) << 24U);
	RegVal[2U] |= ((DdrDataPtr->AddrMapColBit[4U] &
				0xFU) << 16U);
	RegVal[2U] |= ((DdrDataPtr->AddrMapColBit[3U] &
				0xFU) << 8U);
	RegVal[2U] |= ((DdrDataPtr->AddrMapColBit[2U] &
				0xFU) << 0U);

	/* Address Map Register 3U */
	RegVal[3U]  = ((DdrDataPtr->AddrMapColBit[9U] &
				0xFU) << 24U);
	RegVal[3U] |= ((DdrDataPtr->AddrMapColBit[8U] &
				0xFU) << 16U);
	RegVal[3U] |= ((DdrDataPtr->AddrMapColBit[7U] &
				0xFU) << 8U);
	RegVal[3U] |= ((DdrDataPtr->AddrMapColBit[6U] &
				0xFU) << 0U);

	/* Address Map Register 4U */
	RegVal[4U]  = ((DdrDataPtr->AddrMapColBit[11U] &
				0xFU) << 8U);
	RegVal[4U] |= ((DdrDataPtr->AddrMapColBit[10U] &
				0xFU) << 0U);

	/* Address Map Register 5U */
	RegVal[5U] = ((DdrDataPtr->AddrMapRowBit[11U] &
				0xFU) << 24U);
	RegVal[5U] |= ((DdrDataPtr->AddrMapRowBits2To10 &
				0xFU) << 16U);
	RegVal[5U] |= ((DdrDataPtr->AddrMapRowBit[1U] &
				0xFU) << 8U);
	RegVal[5U] |= ((DdrDataPtr->AddrMapRowBit[0U] &
				0xFU) << 0U);

	/* Address Map Register 6U */
	RegVal[6U] = ((DdrDataPtr->AddrMapRowBit[15U] &
				0xFU) << 24U);
	RegVal[6U] |= ((DdrDataPtr->AddrMapRowBit[14U] &
				0xFU) << 16U);
	RegVal[6U] |= ((DdrDataPtr->AddrMapRowBit[13U] &
				0xFU) << 8U);
	RegVal[6U] |= ((DdrDataPtr->AddrMapRowBit[12U] &
				0xFU) << 0U);

	/* Address Map Register 7U */
	RegVal[7U] = ((DdrDataPtr->AddrMapRowBit[17U] &
				0xFU) << 8U);
	RegVal[7U] |= ((DdrDataPtr->AddrMapRowBit[16U] &
				0xFU) << 0U);

	/* Address Map Register 8U */
	RegVal[8U]  = ((DdrDataPtr->AddrMapBgBit[1U] &
				0x1FU) << 8U);
	RegVal[8U] |= ((DdrDataPtr->AddrMapBgBit[0U] &
				0x1FU) << 0U);

	/* Address Map Register 9U */
	RegVal[9U]  = ((DdrDataPtr->AddrMapRowBit[5U] &
				0xFU) << 24U);
	RegVal[9U] |= ((DdrDataPtr->AddrMapRowBit[4U] &
				0xFU) << 16U);
	RegVal[9U] |= ((DdrDataPtr->AddrMapRowBit[3U] &
				0xFU) << 8U);
	RegVal[9U] |= ((DdrDataPtr->AddrMapRowBit[2U] &
				0xFU) << 0U);

	/* Address Map Register 10U */
	RegVal[10U]  = ((DdrDataPtr->AddrMapRowBit[9U] &
				0xFU) << 24U);
	RegVal[10U] |= ((DdrDataPtr->AddrMapRowBit[8U] &
				0xFU) << 16U);
	RegVal[10U] |= ((DdrDataPtr->AddrMapRowBit[7U] &
				0xFU) << 8U);
	RegVal[10U] |= ((DdrDataPtr->AddrMapRowBit[6U] &
				0xFU) << 0U);

	/* Address Map Register 11U */
	RegVal[11U] = ((DdrDataPtr->AddrMapRowBit[10U] &
				0xFU) << 0U);

	for (Index = 0U; Index < 12U; Index ++) {
		Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x200U + (Index * 4U), RegVal[Index]);
	}
}

/*****************************************************************************/
/**
 * This function calculates the DDRC register values common to all DDR types
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	DdrCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrcCalcCommonRegVal(struct DdrcInitData *DdrDataPtr,
		XFsbl_DimmParams *PDimmPtr, u32 *DdrCfg)
{

	DdrCfg[DDR_DEVICE_CONFIG] = (PDimmPtr->DramWidth >= 4U) ? XFsbl_GetLog2(PDimmPtr->DramWidth) - 2U : 0U;

	DdrCfg[DDR_ACTIVE_RANKS] = (PDimmPtr->NumRankAddr * 2U) + 1U;

	DdrCfg[DDR_BURST_RDWR] = PDimmPtr->BurstLength / 2U;

	DdrCfg[DDR_DATA_BUS_WIDTH] = 6U - XFsbl_GetLog2(PDimmPtr->BusWidth);

	DdrCfg[DDR_GEARDOWN_MODE] = PDimmPtr->Geardown;

	if (!(PDimmPtr->Geardown == 1U)) {
		DdrCfg[DDR_EN_2T_TIMING_MODE] = PDimmPtr->En2tTimingMode;
	}

	DdrCfg[DDR_RC_DERATE_VALUE] = (u32)XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod / 2.0);

	DdrCfg[DDR_EN_DFI_DRAM_CLK_DISABLE] = PDimmPtr->ClockStopEn;

	DdrCfg[DDR_POWERDOWN_EN] = PDimmPtr->PwrDnEn;

	DdrCfg[DDR_T_DPD_X4096] = XFSBL_MIN(((u32)XFsbl_Ceil(500000.0 / (4096.0 * PDimmPtr->ClockPeriod)) + 1U), 255U);

	DdrCfg[DDR_REFRESH_MODE] = PDimmPtr->Fgrm;

	DdrCfg[DDR_T_RFC_MIN] = (u32)XFsbl_Ceil(((PDimmPtr->TRfcPs / 1000.0) / 2.0) / PDimmPtr->ClockPeriod);

	DdrCfg[DDR_DIS_SCRUB] = PDimmPtr->EccScrub;

	DdrCfg[DDR_ECC_MODE] = PDimmPtr->Ecc << 2U;

	DdrCfg[DDR_DATA_POISON_EN] = PDimmPtr->EccPoison;

	if (PDimmPtr->Parity | PDimmPtr->Crc) {
		DdrCfg[DDR_ALERT_WAIT_FOR_SW] = 0U;
		if (!PDimmPtr->NoRetry)
			DdrCfg[DDR_CRC_PARITY_RETRY_ENABLE] = 1U;
	}

	if (PDimmPtr->Crc) {
		DdrCfg[DDR_CRC_INC_DM] = 1U;
		DdrCfg[DDR_CRC_ENABLE] = 1U;
	}

	DdrCfg[DDR_PARITY_ENABLE] = PDimmPtr->Parity;

	DdrCfg[DDR_IDLE_AFTER_RESET_X32] = XFSBL_MIN(1U + ((u32)XFsbl_Ceil(1000.0 / (32.0 * PDimmPtr->ClockPeriod))), 255U);

	DdrCfg[DDR_DIMM_ADDR_MIRR_EN] = PDimmPtr->AddrMirror;

	DdrCfg[DDR_T_FAW] = (u32)XFsbl_Ceil((PDimmPtr->TFawPs / 1000.0) /
			PDimmPtr->ClockPeriod / 2.0);

	DdrCfg[DDR_T_XP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp / 2.0), 31U);

	DdrCfg[DDR_T_RC] = (u32)XFsbl_Ceil((PDimmPtr->TRcPs / 1000.0) /
			PDimmPtr->ClockPeriod / 2.0) + 1U;

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		PDimmPtr->WriteLatency = PDimmPtr->WriteLatency +
			PDimmPtr->ParityLatency;
	}
	PDimmPtr->WriteLatency += PDimmPtr->RDimm;
	DdrCfg[DDR_WRITE_LATENCY] = (u32)XFsbl_Ceil(PDimmPtr->WriteLatency / 2.0);

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		PDimmPtr->ReadLatency = PDimmPtr->ReadLatency + PDimmPtr->ParityLatency;
	}
	PDimmPtr->ReadLatency += PDimmPtr->RDimm;
	DdrCfg[DDR_READ_LATENCY] = (u32)XFsbl_Ceil(PDimmPtr->ReadLatency / 2.0);

	DdrCfg[DDR_T_RCD] = (u32)XFsbl_Ceil(((PDimmPtr->TRcdPs / 1000.0) -
				PDimmPtr->AdditiveLatency) / 2.0) + 1U;

	DdrCfg[DDR_T_CCD] = (u32)XFsbl_Ceil((PDimmPtr->TCcdlPs / 1000.0) / 2.0);

	DdrCfg[DDR_T_XS_FAST_X32] = (u32)XFsbl_Ceil(((PDimmPtr->TRfc4Ps /
					1000.0) + 10.0) / PDimmPtr->ClockPeriod / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_XS_ABORT_X32] = DdrCfg[DDR_T_XS_FAST_X32];

	DdrCfg[DDR_DDR4_WR_PREAMBLE] = PDimmPtr->WrPreamble;

	DdrCfg[DDR_T_MRD_PDA] = (u32)XFsbl_Ceil(XFSBL_MAX(16.0, 10.0 / PDimmPtr->ClockPeriod) / 2.0);

	if (!PDimmPtr->DisDfiLpSr) {
		DdrCfg[DDR_DFI_LP_EN_SR] = 0x1U;
	}

	if (!PDimmPtr->DisDfiLpPd) {
		DdrCfg[DDR_DFI_LP_EN_PD] = 0x1U;
	}

	if (!PDimmPtr->DisDfiLpMpsm) {
		DdrCfg[DDR_DFI_LP_EN_MPSM] = 0x1U;
	}

	DdrCfg[DDR_DIS_AUTO_CTRLUPD] = PDimmPtr->Slowboot;

	if (PDimmPtr->WrDrift) {
		DdrCfg[DDR_DFI_T_CTRLUP_MAX] = 0x3FFU;
	}

	DdrCfg[DDR_PHY_DBI_MODE] = !!PDimmPtr->PhyDbiMode;

	/* Calculate Address Map for the DDR */
	XFsbl_DdrCalcAddrMap(DdrDataPtr);

	if (PDimmPtr->LprNumEntries != 0U) {
		DdrCfg[DDR_LPR_NUM_ENTRIES] = PDimmPtr->LprNumEntries;
	}

	DdrCfg[DDR_DQ_NIBBLE_MAP_12_15] = PDimmPtr->Dqmap1215;
	DdrCfg[DDR_DQ_NIBBLE_MAP_8_11] = PDimmPtr->Dqmap811;
	DdrCfg[DDR_DQ_NIBBLE_MAP_4_7] = PDimmPtr->Dqmap47;
	DdrCfg[DDR_DQ_NIBBLE_MAP_0_3] = PDimmPtr->Dqmap03;

	DdrCfg[DDR_DQ_NIBBLE_MAP_28_31] = PDimmPtr->Dqmap2831;
	DdrCfg[DDR_DQ_NIBBLE_MAP_24_27] = PDimmPtr->Dqmap2427;
	DdrCfg[DDR_DQ_NIBBLE_MAP_20_23] = PDimmPtr->Dqmap2023;
	DdrCfg[DDR_DQ_NIBBLE_MAP_16_19] = PDimmPtr->Dqmap1619;

	DdrCfg[DDR_DQ_NIBBLE_MAP_44_47] = PDimmPtr->Dqmap4447;
	DdrCfg[DDR_DQ_NIBBLE_MAP_40_43] = PDimmPtr->Dqmap4043;
	DdrCfg[DDR_DQ_NIBBLE_MAP_36_39] = PDimmPtr->Dqmap3639;
	DdrCfg[DDR_DQ_NIBBLE_MAP_32_35] = PDimmPtr->Dqmap3235;

	DdrCfg[DDR_DQ_NIBBLE_MAP_60_63] = PDimmPtr->Dqmap6063;
	DdrCfg[DDR_DQ_NIBBLE_MAP_56_59] = PDimmPtr->Dqmap5659;
	DdrCfg[DDR_DQ_NIBBLE_MAP_52_55] = PDimmPtr->Dqmap5255;
	DdrCfg[DDR_DQ_NIBBLE_MAP_48_51] = PDimmPtr->Dqmap4851;

	DdrCfg[DDR_DQ_NIBBLE_MAP_CB_4_7] = PDimmPtr->Dqmap6871;
	DdrCfg[DDR_DQ_NIBBLE_MAP_CB_0_3] = PDimmPtr->Dqmap6467;

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function calculates the DDRC register values for DDR4
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	DdrCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrcCalcDdr4RegVal(XFsbl_DimmParams *PDimmPtr, u32 *DdrCfg)
{
	u32 Val;
	u32 Cal = 0U;
	u32 Bit543 = 0U;
	u32 CasLatency;
	u32 Twr;
	u32 Bit2;
	u32 Bit654;
	u32 POdt = 0U;
	u32 POdi = 0U;
	float FVal;

	DdrCfg[DDR_DDR4] = 1U;

	DdrCfg[DDR_MPSM_EN] = PDimmPtr->MaxPwrSavEn;

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / 2.0 / PDimmPtr->ClockPeriod) / 32.0);
	if ((PDimmPtr->RDimm || PDimmPtr->UDimm) && (PDimmPtr->NumRankAddr == 1U)) {
		DdrCfg[DDR_REFRESH_TIMER1_START_VALUE_X32] = Val / 2U;
	}

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / 2.0 / PDimmPtr->ClockPeriod) / 32.0);

	if (PDimmPtr->Fgrm == 1U) {
		Val /= 2U;
	} else if (PDimmPtr->Fgrm == 2U) {
		Val /= 4U;
	}

	if (PDimmPtr->TRefRange) {
		Val /= 2U;
	}

	if (PDimmPtr->Fgrm == 1U) {
		DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MIN(Val, 0x7FFU);
	} else if (PDimmPtr->Fgrm == 2U) {
		DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MIN(Val, 0x3FFU);
	} else {
		DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MIN(Val, 0xFFEU);
	}

	DdrCfg[DDR_T_PAR_ALERT_PW_MAX] = (u32)XFsbl_Ceil((PDimmPtr->SpeedBin * 3.0) / 100.0);

	DdrCfg[DDR_T_CRC_ALERT_PW_MAX] = 5U;

	DdrCfg[DDR_POST_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(400.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_PRE_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(500000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_DRAM_RSTN_X1024] = XFSBL_MAX(((u32)XFsbl_Ceil(100.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2.0) + 1U), 1U);

	CasLatency = PDimmPtr->CasLatency;
	if (CasLatency <= 16U) {
		Bit654 = (CasLatency - 9U) / 2U;
		Bit2 = ((CasLatency - 1U) % 2U);
	} else if ((CasLatency % 2U) == 1U) {
		Bit654 = (CasLatency + 2U) / 6U;
		Bit2 = (((CasLatency + 1U) / 2U) % 2U);
	} else {
		Bit654 = (CasLatency - 1U) / 4U;
		Bit2 = (((CasLatency / 2U) + 1U) % 2U);
	}

	Twr = (u32)XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod);
	if ((Twr >= 10U) || (Twr <= 24U)) {
		if (Twr % 2U == 1U) {
			Twr += 1U;
		}
		if (Twr < 24U) {
			Twr = (Twr - 10U) / 2U;
		} else {
			Twr = 6U;
		}
	}
	DdrCfg[DDR_MR] = XFSBL_SETBITS(Twr, 9U, 3U) + (1U << 8U) +
		XFSBL_SETBITS(Bit654, 4U, 3U) + (Bit2 << 2U) +
		XFSBL_SETBITS(((PDimmPtr->BurstLength == 4U) ? 2U : 0U), 0U, 2U);

	switch (PDimmPtr->DramOdt) {
	case 60U:
		POdt = 1U;
		break;
	case 120U:
		POdt = 2U;
		break;
	case 40U:
		POdt = 3U;
		break;
	case 240U:
		POdt = 4U;
		break;
	case 48U:
		POdt = 5U;
		break;
	case 80U:
		POdt = 6U;
		break;
	case 34U:
		POdt = 7U;
		break;
	default:
		POdt = 0x3U + (PDimmPtr->NumRankAddr * 2U);
		break;
	}

	POdi = (PDimmPtr->DramDrv == 48U) ? 1U : 0U;

	DdrCfg[DDR_EMR] = XFSBL_SETBITS(POdt, 8U, 3U) +
		XFSBL_SETBITS(PDimmPtr->AdditiveLatency, 3U, 2U) +
		XFSBL_SETBITS(POdi, 1U, 2U) + 1U;

	if (PDimmPtr->TRefRange)
		PDimmPtr->LpAsr = 0x3U;

	if ((PDimmPtr->CasWriteLatency >= 9U) && (PDimmPtr->CasWriteLatency <= 12U)) {
		Bit543 = PDimmPtr->CasWriteLatency - 9U;
	} else if ((PDimmPtr->CasWriteLatency > 12U) && (PDimmPtr->CasWriteLatency <= 18U)) {
		Bit543 = (PDimmPtr->CasWriteLatency - 14U) / 2U + 4U;
	} else {
		Bit543 = 0U;
	}

	DdrCfg[DDR_EMR2] =  (PDimmPtr->Crc << 12U) + XFSBL_SETBITS(PDimmPtr->LpAsr, 6U, 2U) + XFSBL_SETBITS(Bit543, 3U, 3U);

	Val = (PDimmPtr->SpeedBin - 1066U) / 800U;
	DdrCfg[DDR_EMR3] = XFSBL_SETBITS(Val, 9U, 2U) + XFSBL_SETBITS(PDimmPtr->Fgrm, 6U, 3U);

	DdrCfg[DDR_DEV_ZQINIT_X32] = 33U;

	if (!PDimmPtr->CalModeEn)  {
		Cal = 0U;
		Val = 0U;
	} else {
		Cal = ((PDimmPtr->SpeedBin - 266U) / 533U) + 1U;
		Val = Cal - 2U;
	}

	DdrCfg[DDR_MR4] = (PDimmPtr->WrPreamble << 12U) + (PDimmPtr->RdPreamble << 11U) + (PDimmPtr->MaxPwrSavEn << 1U) + XFSBL_SETBITS(Val, 6U, 3U) + (PDimmPtr->TRefMode << 3U) + (PDimmPtr->TRefRange << 2U) + (PDimmPtr->SelfRefAbort << 9U);

	Val = PDimmPtr->Parity * ((PDimmPtr->SpeedBin < 2400U) ? 1U : 2U);

	DdrCfg[DDR_MR5] = (PDimmPtr->RdDbi << 12U) + (PDimmPtr->WrDbi << 11U) + (PDimmPtr->DataMask << 10U) + (1U << 9U) + ((0x3U + PDimmPtr->NumRankAddr) << 6U) + Val;

	Val = XFSBL_MAX(0U, XFSBL_MIN(3U, ((PDimmPtr->FreqMhz - 667U) / 266U) + 1U));

	DdrCfg[DDR_MR6] = XFSBL_SETBITS(Val, 10U, 3U) + XFSBL_SETBITS((PDimmPtr->DramOdt == 60U) ? 0x13U : 0x19U, 0U, 6U);

	if (PDimmPtr->RDimm) {
		if (PDimmPtr->DisOpInv)
			DdrCfg[DDR_DIMM_OUTPUT_INV_EN] = 0U;
		else
			DdrCfg[DDR_DIMM_OUTPUT_INV_EN] = 1U;
	}

	if ((PDimmPtr->RDimm || PDimmPtr->UDimm) && (PDimmPtr->NumRankAddr == 1U)) {
		DdrCfg[DDR_DIMM_STAGGER_CS_EN] = 1U;
	}

	if ((PDimmPtr->WrPreamble || PDimmPtr->Crc))  {
		DdrCfg[DDR_DIFF_RANK_WR_GAP] += 1U;
	}

	DdrCfg[DDR_DIFF_RANK_WR_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_WR_GAP] / 2.0) + 3U;

	DdrCfg[DDR_DIFF_RANK_RD_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_RD_GAP] / 2.0) + 3U;


	Twr = (u32)XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod);
	if (PDimmPtr->Crc) {
		if ((PDimmPtr->SpeedBin >= 1866U) && (PDimmPtr->SpeedBin <= 2666U)) {
			Twr += (u32)XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), 5U);
		} else if (PDimmPtr->SpeedBin == 1600U) {
			Twr += (u32)XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), 4U);
		}
	}

	DdrCfg[DDR_WR2PRE] = (u32)(PDimmPtr->WriteLatency + PDimmPtr->BurstLength / 2.0 + Twr + PDimmPtr->WrPreamble) / 2U;

	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_WR2PRE] += 1U;
	}


	DdrCfg[DDR_T_RAS_MAX] = XFSBL_MAX(1U, (u32)((((9.0 * (PDimmPtr->TRefi / 1000.0)) / PDimmPtr->ClockPeriod / 1024.0)) - 1U) / 2.0);

	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_T_RAS_MIN] = (u32)XFsbl_Ceil(((PDimmPtr->TRasPs /
						1000.0) / PDimmPtr->ClockPeriod / 2.0));
	} else {
		DdrCfg[DDR_T_RAS_MIN] = XFsbl_Round(((PDimmPtr->TRasPs / 1000.0) /
					PDimmPtr->ClockPeriod / 2.0));
	}


	PDimmPtr->TXp = XFSBL_MAX(4U, 6.0 / PDimmPtr->ClockPeriod) + PDimmPtr->ParityLatency;

	DdrCfg[DDR_T_XP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp / 2.0), 31U);

	Val = XFSBL_MAX(PDimmPtr->AdditiveLatency +
		  XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U),
		  (PDimmPtr->ReadLatency + PDimmPtr->BurstLength / 2U -
		  (PDimmPtr->TRpPs / 1000.0)));
	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_RD2PRE] = (u32)XFsbl_Ceil(Val / 2.0);
	} else {
		DdrCfg[DDR_RD2PRE] = (u32)Val / 2U;
	}

	DdrCfg[DDR_RD2WR] = (u32)XFsbl_Ceil((PDimmPtr->ReadLatency +
			PDimmPtr->BurstLength / 2U + 2U +
			PDimmPtr->WrPreamble - PDimmPtr->WriteLatency) /
			2.0) + 2U;

	PDimmPtr->TWtr = (u32)XFSBL_MAX(XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U);

	if (PDimmPtr->Crc) {
		if ((PDimmPtr->SpeedBin >= 1866U) && (PDimmPtr->SpeedBin <= 2666U)) {
			PDimmPtr->TWtr += (u32)XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), 5U);
		} else if (PDimmPtr->SpeedBin == 1600U) {
			PDimmPtr->TWtr += (u32)XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), 4U);
		}
	}

	Val = PDimmPtr->WriteLatency + PDimmPtr->TWtr + PDimmPtr->BurstLength / 2U + PDimmPtr->ParityLatency;
	if (PDimmPtr->WrPreamble) {
		Val += 2U;
	}
	DdrCfg[DDR_WR2RD] = (u32)XFsbl_Ceil((float)Val / 2U);

	FVal = (PDimmPtr->SpeedBin < 2666U) ? 8.0 : 9.0;

	if (PDimmPtr->Parity) {
		FVal = XFSBL_MAX(24U, 15U / PDimmPtr->ClockPeriod) +
			PDimmPtr->ParityLatency;
	}

	if (PDimmPtr->CalModeEn) {
		FVal = XFSBL_MAX(24U, 15U / PDimmPtr->ClockPeriod) + Cal;
	}

	DdrCfg[DDR_T_MRD] = (u32)XFsbl_Ceil(FVal / 2.0);

	PDimmPtr->TMod = XFSBL_MAX(24U, 15U / PDimmPtr->ClockPeriod) + (PDimmPtr->Parity * PDimmPtr->ParityLatency) + (PDimmPtr->CalModeEn * Cal) + PDimmPtr->RDimm;
	DdrCfg[DDR_T_MOD] = (u32)XFsbl_Ceil(PDimmPtr->TMod / 2.0);

	DdrCfg[DDR_T_RRD] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil((PDimmPtr->TRrdlPs / 1000.0) / PDimmPtr->ClockPeriod), 4U)) / 2.0);

	DdrCfg[DDR_T_RP] = (u32)(XFsbl_Ceil((PDimmPtr->TRpPs / 1000.0) / 2.0) + 1U) + 1U;

	DdrCfg[DDR_T_CKSRX] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 5.0)) / 2.0);

	DdrCfg[DDR_T_CKSRE] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 5.0) + PDimmPtr->ParityLatency) / 2.0);

	DdrCfg[DDR_T_CKESR] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(5.0 / PDimmPtr->ClockPeriod), 3.0) + 1.0) / 2.0);

	FVal = XFSBL_MAX(XFsbl_Ceil(5.0 / PDimmPtr->ClockPeriod), 3.0);
	DdrCfg[DDR_T_CKE] = (u32)XFsbl_Ceil(FVal / 2.0);
	DdrCfg[DDR_T_CKCSX] = (u32)XFsbl_Ceil((FVal + 2.0) / 2.0);

	DdrCfg[DDR_T_CKPDE] = DdrCfg[DDR_T_CKSRE];

	DdrCfg[DDR_T_CKPDX] = DdrCfg[DDR_T_CKSRX];

	DdrCfg[DDR_T_XS_DLL_X32] = (u32)XFsbl_Ceil(((PDimmPtr->SpeedBin <= 1866U) ? 597.0 : ((PDimmPtr->SpeedBin <= 2400U) ? 768.0 : 1024.0)) / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_XS_X32] = (u32)XFsbl_Ceil((((PDimmPtr->TRfcPs / 1000.0) + 10U) / PDimmPtr->ClockPeriod) / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_CCD_S] = 2U;

	DdrCfg[DDR_T_RRD_S] = (u32)XFsbl_Ceil(XFSBL_MAX((PDimmPtr->TRrdsPs / 1000.0) / PDimmPtr->ClockPeriod , 4U) / 2.0);

	FVal = PDimmPtr->ParityLatency + PDimmPtr->WriteLatency + PDimmPtr->BurstLength / 2U + XFSBL_MAX(XFsbl_Ceil(2.5 / PDimmPtr->ClockPeriod), 2U);

	if (PDimmPtr->Crc) {
		FVal += XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), (PDimmPtr->SpeedBin > 1600U) ? 5.0 : 4.0);
	}

	DdrCfg[DDR_WR2RD_S] = (u32)XFsbl_Ceil((FVal + PDimmPtr->WrPreamble) / 2.0);

	FVal = PDimmPtr->ClockPeriod * ((PDimmPtr->SpeedBin <= 1866U) ? 597.0 : ((PDimmPtr->SpeedBin <= 2400U) ? 768.0 : 1024.0));

	FVal = ((PDimmPtr->TRfcPs / 1000.0) + 10.0 + FVal);

	DdrCfg[DDR_POST_MPSM_GAP_X32] = (u32)XFsbl_Ceil(FVal / (2.0 * 32U * PDimmPtr->ClockPeriod));

	DdrCfg[DDR_T_MPX_LH] = (u32)XFsbl_Ceil(12U / PDimmPtr->ClockPeriod / 2.0);

	FVal = XFSBL_MAX(24U, 15U / PDimmPtr->ClockPeriod) + 4U;
	if (PDimmPtr->SpeedBin == 2666U) {
		FVal += 1U;
	}
	if (PDimmPtr->SpeedBin == 3200U) {
		FVal += 2U;
	}

	DdrCfg[DDR_T_CKMPE] = (u32)XFsbl_Ceil(FVal / 2U);

	DdrCfg[DDR_T_ZQ_LONG_NOP] = 256U;

	DdrCfg[DDR_T_ZQ_SHORT_NOP] = 64U;

	DdrCfg[DDR_T_ZQ_SHORT_INTERVAL_X1024] = (u32)((100000000.0 / PDimmPtr->ClockPeriod) / 1024.0);

	DdrCfg[DDR_DFI_T_RDDATA_EN] = PDimmPtr->ReadLatency - 4U + Cal - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_TPHY_WRLAT] = PDimmPtr->WriteLatency - 3U + Cal - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_T_CMD_LAT] = Cal;

	DdrCfg[DDR_DFI_T_WRDATA_DELAY] = 3U;

	DdrCfg[DDR_DFI_DATA_CS_POLARITY] = 0U;

	if (PDimmPtr->DramWidth != 4U) {
		if (PDimmPtr->RdDbi) {
			DdrCfg[DDR_RD_DBI_EN] = 1U;
		}
		if (PDimmPtr->WrDbi) {
			DdrCfg[DDR_WR_DBI_EN] = 1U;
		}
		if (PDimmPtr->DataMask) {
			DdrCfg[DDR_DM_EN] = 1U;
		} else {
			DdrCfg[DDR_DM_EN] = 0U;
		}
	} else {
		DdrCfg[DDR_DM_EN] = 0U;
	}

	DdrCfg[DDR_WR_ODT_HOLD] = (PDimmPtr->BurstLength / 2U) + 2U;
	if (PDimmPtr->Crc | PDimmPtr->WrPreamble) {
		DdrCfg[DDR_WR_ODT_HOLD] += 1U;
	}

	DdrCfg[DDR_WR_ODT_DELAY] = Cal;

	DdrCfg[DDR_RD_ODT_HOLD] = 6U;

	DdrCfg[DDR_RD_ODT_DELAY] = Cal + (((PDimmPtr->CasLatency -
					PDimmPtr->CasWriteLatency)  < 1U) ? 0U :
			(PDimmPtr->CasLatency - PDimmPtr->CasWriteLatency - 1U));

	DdrCfg[DDR_RANK1_WR_ODT] = 2U * PDimmPtr->NumRankAddr;

	DdrCfg[DDR_RANK0_WR_ODT] = 0x1U;

	if (PDimmPtr->BusWidth == 32U) {
		DdrCfg[DDR_DIS_WC] = 1U;
		DdrCfg[DDR_BL_EXP_MODE] = 1U;
	}
	return XFSBL_SUCCESS;
}

#if !(defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208))
/*****************************************************************************/
/**
 * This function calculates the DDRC register values for DDR3
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	DdrCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrcCalcDdr3RegVal(XFsbl_DimmParams *PDimmPtr, u32 *DdrCfg)
{
	u32 Val;
	u32 BurstLength;
	u32 CasLatency;
	u32 Twr;
	u32 Bit654;
	u32 POdt = 0U;
	float FVal;

	DdrCfg[DDR_DDR3] = 1U;

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / 2.0 / PDimmPtr->ClockPeriod) / 32.0);
	if ((PDimmPtr->RDimm || PDimmPtr->UDimm) && (PDimmPtr->NumRankAddr == 1U)) {
		DdrCfg[DDR_REFRESH_TIMER1_START_VALUE_X32] = Val / 2U;
	}
	if (PDimmPtr->TRefRange) {
		Val /= 2U;
	}
	DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MAX(1U, XFSBL_MIN(Val, 0xFFEU));

	DdrCfg[DDR_POST_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(400.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_PRE_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(500000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_DRAM_RSTN_X1024] = XFSBL_MAX(((u32)XFsbl_Ceil(200000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2.0) + 1U), 1U);

	BurstLength = (PDimmPtr->BurstLength == 4U) ? 2U : 0U;

	CasLatency = PDimmPtr->CasLatency;

	Bit654 = (CasLatency < 12U) ? CasLatency - 4U : CasLatency - 12U;

	Twr = (u32)XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod);
	Twr = XFSBL_MAX(5U, XFSBL_MIN(16U, Twr));
	DdrCfg[DDR_MR] = XFSBL_SETBITS(((Twr < 8U) ? (Twr - 4U) : (u32)XFsbl_Ceil(Twr / 2.0)), 9U, 3U) + (1U << 8U) +
			XFSBL_SETBITS(Bit654, 4U, 3U) + (((CasLatency < 12U) ? 0U : 1U) << 2U) +
			XFSBL_SETBITS(BurstLength, 0U, 2U);

	if (PDimmPtr->DramOdt == 60U) {
		POdt = (1U << 2U);
	} else if (PDimmPtr->DramOdt == 120U) {
		POdt = (1U << 6U);
	} else if (PDimmPtr->DramOdt == 40U) {
		POdt = (1U << 6U) + (1U << 2U);
	} else if (PDimmPtr->DramOdt == 20U) {
		POdt = (1U << 9U);
	} else if (PDimmPtr->DramOdt == 30U) {
		POdt = (1U << 9U) + (1U << 2U);
	} else {
		if (PDimmPtr->NumRankAddr == 0U) {
			POdt = (1U << 2U);
		} else {
			POdt = (1U << 6U);
		}
	}

	DdrCfg[DDR_EMR] = (PDimmPtr->AdditiveLatency << 3U) + POdt + ((PDimmPtr->DramDrv == 34U) ? 0x2U : 0U);

	DdrCfg[DDR_EMR2] = (PDimmPtr->NumRankAddr << 9U) + XFSBL_SETBITS((PDimmPtr->CasWriteLatency - 5U), 3U, 3U);

	DdrCfg[DDR_EMR2] += (PDimmPtr->TRefRange << 7U);

	DdrCfg[DDR_EMR3] = 0U;

	DdrCfg[DDR_DEV_ZQINIT_X32] = (512U / 32U) + 1U;

	if ((PDimmPtr->RDimm || PDimmPtr->UDimm) && (PDimmPtr->NumRankAddr == 1U)) {
		DdrCfg[DDR_DIMM_STAGGER_CS_EN] = 1U;
	}

	DdrCfg[DDR_DIFF_RANK_WR_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_WR_GAP] / 2.0) + 3U;

	DdrCfg[DDR_DIFF_RANK_RD_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_RD_GAP] / 2.0) + 3U;

	Twr = (u32)XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod);
	if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR4) && PDimmPtr->Crc) {
		if ((PDimmPtr->SpeedBin >= 1866U) && (PDimmPtr->SpeedBin <= 2666U)) {
			Twr += (u32)XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), 5U);
		} else if (PDimmPtr->SpeedBin == 1600U) {
			Twr += (u32)XFSBL_MAX(XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod), 4U);
		}
	}

	if (PDimmPtr->WrPreamble && (PDimmPtr->MemType == SPD_MEMTYPE_DDR4)) {
		Twr += 1U;
	}

	DdrCfg[DDR_WR2PRE] = (u32)(PDimmPtr->WriteLatency + PDimmPtr->BurstLength / 2.0 + Twr) / 2U;

	if (PDimmPtr->En2tTimingMode && (PDimmPtr->MemType == SPD_MEMTYPE_DDR4)) {
		DdrCfg[DDR_WR2PRE] += 1U;
	}

	DdrCfg[DDR_T_RAS_MAX] = (u32)XFSBL_MAX(1U, (((9.0 * (PDimmPtr->TRefi / 1000.0)) / PDimmPtr->ClockPeriod / 1024.0) - 1U) / 2.0);

	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_T_RAS_MIN] = (u32)XFsbl_Ceil(((PDimmPtr->TRasPs /
						1000.0) / PDimmPtr->ClockPeriod / 2.0));
	} else {
		DdrCfg[DDR_T_RAS_MIN] = XFsbl_Round(((PDimmPtr->TRasPs / 1000.0) /
					PDimmPtr->ClockPeriod / 2.0));
	}

	PDimmPtr->TXp = XFSBL_MAX(10U, 24.0 / PDimmPtr->ClockPeriod);

	DdrCfg[DDR_T_XP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp / 2.0), 31U);

	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_RD2PRE] = (u32)XFsbl_Ceil((PDimmPtr->AdditiveLatency + XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U)) / 2.0);
	} else {
		DdrCfg[DDR_RD2PRE] = (u32)(PDimmPtr->AdditiveLatency + XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U)) / 2U;
	}

	DdrCfg[DDR_RD2WR] = (u32)XFsbl_Ceil((PDimmPtr->ReadLatency + (PDimmPtr->BurstLength/2U) + 2U - PDimmPtr->WriteLatency) / 2.0) + 2U;

	PDimmPtr->TWtr = (u32)XFSBL_MAX(XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U);

	DdrCfg[DDR_WR2RD] = (u32)XFsbl_Ceil((float)(PDimmPtr->WriteLatency + PDimmPtr->TWtr + PDimmPtr->BurstLength / 2U) / 2.0);

	DdrCfg[DDR_T_MRD] = 0x2U;

	PDimmPtr->TMod = XFSBL_MAX(12U, 15U / PDimmPtr->ClockPeriod) + PDimmPtr->RDimm;

	DdrCfg[DDR_T_MOD] = (u32)XFsbl_Ceil(PDimmPtr->TMod / 2.0);

	DdrCfg[DDR_T_RRD] = (u32)XFsbl_Ceil(XFSBL_MAX(XFsbl_Ceil((PDimmPtr->TRrdPs / 1000.0) / PDimmPtr->ClockPeriod), 4U) / 2.0);

	DdrCfg[DDR_T_RP] = (u32)(XFsbl_Ceil((PDimmPtr->TRpPs / 1000.0) / 2.0) + 1U) + 1U;

	DdrCfg[DDR_T_CKSRX] = (u32)XFsbl_Ceil(XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 5.0) / 2.0);

	DdrCfg[DDR_T_CKSRE] = (u32)XFsbl_Ceil(XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 5.0) / 2.0);

	DdrCfg[DDR_T_CKESR] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil((XFSBL_MAX(5.0, 45.0 / ((PDimmPtr->SpeedBin / 266U) + 3U))) / PDimmPtr->ClockPeriod), 3.0) + 1.0) / 2.0);

	FVal = XFSBL_MAX(XFsbl_Ceil((XFSBL_MAX(5.0, 45.0 / ((PDimmPtr->SpeedBin / 266U) + 3U))) / PDimmPtr->ClockPeriod), 3.0);

	DdrCfg[DDR_T_CKE] = (u32)XFsbl_Ceil(FVal / 2.0);

	DdrCfg[DDR_T_CKCSX] = (u32)XFsbl_Ceil((FVal + 2.0) / 2.0);

	DdrCfg[DDR_T_CKPDE] = DdrCfg[DDR_T_CKSRE];

	DdrCfg[DDR_T_CKPDX] = DdrCfg[DDR_T_CKSRX];

	DdrCfg[DDR_T_XS_DLL_X32] = (u32)XFsbl_Ceil(512.0 / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_XS_X32] = (u32)XFsbl_Ceil(512.0 / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_ZQ_LONG_NOP] = (u32)XFsbl_Ceil(XFSBL_MAX(256.0, 320.0 / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_ZQ_SHORT_NOP] = (u32)XFsbl_Ceil(XFSBL_MAX(64.0, 80.0 / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_ZQ_SHORT_INTERVAL_X1024] = (u32)((100000000.0 / PDimmPtr->ClockPeriod) / 1024.0);

	DdrCfg[DDR_DFI_T_RDDATA_EN] = PDimmPtr->ReadLatency - 4U - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_TPHY_WRLAT] = PDimmPtr->WriteLatency - 3U - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_T_WRDATA_DELAY] = 2U;

	DdrCfg[DDR_DFI_DATA_CS_POLARITY] = 0U;

	DdrCfg[DDR_PHY_DBI_MODE] = PDimmPtr->PhyDbiMode;

	DdrCfg[DDR_DM_EN] = 0U;

	DdrCfg[DDR_WR_ODT_HOLD] = (PDimmPtr->BurstLength / 2U) + 3U;

	DdrCfg[DDR_WR_ODT_DELAY] = 0U;

	DdrCfg[DDR_RD_ODT_HOLD] = (PDimmPtr->BurstLength / 2U) + 2U;

	DdrCfg[DDR_RD_ODT_DELAY] = PDimmPtr->CasLatency - PDimmPtr->CasWriteLatency;

	DdrCfg[DDR_RANK1_WR_ODT] = 2U * PDimmPtr->NumRankAddr;

	DdrCfg[DDR_RANK0_WR_ODT] = 0x1U;

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function calculates the DDRC register values for LPDDR3
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	DdrCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrcCalcLpddr3RegVal(XFsbl_DimmParams *PDimmPtr, u32 *DdrCfg)
{

	u32 Val;
	u32 Twr;
	float FVal;

	DdrCfg[DDR_EN_2T_TIMING_MODE] = PDimmPtr->En2tTimingMode;

	DdrCfg[DDR_LPDDR3] = 1U;

	DdrCfg[DDR_DERATE_VALUE] = ((PDimmPtr->ClockPeriod / 2U) > 1.875) ? 0U : 1U;

	DdrCfg[DDR_MR4_READ_INTERVAL] = (u32)(PDimmPtr->DerateIntD * 1000U / PDimmPtr->ClockPeriod / 2.0);

	DdrCfg[DDR_DEEPPOWERDOWN_EN] = PDimmPtr->DeepPwrDnEn;

	DdrCfg[DDR_PER_BANK_REFRESH] = PDimmPtr->PerBankRefresh;

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / 2.0 / PDimmPtr->ClockPeriod) / 32.0);
	if (PDimmPtr->PerBankRefresh == 0U) {
		if (PDimmPtr->Capacity != 1024U) Val /= 2U;
	} else {
		if (PDimmPtr->Capacity >= 2048U)
			Val = (u32)((487.5 / 2U / PDimmPtr->ClockPeriod) / 32.0);

		else if (PDimmPtr->Capacity == 1024U)
			Val = (u32)((975.0 / 2U / PDimmPtr->ClockPeriod) / 32.0);
	}
	if (PDimmPtr->TRefRange) {
		Val /= 4U;
	}

	DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MIN(Val, 0xFFEU);

	DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MAX(1U, DdrCfg[DDR_T_RFC_NOM_X32]);

	DdrCfg[DDR_POST_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(200000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_PRE_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(100.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	Twr = (u32)XFsbl_Ceil(XFSBL_MAX(15U, 4U * PDimmPtr->ClockPeriod) / PDimmPtr->ClockPeriod);

	Twr = XFSBL_MAX(6U, XFSBL_MIN(12U, Twr));

	DdrCfg[DDR_MR] =  XFSBL_SETBITS(((Twr < 10U) ? (Twr - 2U) : (Twr % 10U)), 5U, 3U) + XFSBL_SETBITS(3U, 0U, 3U);

	Val = XFSBL_MAX(6U, XFSBL_MIN(10U, (PDimmPtr->FreqMhz / 66U) - 2U));

	Twr = (u32)XFsbl_Ceil(XFSBL_MAX(15U, 4U * PDimmPtr->ClockPeriod) /
			PDimmPtr->ClockPeriod);

	DdrCfg[DDR_EMR] = (1U << 6U) + (((Twr >= 10U) ? 1U : 0U) << 4U) +	XFSBL_SETBITS(Val, 0U, 4U);

	if (PDimmPtr->DramDrv == 40U) {
		Val = 2U;
	} else if (PDimmPtr->DramDrv == 48U) {
		Val = 3U;
	} else if (PDimmPtr->DramDrv == 60U) {
		Val = 4U;
	} else if (PDimmPtr->DramDrv == 80U) {
		Val = 6U;
	} else {
		Val = 1U;
	}
	DdrCfg[DDR_EMR2] = Val;

	DdrCfg[DDR_EMR3] = 0U;

	DdrCfg[DDR_DEV_ZQINIT_X32] = XFSBL_MIN(1U + ((u32)XFsbl_Ceil(1000.0 / (32.0 * PDimmPtr->ClockPeriod))), 255U);

	DdrCfg[DDR_MAX_AUTO_INIT_X1024] = XFSBL_MIN(1U + ((u32)XFsbl_Ceil(10000.0 / (1024U * PDimmPtr->ClockPeriod))), 1023U);

	DdrCfg[DDR_DIFF_RANK_WR_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_WR_GAP] / 2.0) + 3U;

	DdrCfg[DDR_DIFF_RANK_RD_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_RD_GAP] / 2.0) + 3U;

	Twr = (u32)XFsbl_Ceil(XFSBL_MAX(15.0 /
				PDimmPtr->ClockPeriod, 4.0));

	DdrCfg[DDR_WR2PRE] = (u32)(PDimmPtr->WriteLatency +
			PDimmPtr->BurstLength / 2.0 + Twr + 1.0) / 2U;

	FVal = XFSBL_MIN(9.0 * (PDimmPtr->TRefi / 1000.0), 70200.0);
	FVal =  (FVal / PDimmPtr->ClockPeriod / 1024.0);
	FVal = (u32)(FVal - 1U) / 2.0;
	DdrCfg[DDR_T_RAS_MAX] = (u32)XFSBL_MAX(1U, FVal);

	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_T_RAS_MIN] = (u32)XFsbl_Ceil(((PDimmPtr->TRasPs /
						1000.0) / PDimmPtr->ClockPeriod / 2.0));
	} else {
		DdrCfg[DDR_T_RAS_MIN] = XFsbl_Round(((PDimmPtr->TRasPs / 1000.0) /
					PDimmPtr->ClockPeriod / 2.0));
	}

	PDimmPtr->TXp =  XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 3U);

	DdrCfg[DDR_T_XP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp / 2.0), 31U);

	Val = PDimmPtr->BurstLength / 2U + XFSBL_MAX(((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod)), 4U) - 4U;

	if (PDimmPtr->En2tTimingMode) {
		DdrCfg[DDR_RD2PRE] = (u32)XFsbl_Ceil(Val / 2.0);
	} else {
		DdrCfg[DDR_RD2PRE] = (u32)Val / 2U;
	}

	FVal = XFsbl_Ceil(5.5 / PDimmPtr->ClockPeriod);
	DdrCfg[DDR_RD2WR] = (u32)XFsbl_Ceil((PDimmPtr->ReadLatency + PDimmPtr->BurstLength / 2U +
			FVal + 1U - PDimmPtr->WriteLatency) / 2.0) + 2U;

	PDimmPtr->TWtr = (u32)XFSBL_MAX(XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U);

	DdrCfg[DDR_WR2RD] = (u32)XFsbl_Ceil((float)(PDimmPtr->WriteLatency + PDimmPtr->TWtr + (PDimmPtr->BurstLength / 2U) + 1U) / 2.0);

	DdrCfg[DDR_T_MRW] = 10U;

	DdrCfg[DDR_T_MRD] = (u32)XFsbl_Ceil(XFSBL_MAX(10U, 15U / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_RRD] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 2U) + (1.875 * PDimmPtr->TRefRange)) / 2.0);

	DdrCfg[DDR_T_RP] = (u32)(XFsbl_Ceil((PDimmPtr->TRpPs / 1000.0) / 2.0) + 1U) + 1U;

	DdrCfg[DDR_T_CKSRX] = 1U;

	DdrCfg[DDR_T_CKSRE] = 1U;

	DdrCfg[DDR_T_CKESR] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod), 3.0)) / 2.0);

	DdrCfg[DDR_T_CKE] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod), 3.0)) / 2.0);

	DdrCfg[DDR_T_CKCSX] = (u32)XFsbl_Ceil((XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 3.0) + 2.0) / 2.0) + 2U;

	DdrCfg[DDR_T_CKPDE] = 1U;

	DdrCfg[DDR_T_CKPDX] = 1U;

	DdrCfg[DDR_T_XS_DLL_X32] = (u32)XFsbl_Ceil((((PDimmPtr->TRfcPs / 1000.0) + 10U) / PDimmPtr->ClockPeriod) / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_XS_X32] = (u32)XFsbl_Ceil((((PDimmPtr->TRfcPs / 1000.0) + 10U) / PDimmPtr->ClockPeriod) / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_ZQ_RESISTOR_SHARED] = (PDimmPtr->Ddp == 1U);

	DdrCfg[DDR_T_ZQ_LONG_NOP] = (u32)XFsbl_Ceil(((360.0 / PDimmPtr->ClockPeriod)) / 2.0);

	DdrCfg[DDR_T_ZQ_SHORT_NOP] = (u32)XFsbl_Ceil((90.0 / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_ZQ_RESET_NOP] = (u32)XFsbl_Ceil(XFSBL_MAX(3U, 50.0 / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_ZQ_SHORT_INTERVAL_X1024] = (u32)((400000000.0 / PDimmPtr->ClockPeriod) / 1024.0);

	DdrCfg[DDR_DFI_T_RDDATA_EN] = (PDimmPtr->ReadLatency + (u32)(1.5 / PDimmPtr->ClockPeriod)) - 4U - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_TPHY_WRLAT] = (PDimmPtr->WriteLatency + 1U) - 3U - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_T_WRDATA_DELAY] = 2U;

	DdrCfg[DDR_DFI_DATA_CS_POLARITY] = 0U;

	DdrCfg[DDR_DM_EN] = 0U;

	DdrCfg[DDR_WR_ODT_HOLD] = PDimmPtr->WriteLatency + 2U + (PDimmPtr->BurstLength / 2U);

	DdrCfg[DDR_WR_ODT_DELAY] = 0U;

	DdrCfg[DDR_RD_ODT_HOLD] = (u32)XFsbl_Ceil(5.5 / PDimmPtr->ClockPeriod) + 5U;

	DdrCfg[DDR_RD_ODT_DELAY] = PDimmPtr->ReadLatency - (u32)XFsbl_Ceil(3.5 / PDimmPtr->ClockPeriod);

	if (PDimmPtr->NumRankAddr == 0U) {
		DdrCfg[DDR_RANK1_WR_ODT] = 0U;
	} else {
		if (PDimmPtr->Ddp == 1U)
			DdrCfg[DDR_RANK1_WR_ODT] = 1U;
		else
			DdrCfg[DDR_RANK1_WR_ODT] = 2U;
	}

	DdrCfg[DDR_RANK0_WR_ODT] = 0x1U;

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function calculates the DDRC register values for LPDDR4
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	DdrCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrcCalcLpddr4RegVal(XFsbl_DimmParams *PDimmPtr, u32 *DdrCfg)
{
	u32 Val;
	u32 Val2 = 0U;
	u32 OdtLon = 0U;
	float FVal;
	float XVal;

	DdrCfg[DDR_EN_2T_TIMING_MODE] = PDimmPtr->En2tTimingMode;

	DdrCfg[DDR_LPDDR4] = 1U;

	DdrCfg[DDR_DERATE_VALUE] = ((PDimmPtr->ClockPeriod / 2U) > 1.875) ? 0U : 1U;

	DdrCfg[DDR_MR4_READ_INTERVAL] = (u32)(PDimmPtr->DerateIntD * 1000U / PDimmPtr->ClockPeriod / 2.0);

	DdrCfg[DDR_PER_BANK_REFRESH] = PDimmPtr->PerBankRefresh;

	Val = (u32)(((3904U / ((PDimmPtr->PerBankRefresh != 0U) ? 8U : 1U)) / 2U / PDimmPtr->ClockPeriod) / 32.0);

	if (PDimmPtr->TRefRange) {
		Val /= 4U;
	}

	DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MIN(Val, 0xFFEU);

	DdrCfg[DDR_T_RFC_NOM_X32] = XFSBL_MAX(1U, DdrCfg[DDR_T_RFC_NOM_X32]);

	DdrCfg[DDR_POST_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(2000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_PRE_CKE_X1024] = XFSBL_MAX(1U, (u32)XFsbl_Ceil(2000000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2U) + 1U);

	DdrCfg[DDR_DRAM_RSTN_X1024] = XFSBL_MAX(((u32)XFsbl_Ceil(200000.0 / (PDimmPtr->ClockPeriod * 1024.0) / 2.0) + 1U), 1U);

	DdrCfg[DDR_MR] = XFSBL_SETBITS((u32)XFSBL_MAX(0U, ((PDimmPtr->FreqMhz / 266U) - 1U)), 4U, 3U) + XFSBL_SETBITS(1U, 2U, 1U) + (PDimmPtr->RdPostamble << 7U);

	Val = XFSBL_MAX(0U, ((PDimmPtr->FreqMhz / 266U) - 1U));

	DdrCfg[DDR_EMR] = (((PDimmPtr->UseSetB) ? 1U : 0U) << 6U) + XFSBL_SETBITS(Val, 3U, 3U) + XFSBL_SETBITS(Val, 0U, 3U);

	if (PDimmPtr->DramDrv == 240U) {
		Val = 1U;
	} else if (PDimmPtr->DramDrv == 120U) {
		Val = 2U;
	} else if (PDimmPtr->DramDrv == 80U) {
		Val = 3U;
	} else if (PDimmPtr->DramDrv == 60U) {
		Val = 4U;
	} else if (PDimmPtr->DramDrv == 48U) {
		Val = 5U;
	} else {
		Val = 6U;
	}
	DdrCfg[DDR_EMR2] = (PDimmPtr->WrDbi << 7U) + (PDimmPtr->RdDbi << 6U)  + XFSBL_SETBITS(Val, 3U, 3U) + (PDimmPtr->WrPostamble << 1U) + 1U;

	DdrCfg[DDR_EMR3] = (((PDimmPtr->DataMask) ? 0U : 1U) << 5U) + (1U << 3U);

	DdrCfg[DDR_DEV_ZQINIT_X32] = (1024U / 32U) + 1U;

	DdrCfg[DDR_DIFF_RANK_WR_GAP] += (1U + PDimmPtr->WrPostamble);
	DdrCfg[DDR_DIFF_RANK_WR_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_WR_GAP] / 2.0) + 3U;

	DdrCfg[DDR_DIFF_RANK_RD_GAP] += (1U + PDimmPtr->RdPostamble);
	DdrCfg[DDR_DIFF_RANK_RD_GAP] = (u32)XFsbl_Ceil(DdrCfg[DDR_DIFF_RANK_RD_GAP] / 2.0) + 3U;

	DdrCfg[DDR_WR2PRE] = (u32)XFsbl_Ceil((PDimmPtr->WriteLatency + (PDimmPtr->BurstLength / 2.0) + ((u32)XFsbl_Ceil(XFSBL_MAX(18.0 / PDimmPtr->ClockPeriod, 4.0))) + 1.0) / 2U);

	DdrCfg[DDR_T_RAS_MAX] = (u32)XFSBL_MAX(1U, ((u32)(((XFSBL_MIN(9.0 * (PDimmPtr->TRefi / 1000.0), 70200.0)) / PDimmPtr->ClockPeriod / 1024.0) - 1U) / 2.0));

	DdrCfg[DDR_T_RAS_MIN] = (u32)XFsbl_Ceil(((PDimmPtr->TRasPs / 1000.0) / PDimmPtr->ClockPeriod / 2.0));

	PDimmPtr->TXp =  XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 5U);
	DdrCfg[DDR_T_XP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp / 2.0), 31U);

	DdrCfg[DDR_RD2PRE] = (u32)(PDimmPtr->BurstLength / 2U + XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 8U) - 8U) / 2U;

	DdrCfg[DDR_RD2PRE] = (u32)XFsbl_Ceil(Val / 2.0);

	if (PDimmPtr->FreqMhz <= 800U) {
		OdtLon = PDimmPtr->UseSetB * 6U;
	} else {
		OdtLon = (PDimmPtr->FreqMhz / 533U) * 2U;
	}

	FVal = XFsbl_Ceil(3.6 / PDimmPtr->ClockPeriod);
	if (PDimmPtr->Lpddr4Hynix == 1U) {
		XVal = PDimmPtr->ReadLatency + PDimmPtr->BurstLength / 2U +
			FVal + 2U - OdtLon - XFsbl_Ceil(1.5 /
					PDimmPtr->ClockPeriod) + (6U) +
			(PDimmPtr->WriteLatency - PDimmPtr->WdqsOn);
	} else {
		XVal = PDimmPtr->ReadLatency + PDimmPtr->BurstLength / 2U +
			FVal + 2U - OdtLon - XFsbl_Ceil(1.5 /
					PDimmPtr->ClockPeriod);
	}

	if (PDimmPtr->RdPostamble == 0U) {
		XVal = XVal + 0.5;
	} else if (PDimmPtr->RdPostamble == 1U) {
		XVal = XVal + 1.5;
	}
	DdrCfg[DDR_RD2WR] = (u32)XFsbl_Ceil(XVal / 2.0) + 2U;

	PDimmPtr->TWtr = (u32)XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 8U);

	Val = PDimmPtr->WriteLatency + PDimmPtr->TWtr + PDimmPtr->BurstLength / 2U;

	if ((PDimmPtr->Lpddr4Hynix == 1U) &&
			(PDimmPtr->NumRankAddr == 1U)) {
		Val2 = PDimmPtr->WdqsOff - PDimmPtr->WriteLatency -
			PDimmPtr->BurstLength / 2U - (2U);
		Val += Val2;
	}

	DdrCfg[DDR_WR2RD] = (u32)XFsbl_Ceil((float)(Val + 1U) / 2U);

	DdrCfg[DDR_T_MRW] = (u32)XFSBL_MAX(XFsbl_Ceil(14.0 / PDimmPtr->ClockPeriod), 10U);

	DdrCfg[DDR_T_MRD] = (u32)XFsbl_Ceil((XFSBL_MAX(14.0 / PDimmPtr->ClockPeriod, 10U)) / 2.0);

	DdrCfg[DDR_T_RRD] = (u32)XFsbl_Ceil((XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 4U) + (1.875 * PDimmPtr->TRefRange)) / 2.0);

	DdrCfg[DDR_T_RP] = (u32)XFsbl_Ceil((PDimmPtr->TRpPs / 1000.0) /	2.0);

	DdrCfg[DDR_T_CKSRX] = 1U;

	DdrCfg[DDR_T_CKSRE] = (u32)XFsbl_Ceil(XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 3U) / 2.0);

	DdrCfg[DDR_T_CKESR] = (u32)XFsbl_Ceil(XFSBL_MAX(15U / PDimmPtr->ClockPeriod, 4U) / 2.0);

	DdrCfg[DDR_T_CKE] = (u32)XFsbl_Ceil(XFSBL_MAX(15U / PDimmPtr->ClockPeriod, 4U) / 2.0);

	DdrCfg[DDR_T_CKCSX] = (u32)XFsbl_Ceil((XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 5.0) + 2.0) / 2.0);

	DdrCfg[DDR_T_CKPDE] = (u32)XFsbl_Ceil(XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 3.0) / 2.0);

	DdrCfg[DDR_T_CKPDX] = 1U;

	DdrCfg[DDR_T_XS_DLL_X32] = (u32)XFsbl_Ceil((((PDimmPtr->TRfcPs / 1000.0) + 7.5) / PDimmPtr->ClockPeriod) / 32.0 / 2.0) + 1U;

	DdrCfg[DDR_T_XS_X32] = (u32)XFsbl_Ceil((((PDimmPtr->TRfcPs / 1000.0) + 7.5) / PDimmPtr->ClockPeriod) / 32.0 / 2.0) + 1U;

	if (PDimmPtr->SpeedBin >= 3200U) {
		DdrCfg[DDR_T_CMDCKE] = 1U;

		DdrCfg[DDR_T_CKEHCMD] = (u32)XFsbl_Ceil(XFSBL_MAX(7.5 / PDimmPtr->ClockPeriod, 3.0) / 2.0);
	}

	DdrCfg[DDR_T_ZQ_LONG_NOP] = (u32)XFsbl_Ceil((1000.0 / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_ZQ_SHORT_NOP] = (u32)XFsbl_Ceil(XFSBL_MAX(30.0 / PDimmPtr->ClockPeriod, 8U) / 2.0);

	DdrCfg[DDR_T_ZQ_RESET_NOP] = (u32)XFsbl_Ceil(XFSBL_MAX(3U, 50.0 / PDimmPtr->ClockPeriod) / 2.0);

	DdrCfg[DDR_T_ZQ_SHORT_INTERVAL_X1024] = (u32)((400000000.0 / PDimmPtr->ClockPeriod) / 1024.0);

	Val = (PDimmPtr->SpeedBin >= 1600U) ? ((u32)(1.5 / PDimmPtr->ClockPeriod)) : 1U;
	DdrCfg[DDR_DFI_T_RDDATA_EN] = (PDimmPtr->ReadLatency + Val) - 4U - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_TPHY_WRLAT] = (PDimmPtr->WriteLatency + 1U) - 3U - PDimmPtr->RDimm;

	DdrCfg[DDR_DFI_T_WRDATA_DELAY] = 3U;

	DdrCfg[DDR_DFI_DATA_CS_POLARITY] = 1U;

	DdrCfg[DDR_RD_DBI_EN] = PDimmPtr->RdDbi;

	DdrCfg[DDR_WR_DBI_EN] = PDimmPtr->WrDbi;

	DdrCfg[DDR_DM_EN] = PDimmPtr->DataMask;

	DdrCfg[DDR_RANK1_WR_ODT] = 0U;

	DdrCfg[DDR_RANK0_WR_ODT] = 0x0U;

	return XFSBL_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * This function writes the DDRC registers with calculated values
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	DdrCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_DdrcRegsWrite(XFsbl_DimmParams *PDimmPtr, u32 *DdrCfg)
{
	u32 Val;

	Val =  ((DdrCfg[DDR_DEVICE_CONFIG] & 0x3U) << 30U)
		+ ((DdrCfg[DDR_FREQUENCY_MODE] & 0x1U) << 29U)
		+ ((DdrCfg[DDR_ACTIVE_RANKS] & 0x3U) << 24U)
		+ ((DdrCfg[DDR_BURST_RDWR] & 0xFU) << 16U)
		+ ((DdrCfg[DDR_DLL_OFF_MODE] & 0x1U) << 15U)
		+ ((DdrCfg[DDR_DATA_BUS_WIDTH] & 0x3U) << 12U)
		+ ((DdrCfg[DDR_GEARDOWN_MODE] & 0x1U) << 11U)
		+ ((DdrCfg[DDR_EN_2T_TIMING_MODE] & 0x1U) << 10U)
		+ ((DdrCfg[DDR_BURSTCHOP] & 0x1U) << 9U)
		+ ((DdrCfg[DDR_LPDDR4] & 0x1U) << 5U)
		+ ((DdrCfg[DDR_DDR4] & 0x1U) << 4U)
		+ ((DdrCfg[DDR_LPDDR3] & 0x1U) << 3U)
		+ ((DdrCfg[DDR_LPDDR2] & 0x1U) << 2U)
		+ ((DdrCfg[DDR_DDR3] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x0U, Val);

	Val =  ((DdrCfg[DDR_MR_WR] & 0x1U) << 31U)
		+ ((DdrCfg[DDR_MR_ADDR] & 0xFU) << 12U)
		+ ((DdrCfg[DDR_MR_RANK] & 0x3U) << 4U)
		+ ((DdrCfg[DDR_PDA_EN] & 0x1U) << 2U)
		+ ((DdrCfg[DDR_MPR_EN] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_MR_TYPE] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x10U, Val);

	Val =  ((DdrCfg[DDR_RC_DERATE_VALUE] & 0x3U) << 8U)
		+ ((DdrCfg[DDR_DERATE_BYTE] & 0xFU) << 4U)
		+ ((DdrCfg[DDR_DERATE_VALUE] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_DERATE_ENABLE] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x20U, Val);

	Val =  ((DdrCfg[DDR_MR4_READ_INTERVAL] & 0xFFFFFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x24U, Val);

	Val =  ((DdrCfg[DDR_STAY_IN_SELFREF] & 0x1U) << 6U)
		+ ((DdrCfg[DDR_SELFREF_SW] & 0x1U) << 5U)
		+ ((DdrCfg[DDR_MPSM_EN] & 0x1U) << 4U)
		+ ((DdrCfg[DDR_EN_DFI_DRAM_CLK_DISABLE] & 0x1U) << 3U)
		+ ((DdrCfg[DDR_DEEPPOWERDOWN_EN] & 0x1U) << 2U)
		+ ((DdrCfg[DDR_POWERDOWN_EN] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_SELFREF_EN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x30U, Val);

	Val =  ((DdrCfg[DDR_SELFREF_TO_X32] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_T_DPD_X4096] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_POWERDOWN_TO_X32] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x34U, Val);

	Val =  ((DdrCfg[DDR_REFRESH_MARGIN] & 0xFU) << 20U)
		+ ((DdrCfg[DDR_REFRESH_TO_X32] & 0x1FU) << 12U)
		+ ((DdrCfg[DDR_REFRESH_BURST] & 0x1FU) << 4U)
		+ ((DdrCfg[DDR_PER_BANK_REFRESH] & 0x1U) << 2U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x50U, Val);

	Val =  ((DdrCfg[DDR_REFRESH_TIMER1_START_VALUE_X32] & 0xFFFU) << 16U)
		+ ((DdrCfg[DDR_REFRESH_TIMER0_START_VALUE_X32] & 0xFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x54U, Val);

	Val =  ((DdrCfg[DDR_REFRESH_MODE] & 0x7U) << 4U)
		+ ((DdrCfg[DDR_REFRESH_UPDATE_LEVEL] & 0x1U) << 1U)
		+ ((0x1U << 0U));
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x60U, Val);

	Val =  ((DdrCfg[DDR_T_RFC_NOM_X32] & 0xFFFU) << 16U)
		+ ((0x1U) << 15U)
		+ ((DdrCfg[DDR_T_RFC_MIN] & 0x3FFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x64U, Val);

	Val =  ((DdrCfg[DDR_DIS_SCRUB] & 0x1U) << 4U)
		+ ((DdrCfg[DDR_ECC_MODE] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x70U, Val);

	Val =  ((DdrCfg[DDR_DATA_POISON_BIT] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_DATA_POISON_EN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x74U, Val);

	Val =  ((DdrCfg[DDR_DFI_T_PHY_RDLAT] & 0x3FU) << 24U)
		+ ((DdrCfg[DDR_ALERT_WAIT_FOR_SW] & 0x1U) << 9U)
		+ ((DdrCfg[DDR_CRC_PARITY_RETRY_ENABLE] & 0x1U) << 8U)
		+ ((DdrCfg[DDR_CRC_INC_DM] & 0x1U) << 7U)
		+ ((DdrCfg[DDR_CRC_ENABLE] & 0x1U) << 4U)
		+ ((DdrCfg[DDR_PARITY_ENABLE] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xC4U, Val);

	Val =  ((DdrCfg[DDR_T_PAR_ALERT_PW_MAX] & 0x1FFU) << 16U)
		+ ((DdrCfg[DDR_T_CRC_ALERT_PW_MAX] & 0x1FU) << 8U)
		+ ((0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xC8U, Val);

	Val =  ((DdrCfg[DDR_SKIP_DRAM_INIT] & 0x3U) << 30U)
		+ ((DdrCfg[DDR_POST_CKE_X1024] & 0x3FFU) << 16U)
		+ ((DdrCfg[DDR_PRE_CKE_X1024] & 0xFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xD0U, Val);

	Val =  ((DdrCfg[DDR_DRAM_RSTN_X1024] & 0x1FFU) << 16U)
		+ ((DdrCfg[DDR_FINAL_WAIT_X32] & 0x7FU) << 8U)
		+ ((DdrCfg[DDR_PRE_OCD_X32] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xD4U, Val);

	Val =  ((DdrCfg[DDR_IDLE_AFTER_RESET_X32] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_MIN_STABLE_CLOCK_X1] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xD8U, Val);

	Val =  ((DdrCfg[DDR_MR] & 0xFFFFU) << 16U)
		+ ((DdrCfg[DDR_EMR] & 0xFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xDCU, Val);

	Val =  ((DdrCfg[DDR_EMR2] & 0xFFFFU) << 16U)
		+ ((DdrCfg[DDR_EMR3] & 0xFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xE0U, Val);

	Val =  ((DdrCfg[DDR_DEV_ZQINIT_X32] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_MAX_AUTO_INIT_X1024] & 0x3FFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xE4U, Val);

	Val =  ((DdrCfg[DDR_MR4] & 0xFFFFU) << 16U)
		+ ((DdrCfg[DDR_MR5] & 0xFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xE8U, Val);

	Val =  ((DdrCfg[DDR_MR6] & 0xFFFFU) << 16U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xECU, Val);

	Val =  ((DdrCfg[DDR_DIMM_DIS_BG_MIRRORING] & 0x1U) << 5U)
		+ ((0x1U) << 4U)
		+ ((DdrCfg[DDR_MRS_A17_EN] & 0x1U) << 3U)
		+ ((DdrCfg[DDR_DIMM_OUTPUT_INV_EN] & 0x1U) << 2U)
		+ ((DdrCfg[DDR_DIMM_ADDR_MIRR_EN] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_DIMM_STAGGER_CS_EN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xF0U, Val);

	Val =  ((DdrCfg[DDR_DIFF_RANK_WR_GAP] & 0xFU) << 8U)
		+ ((DdrCfg[DDR_DIFF_RANK_RD_GAP] & 0xFU) << 4U)
		+ ((DdrCfg[DDR_MAX_RANK_RD] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xF4U, Val);

	Val =  ((DdrCfg[DDR_WR2PRE] & 0x7FU) << 24U)
		+ ((DdrCfg[DDR_T_FAW] & 0x3FU) << 16U)
		+ ((DdrCfg[DDR_T_RAS_MAX] & 0x7FU) << 8U)
		+ ((DdrCfg[DDR_T_RAS_MIN] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x100U, Val);

	Val =  ((DdrCfg[DDR_T_XP] & 0x1FU) << 16U)
		+ ((DdrCfg[DDR_RD2PRE] & 0x1FU) << 8U)
		+ ((DdrCfg[DDR_T_RC] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x104U, Val);

	Val =  ((DdrCfg[DDR_WRITE_LATENCY] & 0x3FU) << 24U)
		+ ((DdrCfg[DDR_READ_LATENCY] & 0x3FU) << 16U)
		+ ((DdrCfg[DDR_RD2WR] & 0x3FU) << 8U)
		+ ((DdrCfg[DDR_WR2RD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x108U, Val);

	Val =  ((DdrCfg[DDR_T_MRW] & 0x3FFU) << 20U)
		+ ((DdrCfg[DDR_T_MRD] & 0x3FU) << 12U)
		+ ((DdrCfg[DDR_T_MOD] & 0x3FFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x10CU, Val);

	Val =  ((DdrCfg[DDR_T_RCD] & 0x1FU) << 24U)
		+ ((DdrCfg[DDR_T_CCD] & 0xFU) << 16U)
		+ ((DdrCfg[DDR_T_RRD] & 0xFU) << 8U)
		+ ((DdrCfg[DDR_T_RP] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x110U, Val);

	Val =  ((DdrCfg[DDR_T_CKSRX] & 0xFU) << 24U)
		+ ((DdrCfg[DDR_T_CKSRE] & 0xFU) << 16U)
		+ ((DdrCfg[DDR_T_CKESR] & 0x3FU) << 8U)
		+ ((DdrCfg[DDR_T_CKE] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x114U, Val);

	Val =  ((0x1U & 0xfU) << 24U)
		+ ((0x1U & 0xfU) << 16U)
		+ ((DdrCfg[DDR_T_CKCSX] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x118U, Val);

	Val =  ((DdrCfg[DDR_T_CKPDE] & 0xFU) << 8U)
		+ ((DdrCfg[DDR_T_CKPDX] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x11CU, Val);

	Val =  ((DdrCfg[DDR_T_XS_FAST_X32] & 0x7FU) << 24U)
		+ ((DdrCfg[DDR_T_XS_ABORT_X32] & 0x7FU) << 16U)
		+ ((DdrCfg[DDR_T_XS_DLL_X32] & 0x7FU) << 8U)
		+ ((DdrCfg[DDR_T_XS_X32] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x120U, Val);

	Val =  ((DdrCfg[DDR_DDR4_WR_PREAMBLE] & 0x1U) << 30U)
		+ ((DdrCfg[DDR_T_CCD_S] & 0x7U) << 16U)
		+ ((DdrCfg[DDR_T_RRD_S] & 0xFU) << 8U)
		+ ((DdrCfg[DDR_WR2RD_S] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x124U, Val);

	Val =  ((DdrCfg[DDR_POST_MPSM_GAP_X32] & 0x7FU) << 24U)
		+ ((DdrCfg[DDR_T_MPX_LH] & 0x1FU) << 16U)
		+ ((0x1U & 0x3U) << 8U)
		+ ((DdrCfg[DDR_T_CKMPE] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x12CU, Val);

	Val =  ((DdrCfg[DDR_T_CMDCKE] & 0x3U) << 16U)
		+ ((DdrCfg[DDR_T_CKEHCMD] & 0xFU) << 8U)
		+ ((DdrCfg[DDR_T_MRD_PDA] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x130U, Val);

	Val =  ((0x1U) << 31U)
		+ ((DdrCfg[DDR_DIS_SRX_ZQCL] & 0x1U) << 30U)
		+ ((DdrCfg[DDR_ZQ_RESISTOR_SHARED] & 0x1U) << 29U)
		+ ((DdrCfg[DDR_DIS_MPSMX_ZQCL] & 0x1U) << 28U)
		+ ((DdrCfg[DDR_T_ZQ_LONG_NOP] & 0x7FFU) << 16U)
		+ ((DdrCfg[DDR_T_ZQ_SHORT_NOP] & 0x3FFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x180U, Val);

	Val =  ((DdrCfg[DDR_T_ZQ_RESET_NOP] & 0x3FFU) << 20U)
		+ ((DdrCfg[DDR_T_ZQ_SHORT_INTERVAL_X1024] & 0xFFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x184U, Val);

	Val =  ((0x4U & 0x1fU) << 24U)
		+ ((0x1U) << 23U)
		+ ((DdrCfg[DDR_DFI_T_RDDATA_EN] & 0x3FU) << 16U)
		+ ((0x1U) << 15U)
		+ ((0x2U & 0x3fU) << 8U)
		+ ((DdrCfg[DDR_DFI_TPHY_WRLAT] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x190U, Val);

	Val =  ((DdrCfg[DDR_DFI_T_CMD_LAT] & 0xFU) << 28U)
		+ ((DdrCfg[DDR_DFI_T_WRDATA_DELAY] & 0x1FU) << 16U)
		+ ((0x3U & 0xfU) << 8U)
		+ ((0x4U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x194U, Val);

	Val =  ((0x7U & 0xfU) << 24U)
		+ ((DdrCfg[DDR_DFI_LP_WAKEUP_DPD] & 0xFU) << 20U)
		+ ((DdrCfg[DDR_DFI_LP_EN_DPD] & 0x1U) << 16U)
		+ ((DdrCfg[DDR_DFI_LP_EN_SR] & 0x1U) << 8U)
		+ ((DdrCfg[DDR_DFI_LP_EN_PD] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x198U, Val);

	Val =  ((0x2U & 0xfU) << 4U)
		+ ((DdrCfg[DDR_DFI_LP_EN_MPSM] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x19CU, Val);

	Val =  ((DdrCfg[DDR_DIS_AUTO_CTRLUPD] & 0x1U) << 31U)
		+ ((DdrCfg[DDR_DIS_AUTO_CTRLUPD_SRX] & 0x1U) << 30U)
		+ ((DdrCfg[DDR_DFI_T_CTRLUP_MAX] & 0x3FFU) << 16U)
		+ ((DdrCfg[DDR_DFI_T_CTRLUP_MIN] & 0x3FFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1A0U, Val);

	Val =  ((200U & 0xFFU) << 16U)
		+ ((255U & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1A4U, Val);

	Val =  ((DdrCfg[DDR_DFI_DATA_CS_POLARITY] & 0x1U) << 2U)
		+ ((DdrCfg[DDR_PHY_DBI_MODE] & 0x1U) << 1U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1B0U, Val);

	Val =  ((DdrCfg[DDR_DFI_TPHY_RDCSLAT] & 0x3FU) << 8U)
		+ ((DdrCfg[DDR_DFI_TPHY_WRCSLAT] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1B4U, Val);

	Val =  ((DdrCfg[DDR_RD_DBI_EN] & 0x1U) << 2U)
		+ ((DdrCfg[DDR_WR_DBI_EN] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_DM_EN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1C0U, Val);

	Val =  ((DdrCfg[DDR_WR_ODT_HOLD] & 0xFU) << 24U)
		+ ((DdrCfg[DDR_WR_ODT_DELAY] & 0x1FU) << 16U)
		+ ((DdrCfg[DDR_RD_ODT_HOLD] & 0xFU) << 8U)
		+ ((DdrCfg[DDR_RD_ODT_DELAY] & 0x1FU) << 2U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x240U, Val);

	Val =  ((DdrCfg[DDR_RANK1_WR_ODT] & 0x3U) << 8U)
		+ ((DdrCfg[DDR_RANK0_WR_ODT] & 0x3U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x244U, Val);

	Val =  ((0x1U & 0x7fU) << 24U)
		+ ((DdrCfg[DDR_GO2CRITICAL_HYSTERESIS] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_LPR_NUM_ENTRIES] & 0x3FU) << 8U)
		+ ((DdrCfg[DDR_PREFER_WRITE] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_FORCE_LOW_PRI_N] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x250U, Val);

	Val =  ((0x8U & 0xffU) << 24U)
		+ ((64U & 0xFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x264U, Val);

	Val =  ((0x8U & 0xffU) << 24U)
		+ ((0x40U & 0xffffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x26CU, Val);

	Val =  ((DdrCfg[DDR_DQ_NIBBLE_MAP_12_15] & 0xFFU) << 24U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_8_11] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_4_7] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_0_3] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x280U, Val);

	Val =  ((DdrCfg[DDR_DQ_NIBBLE_MAP_28_31] & 0xFFU) << 24U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_24_27] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_20_23] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_16_19] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x284U, Val);

	Val =  ((DdrCfg[DDR_DQ_NIBBLE_MAP_44_47] & 0xFFU) << 24U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_40_43] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_36_39] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_32_35] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x288U, Val);

	Val =  ((DdrCfg[DDR_DQ_NIBBLE_MAP_60_63] & 0xFFU) << 24U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_56_59] & 0xFFU) << 16U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_52_55] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_48_51] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x28CU, Val);

	Val =  ((DdrCfg[DDR_DQ_NIBBLE_MAP_CB_4_7] & 0xFFU) << 8U)
		+ ((DdrCfg[DDR_DQ_NIBBLE_MAP_CB_0_3] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x290U, Val);

	Val =  ((0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x294U, Val);

	Val =  ((DdrCfg[DDR_DIS_COLLISION_PAGE_OPT] & 0x1U) << 4U)
		+ ((DdrCfg[DDR_DIS_WC] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x300U, Val);

	Val =  ((DdrCfg[DDR_CTRLUPD] & 0x1U) << 5U)
		+ ((DdrCfg[DDR_ZQ_CALIB_SHORT] & 0x1U) << 4U)
		+ ((DdrCfg[DDR_RANK1_REFRESH] & 0x1U) << 1U)
		+ ((DdrCfg[DDR_RANK0_REFRESH] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x30CU, Val);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x320U, 0x0U);

	Val =  ((DdrCfg[DDR_BL_EXP_MODE] & 0x1U) << 8U)
		+ ((DdrCfg[DDR_PAGEMATCH_LIMIT] & 0x1U) << 4U)
		+ ((0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x400U, Val);

	Val =  ((0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x404U, Val);

	Val =  ((0x1U & 0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x408U, Val);

	Val =  ((0x1U & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x490U, Val);

	Val =  ((0x2U & 0x3U) << 20U)
		+ ((0xBU & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x494U, Val);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x498U, 0x0U);

	Val =  ((0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x4B4U, Val);

	Val =  ((0x1U & 0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x4B8U, Val);

	Val =  ((0x1U & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x540U, Val);

	Val =  ((0x2U & 0x3U) << 24U)
		+ ((0xBU & 0xfU) << 8U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x544U, Val);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x548U, 0x0U);

	Val =  ((0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x564U, Val);

	Val =  ((0x1U & 0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x568U, Val);

	Val =  ((0x1U & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x5F0U, Val);

	Val =  ((0x2U & 0x3U) << 24U)
		+ ((0xBU & 0xfU) << 8U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x5F4U, Val);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x5F8U, 0x0U);

	Val =  ((0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x614U, Val);

	Val =  ((0x1U & 0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x618U, Val);

	Val =  ((0x1U & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6A0U, Val);

	Val =  ((0x1U & 0x3U) << 20U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6A4U, Val);

	Val =  ((0x4FU & 0x7ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6A8U, Val);

	Val =  ((0x1U & 0x3U) << 20U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6ACU, Val);

	Val =  ((0x4FU & 0x7ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6B0U, Val);

	Val =  ((0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6C4U, Val);

	Val =  ((0x1U & 0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x6C8U, Val);

	Val =  ((0x1U & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x750U, Val);

	Val =  ((0x1U & 0x3U) << 20U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x754U, Val);

	Val =  ((0x4FU & 0x7ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x758U, Val);

	Val =  ((0x1U & 0x3U) << 20U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x75CU, Val);

	Val =  ((0x4FU & 0x7ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x760U, Val);

	Val =  ((0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x774U, Val);

	Val =  ((0x1U & 0x1U) << 13U)
		+ ((0xFU & 0x3ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x778U, Val);

	Val =  ((0x1U & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x800U, Val);

	Val =  ((0x1U & 0x3U) << 20U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x804U, Val);

	Val =  ((0x4FU & 0x7ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x808U, Val);

	Val =  ((0x1U & 0x3U) << 20U)
		+ ((0x3U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x80CU, Val);

	Val =  ((0x4FU & 0x7ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x810U, Val);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xF04U, 0x0U);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xF08U, 0x0U);

	Val =  ((0x10U & 0x1ffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xF0CU, Val);

	Val =  ((0xFU & 0xffU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0xF10U, Val);

	Val =  ((0x7U & 0x1fU) << 24U)
		+ ((0x1U) << 23U)
		+ ((0x2U & 0x3fU) << 16U)
		+ ((0x1U) << 15U)
		+ ((0x2U & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x2190U, Val);
}

#if !(defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208))
/*****************************************************************************/
/**
 * This function calculates and writes DDR controller registers
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrcRegsInit(struct DdrcInitData *DdrDataPtr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 DdrCfg[300U] = XFSBL_DDRC_REG_DEFVAL;
	u32 Status;

	Status = XFsbl_DdrcCalcCommonRegVal(DdrDataPtr, PDimmPtr, DdrCfg);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR3) {
		Status = XFsbl_DdrcCalcDdr3RegVal(PDimmPtr, DdrCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		Status = XFsbl_DdrcCalcDdr4RegVal(PDimmPtr, DdrCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		Status = XFsbl_DdrcCalcLpddr3RegVal(PDimmPtr, DdrCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		Status = XFsbl_DdrcCalcLpddr4RegVal(PDimmPtr, DdrCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	DdrCfg[DDR_DFI_TPHY_RDCSLAT] = DdrCfg[DDR_DFI_T_RDDATA_EN] - 2U;

	if (!PDimmPtr->RDimm) {
		DdrCfg[DDR_DFI_TPHY_WRCSLAT] = DdrCfg[DDR_DFI_TPHY_WRLAT] - 2U;
	}

	/* Store the MR values which will be used for PHY Registers */
	PDimmPtr->Mr = DdrCfg[DDR_MR];
	PDimmPtr->Emr = DdrCfg[DDR_EMR];
	PDimmPtr->Emr2 = DdrCfg[DDR_EMR2];
	PDimmPtr->Emr3 = DdrCfg[DDR_EMR3];
	PDimmPtr->Mr4 = DdrCfg[DDR_MR4];
	PDimmPtr->Mr5 = DdrCfg[DDR_MR5];
	PDimmPtr->Mr6 = DdrCfg[DDR_MR6];

	XFsbl_DdrcRegsWrite(PDimmPtr, DdrCfg);


END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * This function calculates the PHY register values common to all DDR types
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	PhyCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_PhyCalcCommonRegVal(XFsbl_DimmParams *PDimmPtr, u32 *PhyCfg)
{
	u32 MemSize = 0U;
	u32 Lp4DramSize = 0U;
	u32 NumRank = 0U;

	PhyCfg[PHY_ADCP] = PDimmPtr->AddrMirror;

	if (!PDimmPtr->PhyClkGate) {
		PhyCfg[PHY_GATEACRDCLK] = 0x2U;
		PhyCfg[PHY_GATEACDDRCLK] = 0x2U;
		PhyCfg[PHY_GATEACCTLCLK] = 0x2U;
	}

	if (PDimmPtr->FreqB) {
		PhyCfg[PHY_DDLPGACT] = 1U;
		PhyCfg[PHY_DDLPGRW] = 1U;
	}

	PhyCfg[PHY_TPLLPD] = (u32)XFsbl_Ceil(1000U / (PDimmPtr->ClockPeriod * 2U));

	PhyCfg[PHY_TPLLGS] = (u32)XFsbl_Ceil(4000.0 / (PDimmPtr->ClockPeriod * 2U));

	PhyCfg[PHY_TPLLLOCK] = (u32)XFsbl_Ceil(100000.0 / (PDimmPtr->ClockPeriod * 2U));

	PhyCfg[PHY_TPLLRST] = (u32)XFsbl_Ceil(9000.0 / (PDimmPtr->ClockPeriod * 2U));

	PhyCfg[PHY_PLLBYP] = PDimmPtr->PllByp;

	if (PDimmPtr->CtlClkFreq >  668U) {
		PhyCfg[PHY_FRQSEL] = 8U;
		PhyCfg[PHY_CPPC] = 5U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  560U) {
		PhyCfg[PHY_FRQSEL] = 0U;
		PhyCfg[PHY_CPPC] = 7U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  471U) {
		PhyCfg[PHY_FRQSEL] = 1U;
		PhyCfg[PHY_CPPC] = 8U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  396U) {
		PhyCfg[PHY_FRQSEL] = 2U;
		PhyCfg[PHY_CPPC] = 9U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  332U) {
		PhyCfg[PHY_FRQSEL] = 3U;
		PhyCfg[PHY_CPPC] = 10U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  279U) {
		PhyCfg[PHY_FRQSEL] = 4U;
		PhyCfg[PHY_CPPC] = 6U;
		PhyCfg[PHY_CPIC] = 1U;
	} else if (PDimmPtr->CtlClkFreq >  235U) {
		PhyCfg[PHY_FRQSEL] = 5U;
		PhyCfg[PHY_CPPC] = 8U;
	} else if (PDimmPtr->CtlClkFreq >  197U) {
		PhyCfg[PHY_FRQSEL] = 6U;
		PhyCfg[PHY_CPPC] = 9U;
		PhyCfg[PHY_CPIC] = 2U;
	} else if (PDimmPtr->CtlClkFreq >= 166U) {
		PhyCfg[PHY_FRQSEL] = 7U;
		PhyCfg[PHY_CPPC] = 10U;
		PhyCfg[PHY_CPIC] = 3U;
	}

	PhyCfg[PHY_PUAD] = (u32)(10U / PDimmPtr->ClockPeriod / 2.0);

	if (PDimmPtr->UDimm & PDimmPtr->AddrMirror)
		PhyCfg[PHY_UDIMM] = 1U;

	if (PDimmPtr->En2tTimingMode) {
		PhyCfg[PHY_TRAS] = (u32)XFsbl_Ceil(((PDimmPtr->TRasPs / 1000.0) /
					PDimmPtr->ClockPeriod / 2.0)) * 2U;
	} else {
		PhyCfg[PHY_TRAS] = XFsbl_Round(((PDimmPtr->TRasPs / 1000.0) /
					PDimmPtr->ClockPeriod / 2.0)) * 2U;
	}

	PhyCfg[PHY_TRP] = (PDimmPtr->TRpPs / 1000.0) + 2U;

	PhyCfg[PHY_TFAW] = (u32)XFsbl_Ceil((PDimmPtr->TFawPs / 1000.0) / PDimmPtr->ClockPeriod);

	PhyCfg[PHY_TRFC] = (u32)XFsbl_Ceil((PDimmPtr->TRfcPs / 1000.0) / PDimmPtr->ClockPeriod);

	PhyCfg[PHY_TRC] = (u32)XFsbl_Ceil((PDimmPtr->TRcPs / 1000.0) / PDimmPtr->ClockPeriod) + 1U;

	PhyCfg[PHY_TWTR] = PDimmPtr->TWtr;

	PhyCfg[PHY_PUBWL] = PDimmPtr->WriteLatency;

	PhyCfg[PHY_PUBRL] = PDimmPtr->ReadLatency;

	PhyCfg[PHY_ERROUTODT] = PDimmPtr->Crc;

	PhyCfg[PHY_RDIMM] = PDimmPtr->RDimm;

	PhyCfg[PHY_DTDRS] = PDimmPtr->NumRankAddr;

	PhyCfg[PHY_RANKEN] = (2U * PDimmPtr->NumRankAddr) + 1U;

	Lp4DramSize = (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) ? 32U : PDimmPtr->DramWidth;

	NumRank = (PDimmPtr->Zc1656 || PDimmPtr->HasEccComp) ? 0U : PDimmPtr->NumRankAddr;
	MemSize = ((PDimmPtr->BusWidth / Lp4DramSize) * PDimmPtr->Capacity * (1U << NumRank)) / 8U;
	PhyCfg[PHY_SEED] = (0x12340000U) | MemSize;

	PhyCfg[PHY_DVMIN] = (PDimmPtr->Lp4NoOdt) ? 0x27U : 0U;

	PhyCfg[PHY_PGWAIT_FRQA] = (u32)(40U / (PDimmPtr->ClockPeriod) / 2U);

	if (PDimmPtr->HostDrv == 34U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xDU;
	} else if (PDimmPtr->HostDrv == 40U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xBU;
	} else if (PDimmPtr->HostDrv == 48U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x9U;
	} else if (PDimmPtr->HostDrv == 60U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x7U;
	} else if (PDimmPtr->HostDrv == 80U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x5U;
	} else if (PDimmPtr->HostDrv == 120U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x3U;
	} else {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xBU;
	}

	PhyCfg[PHY_ZPROG_ASYM_DRV_PU] = PhyCfg[PHY_ZPROG_ASYM_DRV_PD];

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function calculates the PHY register values for DDR4
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	PhyCfg is the array to store register field values
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
static u32 XFsbl_PhyCalcDdr4RegVal(XFsbl_DimmParams *PDimmPtr, u32 *PhyCfg)
{
	u32 Val;
	u32 Cal = 0U;
	u32 BurstLength;
	u32 CasLatency;
	u32 Twr;
	u32 Bit2;
	u32 Bit654;
	u32 POdt = 0U;

	float FVal = 0.0;

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / PDimmPtr->ClockPeriod));
	if (PDimmPtr->Fgrm == 1U)
		Val /= 2U;
	else if (PDimmPtr->Fgrm == 2U)
		Val /= 4U;

	if (PDimmPtr->TRefRange) {
		Val /= 2U;
	}
	PhyCfg[PHY_TREFPRD] = (8U * Val) - 1000U;

	PhyCfg[PHY_NOSRA] = 0x1U;

	PhyCfg[PHY_DDRMD] = 0x4U;

	PhyCfg[PHY_TRRD] = (u32)XFSBL_MAX(XFsbl_Ceil((PDimmPtr->TRrdlPs / 1000.0) / PDimmPtr->ClockPeriod), 4U);

	Val = PDimmPtr->AdditiveLatency + XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U);
	Val = XFSBL_MAX(Val, (PDimmPtr->ReadLatency + PDimmPtr->BurstLength / 2U - (PDimmPtr->TRpPs / 1000.0)));
	PhyCfg[PHY_TRTP] = Val;

	if ((PDimmPtr->TMod >= 24U) || (PDimmPtr->TMod <= 30U)) {
		PhyCfg[PHY_TMOD] = (u32)XFsbl_Ceil(PDimmPtr->TMod - 24.0);
	} else {
		PhyCfg[PHY_TMOD] = 6U;
	}

	FVal = (PDimmPtr->SpeedBin < 2666U) ? 8.0 : 9.0;
	if (PDimmPtr->Parity)
		FVal = XFSBL_MAX(24U, 15U / PDimmPtr->ClockPeriod) + PDimmPtr->ParityLatency;
	if (PDimmPtr->CalModeEn)
		FVal = XFSBL_MAX(24U, 15U / PDimmPtr->ClockPeriod) + Cal;
	PhyCfg[PHY_TMRD] = XFSBL_MIN(((u32)XFsbl_Ceil(FVal / 2.0)) * 2U, 32U);

	PhyCfg[PHY_TXS] = (u32)XFsbl_Ceil((PDimmPtr->SpeedBin <= 1866U) ? 597.0 : ((PDimmPtr->SpeedBin <= 2400U) ? 768.0 : 1023.0));

	PhyCfg[PHY_TDLLK] = (u32)XFsbl_Ceil((PDimmPtr->SpeedBin <= 1866U) ? 597.0 : ((PDimmPtr->SpeedBin <= 2400U) ? 768.0 : ((PDimmPtr->SpeedBin <= 2666U) ? 854U : 1023.0)));

	PhyCfg[PHY_TDQSCK] = 0U;

	PhyCfg[PHY_TXP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp), 31U);

	PhyCfg[PHY_TRCD] = (PDimmPtr->TRcdPs / 1000.0) - PDimmPtr->AdditiveLatency + 2U;

	if (PDimmPtr->Parity && PDimmPtr->RDimm)
		PhyCfg[PHY_A17BID] = 1U;

	if (PDimmPtr->RDimm && PDimmPtr->DisOpInv)
		PhyCfg[PHY_RC0] = 0x1U;

	if (PDimmPtr->AddrMirror && PDimmPtr->RDimm)
		PhyCfg[PHY_RC13] = 0x8U;

	PhyCfg[PHY_RC10] = (PDimmPtr->SpeedBin > 1600U) ? ((PDimmPtr->SpeedBin - 1600U) / 266U) : 0U;

	if (PDimmPtr->Parity && PDimmPtr->RDimm)
		PhyCfg[PHY_RC8] = 0x8U;

	BurstLength = (PDimmPtr->BurstLength == 4U) ? 2U : 0U;

	CasLatency = PDimmPtr->CasLatency;
	if (CasLatency <= 16U) {
		Bit654 = (CasLatency - 9U) / 2U;
		Bit2 = ((CasLatency - 1U) % 2U);
	} else if ((CasLatency % 2U) == 1U) {
		Bit654 = (CasLatency + 2U) / 6U;
		Bit2 = (((CasLatency + 1U) / 2U) % 2U);
	} else {
		Bit654 = (CasLatency - 1U) / 4U;
		Bit2 = (((CasLatency / 2U) + 1U) % 2U);
	}

	Twr = (u32)XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod);
	if ((Twr >= 10U) || (Twr <= 24U)) {
		if (Twr % 2U == 1U) {
			Twr += 1U;
		}
		if (Twr < 24U) {
			Twr = (Twr - 10U) / 2U;
		} else {
			Twr = 6U;
		}
	}

	PhyCfg[PHY_R060] = XFSBL_SETBITS(Twr, 9U, 3U) + (0U << 8U) +
		XFSBL_SETBITS(Bit654, 4U, 3U) + (Bit2 << 2U) +
		XFSBL_SETBITS(BurstLength, 0U, 2U);

	if (PDimmPtr->DramOdt == 60U) {
		POdt = 1U;
	} else if (PDimmPtr->DramOdt == 120U) {
		POdt = 2U;
	} else if (PDimmPtr->DramOdt == 40U) {
		POdt = 3U;
	} else if (PDimmPtr->DramOdt == 240U) {
		POdt = 4U;
	} else if (PDimmPtr->DramOdt == 48U) {
		POdt = 5U;
	} else if (PDimmPtr->DramOdt == 80U) {
		POdt = 6U;
	} else if (PDimmPtr->DramOdt == 34U) {
		POdt = 7U;
	} else {
		POdt = 0x3U + (2U * PDimmPtr->NumRankAddr);
	}

	PhyCfg[PHY_R061] = XFSBL_SETBITS(POdt, 8U, 3U) + XFSBL_SETBITS(PDimmPtr->AdditiveLatency, 3U, 2U) + XFSBL_SETBITS(((PDimmPtr->DramDrv == 48U) ? 1U : 0U), 1U, 2U) + 1U;

	if (PDimmPtr->TRefRange)
		PDimmPtr->LpAsr = 0x3U;

	if ((PDimmPtr->CasWriteLatency >= 9U) &&
			(PDimmPtr->CasWriteLatency <= 12U)) {
		Val = PDimmPtr->CasWriteLatency - 9U;
	} else if ((PDimmPtr->CasWriteLatency > 12U) &&
			(PDimmPtr->CasWriteLatency <= 18U)) {
		Val = (PDimmPtr->CasWriteLatency - 14U) / 2U + 4U;
	}

	PhyCfg[PHY_R062] = (PDimmPtr->Crc << 12U) + XFSBL_SETBITS(PDimmPtr->LpAsr, 6U, 2U) + XFSBL_SETBITS(Val, 3U, 3U);

	PhyCfg[PHY_R063] = XFSBL_SETBITS((((PDimmPtr->SpeedBin - 266U) / 800U) - 1U), 9U, 2U) + XFSBL_SETBITS(PDimmPtr->Fgrm, 6U, 3U);

	PhyCfg[PHY_R064] = PDimmPtr->Mr4;

	PhyCfg[PHY_R065] = PDimmPtr->Mr5;

	PhyCfg[PHY_R066] = PDimmPtr->Mr6;

	if (PDimmPtr->RdDbi) {
		if (!PDimmPtr->RdbiWrkAround) {
			PhyCfg[PHY_DTRDBITR] = 0x3U;
		} else {
			PhyCfg[PHY_DTRDBITR] = 0x0U;
		}
	}
	if (!(PDimmPtr->DataMask || PDimmPtr->WrDbi))
		PhyCfg[PHY_DTWBDDM] = 0U;

	if (PDimmPtr->En2ndClk) {
		PhyCfg[PHY_CKOEMODE] = 0x5U;

		PhyCfg[PHY_CKNCLKGATE0] = 0x0U;

		PhyCfg[PHY_CKCLKGATE0] = 0x0U;

		PhyCfg[PHY_CKNCLKGATE1] = 0x0U;

		PhyCfg[PHY_CKCLKGATE1] = 0x0U;
	} else {
		PhyCfg[PHY_CKOEMODE] = ((1U - PDimmPtr->NumRankAddr) * 4U) + 5U;

		PhyCfg[PHY_CKNCLKGATE0] = (1U - PDimmPtr->NumRankAddr) * 0x2U;

		PhyCfg[PHY_CKCLKGATE0] = PhyCfg[PHY_CKNCLKGATE0];

		PhyCfg[PHY_CKNCLKGATE1] = PhyCfg[PHY_CKNCLKGATE0];

		PhyCfg[PHY_CKCLKGATE1] = PhyCfg[PHY_CKNCLKGATE0];
	}

	PhyCfg[PHY_ACREFSSEL] = 0x30U;

	PhyCfg[PHY_ACVREFISEL]  = 0x4EU;

	PhyCfg[PHY_PDAEN] = 0x1U;

	PhyCfg[PHY_DVINIT] = (PDimmPtr->DramOdt == 60U) ? 0x13U : 0x19U;

	PhyCfg[PHY_A03BD] = 0x0U;

	PhyCfg[PHY_A02BD] = 0x0U;

	PhyCfg[PHY_A01BD] = 0x0U;

	PhyCfg[PHY_A00BD] = 0x0U;

	PhyCfg[PHY_A07BD] = 0x0U;

	PhyCfg[PHY_A06BD] = 0x0U;

	PhyCfg[PHY_A05BD] = 0x0U;

	PhyCfg[PHY_A04BD] = 0x0U;

	PhyCfg[PHY_A11BD] = 0x0U;

	PhyCfg[PHY_A10BD] = 0x0U;

	PhyCfg[PHY_A09BD] = 0x0U;

	PhyCfg[PHY_A08BD] = 0x0U;

	PhyCfg[PHY_ODT_MODE] = 0x1U;

	PhyCfg[PHY_ZPROG_HOST_ODT] = 9U;

	PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xDU;

	PhyCfg[PHY_ZPROG_ASYM_DRV_PU] = 0xDU;

	if (!(PDimmPtr->DataMask || PDimmPtr->WrDbi || PDimmPtr->RdDbi)) {
		PhyCfg[PHY_DMEN] = 0U;
		PhyCfg[PHY_DMOEMODE] = 2U;
		PhyCfg[PHY_DMTEMODE] = 2U;
		PhyCfg[PHY_DMPDRMODE] = 1U;
	}

	PhyCfg[PHY_DXREFSSEL] = 0x30U;

	PhyCfg[PHY_DXREFIEN] = 0xFU;

	if ((PDimmPtr->UDimm == 1U) || (PDimmPtr->RDimm == 1U)) {
		PhyCfg[PHY_DXREFISELR1] = 0x55U;
	} else {
		PhyCfg[PHY_DXREFISELR1] = 0x4FU;
	}

	if ((PDimmPtr->UDimm == 1U) || (PDimmPtr->RDimm == 1U)) {
		PhyCfg[PHY_DXREFISELR0] = 0x55U;
	} else {
		PhyCfg[PHY_DXREFISELR0] = 0x4FU;
	}

	return XFSBL_SUCCESS;
}

#if !(defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208))
/*****************************************************************************/
/**
 * This function calculates the PHY register values for DDR3
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	PhyCfg is the array to store register field values
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
static u32 XFsbl_PhyCalcDdr3RegVal(XFsbl_DimmParams *PDimmPtr, u32 *PhyCfg)
{
	u32 Val;
	u32 BurstLength;
	u32 CasLatency;
	u32 Twr;
	u32 Bit2;
	u32 Bit654;
	u32 POdt = 0U;

	float FVal = 0.0;

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / PDimmPtr->ClockPeriod));
	if (PDimmPtr->TRefRange) {
		Val /= 2U;
	}

	PhyCfg[PHY_TREFPRD] = (8U * Val) - 1000U;

	PhyCfg[PHY_NOSRA] = 0x1U;

	PhyCfg[PHY_DDRMD] = 0x3U;


	PhyCfg[PHY_TRRD] = (u32)XFSBL_MAX(XFsbl_Ceil((PDimmPtr->TRrdPs / 1000.0) /
			PDimmPtr->ClockPeriod), 4U);

	PhyCfg[PHY_TRTP] = PDimmPtr->AdditiveLatency + XFSBL_MAX(((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod)), 4U);

	if ((PDimmPtr->TMod >= 12U) || (PDimmPtr->TMod <= 17U)) {
		PhyCfg[PHY_TMOD] = (u32)XFsbl_Ceil(PDimmPtr->TMod - 12.0);
	} else {
		PhyCfg[PHY_TMOD] = 5U;
	}

	PhyCfg[PHY_TMRD] = 4U;

	FVal = ((PDimmPtr->TRfcPs / 1000.0) + 10.0) / PDimmPtr->ClockPeriod;

	PhyCfg[PHY_TXS] = (u32)XFsbl_Ceil(XFSBL_MAX(512.0, FVal));

	PhyCfg[PHY_TDQSCK] = 0U;

	PhyCfg[PHY_TXP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp), 31U);

	PhyCfg[PHY_TRCD] = (PDimmPtr->TRcdPs / 1000.0) - PDimmPtr->AdditiveLatency + 2U;

	PhyCfg[PHY_RC10] = XFSBL_MAX(0U, (PDimmPtr->SpeedBin / 266U) - 3U);

	BurstLength = (PDimmPtr->BurstLength == 4U) ? 2U : 0U;

	CasLatency = PDimmPtr->CasLatency;

	if (CasLatency < 12U) {
		Bit2 = 0U;

		Bit654 = CasLatency - 4U;

	} else {
		Bit2 = 1U;

		Bit654 = CasLatency - 12U;
	}

	Twr = (u32)XFsbl_Ceil(15.0 / PDimmPtr->ClockPeriod);
	Twr = XFSBL_MAX(5U, XFSBL_MIN(16U, Twr));
	PhyCfg[PHY_R060] = XFSBL_SETBITS(((Twr < 9U) ? (Twr - 4U) : ((Twr + 1U) / 2U)), 9U, 3U) + (1U << 8U) + XFSBL_SETBITS(Bit654, 4U, 3U)
		+ (Bit2 << 2U) + XFSBL_SETBITS(BurstLength, 0U, 2U);


	if (PDimmPtr->DramOdt == 60U) {
		POdt = (0U << 9U) + (0U << 6U) + (1U << 2U);
	} else if (PDimmPtr->DramOdt == 120U) {
		POdt = (0U << 9U) + (1U << 6U) + (0U << 2U);
	} else if (PDimmPtr->DramOdt == 40U) {
		POdt = (0U << 9U) + (1U << 6U) + (1U << 2U);
	} else if (PDimmPtr->DramOdt == 20U) {
		POdt = (1U << 9U) + (0U << 6U) + (0U << 2U);
	} else if (PDimmPtr->DramOdt == 30U) {
		POdt = (1U << 9U) + (0U << 6U) + (1U << 2U);
	} else {
		if (PDimmPtr->NumRankAddr == 0U) {
			POdt = (1U << 2U);
		} else {
			POdt = (1U << 6U);
		}
	}

	PhyCfg[PHY_R061] = (PDimmPtr->AdditiveLatency << 3U) + POdt + ((PDimmPtr->DramDrv == 34U) ? 0x2U : 0x0U);

	PhyCfg[PHY_R062] = (PDimmPtr->NumRankAddr << 9U)  + XFSBL_SETBITS((PDimmPtr->CasWriteLatency - 5U), 3U, 3U) + (PDimmPtr->TRefRange << 7U);

	PhyCfg[PHY_R063] = 0x0U;

	PhyCfg[PHY_DTWBDDM] = 0x0U;

	if (PDimmPtr->En2ndClk) {
		PhyCfg[PHY_CKOEMODE] = 0x5U;

		PhyCfg[PHY_CKNCLKGATE0] = 0x0U;

		PhyCfg[PHY_CKCLKGATE0] = 0x0U;

		PhyCfg[PHY_CKNCLKGATE1] = 0x0U;

		PhyCfg[PHY_CKCLKGATE1] = 0x0U;
	} else {
		PhyCfg[PHY_CKOEMODE] = ((1U - PDimmPtr->NumRankAddr) * 4U) + 5U;

		PhyCfg[PHY_CKNCLKGATE0] = (1U - PDimmPtr->NumRankAddr) * 0x2U;

		PhyCfg[PHY_CKCLKGATE0] = PhyCfg[PHY_CKNCLKGATE0];

		PhyCfg[PHY_CKNCLKGATE1] = PhyCfg[PHY_CKNCLKGATE0];

		PhyCfg[PHY_CKCLKGATE1] = PhyCfg[PHY_CKNCLKGATE0];
	}

	PhyCfg[PHY_ACREFSSEL] = 0x30U;

	PhyCfg[PHY_ACVREFISEL] = 0x30U;

	PhyCfg[PHY_PDAEN] = 0x0U;

	if (PDimmPtr->Zc1656 || PDimmPtr->HasEccComp) {
		PhyCfg[PHY_DVINIT] = 0x2EU;
	} else {
		if (PDimmPtr->Lp4NoOdt) {
			PhyCfg[PHY_DVINIT] = 0x2FU;
		} else {
			PhyCfg[PHY_DVINIT] = 0x19U;
		}
	}

	PhyCfg[PHY_A03BD] = 0x0U;

	PhyCfg[PHY_A02BD] = 0x0U;

	PhyCfg[PHY_A01BD] = 0x0U;

	PhyCfg[PHY_A00BD] = 0x0U;

	PhyCfg[PHY_A07BD] = 0x0U;

	PhyCfg[PHY_A06BD] = 0x0U;

	PhyCfg[PHY_A05BD] = 0x0U;

	PhyCfg[PHY_A04BD] = 0x0U;

	PhyCfg[PHY_A11BD] = 0x0U;

	PhyCfg[PHY_A10BD] = 0x0U;

	PhyCfg[PHY_A09BD] = 0x0U;

	PhyCfg[PHY_A08BD] = 0x0U;

	PhyCfg[PHY_ODT_MODE] = 0x0U;

	PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xBU;

	PhyCfg[PHY_ZPROG_ASYM_DRV_PU] = 0xBU;

	PhyCfg[PHY_DXREFSSEL] = 0x30U;

	PhyCfg[PHY_DXREFIEN] = 0x3U;

	PhyCfg[PHY_DXREFISELR1] = 0x30U;

	PhyCfg[PHY_DXREFISELR0] = 0x30U;

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function calculates the PHY register values for LPDDR3
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	PhyCfg is the array to store register field values
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
static u32 XFsbl_PhyCalcLpddr3RegVal(XFsbl_DimmParams *PDimmPtr, u32 *PhyCfg)
{
	u32 Val;

	Val = (u32)(((PDimmPtr->TRefi / 1000.0) / PDimmPtr->ClockPeriod));
	if (PDimmPtr->PerBankRefresh == 0U) {
		if (PDimmPtr->Capacity != 1024U) {
			Val /= 2U;
		}
	} else {
		if (PDimmPtr->Capacity >= 2048U) {
			Val = (u32)((487.5 / PDimmPtr->ClockPeriod));
		} else if (PDimmPtr->Capacity == 1024U) {
			Val = (u32)((975.0 / PDimmPtr->ClockPeriod));
		}
	}
	if (PDimmPtr->TRefRange) {
		Val /= 4U;
	}
	PhyCfg[PHY_TREFPRD] = (8U * Val) - 1000U;

	PhyCfg[PHY_NOSRA] = 0x1U;

	PhyCfg[PHY_DDRMD] = 0x1U;

	PhyCfg[PHY_TRRD] = (u32)XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 2U) + (1.875 * PDimmPtr->TRefRange);

	PhyCfg[PHY_TRTP] =  PDimmPtr->BurstLength / 2U + XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 4U) - 4U;

	PhyCfg[PHY_TMRD] = XFSBL_MIN(((u32)XFsbl_Ceil((XFSBL_MAX(10U, 15U / PDimmPtr->ClockPeriod)) / 2.0)) * 2U, 32U);

	PhyCfg[PHY_TXS] = (u32)XFsbl_Ceil(XFSBL_MAX(2.0, ((PDimmPtr->TRfcPs / 1000.0) + 10U) / PDimmPtr->ClockPeriod));

	PhyCfg[PHY_TDQSCKMAX] = 0x5U;

	PhyCfg[PHY_TDQSCK] = (u32)(1.5 / PDimmPtr->ClockPeriod);

	PhyCfg[PHY_TXP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp), 31U);

	PhyCfg[PHY_TRCD] = (PDimmPtr->TRcdPs / 1000.0) - PDimmPtr->AdditiveLatency + 2U;

	PhyCfg[PHY_R061] = PDimmPtr->Mr;

	PhyCfg[PHY_R062] = PDimmPtr->Emr;

	PhyCfg[PHY_R063] = PDimmPtr->Emr2;

	PhyCfg[PHY_DQODT] = (PDimmPtr->DramOdt == 240U) ? 3U : 2U;

	if ((PDimmPtr->AddrMirror == 1U) && (PDimmPtr->DramWidth == 16U)) {
		PhyCfg[PHY_CA1BYTE1] = 3U;
	}

	if ((PDimmPtr->AddrMirror == 1U) && (PDimmPtr->DramWidth == 16U)) {
		PhyCfg[PHY_CA1BYTE0] = 2U;
	}

	if (PDimmPtr->En2ndClk) {
		PhyCfg[PHY_CKOEMODE] = 0x5U;

		PhyCfg[PHY_CKNCLKGATE0] = 0x0U;

		PhyCfg[PHY_CKCLKGATE0] = 0x0U;

		PhyCfg[PHY_CKNCLKGATE1] = 0x0U;

		PhyCfg[PHY_CKCLKGATE1] = 0x0U;
	} else {
		PhyCfg[PHY_CKOEMODE] = ((1U - PDimmPtr->NumRankAddr) * 4U) + 5U;

		PhyCfg[PHY_CKNCLKGATE0] = (1U - PDimmPtr->NumRankAddr) * 0x2U;

		PhyCfg[PHY_CKCLKGATE0] = PhyCfg[PHY_CKNCLKGATE0];

		PhyCfg[PHY_CKNCLKGATE1] = PhyCfg[PHY_CKNCLKGATE0];

		PhyCfg[PHY_CKCLKGATE1] = PhyCfg[PHY_CKNCLKGATE0];
	}

	PhyCfg[PHY_ACREFSSEL] = 0x30U;

	PhyCfg[PHY_ACVREFISEL] = 0x30U;

	PhyCfg[PHY_PDAEN] = 0x0U;

	if (PDimmPtr->Zc1656 || PDimmPtr->HasEccComp) {
		PhyCfg[PHY_DVINIT] = 0x2EU;
	} else {
		if (PDimmPtr->Lp4NoOdt) {
			PhyCfg[PHY_DVINIT] = 0x2FU;
		} else {
			PhyCfg[PHY_DVINIT] = 0x19U;
		}
	}

	PhyCfg[PHY_ACTBD] = 4U;

	PhyCfg[PHY_BG1BD] = 4U;

	PhyCfg[PHY_BG0BD] = 4U;

	PhyCfg[PHY_BA1BD] = 4U;

	PhyCfg[PHY_BA0BD] = 4U;

	PhyCfg[PHY_A03BD] = 4U;

	PhyCfg[PHY_A02BD] = 4U;

	PhyCfg[PHY_A01BD] = 4U;

	PhyCfg[PHY_A00BD] = 4U;

	PhyCfg[PHY_A07BD] = 4U;

	PhyCfg[PHY_A06BD] = 4U;

	PhyCfg[PHY_A05BD] = 4U;

	PhyCfg[PHY_A04BD] = 4U;

	PhyCfg[PHY_A11BD] = 4U;

	PhyCfg[PHY_A10BD] = 4U;

	PhyCfg[PHY_A09BD] = 4U;

	PhyCfg[PHY_A08BD] = 4U;

	PhyCfg[PHY_A15BD] = 4U;

	PhyCfg[PHY_A14BD] = 4U;

	PhyCfg[PHY_A13BD] = 4U;

	PhyCfg[PHY_A12BD] = 4U;

	PhyCfg[PHY_ODT_MODE] = 0x1U;

	PhyCfg[PHY_ZPROG_DRAM_ODT] = 0x7U;

	PhyCfg[PHY_ZPROG_HOST_ODT] = 7U;

	PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xBU;

	PhyCfg[PHY_ZPROG_ASYM_DRV_PU] = 0xBU;

	PhyCfg[PHY_DXREFSSEL] = 0x30U;

	PhyCfg[PHY_DXREFIEN] = 0x3U;

	PhyCfg[PHY_DXREFISELR1] = 0x3FU;

	PhyCfg[PHY_DXREFISELR0] = 0x3FU;

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function calculates the PHY register values for LPDDR4
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	PhyCfg is the array to store register field values
 *
 * @return	XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
static u32 XFsbl_PhyCalcLpddr4RegVal(XFsbl_DimmParams *PDimmPtr, u32 *PhyCfg)
{
	u32 Val;

	Val = (u32)(((3904U / ((PDimmPtr->PerBankRefresh != 0U) ? 8U : 1U)) / PDimmPtr->ClockPeriod));
	if (PDimmPtr->TRefRange) {
		Val /= 4U;
	}

	PhyCfg[PHY_TREFPRD] = (8U * Val) - 1000U;

	PhyCfg[PHY_NOSRA] = 0x0U;

	PhyCfg[PHY_DDRMD] = 0x5U;

	PhyCfg[PHY_TRRD] = (u32)XFSBL_MAX(XFsbl_Ceil(10.0 / PDimmPtr->ClockPeriod), 4U) + (1.875 * PDimmPtr->TRefRange);

	PhyCfg[PHY_TRAS] = (u32)XFsbl_Ceil(((PDimmPtr->TRasPs / 1000.0) / PDimmPtr->ClockPeriod / 2.0)) * 2U;

	PhyCfg[PHY_TRTP] = (PDimmPtr->BurstLength / 2U) + XFSBL_MAX((u32)XFsbl_Ceil(7.5 / PDimmPtr->ClockPeriod), 8U) - 8U;

	PhyCfg[PHY_TMOD] = 5U;

	PhyCfg[PHY_TMRD] = XFSBL_MIN(((u32)XFsbl_Ceil((XFSBL_MAX(14.0 / PDimmPtr->ClockPeriod, 10U)) / 2.0)) * 2U, 32U);

	PhyCfg[PHY_TXS] = (u32)XFsbl_Ceil(XFSBL_MAX(2.0, ((PDimmPtr->TRfcPs / 1000.0) + 7.5) / PDimmPtr->ClockPeriod));

	PhyCfg[PHY_TDQSCKMAX] = 0x5U;

	PhyCfg[PHY_TDQSCK] = (PDimmPtr->SpeedBin >= 1600U) ? ((u32)(1.5 / PDimmPtr->ClockPeriod)) : 1U;

	PhyCfg[PHY_TXP] = (u32)XFSBL_MIN(XFsbl_Ceil(PDimmPtr->TXp) + 3U, 31U);

	PhyCfg[PHY_TRCD] = XFSBL_MAX(((PDimmPtr->TRcdPs / 1000.0) - PDimmPtr->AdditiveLatency), (PhyCfg[PHY_TRAS] - 8U));

	PhyCfg[PHY_R061] = PDimmPtr->Mr;

	PhyCfg[PHY_R062] = PDimmPtr->Emr;

	PhyCfg[PHY_R063] = PDimmPtr->Emr2;

	PhyCfg[PHY_R064] = PDimmPtr->Emr3;

	if (PDimmPtr->DramCaOdt == 240U) {
		PhyCfg[PHY_CAODT] = 1U;
	} else if (PDimmPtr->DramCaOdt == 120U) {
		PhyCfg[PHY_CAODT] = 2U;
	} else if (PDimmPtr->DramCaOdt == 80U) {
		PhyCfg[PHY_CAODT] = 3U;
	} else if (PDimmPtr->DramCaOdt == 60U) {
		PhyCfg[PHY_CAODT] = 4U;
	} else if (PDimmPtr->DramCaOdt == 48U) {
		PhyCfg[PHY_CAODT] = 5U;
	} else if (PDimmPtr->DramCaOdt == 40U) {
		PhyCfg[PHY_CAODT] = 6U;
	} else {
		PhyCfg[PHY_CAODT] = 5U;
	}

	PhyCfg[PHY_DQODT] = PDimmPtr->DramOdt ? PhyCfg[PHY_CAODT] : 6U;

	PhyCfg[PHY_VREFCA_RANGE] = 0U;

	PhyCfg[PHY_VREFCA] = 0x21U;

	PhyCfg[PHY_VREFDQ_RANGE] = PDimmPtr->Lp4NoOdt;

	if (PDimmPtr->Zc1656 || PDimmPtr->HasEccComp) {
		PhyCfg[PHY_VREFDQ] = 0x2EU;
	} else {
		if (PDimmPtr->Lp4NoOdt) {
			PhyCfg[PHY_VREFDQ] = 0x2FU;
		} else {
			PhyCfg[PHY_VREFDQ] = 0x19U;
		}
	}

	PhyCfg[PHY_ODTD_CA] = 0U;

	PhyCfg[PHY_ODTE_CS] = 1U;

	PhyCfg[PHY_ODTE_CK] = 0U;

	PhyCfg[PHY_CODT] = 6U;

	if (PDimmPtr->RdDbi) {
		if (!PDimmPtr->RdbiWrkAround) {
			PhyCfg[PHY_DTRDBITR] = 0x3U;
		} else {
			PhyCfg[PHY_DTRDBITR] = 0x0U;
		}
	}
	if (!(PDimmPtr->DataMask || PDimmPtr->WrDbi)) {
		PhyCfg[PHY_DTWBDDM] = 0U;
	}

	PhyCfg[PHY_ODTOEMODE] = 0xAU;

	PhyCfg[PHY_CKNCLKGATE0] = 0x0U;

	PhyCfg[PHY_CKCLKGATE0] = 0x0U;

	PhyCfg[PHY_CKOEMODE] = 0x5U;

	PhyCfg[PHY_CKNCLKGATE1] = 0x0U;

	PhyCfg[PHY_CKCLKGATE1] = 0x0U;

	PhyCfg[PHY_ACREFSSEL] = 0x3DU;

	PhyCfg[PHY_ACVREFISEL]  = 0x19U;

	PhyCfg[PHY_PDAEN] = 0x0U;

	if (PDimmPtr->Zc1656 || PDimmPtr->HasEccComp) {
		PhyCfg[PHY_DVINIT] = 0x2EU;
	} else {
		if (PDimmPtr->Lp4NoOdt) {
			PhyCfg[PHY_DVINIT] = 0x2FU;
		} else {
			PhyCfg[PHY_DVINIT] = 0x19U;
		}
	}

	return XFSBL_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 * This function writes the PHY registers with calculated values
 *
 * @param	PDimmPtr is pointer to DIMM parameters Data Structure
 * @param	PhyCfg is the array to store register field values
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_PhyRegsWrite(XFsbl_DimmParams *PDimmPtr, u32 *PhyCfg)
{
	u32 Val;

	Val = ((PhyCfg[PHY_CALBYP] & 0x1U) << 31U)
		+ ((0x1U) << 30U)
		+ ((PhyCfg[PHY_CODTSHFT] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_DQSDCC] & 0x7U) << 24U)
		+ ((0x8U & 0xfU) << 20U)
		+ ((PhyCfg[PHY_DQSNSEPDR] & 0x1U) << 13U)
		+ ((PhyCfg[PHY_DQSSEPDR] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_RTTOAL] & 0x1U) << 11U)
		+ ((0x3U & 0x3U) << 9U)
		+ ((PhyCfg[PHY_CPDRSHFT] & 0x3U) << 7U)
		+ ((PhyCfg[PHY_DQSRPD] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_DQSGPDR] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_DQSGODT] & 0x1U) << 3U)
		+ ((0x1U) << 2U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x700U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x800U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x900U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xA00U, Val);

	Val = ((PhyCfg[PHY_DXPDRMODE] & 0xFFFFU) << 16U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((0x1U & 0x1U) << 13U)
		+ ((0x1U) << 12U)
		+ ((0x1U) << 11U)
		+ ((0x1U) << 10U)
		+ ((0x1U) << 9U)
		+ ((PhyCfg[PHY_DMEN] & 0x1U) << 8U)
		+ ((0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x704U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x804U, Val);

	Val = ((0x1U) << 29U)
		+ ((0x1U) << 28U)
		+ ((0x1U) << 27U)
		+ ((0x1U) << 26U)
		+ ((0x1U) << 25U)
		+ ((0x1U) << 24U)
		+ ((PhyCfg[PHY_DSNOEMODE] & 0x3U) << 20U)
		+ ((PhyCfg[PHY_DSNTEMODE] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_DSNPDRMODE] & 0x3U) << 16U)
		+ ((PhyCfg[PHY_DMOEMODE] & 0x3U) << 14U)
		+ ((PhyCfg[PHY_DMTEMODE] & 0x3U) << 12U)
		+ ((PhyCfg[PHY_DMPDRMODE] & 0x3U) << 10U)
		+ ((PhyCfg[PHY_DSOEMODE] & 0x3U) << 6U)
		+ ((PhyCfg[PHY_DSTEMODE] & 0x3U) << 4U)
		+ ((0x2U & 0x3U) << 2U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x70CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x80CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x90CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xA0CU, Val);

	Val = ((PhyCfg[PHY_DXREFIOM] & 0x7U) << 29U)
		+ ((PhyCfg[PHY_DXREFPEN] & 0x1U) << 28U)
		+ ((0x3U & 0x3U) << 26U)
		+ ((0x1U) << 25U)
		+ ((PhyCfg[PHY_DXREFESELRANGE] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_DXREFESEL] & 0x7FU) << 16U)
		+ ((0x1U & 0x1U) << 15U)
		+ ((PhyCfg[PHY_DXREFSSEL] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFIEN] & 0xFU) << 2U)
		+ ((PhyCfg[PHY_DXREFIMON] & 0x3U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x710U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x810U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x910U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xA10U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC10U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xE10U, Val);

	if (PDimmPtr->BusWidth == 16U)
		PhyCfg[PHY_DQEN] = 0x0U;


	Val = ((PhyCfg[PHY_DXPDRMODE] & 0xFFFFU) << 16U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((0x1U & 0x1U) << 13U)
		+ ((0x1U) << 12U)
		+ ((0x1U) << 11U)
		+ ((0x1U) << 10U)
		+ ((PhyCfg[PHY_DSEN] & 0x1U) << 9U)
		+ ((PhyCfg[PHY_DMEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_DQEN] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x904U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xA04U, Val);

	if (PDimmPtr->BusWidth == 32U) {
		PhyCfg[PHY_CALBYP] = 1U;
		PhyCfg[PHY_MDLEN] = 0U;
		PhyCfg[PHY_DQSNSEPDR] = 1U;
		PhyCfg[PHY_DQSSEPDR] = 1U;
		PhyCfg[PHY_DQSRPD] = 1U;
		PhyCfg[PHY_DQSGPDR] = 1U;
		PhyCfg[PHY_DQSGODT] = 0U;
		PhyCfg[PHY_DQSGOE] = 0U;
		PhyCfg[PHY_DXPDRMODE] = 0x5555U;
		PhyCfg[PHY_OEEN] = 0x0U;
		PhyCfg[PHY_PDREN] = 0x0U;
		PhyCfg[PHY_TEEN] = 0x0U;
		PhyCfg[PHY_DSEN] = 0x0U;
		PhyCfg[PHY_DMEN] = 0x0U;
		PhyCfg[PHY_DXOEMODE] = 0xAAAAU;
		PhyCfg[PHY_DXTEMODE] = 0xAAAAU;
		PhyCfg[PHY_RDBVT] = 0x0U;
		PhyCfg[PHY_WDBVT] = 0x0U;
		PhyCfg[PHY_RGLVT] = 0x0U;
		PhyCfg[PHY_RDLVT] = 0x0U;
		PhyCfg[PHY_WDLVT] = 0x0U;
		PhyCfg[PHY_WLLVT] = 0x0U;
		PhyCfg[PHY_DSNOEMODE] = 2U;
		PhyCfg[PHY_DSNTEMODE] = 2U;
		PhyCfg[PHY_DSNPDRMODE] = 1U;
		PhyCfg[PHY_DMOEMODE] = 2U;
		PhyCfg[PHY_DMTEMODE] = 2U;
		PhyCfg[PHY_DMPDRMODE] = 1U;
		PhyCfg[PHY_DSOEMODE] = 2U;
		PhyCfg[PHY_DSTEMODE] = 2U;
		PhyCfg[PHY_DSPDRMODE] = 1U;
		PhyCfg[PHY_DXREFSEN] = 0U;
	}

	if (PDimmPtr->BusWidth != 64U) {
		PhyCfg[PHY_DQEN] = 0x0U;
	}

	Val = ((PhyCfg[PHY_CALBYP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_MDLEN] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_CODTSHFT] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_DQSDCC] & 0x7U) << 24U)
		+ ((0x8U & 0xfU) << 20U)
		+ ((PhyCfg[PHY_DQSNSEPDR] & 0x1U) << 13U)
		+ ((PhyCfg[PHY_DQSSEPDR] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_RTTOAL] & 0x1U) << 11U)
		+ ((0x3U & 0x3U) << 9U)
		+ ((PhyCfg[PHY_CPDRSHFT] & 0x3U) << 7U)
		+ ((PhyCfg[PHY_DQSRPD] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_DQSGPDR] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_DQSGODT] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_DQSGOE] & 0x1U) << 2U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xB00U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC00U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xD00U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xE00U, Val);

	Val = ((PhyCfg[PHY_DXPDRMODE] & 0xFFFFU) << 16U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((0x1U & 0x1U) << 13U)
		+ ((PhyCfg[PHY_OEEN] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_PDREN] & 0x1U) << 11U)
		+ ((PhyCfg[PHY_TEEN] & 0x1U) << 10U)
		+ ((PhyCfg[PHY_DSEN] & 0x1U) << 9U)
		+ ((PhyCfg[PHY_DMEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_DQEN] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xB04U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC04U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xD04U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xE04U, Val);

	Val = ((PhyCfg[PHY_RDBVT] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_WDBVT] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_RGLVT] & 0x1U) << 27U)
		+ ((PhyCfg[PHY_RDLVT] & 0x1U) << 26U)
		+ ((PhyCfg[PHY_WDLVT] & 0x1U) << 25U)
		+ ((PhyCfg[PHY_WLLVT] & 0x1U) << 24U)
		+ ((PhyCfg[PHY_DSNOEMODE] & 0x3U) << 20U)
		+ ((PhyCfg[PHY_DSNTEMODE] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_DSNPDRMODE] & 0x3U) << 16U)
		+ ((PhyCfg[PHY_DMOEMODE] & 0x3U) << 14U)
		+ ((PhyCfg[PHY_DMTEMODE] & 0x3U) << 12U)
		+ ((PhyCfg[PHY_DMPDRMODE] & 0x3U) << 10U)
		+ ((PhyCfg[PHY_DSOEMODE] & 0x3U) << 6U)
		+ ((PhyCfg[PHY_DSTEMODE] & 0x3U) << 4U)
		+ ((PhyCfg[PHY_DSPDRMODE] & 0x3U) << 2U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xB0CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC0CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xD0CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xE0CU, Val);

	if (PDimmPtr->BusWidth == 32U)
		PhyCfg[PHY_DXREFIEN] = 0U;
	else {
		if (PDimmPtr->NumRankAddr == 1U)
			PhyCfg[PHY_DXREFIEN] = 3U;
		else
			PhyCfg[PHY_DXREFIEN] = 1U;
	}

	Val = ((PhyCfg[PHY_DXREFIOM] & 0x7U) << 29U)
		+ ((PhyCfg[PHY_DXREFPEN] & 0x1U) << 28U)
		+ ((0x3U & 0x3U) << 26U)
		+ ((PhyCfg[PHY_DXREFSEN] & 0x1U) << 25U)
		+ ((PhyCfg[PHY_DXREFESELRANGE] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_DXREFESEL] & 0x7FU) << 16U)
		+ ((0x1U & 0x1U) << 15U)
		+ ((PhyCfg[PHY_DXREFSSEL] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFIEN] & 0xFU) << 2U)
		+ ((PhyCfg[PHY_DXREFIMON] & 0x3U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xB10U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xD10U, Val);

	if (PDimmPtr->Ecc == 0U) {
		PhyCfg[PHY_CALBYP] = 1U;
		PhyCfg[PHY_MDLEN] = 0U;
		PhyCfg[PHY_DQSNSEPDR] = 1U;
		PhyCfg[PHY_DQSSEPDR] = 1U;
		PhyCfg[PHY_DQSRPD] = 1U;
		PhyCfg[PHY_DQSGPDR] = 1U;
		PhyCfg[PHY_DQSGOE] = 0U;
		PhyCfg[PHY_DXPDRMODE] = 0x5555U;
		PhyCfg[PHY_OEEN] = 0U;
		PhyCfg[PHY_PDREN] = 0U;
		PhyCfg[PHY_TEEN] = 0U;
		PhyCfg[PHY_DSEN] = 0U;
		PhyCfg[PHY_DMEN] = 0U;
		PhyCfg[PHY_DQEN] = 0x0U;
		PhyCfg[PHY_DXOEMODE] = 0xAAAAU;
		PhyCfg[PHY_DXTEMODE] = 0xAAAAU;
		PhyCfg[PHY_RDBVT] = 0U;
		PhyCfg[PHY_WDBVT] = 0U;
		PhyCfg[PHY_RGLVT] = 0U;
		PhyCfg[PHY_RDLVT] = 0U;
		PhyCfg[PHY_WDLVT] = 0U;
		PhyCfg[PHY_WLLVT] = 0U;
		PhyCfg[PHY_DSNOEMODE] = 2U;
		PhyCfg[PHY_DSNTEMODE] = 2U;
		PhyCfg[PHY_DSNPDRMODE] = 1U;
		PhyCfg[PHY_DMOEMODE] = 2U;
		PhyCfg[PHY_DMTEMODE] = 2U;
		PhyCfg[PHY_DMPDRMODE] = 1U;
		PhyCfg[PHY_DSOEMODE] = 2U;
		PhyCfg[PHY_DSTEMODE] = 2U;
		PhyCfg[PHY_DSPDRMODE] = 1U;
		PhyCfg[PHY_DXREFSEN] = 0U;
	} else {
		PhyCfg[PHY_DQSGPDR] = 1U;
	}
	Val = ((PhyCfg[PHY_CALBYP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_MDLEN] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_CODTSHFT] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_DQSDCC] & 0x7U) << 24U)
		+ ((0x8U & 0xfU) << 20U)
		+ ((PhyCfg[PHY_DQSNSEPDR] & 0x1U) << 13U)
		+ ((PhyCfg[PHY_DQSSEPDR] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_RTTOAL] & 0x1U) << 11U)
		+ ((0x3U & 0x3U) << 9U)
		+ ((PhyCfg[PHY_CPDRSHFT] & 0x3U) << 7U)
		+ ((PhyCfg[PHY_DQSRPD] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_DQSGPDR] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_DQSGODT] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_DQSGOE] & 0x1U) << 2U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF00U, Val);

	Val = ((PhyCfg[PHY_DXPDRMODE] & 0xFFFFU) << 16U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((0x1U & 0x1U) << 13U)
		+ ((PhyCfg[PHY_OEEN] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_PDREN] & 0x1U) << 11U)
		+ ((PhyCfg[PHY_TEEN] & 0x1U) << 10U)
		+ ((PhyCfg[PHY_DSEN] & 0x1U) << 9U)
		+ ((PhyCfg[PHY_DMEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_DQEN] & 0xFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF04U, Val);

	Val = ((PhyCfg[PHY_DXOEMODE] & 0xFFFFU) << 16U)
		+ ((PhyCfg[PHY_DXTEMODE] & 0xFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF08U, Val);

	Val = ((PhyCfg[PHY_RDBVT] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_WDBVT] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_RGLVT] & 0x1U) << 27U)
		+ ((PhyCfg[PHY_RDLVT] & 0x1U) << 26U)
		+ ((PhyCfg[PHY_WDLVT] & 0x1U) << 25U)
		+ ((PhyCfg[PHY_WLLVT] & 0x1U) << 24U)
		+ ((PhyCfg[PHY_DSNOEMODE] & 0x3U) << 20U)
		+ ((PhyCfg[PHY_DSNTEMODE] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_DSNPDRMODE] & 0x3U) << 16U)
		+ ((PhyCfg[PHY_DMOEMODE] & 0x3U) << 14U)
		+ ((PhyCfg[PHY_DMTEMODE] & 0x3U) << 12U)
		+ ((PhyCfg[PHY_DMPDRMODE] & 0x3U) << 10U)
		+ ((PhyCfg[PHY_DSOEMODE] & 0x3U) << 6U)
		+ ((PhyCfg[PHY_DSTEMODE] & 0x3U) << 4U)
		+ ((PhyCfg[PHY_DSPDRMODE] & 0x3U) << 2U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF0CU, Val);

	if (PDimmPtr->Ecc == 0U)
		PhyCfg[PHY_DXREFIEN] = 0U;
	else {
		if (PDimmPtr->NumRankAddr == 1U)
			PhyCfg[PHY_DXREFIEN] = 3U;
		else
			PhyCfg[PHY_DXREFIEN] = 1U;
	}

	Val = ((PhyCfg[PHY_DXREFIOM] & 0x7U) << 29U)
		+ ((PhyCfg[PHY_DXREFPEN] & 0x1U) << 28U)
		+ ((0x3U & 0x3U) << 26U)
		+ ((PhyCfg[PHY_DXREFSEN] & 0x1U) << 25U)
		+ ((PhyCfg[PHY_DXREFESELRANGE] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_DXREFESEL] & 0x7FU) << 16U)
		+ ((0x1U & 0x1U) << 15U)
		+ ((PhyCfg[PHY_DXREFSSEL] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFIEN] & 0xFU) << 2U)
		+ ((PhyCfg[PHY_DXREFIMON] & 0x3U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF10U, Val);

	if (!PDimmPtr->PhyClkGate)
		PhyCfg[PHY_GATEDXRDCLK] = 0x2U;

	if (!PDimmPtr->PhyClkGate)
		PhyCfg[PHY_GATEDXDDRCLK] = 0x2U;

	if (!PDimmPtr->PhyClkGate)
		PhyCfg[PHY_GATEDXCTLCLK] = 0x2U;

	if (PDimmPtr->PllByp)
		PhyCfg[PHY_PLLBYP] = 1U;

	if (PDimmPtr->CtlClkFreq >  668U) {
		PhyCfg[PHY_FRQSEL] = 8U;
		PhyCfg[PHY_CPPC] = 5U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  560U) {
		PhyCfg[PHY_FRQSEL] = 0U;
		PhyCfg[PHY_CPPC] = 7U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  471U) {
		PhyCfg[PHY_FRQSEL] = 1U;
		PhyCfg[PHY_CPPC] = 8U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  396U) {
		PhyCfg[PHY_FRQSEL] = 2U;
		PhyCfg[PHY_CPPC] = 9U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  332U) {
		PhyCfg[PHY_FRQSEL] = 3U;
		PhyCfg[PHY_CPPC] = 10U;
		PhyCfg[PHY_CPIC] = 0U;
	} else if (PDimmPtr->CtlClkFreq >  279U) {
		PhyCfg[PHY_FRQSEL] = 4U;
		PhyCfg[PHY_CPPC] = 6U;
		PhyCfg[PHY_CPIC] = 1U;
	} else if (PDimmPtr->CtlClkFreq >  235U) {
		PhyCfg[PHY_FRQSEL] = 5U;
		PhyCfg[PHY_CPPC] = 8U;
	} else if (PDimmPtr->CtlClkFreq >  197U) {
		PhyCfg[PHY_FRQSEL] = 6U;
		PhyCfg[PHY_CPPC] = 9U;
		PhyCfg[PHY_CPIC] = 2U;
	} else if (PDimmPtr->CtlClkFreq >= 166U) {
		PhyCfg[PHY_FRQSEL] = 7U;
		PhyCfg[PHY_CPPC] = 10U;
		PhyCfg[PHY_CPIC] = 3U;
	}

	if (((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3)) &&
			((PDimmPtr->GateExt == 1U))) {
		if (PDimmPtr->NoGateExtNoTrain == 1U) {
			PhyCfg[PHY_DQSGX] = 0x0U;
		} else {
			PhyCfg[PHY_DQSGX] = 0x3U;
		}
	} else {
		PhyCfg[PHY_DQSGX] = 0x0U;
	}

	PhyCfg[PHY_DXSR] = 0x3U;

	if ((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) ||
			((PDimmPtr->MemType == SPD_MEMTYPE_DDR4) &&
			 PDimmPtr->WrPreamble))
		PhyCfg[PHY_PREOEX] = 0x3U;

	if (PDimmPtr->Preoex != 0U)
		PhyCfg[PHY_PREOEX] = PDimmPtr->Preoex;

	if (PDimmPtr->PhyDbiMode && PDimmPtr->RdDbi)
		PhyCfg[PHY_RDBI] = 1U;

	if (PDimmPtr->PhyDbiMode && PDimmPtr->WrDbi)
		PhyCfg[PHY_WDBI] = 1U;

	PhyCfg[PHY_DXDACRANGE] = 0x7U;

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4)
		PhyCfg[PHY_DXIOM] = 0x2U;

	else if (PDimmPtr->MemType == SPD_MEMTYPE_DDR3)
		PhyCfg[PHY_DXIOM] = 0x0U;

	else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4)
		PhyCfg[PHY_DXIOM] = 0x4U;

	else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3)
		PhyCfg[PHY_DXIOM] = 0x2U;


	Val = ((PhyCfg[PHY_GATEDXRDCLK] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_GATEDXDDRCLK] & 0x3U) << 26U)
		+ ((PhyCfg[PHY_GATEDXCTLCLK] & 0x3U) << 24U)
		+ ((PhyCfg[PHY_CLKLEVEL] & 0x3U) << 22U)
		+ ((PhyCfg[PHY_LBMODE] & 0x1U) << 21U)
		+ ((PhyCfg[PHY_LBGSDQS] & 0x1U) << 20U)
		+ ((PhyCfg[PHY_LBDGDQS] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_LBDQSS] & 0x1U) << 17U)
		+ ((0x1U & 0x1U) << 16U)
		+ ((0x1U & 0x1U) << 15U)
		+ ((PhyCfg[PHY_DLTST] & 0x1U) << 14U)
		+ ((PhyCfg[PHY_DLTMODE] & 0x1U) << 13U)
		+ ((0x3U & 0x3U) << 11U)
		+ ((0x3U & 0x3U) << 9U)
		+ ((0x3U & 0x3U) << 7U)
		+ ((0x3U & 0x3U) << 5U)
		+ ((0xFU & 0xfU) << 1U)
		+ ((PhyCfg[PHY_OSCEN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1400U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1440U, Val);

	Val = ((PhyCfg[PHY_PLLBYP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_PLLRST] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_PLLPD] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_RSTOPM] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_FRQSEL] & 0xFU) << 24U)
		+ ((PhyCfg[PHY_RLOCKM] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_CPPC] & 0x3FU) << 17U)
		+ ((PhyCfg[PHY_CPIC] & 0xFU) << 13U)
		+ ((PhyCfg[PHY_GSHIFT] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_ATOEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_ATC] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DTC] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1404U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1444U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x17C4U, Val);

	Val = ((0x1U & 0x1U) << 24U)
		+ ((0x1U) << 21U)
		+ ((PhyCfg[PHY_DQSGX] & 0x3U) << 19U)
		+ ((0x1U & 0x1U) << 18U)
		+ ((0x1U & 0x1U) << 17U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((PhyCfg[PHY_UDQIOM] & 0x1U) << 13U)
		+ ((PhyCfg[PHY_DXSR] & 0x3U) << 8U)
		+ ((PhyCfg[PHY_DQSNRES] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DQSRES] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x141CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x145CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x17DCU, Val);

	Val = ((PhyCfg[PHY_CRDEN] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_PREOEX] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_IOAG] & 0x1U) << 16U)
		+ ((PhyCfg[PHY_IOLB] & 0x1U) << 15U)
		+ ((0xCU & 0xfU) << 9U)
		+ ((PhyCfg[PHY_RDBI] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_WDBI] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_PRFBYP] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_RDMODE] & 0x3U) << 4U)
		+ ((PhyCfg[PHY_DISRST] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_DQSGLB] & 0x3U) << 1U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x142CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x146CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14ACU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14ECU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x152CU, Val);

	Val = ((PhyCfg[PHY_DXDACRANGE] & 0x7U) << 28U)
		+ ((PhyCfg[PHY_DXVREFIOM] & 0x7U) << 25U)
		+ ((PhyCfg[PHY_DXIOM] & 0x7U) << 22U)
		+ ((PhyCfg[PHY_DXTXM] & 0x7FFU) << 11U)
		+ ((PhyCfg[PHY_DXRXM] & 0x7FFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1430U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1470U, Val);

	if (PDimmPtr->BusWidth != 64U) {
		PhyCfg[PHY_GATEDXRDCLK] = 0x1U;
	}

	if (PDimmPtr->BusWidth != 64U) {
		PhyCfg[PHY_GATEDXDDRCLK] = 0x1U;
	}

	if (PDimmPtr->BusWidth != 64U) {
		PhyCfg[PHY_GATEDXCTLCLK] = 0x1U;
	}

	if (PDimmPtr->BusWidth == 32U) {
		PhyCfg[PHY_PLLPD] = 1U;
	}

	if (PDimmPtr->BusWidth == 32U) {
		PhyCfg[PHY_UDQIOM] = 1U;
	}

	if (PDimmPtr->BusWidth == 32U) {
		PhyCfg[PHY_DXIOM] = 0x1U;
	}

	Val = ((PhyCfg[PHY_GATEDXRDCLK] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_GATEDXDDRCLK] & 0x3U) << 26U)
		+ ((PhyCfg[PHY_GATEDXCTLCLK] & 0x3U) << 24U)
		+ ((PhyCfg[PHY_CLKLEVEL] & 0x3U) << 22U)
		+ ((PhyCfg[PHY_LBMODE] & 0x1U) << 21U)
		+ ((PhyCfg[PHY_LBGSDQS] & 0x1U) << 20U)
		+ ((PhyCfg[PHY_LBDGDQS] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_LBDQSS] & 0x1U) << 17U)
		+ ((0x1U & 0x1U) << 16U)
		+ ((0x1U & 0x1U) << 15U)
		+ ((PhyCfg[PHY_DLTST] & 0x1U) << 14U)
		+ ((PhyCfg[PHY_DLTMODE] & 0x1U) << 13U)
		+ ((0x3U & 0x3U) << 11U)
		+ ((0x3U & 0x3U) << 9U)
		+ ((0x3U & 0x3U) << 7U)
		+ ((0x3U & 0x3U) << 5U)
		+ ((0xFU & 0xfU) << 1U)
		+ ((PhyCfg[PHY_OSCEN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1480U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14C0U, Val);

	Val = ((PhyCfg[PHY_PLLBYP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_PLLRST] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_PLLPD] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_RSTOPM] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_FRQSEL] & 0xFU) << 24U)
		+ ((PhyCfg[PHY_RLOCKM] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_CPPC] & 0x3FU) << 17U)
		+ ((PhyCfg[PHY_CPIC] & 0xFU) << 13U)
		+ ((PhyCfg[PHY_GSHIFT] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_ATOEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_ATC] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DTC] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1484U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14C4U, Val);

	Val = ((0x1U & 0x1U) << 24U)
		+ ((0x1U & 0x1U) << 21U)
		+ ((PhyCfg[PHY_DQSGX] & 0x3U) << 19U)
		+ ((0x1U & 0x1U) << 18U)
		+ ((0x1U & 0x1U) << 17U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((PhyCfg[PHY_UDQIOM] & 0x1U) << 13U)
		+ ((PhyCfg[PHY_DXSR] & 0x3U) << 8U)
		+ ((PhyCfg[PHY_DQSNRES] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DQSRES] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x149CU, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14DCU, Val);

	Val = ((PhyCfg[PHY_DXDACRANGE] & 0x7U) << 28U)
		+ ((PhyCfg[PHY_DXVREFIOM] & 0x7U) << 25U)
		+ ((PhyCfg[PHY_DXIOM] & 0x7U) << 22U)
		+ ((PhyCfg[PHY_DXTXM] & 0x7FFU) << 11U)
		+ ((PhyCfg[PHY_DXRXM] & 0x7FFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14B0U, Val);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x14F0U, Val);

	if (PDimmPtr->Ecc != 1U) {
		PhyCfg[PHY_GATEDXRDCLK] = 0x1U;
		PhyCfg[PHY_GATEDXDDRCLK] = 0x1U;
		PhyCfg[PHY_GATEDXCTLCLK] = 0x1U;
		PhyCfg[PHY_PLLPD] = 1U;
		PhyCfg[PHY_UDQIOM] = 1U;
		PhyCfg[PHY_DXIOM] = 1U;
	}

	Val = ((PhyCfg[PHY_GATEDXRDCLK] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_GATEDXDDRCLK] & 0x3U) << 26U)
		+ ((PhyCfg[PHY_GATEDXCTLCLK] & 0x3U) << 24U)
		+ ((PhyCfg[PHY_CLKLEVEL] & 0x3U) << 22U)
		+ ((PhyCfg[PHY_LBMODE] & 0x1U) << 21U)
		+ ((PhyCfg[PHY_LBGSDQS] & 0x1U) << 20U)
		+ ((PhyCfg[PHY_LBDGDQS] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_LBDQSS] & 0x1U) << 17U)
		+ ((0x1U & 0x1U) << 16U)
		+ ((0x1U & 0x1U) << 15U)
		+ ((PhyCfg[PHY_DLTST] & 0x1U) << 14U)
		+ ((PhyCfg[PHY_DLTMODE] & 0x1U) << 13U)
		+ ((0x3U & 0x3U) << 11U)
		+ ((0x3U & 0x3U) << 9U)
		+ ((0x3U & 0x3U) << 7U)
		+ ((0x3U & 0x3U) << 5U)
		+ ((0xFU & 0xfU) << 1U)
		+ ((PhyCfg[PHY_OSCEN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1500U, Val);

	Val = ((PhyCfg[PHY_PLLBYP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_PLLRST] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_PLLPD] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_RSTOPM] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_FRQSEL] & 0xFU) << 24U)
		+ ((PhyCfg[PHY_RLOCKM] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_CPPC] & 0x3FU) << 17U)
		+ ((PhyCfg[PHY_CPIC] & 0xFU) << 13U)
		+ ((PhyCfg[PHY_GSHIFT] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_ATOEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_ATC] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DTC] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1504U, Val);

	Val = ((0x1U & 0x1U) << 24U)
		+ ((0x1U & 0x1U) << 21U)
		+ ((PhyCfg[PHY_DQSGX] & 0x3U) << 19U)
		+ ((0x1U & 0x1U) << 18U)
		+ ((0x1U & 0x1U) << 17U)
		+ ((0x1U & 0x1U) << 14U)
		+ ((PhyCfg[PHY_UDQIOM] & 0x1U) << 13U)
		+ ((PhyCfg[PHY_DXSR] & 0x3U) << 8U)
		+ ((PhyCfg[PHY_DQSNRES] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DQSRES] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x151CU, Val);

	Val = ((PhyCfg[PHY_DXDACRANGE] & 0x7U) << 28U)
		+ ((PhyCfg[PHY_DXVREFIOM] & 0x7U) << 25U)
		+ ((PhyCfg[PHY_DXIOM] & 0x7U) << 22U)
		+ ((PhyCfg[PHY_DXTXM] & 0x7FFU) << 11U)
		+ ((PhyCfg[PHY_DXRXM] & 0x7FFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1530U, Val);


	Val = ((PhyCfg[PHY_ADCP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_RESERVED_30_27] & 0xFU) << 27U)
		+ ((0x1U & 0x1U) << 26U)
		+ ((0x3U) << 24U)
		+ ((PhyCfg[PHY_RESERVED_23_19] & 0x1FU) << 19U)
		+ ((PhyCfg[PHY_DTOSEL] & 0x1FU) << 14U)
		+ ((0xFU & 0xfU) << 9U)
		+ ((PhyCfg[PHY_OSCEN] & 0x1U) << 8U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x10U, Val);

	Val = ((PhyCfg[PHY_CLRTSTAT] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_CLRZCAL] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_CLRPERR] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_ICPC] & 0x1U) << 28U)
		+ ((0xFU & 0xffU) << 20U)
		+ ((PhyCfg[PHY_INITFSMBYP] & 0x1U) << 19U)
		+ ((PhyCfg[PHY_PLLFSMBYP] & 0x1U) << 18U)
		+ ((PhyCfg[PHY_TREFPRD] & 0x3FFFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x18U, Val);

	Val = ((0x55U & 0xffU) << 24U)
		+ ((0xAAU & 0xffU) << 16U)
		+ ((PhyCfg[PHY_GATEACRDCLK] & 0x3U) << 13U)
		+ ((PhyCfg[PHY_GATEACDDRCLK] & 0x3U) << 11U)
		+ ((PhyCfg[PHY_GATEACCTLCLK] & 0x3U) << 9U)
		+ ((0x2U & 0x3U) << 6U)
		+ ((PhyCfg[PHY_IOLB0] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_RDMODE0] & 0x3U) << 3U)
		+ ((PhyCfg[PHY_DISRST0] & 0x1U) << 2U)
		+ ((PhyCfg[PHY_CLKLEVEL0] & 0x3U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1CU, Val);

	Val = ((0x1U & 0xffU) << 24U)
		+ ((0x1U & 0xffU) << 16U)
		+ ((PhyCfg[PHY_DISCNPERIOD] & 0xFFU) << 8U)
		+ ((0xFU) << 4U)
		+ ((0x1U) << 2U)
		+ ((PhyCfg[PHY_DDLPGACT] & 0x1U) << 1U)
		+ ((PhyCfg[PHY_DDLPGRW] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x24U, Val);

	Val = ((PhyCfg[PHY_TPLLPD] & 0x7FFU) << 21U)
		+ ((PhyCfg[PHY_TPLLGS] & 0x7FFFU) << 6U)
		+ ((0x10U & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x40U, Val);

	Val = ((PhyCfg[PHY_TPLLLOCK] & 0xFFFFU) << 16U)
		+ ((PhyCfg[PHY_TPLLRST] & 0x1FFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x44U, Val);

	PhyCfg[PHY_PLLPD] = 0U;

	Val = ((PhyCfg[PHY_PLLBYP] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_PLLRST] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_PLLPD] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_RSTOPM] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_FRQSEL] & 0xFU) << 24U)
		+ ((PhyCfg[PHY_RLOCKM] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_CPPC] & 0x3FU) << 17U)
		+ ((PhyCfg[PHY_CPIC] & 0xFU) << 13U)
		+ ((PhyCfg[PHY_GSHIFT] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_ATOEN] & 0x1U) << 8U)
		+ ((PhyCfg[PHY_ATC] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DTC] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x68U, Val);

	Val = ((PhyCfg[PHY_RDBICLSEL] & 0x1U) << 27U)
		+ ((0x2U & 0x7U) << 24U)
		+ ((0x1U) << 23U)
		+ ((0x1U) << 21U)
		+ ((PhyCfg[PHY_SDRMODE] & 0x3U) << 19U)
		+ ((PhyCfg[PHY_ATOAE] & 0x1U) << 17U)
		+ ((PhyCfg[PHY_DTOOE] & 0x1U) << 16U)
		+ ((PhyCfg[PHY_DTOIOM] & 0x1U) << 15U)
		+ ((0x1U) << 14U)
		+ ((PhyCfg[PHY_DTOODT] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_PUAD] & 0x3FU) << 6U)
		+ ((0x1U) << 5U)
		+ ((PhyCfg[PHY_CTLZUEN] & 0x1U) << 2U)
		+ ((PhyCfg[PHY_RESERVED_1] & 0x1U) << 1U)
		+ ((0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x90U, Val);

	Val = ((PhyCfg[PHY_GPR1] & 0xFFFFFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC4U, Val);

	Val = ((PhyCfg[PHY_GEARDN] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_UBG] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_UDIMM] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_DDR2T] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_NOSRA] & 0x1U) << 27U)
		+ ((0x1U & 0xffU) << 10U)
		+ ((PhyCfg[PHY_DDRTYPE] & 0x3U) << 8U)
		+ ((PhyCfg[PHY_MPRDQ] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_PDQ] & 0x7U) << 4U)
		+ ((0x1U) << 3U)
		+ ((PhyCfg[PHY_DDRMD] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x100U, Val);

	Val = ((PhyCfg[PHY_TRRD] & 0x1FU) << 24U)
		+ ((PhyCfg[PHY_TRAS] & 0x7FU) << 16U)
		+ ((PhyCfg[PHY_TRP] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_TRTP] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x110U, Val);

	Val = ((0x28U & 0x7fU) << 24U)
		+ ((PhyCfg[PHY_TFAW] & 0x7FU) << 16U)
		+ ((PhyCfg[PHY_TMOD] & 0x7U) << 8U)
		+ ((PhyCfg[PHY_TMRD] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x114U, Val);

	Val = ((PhyCfg[PHY_TRTW] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_TRTODT] & 0x1U) << 24U)
		+ ((0xFU) << 16U)
		+ ((PhyCfg[PHY_TXS] & 0x3FFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x118U, Val);

	Val = ((0x4 & 0x7U) << 29U)
		+ ((PhyCfg[PHY_TDLLK] & 0x3FFU) << 16U)
		+ ((PhyCfg[PHY_TDQSCKMAX] & 0xFU) << 8U)
		+ ((PhyCfg[PHY_TDQSCK] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x11CU, Val);

	Val = ((PhyCfg[PHY_TAOND_TAOFD] & 0x3U) << 28U)
		+ ((PhyCfg[PHY_TRFC] & 0x3FFU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((PhyCfg[PHY_TXP] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x120U, Val);

	Val = ((PhyCfg[PHY_TRC] & 0xFFU) << 16U)
		+ ((PhyCfg[PHY_TRCD] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_TWTR] & 0x1FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x124U, Val);

	Val = ((PhyCfg[PHY_PUBWL] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_PUBRL] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x128U, Val);

	Val = ((PhyCfg[PHY_QCSEN] & 0x1U) << 30U)
		+ ((0x1U) << 27U)
		+ ((PhyCfg[PHY_ERROUTOE] & 0x1U) << 23U)
		+ ((0x1U) << 22U)
		+ ((PhyCfg[PHY_ERROUTPDR] & 0x1U) << 21U)
		+ ((PhyCfg[PHY_ERROUTODT] & 0x1U) << 19U)
		+ ((PhyCfg[PHY_LRDIMM] & 0x1U) << 18U)
		+ ((PhyCfg[PHY_PARINIOM] & 0x1U) << 17U)
		+ ((0x2 & 0x3U) << 4U)
		+ ((PhyCfg[PHY_SOPERR] & 0x1U) << 2U)
		+ ((PhyCfg[PHY_ERRNOREG] & 0x1U) << 1U)
		+ ((PhyCfg[PHY_RDIMM] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x140U, Val);

	Val = ((PhyCfg[PHY_A17BID] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_TBCMRD_L2] & 0x7U) << 24U)
		+ ((PhyCfg[PHY_TBCMRD_L] & 0x7U) << 20U)
		+ ((PhyCfg[PHY_TBCMRD] & 0x7U) << 16U)
		+ ((0xC80U & 0x3fffU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x144U, Val);

	Val = ((PhyCfg[PHY_RC7] & 0xFU) << 28U)
		+ ((PhyCfg[PHY_RC6] & 0xFU) << 24U)
		+ ((PhyCfg[PHY_RC5] & 0xFU) << 20U)
		+ ((PhyCfg[PHY_RC4] & 0xFU) << 16U)
		+ ((PhyCfg[PHY_RC3] & 0xFU) << 12U)
		+ ((PhyCfg[PHY_RC2] & 0xFU) << 8U)
		+ ((PhyCfg[PHY_RC1] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_RC0] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x150U, Val);

	Val = ((PhyCfg[PHY_RC15] & 0xFU) << 28U)
		+ ((PhyCfg[PHY_RC14] & 0xFU) << 24U)
		+ ((PhyCfg[PHY_RC13] & 0xFU) << 20U)
		+ ((PhyCfg[PHY_RC12] & 0xFU) << 16U)
		+ ((PhyCfg[PHY_RC11] & 0xFU) << 12U)
		+ ((PhyCfg[PHY_RC10] & 0xFU) << 8U)
		+ ((PhyCfg[PHY_RC9] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_RC8] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x154U, Val);

	Val = ((0x2U & 0x3U) << 5U)
		+ ((0x2U & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x180U, PhyCfg[PHY_R060]);

	Val = ((PhyCfg[PHY_RDPST] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_NWR] & 0x7U) << 4U)
		+ ((PhyCfg[PHY_RDPRE] & 0x1U) << 3U)
		+ ((0x1U) << 2U)
		+ ((PhyCfg[PHY_BL] & 0x3U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x184U, PhyCfg[PHY_R061]);

	Val = ((PhyCfg[PHY_WRLEV] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_WLS] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_WL0] & 0x7U) << 3U)
		+ ((PhyCfg[PHY_RL] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x188U, PhyCfg[PHY_R062]);

	Val = ((PhyCfg[PHY_DBIWR] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_DBIRD] & 0x1U) << 6U)
		+ ((0x6U & 0x7U) << 3U)
		+ ((PhyCfg[PHY_RSVD] & 0x1U) << 2U)
		+ ((PhyCfg[PHY_WRPST] & 0x1U) << 1U)
		+ ((0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x18CU, PhyCfg[PHY_R063]);

	Val = ((PhyCfg[PHY_RSVD_15_13] & 0x7U) << 13U)
		+ ((PhyCfg[PHY_WRP] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_RDP] & 0x1U) << 11U)
		+ ((PhyCfg[PHY_RPTM] & 0x1U) << 10U)
		+ ((PhyCfg[PHY_SRA] & 0x1U) << 9U)
		+ ((PhyCfg[PHY_CS2CMDL] & 0x7U) << 6U)
		+ ((PhyCfg[PHY_RSVD] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_IVM] & 0x1U) << 4U)
		+ ((PhyCfg[PHY_TCRM] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_TCRR] & 0x1U) << 2U)
		+ ((PhyCfg[PHY_MPDM] & 0x1U) << 1U)
		+ ((PhyCfg[PHY_RSVD_0] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x190U, PhyCfg[PHY_R064]);

	Val = ((PhyCfg[PHY_RSVD] & 0x7U) << 13U)
		+ ((PhyCfg[PHY_RDBI0] & 0x1U) << 12U)
		+ ((PhyCfg[PHY_WDBI0] & 0x1U) << 11U)
		+ ((PhyCfg[PHY_DM] & 0x1U) << 10U)
		+ ((PhyCfg[PHY_CAPPE] & 0x1U) << 9U)
		+ ((PhyCfg[PHY_RTTPARK] & 0x7U) << 6U)
		+ ((PhyCfg[PHY_ODTIBPD] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_CAPES] & 0x1U) << 4U)
		+ ((PhyCfg[PHY_CRCEC] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_CAPM] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x194U, PhyCfg[PHY_R065]);

	Val = ((PhyCfg[PHY_TCCDL] & 0x7U) << 10U)
		+ ((PhyCfg[PHY_RSVD_9_8] & 0x3U) << 8U)
		+ ((PhyCfg[PHY_VDDQTEN] & 0x1U) << 7U)
		+ ((0x1U) << 6U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x198U, PhyCfg[PHY_R066]);

	Val = ((PhyCfg[PHY_RSVD_7] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_CAODT] & 0x7U) << 4U)
		+ ((PhyCfg[PHY_RSVD_3] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_DQODT] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1ACU, Val);

	Val = ((PhyCfg[PHY_VREFCA_RANGE] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_VREFCA] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1B0U, Val);

	Val = ((PhyCfg[PHY_FSPOP] & 0x1U) << 7U)
		+ ((PhyCfg[PHY_FSPWR] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_DMD] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_RRO] & 0x1U) << 4U)
		+ ((0x1U) << 3U)
		+ ((PhyCfg[PHY_VRO] & 0x1U) << 2U)
		+ ((PhyCfg[PHY_RPT] & 0x1U) << 1U)
		+ ((PhyCfg[PHY_CBT] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1B4U, Val);

	Val = ((PhyCfg[PHY_VREFDQ_RANGE] & 0x1U) << 6U)
		+ ((PhyCfg[PHY_VREFDQ] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1B8U, Val);

	Val = ((PhyCfg[PHY_ODTD_CA] & 0x1U) << 5U)
		+ ((PhyCfg[PHY_ODTE_CS] & 0x1U) << 4U)
		+ ((PhyCfg[PHY_ODTE_CK] & 0x1U) << 3U)
		+ ((PhyCfg[PHY_CODT] & 0x7U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x1D8U, Val);

	Val = ((0x8 & 0xfU) << 28U)
		+ ((PhyCfg[PHY_DTDRS] & 0x3U) << 24U)
		+ ((PhyCfg[PHY_DTEXG] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_DTEXD] & 0x1U) << 22U)
		+ ((PhyCfg[PHY_DTDSTP] & 0x1U) << 21U)
		+ ((PhyCfg[PHY_DTDEN] & 0x1U) << 20U)
		+ ((PhyCfg[PHY_DTDBS] & 0xFU) << 16U)
		+ ((PhyCfg[PHY_DTRDBITR] & 0x3U) << 14U)
		+ ((PhyCfg[PHY_DTWBDDM] & 0x1U) << 12U)
		+ ((0x1 & 0xfU) << 8U)
		+ ((0x1U) << 7U)
		+ ((0x1U) << 6U)
		+ ((0x7U & 0xfU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x200U, Val);

	Val = ((PhyCfg[PHY_RANKEN] & 0x3U) << 16U)
		+ ((PhyCfg[PHY_DTRANK] & 0x3U) << 12U)
		+ ((0x2U & 0x7U) << 8U)
		+ ((0x3 & 0x7U) << 4U)
		+ ((0x1U) << 2U)
		+ ((0x1U) << 1U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x204U, Val);

	Val = ((0x14U & 0x1fU) << 16U)
		+ ((0x10U & 0x1fU) << 8U)
		+ ((PhyCfg[PHY_CA1BYTE1] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_CA1BYTE0] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x240U, Val);

	Val = ((PhyCfg[PHY_DFTDLY] & 0xFU) << 28U)
		+ ((PhyCfg[PHY_DFTZQUP] & 0x1U) << 27U)
		+ ((PhyCfg[PHY_DFTDDLUP] & 0x1U) << 26U)
		+ ((PhyCfg[PHY_DFTRDSPC] & 0x3U) << 20U)
		+ ((0x8U & 0xfU) << 16U)
		+ ((0x8U & 0xfU) << 12U)
		+ ((PhyCfg[PHY_RESERVED_11_8] & 0xFU) << 8U)
		+ ((PhyCfg[PHY_DFTGPULSE] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_DFTUPMODE] & 0x3U) << 2U)
		+ ((PhyCfg[PHY_DFTDTMODE] & 0x1U) << 1U)
		+ ((PhyCfg[PHY_DFTDTEN] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x250U, Val);

	Val = ((PhyCfg[PHY_SEED] & 0xFFFFFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x414U, Val);

	Val = ((PhyCfg[PHY_RESERVED_31_16] & 0xFFFFU) << 16U)
		+ ((PhyCfg[PHY_ODTOEMODE] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x4F4U, Val);

	Val = ((0x1U) << 29U)
		+ ((0x1U) << 28U)
		+ ((PhyCfg[PHY_RSTODT] & 0x1U) << 26U)
		+ ((PhyCfg[PHY_CKDCC] & 0xFU) << 6U)
		+ ((0x2U & 0x3U) << 4U)
		+ ((0x2U & 0x3U) << 2U)
		+ ((PhyCfg[PHY_ACRANKCLKSEL] & 0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x500U, Val);

	Val = ((PhyCfg[PHY_CLKGENCLKGATE] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_ACOECLKGATE0] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_ACPDRCLKGATE0] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_ACTECLKGATE0] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_CKNCLKGATE0] & 0x3U) << 26U)
		+ ((PhyCfg[PHY_CKCLKGATE0] & 0x3U) << 24U)
		+ ((PhyCfg[PHY_ACCLKGATE0] & 0xFFFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x508U, Val);

	Val = ((PhyCfg[PHY_PAROEMODE] & 0x3U) << 30U)
		+ ((PhyCfg[PHY_BGOEMODE] & 0xFU) << 26U)
		+ ((PhyCfg[PHY_BAOEMODE] & 0xFU) << 22U)
		+ ((PhyCfg[PHY_A17OEMODE] & 0x3U) << 20U)
		+ ((PhyCfg[PHY_A16OEMODE] & 0x3U) << 18U)
		+ ((PhyCfg[PHY_ACTOEMODE] & 0x3U) << 16U)
		+ ((PhyCfg[PHY_CKOEMODE] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x50CU, Val);

	Val = ((PhyCfg[PHY_LBCLKGATE] & 0x1U) << 31U)
		+ ((PhyCfg[PHY_ACOECLKGATE1] & 0x1U) << 30U)
		+ ((PhyCfg[PHY_ACPDRCLKGATE1] & 0x1U) << 29U)
		+ ((PhyCfg[PHY_ACTECLKGATE1] & 0x1U) << 28U)
		+ ((PhyCfg[PHY_CKNCLKGATE1] & 0x3U) << 26U)
		+ ((PhyCfg[PHY_CKCLKGATE1] & 0x3U) << 24U)
		+ ((PhyCfg[PHY_ACCLKGATE1] & 0xFFFFFFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x510U, Val);

	Val = ((PhyCfg[PHY_ACREFIOM] & 0x7U) << 29U)
		+ ((PhyCfg[PHY_ACREFPEN] & 0x1U) << 28U)
		+ ((0x1U) << 25U)
		+ ((0x1U) << 24U)
		+ ((PhyCfg[PHY_ACREFESELRANGE] & 0x1U) << 23U)
		+ ((PhyCfg[PHY_ACREFESEL] & 0x7FU) << 16U)
		+ ((0x1U) << 15U)
		+ ((PhyCfg[PHY_ACREFSSEL] & 0x7FU) << 8U)
		+ ((0x1U) << 7U)
		+ ((PhyCfg[PHY_ACVREFISEL] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x520U, Val);

	Val = ((0x7U) << 29U)
		+ ((0x1U) << 28U)
		+ ((PhyCfg[PHY_PDAEN] & 0x1U) << 27U)
		+ ((0x4U & 0xfU) << 22U)
		+ ((PhyCfg[PHY_DVSS] & 0xFU) << 18U)
		+ ((0x32U & 0x3fU) << 12U)
		+ ((PhyCfg[PHY_DVMIN] & 0x3FU) << 6U)
		+ ((PhyCfg[PHY_DVINIT] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x528U, Val);

	Val = ((PhyCfg[PHY_HVSS] & 0xFU) << 28U)
		+ ((0x7FU) << 20U)
		+ ((PhyCfg[PHY_HVMIN] & 0x7FU) << 12U)
		+ ((PhyCfg[PHY_SHRNK] & 0x3U) << 9U)
		+ ((0x1U) << 8U)
		+ ((0x7U) << 5U)
		+ ((0x1U) << 1U)
		+ ((0x1U) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x52CU, Val);

	Val = ((PhyCfg[PHY_PARBD] & 0x3FU) << 24U)
		+ ((PhyCfg[PHY_A16BD] & 0x3FU) << 16U)
		+ ((PhyCfg[PHY_A17BD] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_ACTBD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x544U, Val);

	Val = ((PhyCfg[PHY_BG1BD] & 0x3FU) << 24U)
		+ ((PhyCfg[PHY_BG0BD] & 0x3FU) << 16U)
		+ ((PhyCfg[PHY_BA1BD] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_BA0BD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x548U, Val);

	Val = ((PhyCfg[PHY_A03BD] & 0x3FU) << 24U)
		+ ((PhyCfg[PHY_A02BD] & 0x3FU) << 16U)
		+ ((PhyCfg[PHY_A01BD] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_A00BD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x558U, Val);

	Val = ((PhyCfg[PHY_A07BD] & 0x3FU) << 24U)
		+ ((PhyCfg[PHY_A06BD] & 0x3FU) << 16U)
		+ ((PhyCfg[PHY_A05BD] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_A04BD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x55CU, Val);

	Val = ((PhyCfg[PHY_A11BD] & 0x3FU) << 24U)
		+ ((PhyCfg[PHY_A10BD] & 0x3FU) << 16U)
		+ ((PhyCfg[PHY_A09BD] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_A08BD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x560U, Val);

	Val = ((PhyCfg[PHY_A15BD] & 0x3FU) << 24U)
		+ ((PhyCfg[PHY_A14BD] & 0x3FU) << 16U)
		+ ((PhyCfg[PHY_A13BD] & 0x3FU) << 8U)
		+ ((PhyCfg[PHY_A12BD] & 0x3FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x564U, Val);

	Val = ((PhyCfg[PHY_ZQREFISELRANGE] & 0x1U) << 25U)
		+ ((0x11U & 0x3fU) << 19U)
		+ ((PhyCfg[PHY_PGWAIT_FRQA] & 0x3FU) << 13U)
		+ ((PhyCfg[PHY_ZQREFPEN] & 0x1U) << 12U)
		+ ((0x1U) << 11U)
		+ ((PhyCfg[PHY_ODT_MODE] & 0x3U) << 9U)
		+ ((PhyCfg[PHY_FORCE_ZCAL_VT_UPDATE] & 0x1U) << 8U)
		+ ((0x2U & 0x7U) << 5U)
		+ ((0x1U) << 4U)
		+ ((0x2U & 0x3U) << 2U)
		+ ((PhyCfg[PHY_ZCALT] & 0x1U) << 1U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x680U, Val);

	Val = ((PhyCfg[PHY_ZSEGBYP] & 0x1U) << 27U)
		+ ((PhyCfg[PHY_ZLE_MODE] & 0x3U) << 25U)
		+ ((PhyCfg[PHY_ODT_ADJUST] & 0x7U) << 22U)
		+ ((PhyCfg[PHY_PD_DRV_ADJUST] & 0x7U) << 19U)
		+ ((PhyCfg[PHY_PU_DRV_ADJUST] & 0x7U) << 16U)
		+ ((PhyCfg[PHY_ZPROG_DRAM_ODT] & 0xFU) << 12U)
		+ ((PhyCfg[PHY_ZPROG_HOST_ODT] & 0xFU) << 8U)
		+ ((PhyCfg[PHY_ZPROG_ASYM_DRV_PD] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_ZPROG_ASYM_DRV_PU] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x684U, Val);

	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x694U, 0x01e10210U);

	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x698U, 0x01e10000U);

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		PhyCfg[PHY_PD_DRV_ADJUST] = 0x1U;
	} else {
		PhyCfg[PHY_PD_DRV_ADJUST] = 0U;
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		PhyCfg[PHY_PU_DRV_ADJUST] = 0x1U;
	} else {
		PhyCfg[PHY_PU_DRV_ADJUST] = 0U;
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		PhyCfg[PHY_ZPROG_DRAM_ODT] = 0x7U;
	} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		PhyCfg[PHY_ZPROG_DRAM_ODT] = 0xBU;
	}


	if (PDimmPtr->HostOdt == 40U) {
		PhyCfg[PHY_ZPROG_HOST_ODT] = 0xBU;
	} else if (PDimmPtr->HostOdt == 48U) {
		PhyCfg[PHY_ZPROG_HOST_ODT] = 0x9U;
	} else if (PDimmPtr->HostOdt == 60U) {
		PhyCfg[PHY_ZPROG_HOST_ODT] = 0x7U;
	} else if (PDimmPtr->HostOdt == 80U) {
		PhyCfg[PHY_ZPROG_HOST_ODT] = 0x5U;
	} else if (PDimmPtr->HostOdt == 120U) {
		PhyCfg[PHY_ZPROG_HOST_ODT] = 0x3U;
	} else {
		if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
			if (PDimmPtr->NumRankAddr == 1U) {
				PhyCfg[PHY_ZPROG_HOST_ODT] = 0x9U;
			} else {
				PhyCfg[PHY_ZPROG_HOST_ODT] = 0xBU;
			}
		} else if (PDimmPtr->MemType == SPD_MEMTYPE_DDR3) {
			PhyCfg[PHY_ZPROG_HOST_ODT] = 0x6U;
		} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
			PhyCfg[PHY_ZPROG_HOST_ODT] = 0xBU;
		} else {
			PhyCfg[PHY_ZPROG_HOST_ODT] = 0x3U;
		}
	}

	if (PDimmPtr->HostDrv == 34U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xDU;
	} else if (PDimmPtr->HostDrv == 40U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xBU;
	} else if (PDimmPtr->HostDrv == 48U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x9U;
	} else if (PDimmPtr->HostDrv == 60U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x7U;
	} else if (PDimmPtr->HostDrv == 80U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x5U;
	} else if (PDimmPtr->HostDrv == 120U) {
		PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x3U;
	} else {
		if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
			PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xDU;
		} else if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR3) ||
				(PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3)) {
			PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0xBU;
		} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
			PhyCfg[PHY_ZPROG_ASYM_DRV_PD] = 0x9U;
		}
	}

	PhyCfg[PHY_ZPROG_ASYM_DRV_PU] = PhyCfg[PHY_ZPROG_ASYM_DRV_PD];

	Val = ((PhyCfg[PHY_ZSEGBYP] & 0x1U) << 27U)
		+ ((PhyCfg[PHY_ZLE_MODE] & 0x3U) << 25U)
		+ ((PhyCfg[PHY_ODT_ADJUST] & 0x7U) << 22U)
		+ ((PhyCfg[PHY_PD_DRV_ADJUST] & 0x7U) << 19U)
		+ ((PhyCfg[PHY_PU_DRV_ADJUST] & 0x7U) << 16U)
		+ ((PhyCfg[PHY_ZPROG_DRAM_ODT] & 0xFU) << 12U)
		+ ((PhyCfg[PHY_ZPROG_HOST_ODT] & 0xFU) << 8U)
		+ ((PhyCfg[PHY_ZPROG_ASYM_DRV_PD] & 0xFU) << 4U)
		+ ((PhyCfg[PHY_ZPROG_ASYM_DRV_PU] & 0xFU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x6A4U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x714U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x718U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x814U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x818U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x914U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x918U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xA14U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xA18U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xB14U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xB18U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC14U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xC18U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xD14U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xD18U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xE14U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xE18U, Val);

	Val = ((0x9U & 0x7fU) << 24U)
		+ ((0x9U & 0x7fU) << 16U)
		+ ((PhyCfg[PHY_DXREFISELR1] & 0x7FU) << 8U)
		+ ((PhyCfg[PHY_DXREFISELR0] & 0x7FU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF14U, Val);

	Val = ((0x9U & 0x3fU) << 24U)
		+ ((0x9U & 0x3fU) << 16U)
		+ ((0x2BU & 0x3fU) << 8U)
		+ ((0x2BU & 0x3fU) << 0U);
	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0xF18U, Val);
}

#if !(defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208))
/*****************************************************************************/
/**
 * This function calculates and writes the DDR-PHY registers
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_PhyRegsInit(struct DdrcInitData *DdrDataPtr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 PhyCfg[512U] = XFSBL_PHY_REG_DEFVAL;
	u32 Status;

	Status = XFsbl_PhyCalcCommonRegVal(PDimmPtr, PhyCfg);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR3) {
		Status = XFsbl_PhyCalcDdr3RegVal(PDimmPtr, PhyCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		Status = XFsbl_PhyCalcDdr4RegVal(PDimmPtr, PhyCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		Status = XFsbl_PhyCalcLpddr3RegVal(PDimmPtr, PhyCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		Status = XFsbl_PhyCalcLpddr4RegVal(PDimmPtr, PhyCfg);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}

	XFsbl_PhyRegsWrite(PDimmPtr, PhyCfg);

END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * This function initializes the registers affected by enabling the Read DBI.
 *
 * @param	PDimmPtr is pointer to DDR parameters structure
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_RdbiWrkAround(XFsbl_DimmParams *PDimmPtr)
{
	u32 CalByte[9U]={0U};
	u32 DqRbd[9U][8U];
	u32 Index;
	u32 Index1;

	XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET, DDR_PHY_PGCR1_PUBMODE_MASK,
			DDR_PHY_PGCR1_PUBMODE_SHIFT, 1U);
	XFSBL_PROG_REG(DDRC_SWCTL_OFFSET, DDRC_SWCTL_SW_DONE_MASK,
			DDRC_SWCTL_SW_DONE_SHIFT, 0U);
	XFSBL_PROG_REG(DDRC_DFIUPD0_OFFSET, DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
			DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 1U);

	DqRbd[0U][0U] = ((DDR_PHY_DX0BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[0U][1U] = ((DDR_PHY_DX0BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[0U][2U] = ((DDR_PHY_DX0BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[0U][3U] = ((DDR_PHY_DX0BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[0U][4U] = ((DDR_PHY_DX0BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[0U][5U] = ((DDR_PHY_DX0BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[0U][6U] = ((DDR_PHY_DX0BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[0U][7U] = ((DDR_PHY_DX0BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[1U][0U] = ((DDR_PHY_DX1BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[1U][1U] = ((DDR_PHY_DX1BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[1U][2U] = ((DDR_PHY_DX1BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[1U][3U] = ((DDR_PHY_DX1BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[1U][4U] = ((DDR_PHY_DX1BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[1U][5U] = ((DDR_PHY_DX1BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[1U][6U] = ((DDR_PHY_DX1BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[1U][7U] = ((DDR_PHY_DX1BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[2U][0U] = ((DDR_PHY_DX2BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[2U][1U] = ((DDR_PHY_DX2BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[2U][2U] = ((DDR_PHY_DX2BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[2U][3U] = ((DDR_PHY_DX2BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[2U][4U] = ((DDR_PHY_DX2BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[2U][5U] = ((DDR_PHY_DX2BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[2U][6U] = ((DDR_PHY_DX2BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[2U][7U] = ((DDR_PHY_DX2BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[3U][0U] = ((DDR_PHY_DX3BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[3U][1U] = ((DDR_PHY_DX3BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[3U][2U] = ((DDR_PHY_DX3BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[3U][3U] = ((DDR_PHY_DX3BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[3U][4U] = ((DDR_PHY_DX3BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[3U][5U] = ((DDR_PHY_DX3BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[3U][6U] = ((DDR_PHY_DX3BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[3U][7U] = ((DDR_PHY_DX3BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[4U][0U] = ((DDR_PHY_DX4BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[4U][1U] = ((DDR_PHY_DX4BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[4U][2U] = ((DDR_PHY_DX4BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[4U][3U] = ((DDR_PHY_DX4BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[4U][4U] = ((DDR_PHY_DX4BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[4U][5U] = ((DDR_PHY_DX4BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[4U][6U] = ((DDR_PHY_DX4BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[4U][7U] = ((DDR_PHY_DX4BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[5U][0U] = ((DDR_PHY_DX5BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[5U][1U] = ((DDR_PHY_DX5BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[5U][2U] = ((DDR_PHY_DX5BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[5U][3U] = ((DDR_PHY_DX5BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[5U][4U] = ((DDR_PHY_DX5BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[5U][5U] = ((DDR_PHY_DX5BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[5U][6U] = ((DDR_PHY_DX5BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[5U][7U] = ((DDR_PHY_DX5BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[6U][0U] = ((DDR_PHY_DX6BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[6U][1U] = ((DDR_PHY_DX6BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[6U][2U] = ((DDR_PHY_DX6BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[6U][3U] = ((DDR_PHY_DX6BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[6U][4U] = ((DDR_PHY_DX6BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[6U][5U] = ((DDR_PHY_DX6BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[6U][6U] = ((DDR_PHY_DX6BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[6U][7U] = ((DDR_PHY_DX6BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	DqRbd[7U][0U] = ((DDR_PHY_DX7BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[7U][1U] = ((DDR_PHY_DX7BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[7U][2U] = ((DDR_PHY_DX7BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[7U][3U] = ((DDR_PHY_DX7BDLR3_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	DqRbd[7U][4U] = ((DDR_PHY_DX7BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
	DqRbd[7U][5U] = ((DDR_PHY_DX7BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
	DqRbd[7U][6U] = ((DDR_PHY_DX7BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
	DqRbd[7U][7U] = ((DDR_PHY_DX7BDLR4_OFFSET &
				DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
			DDR_PHY_DXBDLR_DQ3RBD_SHIFT);

	if (PDimmPtr->Ecc) {
		DqRbd[8U][0U] = ((DDR_PHY_DX8BDLR3_OFFSET &
					DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
		DqRbd[8U][1U] = ((DDR_PHY_DX8BDLR3_OFFSET &
					DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
		DqRbd[8U][2U] = ((DDR_PHY_DX8BDLR3_OFFSET &
					DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
		DqRbd[8U][3U] = ((DDR_PHY_DX8BDLR3_OFFSET &
					DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
		DqRbd[8U][4U] = ((DDR_PHY_DX8BDLR4_OFFSET &
					DDR_PHY_DXBDLR_DQ0RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ0RBD_SHIFT);
		DqRbd[8U][5U] = ((DDR_PHY_DX8BDLR4_OFFSET &
					DDR_PHY_DXBDLR_DQ1RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ1RBD_SHIFT);
		DqRbd[8U][6U] = ((DDR_PHY_DX8BDLR4_OFFSET &
					DDR_PHY_DXBDLR_DQ2RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ2RBD_SHIFT);
		DqRbd[8U][7U] = ((DDR_PHY_DX8BDLR4_OFFSET &
					DDR_PHY_DXBDLR_DQ3RBD_MASK) <<
				DDR_PHY_DXBDLR_DQ3RBD_SHIFT);
	}

	for (Index = 0U; Index < (PDimmPtr->Ecc ? 9U : 8U); Index++) {
		CalByte[Index] = 0U;
		for (Index1 = 0U; Index1 < 8U; Index1++) {
			CalByte[Index] = CalByte[Index] + DqRbd[Index][Index1];
		}
		CalByte[Index] = CalByte[Index] / 8U;
	}

	XFSBL_PROG_REG(DDR_PHY_DX0BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[0U]);
	XFSBL_PROG_REG(DDR_PHY_DX1BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[1U]);
	XFSBL_PROG_REG(DDR_PHY_DX2BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[2U]);
	XFSBL_PROG_REG(DDR_PHY_DX3BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[3U]);
	XFSBL_PROG_REG(DDR_PHY_DX4BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[4U]);
	XFSBL_PROG_REG(DDR_PHY_DX5BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[5U]);
	XFSBL_PROG_REG(DDR_PHY_DX6BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[6U]);
	XFSBL_PROG_REG(DDR_PHY_DX7BDLR5_OFFSET,
			DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[7U]);

	if (PDimmPtr->Ecc) {
		XFSBL_PROG_REG(DDR_PHY_DX8BDLR5_OFFSET,
				DDR_PHY_DXBDLR5_DMRBD_MASK, 0U, CalByte[8U]);
	}

	XFSBL_PROG_REG(DDRC_DFIUPD0_OFFSET,
			DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
			DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 0U);
	XFSBL_PROG_REG(DDRC_SWCTL_OFFSET,
			DDRC_SWCTL_SW_DONE_MASK,
			DDRC_SWCTL_SW_DONE_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
			DDR_PHY_PGCR1_PUBMODE_MASK,
			DDR_PHY_PGCR1_PUBMODE_SHIFT, 0U);
}

/*****************************************************************************/
/**
 * This function initializes the registers affected by enabling the Read DBI.
 *
 * @param	PDimmPtr is pointer to DDR parameters structure
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_DdrPllBypass(u32 En)
{
	XFSBL_PROG_REG(DDR_QOS_CTRL_DDRPHY_CTRL_OFFSET,
			DDR_QOS_CTRL_DDRPHY_CTRL_BYP_MODE_MASK,
			DDR_QOS_CTRL_DDRPHY_CTRL_BYP_MODE_SHIFT, En);

	XFSBL_PROG_REG(DDR_PHY_PLLCR0_OFFSET, DDR_PHY_PLLCR0_PLLBYP_MASK,
			DDR_PHY_PLLCR0_PLLBYP_SHIFT, En);
	XFSBL_PROG_REG(DDR_PHY_DX8SL0PLLCR0_OFFSET,
			DDR_PHY_DX8SL0PLLCR0_PLLBYP_MASK,
			DDR_PHY_DX8SL0PLLCR0_PLLBYP_SHIFT, En);
	XFSBL_PROG_REG(DDR_PHY_DX8SL1PLLCR0_OFFSET,
			DDR_PHY_DX8SL1PLLCR0_PLLBYP_MASK,
			DDR_PHY_DX8SL1PLLCR0_PLLBYP_SHIFT, En);
	XFSBL_PROG_REG(DDR_PHY_DX8SL2PLLCR0_OFFSET,
			DDR_PHY_DX8SL2PLLCR0_PLLBYP_MASK,
			DDR_PHY_DX8SL2PLLCR0_PLLBYP_SHIFT, En);
	XFSBL_PROG_REG(DDR_PHY_DX8SL3PLLCR0_OFFSET,
			DDR_PHY_DX8SL3PLLCR0_PLLBYP_MASK,
			DDR_PHY_DX8SL3PLLCR0_PLLBYP_SHIFT, En);
	XFSBL_PROG_REG(DDR_PHY_DX8SL4PLLCR0_OFFSET,
			DDR_PHY_DX8SL4PLLCR0_PLLBYP_MASK,
			DDR_PHY_DX8SL4PLLCR0_PLLBYP_SHIFT, En);

	XFSBL_PROG_REG(DDR_PHY_PLLCR0_OFFSET, DDR_PHY_PLLCR0_PLLRST_MASK,
			DDR_PHY_PLLCR0_PLLRST_SHIFT, 1U);

	XFSBL_PROG_REG(DDR_PHY_DX8SL0PLLCR0_OFFSET,
			DDR_PHY_DX8SL0PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL0PLLCR0_PLLRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL1PLLCR0_OFFSET,
			DDR_PHY_DX8SL1PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL1PLLCR0_PLLRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL2PLLCR0_OFFSET,
			DDR_PHY_DX8SL2PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL2PLLCR0_PLLRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL3PLLCR0_OFFSET,
			DDR_PHY_DX8SL3PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL3PLLCR0_PLLRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL4PLLCR0_OFFSET,
			DDR_PHY_DX8SL4PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL4PLLCR0_PLLRST_SHIFT, 1U);

	XFSBL_PROG_REG(DDR_QOS_CTRL_DDR_CLK_CTRL_OFFSET,
			DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_MASK,
			DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_PLLCR0_OFFSET, DDR_PHY_PLLCR0_PLLRST_MASK,
			DDR_PHY_PLLCR0_PLLRST_SHIFT, 0U);

	XFSBL_PROG_REG(DDR_PHY_DX8SL0PLLCR0_OFFSET,
			DDR_PHY_DX8SL0PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL0PLLCR0_PLLRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL1PLLCR0_OFFSET,
			DDR_PHY_DX8SL1PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL1PLLCR0_PLLRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL2PLLCR0_OFFSET,
			DDR_PHY_DX8SL2PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL2PLLCR0_PLLRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL3PLLCR0_OFFSET,
			DDR_PHY_DX8SL3PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL3PLLCR0_PLLRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL4PLLCR0_OFFSET,
			DDR_PHY_DX8SL4PLLCR0_PLLRST_MASK,
			DDR_PHY_DX8SL4PLLCR0_PLLRST_SHIFT, 0U);

	XFSBL_PROG_REG(DDR_QOS_CTRL_DDR_CLK_CTRL_OFFSET,
			DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_MASK,
			DDR_QOS_CTRL_DDR_CLK_CTRL_CLKACT_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
			DDR_PHY_PGCR1_PHYHRST_MASK,
			DDR_PHY_PGCR1_PHYHRST_SHIFT, 0U);

	XFSBL_PROG_REG(DDR_PHY_DX8SL0OSC_OFFSET,
			DDR_PHY_DX8SL0OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL0OSC_PHYHRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL1OSC_OFFSET,
			DDR_PHY_DX8SL1OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL1OSC_PHYHRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL2OSC_OFFSET,
			DDR_PHY_DX8SL2OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL2OSC_PHYHRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL3OSC_OFFSET,
			DDR_PHY_DX8SL3OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL3OSC_PHYHRST_SHIFT, 0U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL4OSC_OFFSET,
			DDR_PHY_DX8SL4OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL4OSC_PHYHRST_SHIFT, 0U);

	XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET, DDR_PHY_PGCR1_PHYHRST_MASK,
			DDR_PHY_PGCR1_PHYHRST_SHIFT, 1U);

	XFSBL_PROG_REG(DDR_PHY_DX8SL0OSC_OFFSET,
			DDR_PHY_DX8SL0OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL0OSC_PHYHRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL1OSC_OFFSET,
			DDR_PHY_DX8SL1OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL1OSC_PHYHRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL2OSC_OFFSET,
			DDR_PHY_DX8SL2OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL2OSC_PHYHRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL3OSC_OFFSET,
			DDR_PHY_DX8SL3OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL3OSC_PHYHRST_SHIFT, 1U);
	XFSBL_PROG_REG(DDR_PHY_DX8SL4OSC_OFFSET,
			DDR_PHY_DX8SL4OSC_PHYHRST_MASK,
			DDR_PHY_DX8SL4OSC_PHYHRST_SHIFT, 1U);

	return 0U;
}

/*****************************************************************************/
/**
 * This function enables/disables the DFI Init in DDR
 *
 * @param	Value is the value to be set in DFI Misc Control Register
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_CfgDfiInitComplete(u32 Value)
{
	u32 RegVal;

	RegVal = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x1B0U);
	RegVal &= ~(0x1U << 0U);
	RegVal |= (Value << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1B0U, RegVal);
}

/*****************************************************************************/
/**
 * This function enables/disables the Software Intervention in DDR
 *
 * @param	Value is the value to be set in MR Control Register
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_CfgSwInitInt(u32 Value)
{
	u32 RegVal;

	RegVal = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x10U);
	RegVal &= ~(0x1U << 3U);
	RegVal |= (Value << 3U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x10U, RegVal);
}

/*****************************************************************************/
/**
 * This function sets the Mode Register with specific rank of DDR
 *
 * @param	Addr is the Address of the MR Register
 * @param	Rank is the rank of the DDR
 * @param	Mr07 is the value to be set in MR Register
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_MrsFunc(u32 Addr, u32 Rank, u32 Mr07)
{
	u32 Val;
	u32 RegVal;

	Val =  ((Addr & 0x3FFFFU) << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x14U, Val);

	RegVal = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x10U);
	RegVal |= (0x1U << 31U);

	RegVal &= ~(0x1U << 31U);
	RegVal |= (0x1U << 31U);

	if (Mr07 != 0U) {
		RegVal &= ~(0xFU << 12U);
		RegVal |= (Mr07 << 12U);
	}

	RegVal &= ~(0x3U << 4U);
	RegVal |= (Rank << 4U);

	RegVal &= ~(0x1U << 0U);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x10U, RegVal);

	RegVal = Xil_In32(DDRC_MRSTAT_OFFSET);
	while ((RegVal & 0x1U) != 0x0U)
		RegVal = Xil_In32(DDRC_MRSTAT_OFFSET);

	for (u32 i = 0U; i < 10U; i++) {
		RegVal = Xil_In32(DDRC_MRSTAT_OFFSET);
	}
}

/*****************************************************************************/
/**
 * This function performs the DDR/PHY training sequence to initialize the DDR
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	Returns XFSBL_SUCCESS or XFSBL_FAILURE
 *
 *****************************************************************************/
static u32 XFsbl_DdrcPhyTraining(struct DdrcInitData *DdrDataPtr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 ActiveRanks;
	u32 PollVal = 0U;
	u32 CurTRefPrd;
	u32 RegVal = 0U;
	u32 PllRetry = 100U;
	u32 PllLocked = 0U;
	u32 Puad;
	u32 Status = XFSBL_FAILURE;

	ActiveRanks = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x0000U);

	while ((PllRetry > 0U) && (!PllLocked)) {
		if ((PllRetry % 10U) == 0U) {
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x00040010U);
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x00040011U);
		}

		while ((Xil_In32(DDR_PHY_PGSR0_OFFSET) & 0x1U) != 1U);

		PllLocked = (Xil_In32(DDR_PHY_PGSR0_OFFSET) & 0x80000000U) >> 31U;
		PllLocked &= (Xil_In32(DDR_PHY_DX0GSR0_OFFSET) & 0x10000U) >> 16U;
		PllLocked &= (Xil_In32(DDR_PHY_DX2GSR0_OFFSET) & 0x10000U) >> 16U;

		if (PDimmPtr->BusWidth == 64U) {
			PllLocked &= (Xil_In32(DDR_PHY_DX4GSR0_OFFSET) & 0x10000U) >> 16U;
			PllLocked &= (Xil_In32(DDR_PHY_DX6GSR0_OFFSET) & 0x10000U) >> 16U;
		}

		if (PDimmPtr->Ecc) {
			PllLocked &= (Xil_In32(DDR_PHY_DX8GSR0_OFFSET) & 0x10000U) >> 16U;
		}

		PllRetry--;
	}

	Xil_Out32(DDR_PHY_GPR1_OFFSET, Xil_In32(DDR_PHY_GPR1_OFFSET) | (PllRetry << 16U));

	if (PllLocked == 0U) {
		XFsbl_Printf(DEBUG_INFO,"DDR-PHY Training failed\n\r");
		/* Do nothing as Status is initialized to XFSBL_FAILURE */
		goto END;
	}

	RegVal = ((PDimmPtr->RDimm ? 0x1U : 0x0U) << 19U) | (0x1U << 18U) | 0x73U;

	/* Now PLL lock is done, resume with other training */
	RegVal = RegVal & ~DDR_PHY_PIR_PLLINIT_MASK;

	/* End of pll lock retry */

	Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x4U, RegVal);

	if (PDimmPtr->PllByp) {
		XFsbl_DdrPllBypass(1U);
	}

	XFSBL_POLL(DDR_PHY_PGSR0_OFFSET, 0xFU, 0xFU);

	XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_INIT_MASK,
			DDR_PHY_PIR_INIT_SHIFT, 1U);
	XFSBL_POLL(DDR_PHY_PGSR0_OFFSET, 0xFFU, 0x1FU);

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		XFsbl_CfgSwInitInt(1U);
		XFsbl_CfgDfiInitComplete(1U);
		XFsbl_MrsFunc(0x331U, ActiveRanks, 0U);
		XFsbl_MrsFunc(0xB36U, ActiveRanks, 0U);
		if ((PDimmPtr->Zc1656) || (PDimmPtr->HasEccComp)) {
			XFsbl_MrsFunc(0xC4DU, ActiveRanks, 0U);
			if (PDimmPtr->Lp4NoOdt) {
				XFsbl_MrsFunc(0xE6EU, ActiveRanks, 0U);
			} else {
				XFsbl_MrsFunc(0xE1EU, ActiveRanks, 0U);
			}
			XFsbl_MrsFunc(0x1606U, ActiveRanks, 0U);
		} else {
			XFsbl_MrsFunc(0xC21U, ActiveRanks, 0U);

			if (PDimmPtr->Lp4NoOdt) {
				XFsbl_MrsFunc(0xE6FU, ActiveRanks, 0U);
			} else {
				XFsbl_MrsFunc(0xE19U, ActiveRanks, 0U);
			}
			XFsbl_MrsFunc(0x1616U, ActiveRanks, 0U);
		}
		XFsbl_CfgSwInitInt(0U);
	}

	if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR4) && PDimmPtr->RDimm) {
		XFsbl_CfgSwInitInt(1U);
		XFsbl_CfgDfiInitComplete(1U);

		if (PDimmPtr->Parity)
			XFsbl_MrsFunc(0x88U, 1U, 7U);
		if (PDimmPtr->AddrMirror)
			XFsbl_MrsFunc(0xD8U, 1U, 7U);
		if (PDimmPtr->DisOpInv)
			XFsbl_MrsFunc(0x01U, 1U, 7U);
		if (PDimmPtr->SpeedBin == 1866U)
			XFsbl_MrsFunc(0xA1U, 1U, 7U);
		if (PDimmPtr->SpeedBin == 2133U)
			XFsbl_MrsFunc(0xA2U, 1U, 7U);
		if (PDimmPtr->SpeedBin == 2400U)
			XFsbl_MrsFunc(0xA3U, 1U, 7U);
		if (PDimmPtr->SpeedBin == 2666U)
			XFsbl_MrsFunc(0xA4U, 1U, 7U);

		XFsbl_CfgSwInitInt(0U);
	}

	RegVal = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x1B0U);
	RegVal |=  (0x1U << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x1B0U, RegVal);

	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x320U, 0x1U);

	XFSBL_POLL(DDRC_STAT_OFFSET, 0xFU, 1U);

	if (PDimmPtr->Slowboot == 1U) {
		XFSBL_PROG_REG(DDRC_DFIMISC_OFFSET,
				DDRC_DFIMISC_DFI_INIT_COMPLETE_EN_MASK,
				DDRC_DFIMISC_DFI_INIT_COMPLETE_EN_SHIFT, 0U);

		XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
				DDR_PHY_PGCR1_PUBMODE_MASK,
				DDR_PHY_PGCR1_PUBMODE_SHIFT, 0x1U);
		XFSBL_PROG_REG(DDR_PHY_PGCR6_OFFSET,
				DDR_PHY_PGCR6_INHVT_MASK,
				DDR_PHY_PGCR6_INHVT_SHIFT, 0x1U);
		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET,
				DDR_PHY_PIR_DCALPSE_MASK,
				DDR_PHY_PIR_DCALPSE_SHIFT, 0x1U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR1_OFFSET,
				DDR_PHY_SCHCR1_ALLRANK_MASK,
				DDR_PHY_SCHCR1_ALLRANK_SHIFT, 0x1U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR0_OFFSET,
				DDR_PHY_SCHCR0_CMD_MASK,
				DDR_PHY_SCHCR0_CMD_SHIFT, 0x7U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR0_OFFSET,
				DDR_PHY_SCHCR0_SP_CMD_MASK,
				DDR_PHY_SCHCR0_SP_CMD_SHIFT, 0x2U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR0_OFFSET,
				DDR_PHY_SCHCR0_SCHTRIG_MASK,
				DDR_PHY_SCHCR0_SCHTRIG_SHIFT, 0x1U);

		Xil_Out32(DDR_PHY_PLLCR0_OFFSET, Xil_In32(DDR_PHY_PLLCR0_OFFSET));
		Xil_Out32(DDR_PHY_DX8SL0PLLCR0_OFFSET, Xil_In32(DDR_PHY_DX8SL0PLLCR0_OFFSET));
		Xil_Out32(DDR_PHY_DX8SL1PLLCR0_OFFSET, Xil_In32(DDR_PHY_DX8SL1PLLCR0_OFFSET));
		Xil_Out32(DDR_PHY_DX8SL2PLLCR0_OFFSET, Xil_In32(DDR_PHY_DX8SL2PLLCR0_OFFSET));
		Xil_Out32(DDR_PHY_DX8SL3PLLCR0_OFFSET, Xil_In32(DDR_PHY_DX8SL3PLLCR0_OFFSET));
		Xil_Out32(DDR_PHY_DX8SL4PLLCR0_OFFSET, Xil_In32(DDR_PHY_DX8SL4PLLCR0_OFFSET));
		Xil_Out32(DDR_PHY_DX8SLBPLLCR0_OFFSET, Xil_In32(DDR_PHY_DX8SLBPLLCR0_OFFSET));

		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_DCALPSE_MASK,
				DDR_PHY_PIR_DCALPSE_SHIFT, 0U);

		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_CTLDINIT_MASK,
				DDR_PHY_PIR_CTLDINIT_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_PHYRST_MASK,
				DDR_PHY_PIR_PHYRST_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_DCAL_MASK,
				DDR_PHY_PIR_DCAL_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_PLLINIT_MASK,
				DDR_PHY_PIR_PLLINIT_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_INIT_MASK,
				DDR_PHY_PIR_INIT_SHIFT, 1U);

		XFSBL_POLL(DDR_PHY_PGSR0_OFFSET, 0xFU, 0xFU);

		XFSBL_PROG_REG(DDR_PHY_PIR_OFFSET, DDR_PHY_PIR_INIT_MASK,
				DDR_PHY_PIR_INIT_SHIFT, 1U);
		XFSBL_POLL(DDR_PHY_PGSR0_OFFSET, 0xFFU, 0x1FU);

		XFSBL_PROG_REG(DDR_PHY_PGCR6_OFFSET, DDR_PHY_PGCR6_INHVT_MASK,
				DDR_PHY_PGCR6_INHVT_SHIFT, 0x0U);

		XFSBL_PROG_REG(DDR_PHY_SCHCR1_OFFSET,
				DDR_PHY_SCHCR1_ALLRANK_MASK,
				DDR_PHY_SCHCR1_ALLRANK_SHIFT, 0x1U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR0_OFFSET,
				DDR_PHY_SCHCR0_CMD_MASK,
				DDR_PHY_SCHCR0_CMD_SHIFT, 0x7U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR0_OFFSET,
				DDR_PHY_SCHCR0_SP_CMD_MASK,
				DDR_PHY_SCHCR0_SP_CMD_SHIFT, 0x3U);
		XFSBL_PROG_REG(DDR_PHY_SCHCR0_OFFSET,
				DDR_PHY_SCHCR0_SCHTRIG_MASK,
				DDR_PHY_SCHCR0_SCHTRIG_SHIFT, 0x1U);

	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		XFsbl_CfgSwInitInt(1U);
		XFsbl_CfgDfiInitComplete(1U);
		XFsbl_MrsFunc(0xB02U, ActiveRanks, 0U);
		XFsbl_CfgSwInitInt(0U);
	}

	if ((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) && (PDimmPtr->Lp4catrain == 1U)) {
		XFSBL_PROG_REG(DDRC_SWCTL_OFFSET, DDRC_SWCTL_SW_DONE_MASK,
				DDRC_SWCTL_SW_DONE_SHIFT, 1U);
		XFSBL_PROG_REG(DDRC_DFIUPD0_OFFSET,
				DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
				DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 1U);
		XFSBL_PROG_REG(DDRC_RFSHCTL3_OFFSET,
				DDRC_RFSHCTL3_DIS_AUTO_REFRESH_MASK,
				DDRC_RFSHCTL3_DIS_AUTO_REFRESH_SHIFT, 1U);
		XFSBL_PROG_REG(DDRC_ZQCTL0_OFFSET,
				DDRC_ZQCTL0_DIS_AUTO_ZQ_MASK,
				DDRC_ZQCTL0_DIS_AUTO_ZQ_SHIFT, 1U);
		XFSBL_PROG_REG(DDRC_RFSHCTL3_OFFSET,
				DDRC_RFSHCTL3_DIS_AUTO_REFRESH_MASK,
				DDRC_RFSHCTL3_DIS_AUTO_REFRESH_SHIFT, 0U);
		XFSBL_PROG_REG(DDRC_ZQCTL0_OFFSET,
				DDRC_ZQCTL0_DIS_AUTO_ZQ_MASK,
				DDRC_ZQCTL0_DIS_AUTO_ZQ_SHIFT, 0U);
	}

	XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET, DDR_PHY_PGCR1_PUBMODE_MASK,
			DDR_PHY_PGCR1_PUBMODE_SHIFT, 1U);

	if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR3) ||
			(PDimmPtr->MemType == SPD_MEMTYPE_DDR4)) {
		if (PDimmPtr->DeskewTrn == 0U)
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004CE01U);
		else
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004FE01U);
	} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		if (PDimmPtr->Ddp == 1U) {
			XFSBL_PROG_REG(DDR_PHY_RANKIDR_OFFSET,
					DDR_PHY_RANKIDR_RANKWID_MASK,
					DDR_PHY_RANKIDR_RANKWID_SHIFT, 1U);
			XFSBL_PROG_REG(DDR_PHY_ODTCR_OFFSET,
					DDR_PHY_ODTCR_WRODT_MASK,
					DDR_PHY_ODTCR_WRODT_SHIFT, 1U);
			XFSBL_PROG_REG(DDR_PHY_RANKIDR_OFFSET,
					DDR_PHY_RANKIDR_RANKWID_MASK,
					DDR_PHY_RANKIDR_RANKWID_SHIFT, 0U);
		}

		if (PDimmPtr->Zc1650)
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004FE01U);
		else {
			if (PDimmPtr->GateExt)
				if (PDimmPtr->DeskewTrn == 0U)
					Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004CA05U);
				else
					Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004FA05U);
			else
				if (PDimmPtr->DeskewTrn == 0U)
					Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004CE05U);
				else
					Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0004FE05U);
		}
	} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		if (PDimmPtr->DeskewTrn == 0U)
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0014CE01U);
		else
			Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0014FE01U);
	}

	if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR3) ||
			(PDimmPtr->MemType == SPD_MEMTYPE_DDR4)) {
		PollVal = 0x80000CFFU;
	} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		if (PDimmPtr->Zc1650) {
			PollVal = 0x80000FFFU;
		} else {
			if (PDimmPtr->GateExt)
				PollVal = 0x80001CBFU;
			else
				PollVal = 0x80001CFFU;
		}
	} else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
		PollVal = 0x80008CFFU;
	}
	if (PDimmPtr->DeskewTrn != 0U) {
		PollVal |= 0x300U;
	}

	if (PDimmPtr->MemType != SPD_MEMTYPE_LPDDR4) {
		RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);
		while (RegVal != PollVal) {
			RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);
		}
	} else {
		while (RegVal != 0x8000007EU)
			RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);

		RegVal = Xil_In32(XFSBL_DDRPHY_BASE_ADDR + 0x200U);
		RegVal &= ~(0xFU << 28U);
		Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x200U, RegVal);

		RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);
		while (RegVal != PollVal)
			RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);


		RegVal &= ~(0xFU << 28U);
		RegVal |= (0x8U << 28U);
		Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x200U, RegVal);
	}

	if (PDimmPtr->MemType != SPD_MEMTYPE_LPDDR4) {
		RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);
	}

	RegVal = ((Xil_In32(DDR_PHY_PGSR0_OFFSET) & 0x1FFF0000U) >> 18U);

	if ((PDimmPtr->Vref == 1U) && (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4 ||
				PDimmPtr->MemType == SPD_MEMTYPE_DDR4)) {


		RegVal = Xil_In32(XFSBL_DDRPHY_BASE_ADDR + 0x200U);
		RegVal &= ~(0xFU << 28U);
		if (((Xil_In32(XFSBL_DDRPHY_BASE_ADDR + 0x528U) >> 27U) & 0x1U) == 0x1U) {
			RegVal |= (0x1U << 28U);
		}
		Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x200U, RegVal);

		CurTRefPrd = (Xil_In32(DDR_PHY_PGCR2_OFFSET) &
				DDR_PHY_PGCR2_TREFPRD_MASK) >>
			DDR_PHY_PGCR2_TREFPRD_SHIFT;
		XFSBL_PROG_REG(DDR_PHY_PGCR2_OFFSET,
				DDR_PHY_PGCR2_TREFPRD_MASK,
				DDR_PHY_PGCR2_TREFPRD_SHIFT, CurTRefPrd - 400U);

		XFSBL_PROG_REG(DDR_PHY_PGCR3_OFFSET, DDR_PHY_PGCR3_RDMODE_MASK,
				DDR_PHY_PGCR3_RDMODE_SHIFT, 3U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL0DXCTL2_OFFSET,
				DDR_PHY_DX8SL0DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL0DXCTL2_RDMODE_SHIFT, 3U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL1DXCTL2_OFFSET,
				DDR_PHY_DX8SL1DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL1DXCTL2_RDMODE_SHIFT, 3U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL2DXCTL2_OFFSET,
				DDR_PHY_DX8SL2DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL2DXCTL2_RDMODE_SHIFT, 3U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL3DXCTL2_OFFSET,
				DDR_PHY_DX8SL3DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL3DXCTL2_RDMODE_SHIFT, 3U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL4DXCTL2_OFFSET,
				DDR_PHY_DX8SL4DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL4DXCTL2_RDMODE_SHIFT, 3U);

		Xil_Out32(DDR_PHY_PIR_OFFSET, 0x00060001U);
		RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);
		while ((RegVal & 0x80004001U) != 0x80004001U)
			RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);

		XFSBL_PROG_REG(DDR_PHY_PGCR3_OFFSET, DDR_PHY_PGCR3_RDMODE_MASK,
				DDR_PHY_PGCR3_RDMODE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL0DXCTL2_OFFSET,
				DDR_PHY_DX8SL0DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL0DXCTL2_RDMODE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL1DXCTL2_OFFSET,
				DDR_PHY_DX8SL1DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL1DXCTL2_RDMODE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL2DXCTL2_OFFSET,
				DDR_PHY_DX8SL2DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL2DXCTL2_RDMODE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL3DXCTL2_OFFSET,
				DDR_PHY_DX8SL3DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL3DXCTL2_RDMODE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8SL4DXCTL2_OFFSET,
				DDR_PHY_DX8SL4DXCTL2_RDMODE_MASK,
				DDR_PHY_DX8SL4DXCTL2_RDMODE_SHIFT, 0U);

		RegVal = Xil_In32(XFSBL_DDRPHY_BASE_ADDR + 0x200U);
		RegVal &= ~(0xFU << 28U);
		RegVal |= (0x8U << 28U);
		Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x200U, RegVal);

		XFSBL_PROG_REG(DDR_PHY_PGCR2_OFFSET,
				DDR_PHY_PGCR2_TREFPRD_MASK,
				DDR_PHY_PGCR2_TREFPRD_SHIFT, CurTRefPrd);

		Xil_Out32(DDR_PHY_PIR_OFFSET, 0x0000C001U);
		RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);
		while ((RegVal & 0x80000C01U) != 0x80000C01U)
			RegVal = Xil_In32(DDR_PHY_PGSR0_OFFSET);

	}

	if (PDimmPtr->Slowboot == 1U) {
		XFSBL_PROG_REG(DDRC_SWCTL_OFFSET, DDRC_SWCTL_SW_DONE_MASK,
				DDRC_SWCTL_SW_DONE_SHIFT, 0U);
		XFSBL_PROG_REG(DDRC_DFIMISC_OFFSET,
				DDRC_DFIMISC_DFI_INIT_COMPLETE_EN_MASK,
				DDRC_DFIMISC_DFI_INIT_COMPLETE_EN_SHIFT, 1U);
		XFSBL_PROG_REG(DDRC_DFIUPD0_OFFSET,
				DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_MASK,
				DDRC_DFIUPD0_DIS_AUTO_CTRLUPD_SHIFT, 0U);
		XFSBL_PROG_REG(DDRC_SWCTL_OFFSET, DDRC_SWCTL_SW_DONE_MASK,
				DDRC_SWCTL_SW_DONE_SHIFT, 1U);
	}

	RegVal = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x180U);
	RegVal &= ~(0x1U << 31U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x180U, RegVal);

	RegVal = Xil_In32(XFSBL_DDRC_BASE_ADDR + 0x60U);
	RegVal &= ~(0x1U << 0U);
	Xil_Out32(XFSBL_DDRC_BASE_ADDR + 0x60U, RegVal);

	if ((PDimmPtr->RdDqsCenter) && (PDimmPtr->MemType != SPD_MEMTYPE_LPDDR4)) {
		XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
				DDR_PHY_PGCR1_PUBMODE_MASK,
				DDR_PHY_PGCR1_PUBMODE_SHIFT, 0U);

		XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
				DDR_PHY_PGCR1_PUBMODE_MASK,
				DDR_PHY_PGCR1_PUBMODE_SHIFT, 1U);
	}

	XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
			DDR_PHY_PGCR1_PUBMODE_MASK,
			DDR_PHY_PGCR1_PUBMODE_SHIFT, 0U);

	if ((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) && (PDimmPtr->Slowboot == 1U)) {
		if ((PDimmPtr->RdDqsCenter) &&
				(PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4)) {
			XFSBL_PROG_REG(DDR_PHY_PGCR1_OFFSET,
					DDR_PHY_PGCR1_PUBMODE_MASK,
					DDR_PHY_PGCR1_PUBMODE_SHIFT, 0U);

		}
	}

	if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR4) && PDimmPtr->RDimm) {
		if (PDimmPtr->DisOpInv && (PDimmPtr->EnOpInvAfterTrain)) {
			XFsbl_MrsFunc(0x00U, 1U, 7U);
			RegVal = Xil_In32(XFSBL_DDRPHY_BASE_ADDR + 0x150U);
			RegVal &= ~(0x1U << 0U);
			Xil_Out32(XFSBL_DDRPHY_BASE_ADDR + 0x150U, RegVal);
		}
	}


	if (PDimmPtr->RdbiWrkAround == 1U) {
		XFsbl_RdbiWrkAround(PDimmPtr);
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		XFsbl_CfgSwInitInt(1U);
		XFsbl_CfgDfiInitComplete(1U);
		XFsbl_MrsFunc(0xB02U, ActiveRanks, 0U);
		XFsbl_CfgSwInitInt(0U);
	}

	if (PDimmPtr->WrDrift) {
		Puad = (u32)XFSBL_MAX(30.0 / PDimmPtr->ClockPeriod / 2.0, 8U) / 2U;
		XFSBL_PROG_REG(DDR_PHY_DSGCR_OFFSET,
				DDR_PHY_DSGCR_PUAD_MASK,
				DDR_PHY_DSGCR_PUAD_SHIFT, Puad);
		XFSBL_PROG_REG(DDR_PHY_DSGCR_OFFSET,
				DDR_PHY_DSGCR_CTLZUEN_MASK,
				DDR_PHY_DSGCR_CTLZUEN_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_DX0GCR3_OFFSET,
				DDR_PHY_DX0GCR3_WDLVT_MASK,
				DDR_PHY_DX0GCR3_WDLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX1GCR3_OFFSET,
				DDR_PHY_DX1GCR3_WDLVT_MASK,
				DDR_PHY_DX1GCR3_WDLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX2GCR3_OFFSET,
				DDR_PHY_DX2GCR3_WDLVT_MASK,
				DDR_PHY_DX2GCR3_WDLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX3GCR3_OFFSET,
				DDR_PHY_DX3GCR3_WDLVT_MASK,
				DDR_PHY_DX3GCR3_WDLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8GCR3_OFFSET,
				DDR_PHY_DX8GCR3_WDLVT_MASK,
				DDR_PHY_DX8GCR3_WDLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DTCR0_OFFSET,
				DDR_PHY_DTCR0_INCWEYE_MASK,
				DDR_PHY_DTCR0_INCWEYE_SHIFT, 1U);
	}
	if (PDimmPtr->RdDrift) {
		XFSBL_PROG_REG(DDR_PHY_DQSDR0_OFFSET,
				DDR_PHY_DQSDR0_DFTDTMODE_MASK,
				DDR_PHY_DQSDR0_DFTDTMODE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR0_OFFSET,
				DDR_PHY_DQSDR0_DFTUPMODE_MASK,
				DDR_PHY_DQSDR0_DFTUPMODE_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR0_OFFSET,
				DDR_PHY_DQSDR0_DFTGPULSE_MASK,
				DDR_PHY_DQSDR0_DFTGPULSE_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR0_OFFSET,
				DDR_PHY_DQSDR0_DFTRDSPC_MASK,
				DDR_PHY_DQSDR0_DFTRDSPC_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR0_OFFSET,
				DDR_PHY_DQSDR0_DFTDLY_MASK,
				DDR_PHY_DQSDR0_DFTDLY_SHIFT, 2U);
		XFSBL_PROG_REG(DDR_PHY_DX0GCR3_OFFSET,
				DDR_PHY_DX0GCR3_RGLVT_MASK,
				DDR_PHY_DX0GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX1GCR3_OFFSET,
				DDR_PHY_DX1GCR3_RGLVT_MASK,
				DDR_PHY_DX1GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX2GCR3_OFFSET,
				DDR_PHY_DX2GCR3_RGLVT_MASK,
				DDR_PHY_DX2GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX3GCR3_OFFSET,
				DDR_PHY_DX3GCR3_RGLVT_MASK,
				DDR_PHY_DX3GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX4GCR3_OFFSET,
				DDR_PHY_DX4GCR3_RGLVT_MASK,
				DDR_PHY_DX4GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX5GCR3_OFFSET,
				DDR_PHY_DX5GCR3_RGLVT_MASK,
				DDR_PHY_DX5GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX6GCR3_OFFSET,
				DDR_PHY_DX6GCR3_RGLVT_MASK,
				DDR_PHY_DX6GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX7GCR3_OFFSET,
				DDR_PHY_DX7GCR3_RGLVT_MASK,
				DDR_PHY_DX7GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DX8GCR3_OFFSET,
				DDR_PHY_DX8GCR3_RGLVT_MASK,
				DDR_PHY_DX8GCR3_RGLVT_SHIFT, 0U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR1_OFFSET,
				DDR_PHY_DQSDR1_DFTRDIDLC_MASK,
				DDR_PHY_DQSDR1_DFTRDIDLC_SHIFT, 1U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR1_OFFSET,
				DDR_PHY_DQSDR1_DFTRDIDLF_MASK,
				DDR_PHY_DQSDR1_DFTRDIDLF_SHIFT, 10U);
		XFSBL_PROG_REG(DDR_PHY_DQSDR0_OFFSET,
				DDR_PHY_DQSDR0_DFTDTEN_MASK,
				DDR_PHY_DQSDR0_DFTDTEN_SHIFT, 1U);
	}

	Status = XFSBL_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function sets the DDR config parameters used to initialize the DDR
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static void XFsbl_InitilizeDdrParams(struct DdrcInitData *DdrDataPtr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;

	PDimmPtr->DataMask = (XFSBL_DBI_INFO & 0x4U) >> 2U;

#if (XFSBL_DBI_INFO == 1U) || (XFSBL_DBI_INFO == 4U)
	PDimmPtr->RdDbi = 1U;
	PDimmPtr->WrDbi = 1U;
#elif (XFSBL_DBI_INFO == 2U) || (XFSBL_DBI_INFO == 5U)
	PDimmPtr->RdDbi = 1U;
	PDimmPtr->WrDbi = 0U;
#elif (XFSBL_DBI_INFO == 3U) || (XFSBL_DBI_INFO == 6U)
	PDimmPtr->RdDbi = 0U;
	PDimmPtr->WrDbi = 1U;
#else
	PDimmPtr->RdDbi = 0U;
	PDimmPtr->WrDbi = 0U;
#endif

	PDimmPtr->Ecc = XPAR_PSU_DDRC_0_HAS_ECC;
	PDimmPtr->En2ndClk = XPAR_PSU_DDRC_0_DDR_2ND_CLOCK;
	PDimmPtr->Parity = XPAR_PSU_DDRC_0_DDR_PARITY;
	PDimmPtr->PwrDnEn = XPAR_PSU_DDRC_0_DDR_POWER_DOWN_ENABLE;
	PDimmPtr->ClockStopEn = XPAR_PSU_DDRC_0_CLOCK_STOP;
	PDimmPtr->LpAsr = XPAR_PSU_DDRC_0_DDR_LOW_POWER_AUTO_SELF_REFRESH;
	PDimmPtr->TRefMode = XPAR_PSU_DDRC_0_DDR_TEMP_CONTROLLED_REFRESH;
	PDimmPtr->TRefRange = XPAR_PSU_DDRC_0_DDR_MAX_OPERATING_TEMPARATURE;
	PDimmPtr->Fgrm = XPAR_PSU_DDRC_0_DDR_FINE_GRANULARITY_REFRESH_MODE;
	PDimmPtr->SelfRefAbort = XPAR_PSU_DDRC_0_DDR_SELF_REFRESH_ABORT;

	if (((PDimmPtr->MemType == SPD_MEMTYPE_DDR4) ||
				(PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4)) &&
			(PDimmPtr->RdDbi == 1U)) {
		PDimmPtr->RdbiWrkAround =  1U;
	}

	PDimmPtr->ClockPeriod = 1000.0 / PDimmPtr->FreqMhz;

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		if (PDimmPtr->WrDbi == 1U) {
			PDimmPtr->DataMask = 0U;
		}
		if (PDimmPtr->RdDbi == 1U) {
			if (PDimmPtr->FreqMhz <= 933U) {
				PDimmPtr->CasLatency = PDimmPtr->CasLatency + 2U;
			} else {
				PDimmPtr->CasLatency = PDimmPtr->CasLatency + 3U;
			}
		}
	}

	if ((PDimmPtr->TRefRange == 1U) &&
			((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) ||
			 (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3))) {
		PDimmPtr->ClockPeriod = 1000.0 / PDimmPtr->FreqMhz;
		PDimmPtr->TRpPs += XFsbl_Ceil(1.875 / PDimmPtr->ClockPeriod) * 1000.0;
		PDimmPtr->TRcdPs += XFsbl_Ceil(1.875 / PDimmPtr->ClockPeriod) * 1000.0;
		PDimmPtr->TRasPs += XFsbl_Ceil(1.875 / PDimmPtr->ClockPeriod) * 1000.0;

		if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {
			PDimmPtr->TRcPs += XFsbl_Ceil(3.75 / PDimmPtr->ClockPeriod) * 1000.0;
		} else {
			PDimmPtr->TRcPs += XFsbl_Ceil(1.875 / PDimmPtr->ClockPeriod) * 1000.0;
		}
	}

	if ((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) &&
			((PDimmPtr->Lpddr4Samsung == 1U) ||
			 (PDimmPtr->Lpddr4Hynix == 1U))) {
		if ((PDimmPtr->SpeedBin <= 1600U) &&
				(PDimmPtr->SpeedBin > 1066U))  {
			PDimmPtr->UseSetB = 1U;
		} else if (PDimmPtr->SpeedBin <= 1066U) {
			PDimmPtr->Lp4NoOdt = 1U;
		}
	}

	PDimmPtr->Vref = 1U;

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4) {
		if (PDimmPtr->Fgrm == 0U) {
			PDimmPtr->TRfcPs = PDimmPtr->TRfc1Ps;
		}
		else if (PDimmPtr->Fgrm == 1U) {
			PDimmPtr->TRfcPs = PDimmPtr->TRfc2Ps;
		}
		else if (PDimmPtr->Fgrm == 2U) {
			PDimmPtr->TRfcPs = PDimmPtr->TRfc4Ps;
		}
	}

	else if ((PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) || (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4)) {
		if (PDimmPtr->PerBankRefresh) {
			PDimmPtr->TRfcPs = PDimmPtr->TRfcPbPs;
		} else {
			PDimmPtr->TRfcPs = PDimmPtr->TRfcAbPs;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		PDimmPtr->ReadLatency = (PDimmPtr->FreqMhz <= 733U) ? (PDimmPtr->FreqMhz / 66U) : 12U;
	}

	else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR4) {

		PDimmPtr->ReadLatency = (PDimmPtr->FreqMhz / 66U) + 4U;

		if (PDimmPtr->RdDbi) {
			if (PDimmPtr->FreqMhz <= 266U)
				PDimmPtr->ReadLatency = 6U;

			else if (PDimmPtr->FreqMhz <= 1066U)
				PDimmPtr->ReadLatency = PDimmPtr->ReadLatency + 2U;

			else if (PDimmPtr->FreqMhz <= 2133U)
				PDimmPtr->ReadLatency = PDimmPtr->ReadLatency + 4U;
		}
	}

	else
		PDimmPtr->ReadLatency = PDimmPtr->AdditiveLatency + PDimmPtr->CasLatency;

	if ((PDimmPtr->MemType == SPD_MEMTYPE_DDR3) || (PDimmPtr->MemType == SPD_MEMTYPE_DDR4))
		PDimmPtr->WriteLatency = PDimmPtr->AdditiveLatency + PDimmPtr->CasWriteLatency;

	else if (PDimmPtr->MemType == SPD_MEMTYPE_LPDDR3) {
		if      (PDimmPtr->FreqMhz <= 167U) {
			PDimmPtr->WriteLatency = 4U;
		}

		else if (PDimmPtr->FreqMhz <= 400U) {
			PDimmPtr->WriteLatency = 4U;
		}

		else if (PDimmPtr->FreqMhz <= 533U) {
			PDimmPtr->WriteLatency = 4U;
		}

		else if (PDimmPtr->FreqMhz <= 600U) {
			PDimmPtr->WriteLatency = 5U;
		}

		else if (PDimmPtr->FreqMhz <= 667U) {
			PDimmPtr->WriteLatency = 8U;
		}

		else if (PDimmPtr->FreqMhz <= 733U) {
			PDimmPtr->WriteLatency = 9U;
		}

		else if (PDimmPtr->FreqMhz <= 800U) {
			PDimmPtr->WriteLatency = 9U;
		}

		else {
			PDimmPtr->WriteLatency = 9U;
		}
	}

	else {
		if (PDimmPtr->FreqMhz <= 266U) {
			PDimmPtr->WriteLatency = 4U;
			PDimmPtr->WdqsOn  = 0U;
			PDimmPtr->WdqsOff = 15U;
		} else if (PDimmPtr->FreqMhz <= 533U)  {
			PDimmPtr->WriteLatency = 6U;
			PDimmPtr->WdqsOn  = 0U;
			PDimmPtr->WdqsOff = 18U;
		} else if (PDimmPtr->FreqMhz <= 800U)  {
			PDimmPtr->WriteLatency = 8U;

			if (PDimmPtr->UseSetB == 1U) PDimmPtr->WriteLatency = 12U;
			PDimmPtr->WdqsOn  = 0U;
			PDimmPtr->WdqsOff = 21U;
		} else if (PDimmPtr->FreqMhz <= 1066U) {
			PDimmPtr->WriteLatency = 10U;
			PDimmPtr->WdqsOn  = 4U;
			PDimmPtr->WdqsOff = 24U;
		} else if (PDimmPtr->FreqMhz <= 1333U) {
			PDimmPtr->WriteLatency = 12U;
			PDimmPtr->WdqsOn  = 4U;
			PDimmPtr->WdqsOff = 27U;
		} else if (PDimmPtr->FreqMhz <= 1600U) {
			PDimmPtr->WriteLatency = 14U;
			PDimmPtr->WdqsOn  = 6U;
			PDimmPtr->WdqsOff = 30U;
		} else if (PDimmPtr->FreqMhz <= 1866U) {
			PDimmPtr->WriteLatency = 16U;
			PDimmPtr->WdqsOn  = 6U;
			PDimmPtr->WdqsOff = 33U;
		} else if (PDimmPtr->FreqMhz <= 2133U) {
			PDimmPtr->WriteLatency = 18U;
			PDimmPtr->WdqsOn  = 8U;
			PDimmPtr->WdqsOff = 36U;
		}
	}

	if (PDimmPtr->MemType == SPD_MEMTYPE_DDR4)
	{
		if (PDimmPtr->Parity)
		{
			PDimmPtr->ParityLatency =  (PDimmPtr->SpeedBin < 2400U) ? 4U : 5U;
		}
	}
	PDimmPtr->CtlClkFreq = PDimmPtr->FreqMhz / 2U;

}

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208)
/*****************************************************************************/
/**
 * This function calculates and writes DDR controller registers
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	None
 *
 *****************************************************************************/
static u32 XFsbl_Ddr4Init(u8 *SpdData, struct DdrcInitData *DdrDataPtr)
{
	XFsbl_DimmParams *PDimmPtr = &DdrDataPtr->PDimm;
	u32 DdrCfg[300U] = XFSBL_DDRC_REG_DEFVAL;
	u32 PhyCfg[512U] = XFSBL_PHY_REG_DEFVAL;
	u32 Status;
	u32 RegVal;

	XFsbl_ComputeDdr4Params(SpdData, DdrDataPtr);
	/* Initialize the Parameters with their default values */
	XFsbl_InitilizeDdrParams(DdrDataPtr);

	/* Assert Reset for DDR controller */
	RegVal = Xil_In32(CRF_APB_RST_DDR_SS_OFFSET);
	RegVal |= 0x00000008U;
	Xil_Out32(CRF_APB_RST_DDR_SS_OFFSET, RegVal);


	Status = XFsbl_DdrcCalcCommonRegVal(DdrDataPtr, PDimmPtr, DdrCfg);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	Status = XFsbl_DdrcCalcDdr4RegVal(PDimmPtr, DdrCfg);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	DdrCfg[DDR_DFI_TPHY_RDCSLAT] = DdrCfg[DDR_DFI_T_RDDATA_EN] - 2U;

	if (!PDimmPtr->RDimm) {
		DdrCfg[DDR_DFI_TPHY_WRCSLAT] = DdrCfg[DDR_DFI_TPHY_WRLAT] - 2U;
	}

	/* Store the MR values which will be used for PHY Registers */
	PDimmPtr->Mr = DdrCfg[DDR_MR];
	PDimmPtr->Emr = DdrCfg[DDR_EMR];
	PDimmPtr->Emr2 = DdrCfg[DDR_EMR2];
	PDimmPtr->Emr3 = DdrCfg[DDR_EMR3];
	PDimmPtr->Mr4 = DdrCfg[DDR_MR4];
	PDimmPtr->Mr5 = DdrCfg[DDR_MR5];
	PDimmPtr->Mr6 = DdrCfg[DDR_MR6];

	XFsbl_DdrcRegsWrite(PDimmPtr, DdrCfg);

	/* De-assert Reset for DDR controller */
	RegVal = Xil_In32(CRF_APB_RST_DDR_SS_OFFSET);
	RegVal &= ~0x0000000CU;
	Xil_Out32(CRF_APB_RST_DDR_SS_OFFSET, RegVal);

	Status = XFsbl_PhyCalcCommonRegVal(PDimmPtr, PhyCfg);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	Status = XFsbl_PhyCalcDdr4RegVal(PDimmPtr, PhyCfg);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	XFsbl_PhyRegsWrite(PDimmPtr, PhyCfg);

	Status = XFSBL_SUCCESS;
END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * This function Reads the DDR4 SPD from EEPROM via I2C
 *
 * @param	DdrDataPtr is pointer to DDR Initialization Data Structure
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 *			returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
static u32 XFsbl_IicReadSpdEeprom(u8 *SpdData)
{
	XIicPs IicInstance;		/* The instance of the IIC device. */
	XIicPs_Config *ConfigIic;
	u8 TxArray;
	s32 Status;
	u32 UStatus;
	u32 Regval = 0U;

	/* Lookup for I2C-1U device */
	ConfigIic = XIicPs_LookupConfig(XPAR_PSU_I2C_1_DEVICE_ID);
	if (!ConfigIic) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/* Initialize the I2C device */
	Status = XIicPs_CfgInitialize(&IicInstance, ConfigIic,
			ConfigIic->BaseAddress);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/* Set the Serial Clock for I2C */
	Status = XIicPs_SetSClk(&IicInstance, XFSBL_IIC_SCLK_RATE);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Configure I2C Mux to select DDR4 SODIMM Slave
	 * 0x08U - Enable DDR4 SODIMM module
	 */
	TxArray = 0x08U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U, XFSBL_MUX_ADDR);

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval & XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Get Configuration to confirm the selection of the slave
	 * device.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, SpdData, 1U,
			XFSBL_MUX_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Set SODIMM control address to enable access to lower
	 * EEPROM page (0U to 255U Bytes).
	 * 0x00U - Enable Read of Lower Page from EEPROM
	 */
	TxArray = 0x00U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U,
			XFSBL_SODIMM_CONTROL_ADDR_LOW);
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Configure SODIMM Slave address to select starting address of the
	 * read bytes.
	 * 0x00U - Set starting byte address of read Lowe Page from EEPROM
	 * This will result in to starting address of 0x149U (0x100U + 0x49U) in
	 * the EEPROM.
	 */
	TxArray = 0x00U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U,
			XFSBL_SODIMM_SLAVE_ADDR);
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Receive the Data of 256U Bytes from SPD EEPROM via I2C.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, SpdData, 256U,
			XFSBL_SODIMM_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Wait until bus is idle.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}


	/*
	 * Set SODIMM control address to enable access to upper
	 * EEPROM page (256U to 511U Bytes).
	 * 0x01U - Enable Read of Upper Page from EEPROM
	 */
	TxArray = 0x01U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U,
			XFSBL_SODIMM_CONTROL_ADDR_HIGH);
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Configure SODIMM Slave address to select starting address of the
	 * read bytes.
	 * 0x00U - Set starting byte address of read Upper Page from EEPROM
	 * This will result in to starting address of 0x149U (0x100U + 0x49U) in
	 * the EEPROM.
	 */
	TxArray = 0x00U;
	XIicPs_MasterSendPolled(&IicInstance, &TxArray, 1U,
			XFSBL_SODIMM_SLAVE_ADDR);
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Receive the Data of 256U Bytes from SPD EEPROM via I2C.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, &SpdData[256U], 256U,
			XFSBL_SODIMM_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	/*
	 * Wait until bus is idle.
	 */
	Status = XFsbl_PollTimeout(IicInstance.Config.BaseAddress +
			XIICPS_SR_OFFSET, Regval, (Regval &
				XIICPS_SR_BA_MASK) == 0x0U,
			XFSBL_IIC_BUS_TIMEOUT);
	if (Status != XST_SUCCESS) {
		UStatus = XFSBL_FAILURE;
		goto END;
	}

	UStatus = XFSBL_SUCCESS;

END:
	return UStatus;
}

/*****************************************************************************/
/**
 * This function checks for the DDR SPD data and Initializes the same based on
 * configuration parameters obtained from SPD data.
 *
 * @param	None
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 *			returns XFSBL_SUCCESS on success
 *
 *****************************************************************************/
u32 XFsbl_DdrInit(void)
{
	u32 Status;
	u8 SpdData[512U];
#if !(defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208)) || defined(XFSBL_ENABLE_DDR_SR)
	u32 RegVal;
#endif

	/* Define and Initialize the DDR Initialization data */
	struct DdrcInitData DdrData = {
		.AddrMapCsBit0 = 0x0U,
		.AddrMapRowBits2To10 = 0x0U,
	};

	/* Get the Model Part Number from the SPD stored in EEPROM */
	Status = XFsbl_IicReadSpdEeprom(SpdData);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

#if defined(XPS_BOARD_ZCU102) || defined(XPS_BOARD_ZCU106) \
	|| defined(XPS_BOARD_ZCU111) || defined(XPS_BOARD_ZCU216) \
	|| defined(XPS_BOARD_ZCU208)
	/* ZCU102, ZCU106 and ZCU111, ZCU216 and ZCU208 Boards have support
	 * only for DDR4 DIMMs. Skip checking for DDR type for these boards.
	 */
	Status = XFsbl_Ddr4Init(SpdData, &DdrData);
	if (XFSBL_SUCCESS != Status) {
		Status = XFSBL_FAILURE;
		goto END;
	}
#else
	/* Determine the DIMM parameters to be used for register writes */
	Status = XFsbl_DdrComputeDimmParameters(SpdData, &DdrData);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	/* Initialize the Parameters with their default values */
	XFsbl_InitilizeDdrParams(&DdrData);

	/* Assert Reset for DDR controller */
	RegVal = Xil_In32(CRF_APB_RST_DDR_SS_OFFSET);
	RegVal |= 0x00000008U;
	Xil_Out32(CRF_APB_RST_DDR_SS_OFFSET, RegVal);

	/* Calculate and Write all the registers of DDR Controller */
	Status = XFsbl_DdrcRegsInit(&DdrData);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}

	/* De-assert Reset for DDR controller */
	RegVal = Xil_In32(CRF_APB_RST_DDR_SS_OFFSET);
	RegVal &= ~0x0000000CU;
	Xil_Out32(CRF_APB_RST_DDR_SS_OFFSET, RegVal);

	/* Calculate and Write all the registers of DDR-PHY Controller */
	Status = XFsbl_PhyRegsInit(&DdrData);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}
#endif

#ifdef XFSBL_ENABLE_DDR_SR
	/* Check if DDR is in self refresh mode */
	RegVal = Xil_In32(XFSBL_DDR_STATUS_REGISTER_OFFSET) &
		DDR_STATUS_FLAG_MASK;
	if (!RegVal) {
		/* Execute the Training Sequence */
		Status = XFsbl_DdrcPhyTraining(&DdrData);
		if (Status != XFSBL_SUCCESS) {
			Status = XFSBL_FAILURE;
			goto END;
		}
	}
#else
	/* Execute the Training Sequence */
	Status = XFsbl_DdrcPhyTraining(&DdrData);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_FAILURE;
		goto END;
	}
#endif

	Status = XFSBL_SUCCESS;
END:
	return Status;
}
#endif /* XPAR_DYNAMIC_DDR_ENABLED */
#endif /* XFSBL_PS_DDR */
