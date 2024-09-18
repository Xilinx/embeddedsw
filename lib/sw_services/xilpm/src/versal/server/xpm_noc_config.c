/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpm_noc_config.c
*
* NOC configuration implementation
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver	Who  Date	 Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  rpoolla	 23/01/18 Initial release
* 2.0  tnt	 01/11/22 Porting to xilpm
*
* </pre>
*
* @note
*
******************************************************************************/

#include <xpm_common.h>
#include <xpm_regs.h>
#include <sleep.h>
#include "xil_util.h"
#include "xpm_noc_config.h"
#include "xpm_bisr.h"

#define NPI_ROOT_BASE_ADDR		(0xF6000000U)
#define NSU_ID_SLR1			(0x00000401U)
#define NSU_ID_SLR2			(0x00000801U)
#define NSU_ID_SLR3			(0x00000C01U)
#define MASTER_PMC_NMU_ID		(0x00000000U)

#define NPS_ADDR_ADDITIONAL_OFFSET	(0x00002000U)
#define NIDB_OFFSET_DIFFERENCE		(0x00010000U)

#define HIGH_ID_MASK			(0x00000FC0U)
#define MID_ID_MASK			(0x00000030U)
#define LOW_ID_MASK			(0x0000000FU)
#define HIGH_ID_SHIFT			(0x00000006U)
#define MID_ID_SHIFT			(0x00000004U)
#define HIGH_ID_OFFSET			(0x00000100U)
#define MID_ID_OFFSET			(0x00000300U)
#define LOW_ID_OFFSET			(0x00000320U)
#define HALFWORD_OUT_PORT_0		(0x00000000U)
#define HALFWORD_OUT_PORT_1		(0x00005555U)
#define HALFWORD_OUT_PORT_2		(0x0000AAAAU)
#define HALFWORD_OUT_PORT_3		(0x0000FFFFU)

#define PORT_0				(0x00000000U)
#define PORT_1				(0x00000001U)
#define PORT_2				(0x00000002U)
#define PORT_3				(0x00000003U)

#define PORT_0_2_MASK			(0xFFFF0000U)
#define PORT_1_3_MASK			(0x0000FFFFU)
#define SSIT_INVALID_DEVICE		(0U)
#define SSIT_SLR3_TOP_SLR_DEVICE	(1U)
#define SSIT_SLR2_NOTTOP_SLR_DEVICE	(2U)
#define SSIT_SLR2_TOP_SLR_DEVICE	(3U)
#define SSIT_SLR1_NOTTOP_SLR_DEVICE	(4U)
#define SSIT_SLR1_TOP_SLR_DEVICE	(5U)
#define SSIT_MASTER_SLR_DEVICE		(6U)
#define SSIT_MONO_DEVICE		(7U)

#define PMC_GLOBAL_SSIT_NOC_ID (PMC_GLOBAL_BASEADDR + PMC_GLOBAL_SSIT_NOC_ID_OFFSET)

#define XPM_ALL_ZERO	(0x00000000U)

static XStatus XPm_NpsTurn(u32 NpsAddr, u32 ReqID, u32 RespID, u32 InputPort,
			u32 OutputPort);
static XStatus XPm_SlvSlrNidbCfg(u32 NidbAddr);
static XStatus XPm_SlvSlrBootNocCfg(u32 NirAddr, u32 RelNpsAddr, u32 PmcNsuID,
				u32 MstPmcNmuID, u32 SsitType);
static XStatus XPm_NocSwitchConfig(u32 SlrType);

XStatus XPm_NoCConfig(void)
{
	u32 SlrType;
	volatile XStatus Status = XST_FAILURE;
	SlrType = XPm_GetSlrType();
	if (SSIT_INVALID_DEVICE == SlrType) {
		Status = XST_FAILURE;
		goto END;
	}

	if ((SSIT_MONO_DEVICE == SlrType) ||
	   (SSIT_MASTER_SLR_DEVICE == SlrType)) {
		Status = XST_SUCCESS;
		goto END;
	}
	/**
	* NoC Power is checked in the HW config routine
	*/
	Status = XPm_NoCHWConfig();
	if (XST_SUCCESS != Status) {
		goto END;
	}
	/**
	 * NIDB Repair
	 * Enable NIDB
	 */
	Status = XPmBisr_NidbLeftMostLaneRepair();
	if (XST_SUCCESS != Status) {
		goto END;
	}
	/**
	 *
	 * Temporal check on return value added
	 */
	XSECURE_TEMPORAL_CHECK(END, Status, XPm_NocSwitchConfig, SlrType);
END:
	return Status;
}

