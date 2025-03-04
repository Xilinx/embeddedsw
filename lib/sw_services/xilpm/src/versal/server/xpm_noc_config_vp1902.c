/******************************************************************************
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include <xpm_common.h>
#include <xpm_regs.h>
#include <sleep.h>
#include "xil_util.h"
#include "xpm_bisr.h"
#include "xpm_noc_config.h"
#include "xplmi_ssit.h"

#ifdef XCVP1902
#define SLR1_NPS_13_REQUEST_PATH_WRITE_VALUE	(0xFFFF5555U)
#define SLR1_NPS_13_RESPONSE_PATH_WRITE_VALUE	(0x0000AAAAU)
#define SLR1_NPS_12_REQUEST_PATH_WRITE_VALUE	(0x0U)
#define SLR1_NPS_12_RESPONSE_PATH_WRITE_VALUE	(0xFFFFFFFFU)

#define SLR3_NPS_13_REQUEST_PATH_WRITE_VALUE	(0xFFFF5555U)
#define SLR3_NPS_13_RESPONSE_PATH_WRITE_VALUE	(0x0000AAAAU)
#define SLR3_NPS_12_REQUEST_PATH_WRITE_VALUE	(0x0U)
#define SLR3_NPS_12_RESPONSE_PATH_WRITE_VALUE	(0xFFFFFFFFU)

/* following values are used as request and response ids */
#define NSU_ID_SLR1				(0x00000401U)
#define NSU_ID_SLR2				(0x00000801U)
#define NSU_ID_SLR3				(0x00000C01U)

static XStatus XPm_SlaveNocConfigure_vp1902(const u32 SlrType);
static XStatus XPm_NidbStartup_vp1902(const u32 NidbAddress);

/*****************************************************************************
* NPS Turn configuration
******************************************************************************/
static XStatus XPm_NpsTurn_vp1902(const u32 NpsBaseAddress,
				  const u32 NpsReqPathRegAddr,
				  const u32 NpsRespPathRegAddr,
				  const u32 ReqPathWriteData,
				  const u32 RespPathWriteData) {
	volatile XStatus Status = XST_FAILURE;

	/**
	 * Unlock PCSR
	 * Program Mask
	 * Program Control
	 * Lock PCSR, in case of success or failure
	 */
	XPm_UnlockPcsr(NpsBaseAddress);

	/**
	 * NoC Switch configuration should read back and compare
	 * to make sure that correct data is written.
	 */
	XSECURE_TEMPORAL_CHECK(done, Status, Xil_SecureOut32,
			NpsReqPathRegAddr, ReqPathWriteData);

	/**
	 * NoC Switch configuration should read back and compare
	 * to make sure that correct data is written.
	 */
	XSECURE_TEMPORAL_CHECK(done, Status, Xil_SecureOut32,
			NpsRespPathRegAddr, RespPathWriteData);

	/**
	 * Set the PCOMPLETE
	 */
	Xil_Out32((NpsBaseAddress + NIDB_REG_PCSR_MASK_OFFSET),
			NIDB_REG_PCSR_MASK_PCOMPLETE_MASK);
	XPm_RMW32((NpsBaseAddress + NIDB_REG_PCSR_CONTROL_OFFSET),
		NIDB_REG_PCSR_MASK_PCOMPLETE_MASK, NIDB_REG_PCSR_CONTROL_PCOMPLETE_MASK);

done:
	/**
	 * REG_PCSR_LOCK
	 * instead of blind writes, writing twice
	 */
	XPm_LockPcsr(NpsBaseAddress);
	XPm_LockPcsr(NpsBaseAddress);

	return Status;
}

/*****************************************************************************
 * @brief	This NSU startup, moves to a functional state.
 *
 * @param	NSUId NSU id
 *
 * @return	none
 *
 ****************************************************************************/