XStatus XPm_NoCHWConfig(void)
{
	/**
	 * Check the power supply for SoC (NoC)
	 */

	/**
	 * Remove isolation for SoC
	 */
	XPm_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
		(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK),
		~PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_NPI_MASK);

	/* Delay after isolation removal */
	usleep(10);
	XPm_RMW32(CRP_RST_NONPS,
		(CRP_RST_NONPS_NOC_POR_MASK),
		~(CRP_RST_NONPS_NOC_POR_MASK));
	usleep(1);
		/**
		 * NPI Reset
		 */
		XPm_RMW32(CRP_RST_NONPS,
			(CRP_RST_NONPS_NPI_RESET_MASK),
			~(CRP_RST_NONPS_NPI_RESET_MASK));
	/**
	 * Remove isolation between PMC_SOC
	 */
	XPm_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
			(PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK),
			~PMC_GLOBAL_DOMAIN_ISO_CNTRL_PMC_SOC_MASK);

	usleep(10);
	XPm_RMW32(CRP_RST_NONPS,
		(CRP_RST_NONPS_NOC_RESET_MASK),
		~(CRP_RST_NONPS_NOC_RESET_MASK));
	return XST_SUCCESS;
}

static XStatus XPm_NocSwitchConfig(u32 SlrType)
{
	u32 NoCswid;
	u32 RelNpsAddr;
	u32 NsuID;
	volatile XStatus Status = XST_FAILURE;
	/**
	 * Configure NoC switches.
	 */

	NoCswid = Xil_In32(PMC_GLOBAL_SSIT_NOC_ID) &
			PMC_GLOBAL_SSIT_NOC_ID_SWITCHID_MASK;
	RelNpsAddr = NoCswid << 10U;

	if ((SSIT_SLR1_TOP_SLR_DEVICE == SlrType) ||
			(SSIT_SLR1_NOTTOP_SLR_DEVICE == SlrType)) {
		NsuID = NSU_ID_SLR1;
	}
	else if ((SSIT_SLR2_TOP_SLR_DEVICE == SlrType) ||
			(SSIT_SLR2_NOTTOP_SLR_DEVICE == SlrType)) {
		NsuID = NSU_ID_SLR2;
	}
	else if (SSIT_SLR3_TOP_SLR_DEVICE == SlrType) {
		NsuID = NSU_ID_SLR3;
	}
	else {
		NsuID = NSU_ID_SLR1;
	}
	/**
	 * Temproal check added
	 */
	XSECURE_TEMPORAL_CHECK(END, Status,
		XPm_SlvSlrBootNocCfg,
		NPI_ROOT_BASE_ADDR,
		RelNpsAddr,
		NsuID,
		MASTER_PMC_NMU_ID,
		SlrType);
END:
	return Status;
}

/*****************************************************************************
*
* NoC configuration for slave SLR
*
******************************************************************************/
static XStatus XPm_SlvSlrBootNocCfg(u32 NirAddr,
				u32 RelNpsAddr,
				u32 PmcNsuID,
				u32 MstPmcNmuID,
				u32 SsitType)
{
	volatile XStatus Status = XST_FAILURE;
	u32 NpsAddr;
	u32 NidbAddr;
	(void)SsitType;

	NpsAddr = (NirAddr + RelNpsAddr + NPS_ADDR_ADDITIONAL_OFFSET);
	NidbAddr = (NirAddr + RelNpsAddr - NIDB_OFFSET_DIFFERENCE);

	/**
	 * Temproal check added
	 */

	XSECURE_TEMPORAL_CHECK(END, Status,
		XPm_NpsTurn,
		NpsAddr,
		PmcNsuID,
		MstPmcNmuID,
		PORT_1,
		PORT_0);
	/**
	 * Unlock
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_LOCK
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_LOCK_OFFSET), NPI_PCSR_UNLOCK_VAL);
	/**
	 * Set Source Id
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_SRC
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_SRC_OFFSET), PmcNsuID);
	/**
	 * Mask Removed and Hold State
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_MASK
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_CONTROL
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_MASK_OFFSET),
		NOC_NSU_1_REG_PCSR_MASK_HOLDSTATE_MASK);
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_CONTROL_OFFSET), XPM_ALL_ZERO);
	/**
	 * Init State
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_MASK
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_CONTROL
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_MASK_OFFSET),
		NOC_NSU_1_REG_PCSR_MASK_INITSTATE_MASK);
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_CONTROL_OFFSET), XPM_ALL_ZERO);
	/**
	 * ODISABLE AXI
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_MASK
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_CONTROL
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_MASK_OFFSET),
		NOC_NSU_1_REG_PCSR_MASK_ODISABLE_AXI_MASK);
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_CONTROL_OFFSET), XPM_ALL_ZERO);
	/**
	 * Set PCOMPLETE
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_MASK_OFFSET),
		NOC_NSU_1_REG_PCSR_MASK_PCOMPLETE_MASK);
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_CONTROL_OFFSET),
		NOC_NSU_1_REG_PCSR_MASK_PCOMPLETE_MASK);

	/**
	 * PCSR Lock
	 * reg_model_global.u_noc_pmc_nsu.noc_nsu_bank.REG_PCSR_LOCK
	 */
	Xil_Out32((NOC_NSU_1_BASEADDR + NOC_NSU_1_REG_PCSR_LOCK_OFFSET),XPM_ALL_ZERO);
	Status = XPm_SlvSlrNidbCfg(NidbAddr);
END:
	return Status;
}

static XStatus XPm_SlvSlrNidbCfg(u32 NidbAddr)
{
	XStatus Status = XST_FAILURE;
	/**
	 * Unlock Change
	 * reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
	 */
	Xil_Out32((NidbAddr + NIDB_REG_PCSR_LOCK_OFFSET), NPI_PCSR_UNLOCK_VAL);
	/**
	 * This is to de-asert pdisable field
	 * reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_MASK
	 * reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_CONTROL
	 */

	Xil_Out32((NidbAddr + NIDB_REG_PCSR_MASK_OFFSET), NIDB_REG_PCSR_MASK_ODISABLE_MASK);
	Xil_Out32((NidbAddr + NIDB_REG_PCSR_CONTROL_OFFSET), XPM_ALL_ZERO);

	/**
	 * Set the PCOMPLETE
	 */
	Xil_Out32((NidbAddr + NIDB_REG_PCSR_MASK_OFFSET), NIDB_REG_PCSR_MASK_PCOMPLETE_MASK);
	Xil_Out32((NidbAddr + NIDB_REG_PCSR_CONTROL_OFFSET),
		NIDB_REG_PCSR_CONTROL_PCOMPLETE_MASK);
	/**
	 * Lock
	 * reg_model_global.u_npi_nir_top.npi_nir_bank.REG_PCSR_LOCK
	 */
	Xil_Out32((NidbAddr + NIDB_REG_PCSR_LOCK_OFFSET),XPM_ALL_ZERO);
	// // Check REMOTE_LINK and LOCAL_LINK ready
	Status = XPm_PollForMask(NidbAddr + NIDB_REG_PCSR_STATUS_OFFSET,
		NIDB_REG_PCSR_STATUS_REMOTE_LINK_READY_MASK |
		NIDB_REG_PCSR_STATUS_LOCAL_LINK_READY_MASK, 100);
	if (XST_SUCCESS != Status) {
		PmErr("Error during check link. PCSR_Status=0x%x\n\r",
			Xil_In32(NidbAddr + NIDB_REG_PCSR_STATUS_OFFSET));
	}
	Status = XST_SUCCESS;
	return Status;
}

/*****************************************************************************
*
* NPS Turn configuration
*
******************************************************************************/