static void XPm_NsuStartup_vp1902(const u32 NsuId) {
	/**
	 * Unlock PCSR
	 */
	XPm_UnlockPcsr(NOC_NSU_1_BASEADDR);

	/**
	 * Set Source Id
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_SRC_OFFSET) , NsuId);
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_SRC_OFFSET) , NsuId);

	/**
	 * Mask Removed and Hold State
	 */
	XPm_PcsrWrite(NOC_NSU_1_BASEADDR, NOC_NSU_1_REG_PCSR_MASK_HOLDSTATE_MASK, 0U);

	/**
	 * Deassert Init State
	 */
	XPm_PcsrWrite(NOC_NSU_1_BASEADDR, NOC_NSU_1_REG_PCSR_MASK_INITSTATE_MASK, 0U);

	/**
	 * ODISABLE AXI
	 */
	XPm_PcsrWrite(NOC_NSU_1_BASEADDR, NOC_NSU_1_REG_PCSR_MASK_ODISABLE_AXI_MASK, 0U);

	/**
	 * Set PCOMPLETE
	 */
	XPm_PcsrWrite(NOC_NSU_1_BASEADDR, NOC_NSU_1_REG_PCSR_MASK_PCOMPLETE_MASK, NOC_NSU_1_REG_PCSR_MASK_PCOMPLETE_MASK);

	/**
	 * PCSR Lock
	 * instead of blind writes, writing twice
	 */
	XPm_LockPcsr(NOC_NSU_1_BASEADDR);
	XPm_LockPcsr(NOC_NSU_1_BASEADDR);
}

/**********************************************************************************
 * @brief	This function calculates offset addresses from base based on output
 *		and input ports, configues swith (NPS) and removes isolation, then
 *		configues NIDBs required.
 *
 * @param	SlrType if the current SLR is 1, 2 or 3
 *
 * @return
 *		- XST_SUCCESS if successful,
 *		- XST_FAILURE if unsuccessful.
 *
 *********************************************************************************/
static XStatus XPm_SlaveNocConfigure_vp1902(const u32 SlrType) {
	volatile XStatus Status = XST_FAILURE;

	if((SLR_TYPE_MONOLITHIC_DEV == SlrType) ||
	   (SLR_TYPE_SSIT_DEV_MASTER_SLR == SlrType)) {
		goto done;
	}

	/**
	 * SLRs NPS turn configuration
	 * under SLR1 & SLR3 NPS turn configuration is performed and SLR2 no config
	 * required
	 *
	 * SLR1 NPS13 request & response path - 0xF61D2180U & 0xF61D2320U
	 * SLR1 NPS12 request & response path - 0xF61D0184U & 0xF61D0320U
	 * SLR3 NPS13 request & response path - 0xF61D2280U & 0xF61D2320U
	 * SLR3 NPS12 request & response path - 0xF61D0284U & 0xF61D0320U
	 *
	 * For Slave SLR1 -> Program/repair only NIDB8 (Vertical path
	 * coming from SLR0)
	 *
	 * For Slave SLR2 -> Program/repair only NIDB0
	 * (Horizontal path coming from SLR1)
	 *
	 * For Slave SLR3 -> Program/repair only NIDB8
	 * (Vertical path coming from SLR2)
	 */

	switch (SlrType) {
		case SLR_TYPE_SSIT_DEV_SLAVE_3_SLR_TOP:
		{
			/**
			 * NOC Conifiguration Security Modifications
			 *
			 * Temproal check added
			 */
			XSECURE_TEMPORAL_CHECK(
				done, Status, XPm_NpsTurn_vp1902, NPS_13_BASE_ADDRESS,
					SLR3_NPS_13_REQUEST_PATH_REG_ADDRESS,
					SLR3_NPS_13_RESPONSE_PATH_REG_ADDRESS,
					SLR3_NPS_13_REQUEST_PATH_WRITE_VALUE,
					SLR3_NPS_13_RESPONSE_PATH_WRITE_VALUE);

			/**
			 * NOC Conifiguration Security Modifications
			 *
			 * Temproal check added
			 */
			XSECURE_TEMPORAL_CHECK(
				done, Status, XPm_NpsTurn_vp1902, NPS_12_BASE_ADDRESS,
					SLR3_NPS_12_REQUEST_PATH_REG_ADDRESS,
					SLR3_NPS_12_RESPONSE_PATH_REG_ADDRESS,
					SLR3_NPS_12_REQUEST_PATH_WRITE_VALUE,
					SLR3_NPS_12_RESPONSE_PATH_WRITE_VALUE);

			XPm_NsuStartup_vp1902(NSU_ID_SLR3);

			XSECURE_TEMPORAL_CHECK(done, Status,
					XPm_NidbStartup_vp1902, NIDB_8_BASE_ADDRESS);
			break;
		}
		case SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_NTOP:
		{
			XPm_NsuStartup_vp1902(NSU_ID_SLR2);

			XSECURE_TEMPORAL_CHECK(done, Status,
					XPm_NidbStartup_vp1902, NIDB_0_BASE_ADDRESS);
			break;
		}
		case SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_NTOP:
		{
			/**
			 * NOC Conifiguration Security Modifications
			 *
			 * Temproal check added
			 */
			XSECURE_TEMPORAL_CHECK(
				done, Status, XPm_NpsTurn_vp1902, NPS_13_BASE_ADDRESS,
					SLR1_NPS_13_REQUEST_PATH_REG_ADDRESS,
					SLR1_NPS_13_RESPONSE_PATH_REG_ADDRESS,
					SLR1_NPS_13_REQUEST_PATH_WRITE_VALUE,
					SLR1_NPS_13_RESPONSE_PATH_WRITE_VALUE);

			/**
			 * NOC Conifiguration Security Modifications
			 *
			 * Temproal check added
			 */
			XSECURE_TEMPORAL_CHECK(
				done, Status, XPm_NpsTurn_vp1902, NPS_12_BASE_ADDRESS,
					SLR1_NPS_12_REQUEST_PATH_REG_ADDRESS,
					SLR1_NPS_12_RESPONSE_PATH_REG_ADDRESS,
					SLR1_NPS_12_REQUEST_PATH_WRITE_VALUE,
					SLR1_NPS_12_RESPONSE_PATH_WRITE_VALUE);

			XPm_NsuStartup_vp1902(NSU_ID_SLR1);

			XSECURE_TEMPORAL_CHECK(done, Status,
					XPm_NidbStartup_vp1902,
					NIDB_8_BASE_ADDRESS);
			break;
		}
		default:
		{
			/**
			 * something is seriously wrong, we shouldn't get this
			 */
			break;
		}
	}

done:
     return Status;
}