static XStatus XPm_NpsTurn(u32 NpsAddr,
			  u32 ReqID,
			  u32 RespID,
			  u32 InputPort,
			  u32 OutputPort)
{
	XStatus Status = XST_FAILURE;
	u32 TempAddr;
	u32 HighID;
	u32 MidID;
	u32 LowID;
	u32 WrData = XPM_ALL_ZERO;
	u32 WrHalfWord = HALFWORD_OUT_PORT_1;
	/**
	 * Unlock PCSR
	 * Program Mask
	 * Program Control
	 */
	Xil_Out32((NpsAddr + NPS_REG_PCSR_LOCK_OFFSET), NPI_PCSR_UNLOCK_VAL);
	/**
	 * Configuring the request path
	 */
	HighID = ((ReqID & HIGH_ID_MASK) >> HIGH_ID_SHIFT);
	MidID = ((ReqID & MID_ID_MASK) >> MID_ID_SHIFT);
	LowID = (ReqID & LOW_ID_MASK);
	if (XPM_ALL_ZERO != HighID) {
		/**
		 * HIGHID0 Address
		 */
		TempAddr = (NpsAddr + HIGH_ID_OFFSET + (HighID * 8U));
	} else if (XPM_ALL_ZERO != MidID) {
		/**
		 * MIDID0 Address
		 */
		TempAddr = (NpsAddr + MID_ID_OFFSET + (MidID * 8U));
	} else {
		/**
		 * LOWID0 Address
		 */
		TempAddr = (NpsAddr + LOW_ID_OFFSET + (LowID * 8U));
	}

	if (PORT_0 == OutputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_0;
	}
	if (PORT_1 == OutputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_1;
	}
	if (PORT_2 == OutputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_2;
	}
	if (PORT_3 == OutputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_3;
	}

	if (PORT_0 == InputPort) {

		WrData = (Xil_In32(TempAddr) & PORT_0_2_MASK);
		WrData = (WrData + WrHalfWord);
	}

	if (PORT_1 == InputPort) {
		WrData = (Xil_In32(TempAddr) & PORT_1_3_MASK);
		WrData = (WrData & 0x0000FFFFU) + (WrHalfWord << 16U);
	}

	if (PORT_2 == InputPort) {
		TempAddr = TempAddr + 4U;
		WrData = (Xil_In32(TempAddr) & PORT_0_2_MASK);
		WrData = (WrData  + WrHalfWord);
	}

	if (PORT_3 == InputPort) {
		TempAddr = TempAddr + 4U;
		WrData = (Xil_In32(TempAddr) & PORT_1_3_MASK);
		WrData = (WrData + (WrHalfWord << 16U));
	}
	Status = (XStatus)Xil_SecureOut32(TempAddr, WrData);

	if (XST_SUCCESS != Status) {
		goto END;
	}
	/**
	 * Configuring for Response path
	 */

	HighID = ((RespID & HIGH_ID_MASK)>>HIGH_ID_SHIFT);
	MidID = ((RespID & MID_ID_MASK)>>MID_ID_SHIFT);
	LowID = (RespID & LOW_ID_MASK);

	if (XPM_ALL_ZERO != HighID) {
		TempAddr = (NpsAddr + HIGH_ID_OFFSET + (HighID * 8U));
	} else if (XPM_ALL_ZERO != MidID) {
		TempAddr = (NpsAddr + MID_ID_OFFSET + (MidID * 8U));
	} else {
		TempAddr = (NpsAddr + LOW_ID_OFFSET + (LowID * 8U));
	}


	if (PORT_0 == InputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_0;
	}
	if (PORT_1 == InputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_1;
	}
	if (PORT_2 == InputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_2;
	}
	if (PORT_3 == InputPort) {
		WrHalfWord = HALFWORD_OUT_PORT_3;
	}

	if (PORT_0 == OutputPort) {
		WrData = (Xil_In32(TempAddr) & PORT_0_2_MASK);
		WrData = (WrData + WrHalfWord);
	}

	if (PORT_1 == OutputPort) {
		WrData = (Xil_In32(TempAddr) & PORT_1_3_MASK);
		WrData = (WrData  + (WrHalfWord << 16U));
	}

	if (PORT_2 == OutputPort) {
		TempAddr = TempAddr + 4U;
		WrData = (Xil_In32(TempAddr) & PORT_0_2_MASK);
		WrData = (WrData  + WrHalfWord);
	}

	if (PORT_3 == OutputPort) {
		TempAddr = TempAddr + 4U;
		WrData = (Xil_In32(TempAddr) & PORT_1_3_MASK);
		WrData = (WrData & 0x0000FFFFU) + (WrHalfWord << 16U);
	}

	Status = (XStatus)Xil_SecureOut32(TempAddr, WrData);
	if (XST_SUCCESS != Status) {
		goto END;
	}
	/**
	 * Set the PCOMPLETE
	 */
	Xil_Out32((NpsAddr + NPS_REG_PCSR_MASK_OFFSET),
		NPS_REG_PCSR_MASK_PCOMPLETE_MASK);
	XPm_RMW32((NpsAddr + NPS_REG_PCSR_CONTROL_OFFSET),
		NPS_REG_PCSR_MASK_PCOMPLETE_MASK,
		NPS_REG_PCSR_MASK_PCOMPLETE_MASK);
END:
	/**
	 * REG_PCSR_LOCK
	 */
	Xil_Out32((NpsAddr + NPS_REG_PCSR_LOCK_OFFSET), XPM_ALL_ZERO);
	return Status;
}