/****************************************************************************
* @brief	NoC Configuration for VP1902.
*
* @param	None
*
* @return	XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
XStatus XPm_NocConfig_vp1902(void)
{
	volatile u32 SlrType = 0U;
	volatile XStatus Status = XST_FAILURE;

	SlrType = (Xil_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);

	if (SLR_TYPE_INVALID == SlrType) {
		goto done;
	}

	if((SLR_TYPE_MONOLITHIC_DEV == SlrType) ||
	   (SLR_TYPE_SSIT_DEV_MASTER_SLR == SlrType)) {
		Status = XST_SUCCESS;
		goto done;
	}
	/**
	 * NoC Power is checked in the HW config routine
	 */
	Status = XPm_NoCHWConfig();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/**
	 * Temporal check on return value added
	 */
	XSECURE_TEMPORAL_CHECK(done, Status,
			XPm_SlaveNocConfigure_vp1902, SlrType);

done:
	return Status;
}

/*****************************************************************************
 * @brief	This function moves NIDB to functional state.
 *
 * @param	NidbAddress NPS base address
 *
 * @return
 *		- XST_SUCCESS if successful,
 *		- XST_FAILURE if unsuccessful.
 *
 ****************************************************************************/
static XStatus XPm_NidbStartup_vp1902(const u32 NidbAddress)
{
	volatile XStatus Status = XST_FAILURE;

	/**
	 * Unlock Change
	 */
	XPm_UnlockPcsr(NidbAddress);

	/**
	 * This is to de-asert odisable field
	 */
	XSECURE_TEMPORAL_CHECK(done, Status, Xil_SecureOut32,
			(NidbAddress + NIDB_REG_PCSR_MASK_OFFSET),
		(NIDB_REG_PCSR_MASK_ODISABLE_MASK & NIDB_REG_PCSR_MASK_ODISABLE_MASK));

	Xil_Out32((NidbAddress + NIDB_REG_PCSR_CONTROL_OFFSET),0);

	/**
	 * Set the PCOMPLETE
	 */
	XPm_PcsrWrite(NidbAddress, NIDB_REG_PCSR_MASK_PCOMPLETE_MASK, NIDB_REG_PCSR_CONTROL_PCOMPLETE_MASK);

	Xil_Out32((NidbAddress + NIDB_REG_PCSR_CONTROL_OFFSET),
			(NIDB_REG_PCSR_MASK_PCOMPLETE_MASK & NIDB_REG_PCSR_CONTROL_PCOMPLETE_MASK));

done:
	/**
	 * Lock PCSR. instead of blind writes, writing twice
	 */
	XPm_LockPcsr(NidbAddress);
	XPm_LockPcsr(NidbAddress);
	return Status;
}
#endif
