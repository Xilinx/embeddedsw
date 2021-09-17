/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi_util.h"
#include "xplmi_dma.h"
#include "xpm_common.h"
#include "xpm_aie.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_device.h"
#include "xpm_debug.h"

#define AIE_POLL_TIMEOUT 0X1000000U

/* TODO: Remove hard coded base addresses and row/col shifts for AIE */
#define COL_SHIFT 23U
#define ROW_SHIFT 18U
#define TILE_BASEADDRESS(col, row) ((u64)VIVADO_ME_BASEADDR +\
			((u64)(col) << COL_SHIFT)+\
			((u64)(row) << ROW_SHIFT))

/* TODO: Remove hard coded base addresses and row/col shifts for AIE2 */
#define AIE2_COL_SHIFT 25U
#define AIE2_ROW_SHIFT 20U
#define AIE2_TILE_BASEADDRESS(col, row) ((u64)VIVADO_ME_BASEADDR +\
		            ((u64)(col) << AIE2_COL_SHIFT) +\
		            ((u64)(row) << AIE2_ROW_SHIFT))

#define AIE_CORE_STATUS_DONE_MASK   (1UL<<20U)

#define XPM_AIE_OPS                    0U
#define XPM_AIE2_OPS                   1U
#define XPM_AIE_OPS_MAX                2U

#define AieWrite64(addr, val) swea(addr, val)
#define AieRead64(addr) lwea(addr)

static inline void AieRMW64(u64 addr, u32 Mask, u32 Value)
{
	u32 l_val;
	l_val = AieRead64(addr);
	l_val = (l_val & (~Mask)) | (Mask & Value);
	AieWrite64(addr, l_val);
}

/* Buffer to hold AIE data memory zeroization elf */
/**
 * NOTE: If ProgramMem[] is updated in future, then check if the current
 * AieWaitForCoreDone() implementation is still valid or needs to be
 * updated to use events.
 */
static const u32 ProgramMem[] __attribute__ ((aligned(16))) = {
	0x0600703fU,
	0x0a000804U,
	0x000018c0U,
	0x603803f7U,
	0x00000203U,
	0x400c9803U,
	0x13201803U,
	0x31009803U,
	0x200003f7U,
	0x00000277U,
	0x800003f7U,
	0x00000257U,
	0x00000000U,
	0x39200000U,
	0x0000003dU,
	0x00000000U,
	0x00000000U,
	0x00000000U,
	0x40000000U,
	0x00001888U,
	0x00000000U,
	0x00000000U,
	0x00000000U,
	0x0000079aU,
	0x00000000U,
	0x00000000U,
	0x00000000U,
	0x00002614U,
	0x00000000U,
	0x00000000U,
	0x07428800U,
	0x00000000U,
	0x00010001U,
	0x00010001U,
	0x00030001U,
	0x00011000U,
	0x00010001U,
	0x00010001U,
	0x00010001U,
	0x00010001U,
	};



struct AieArray {
	u64 NocAddress;
	u32 NumCols;
	u32 NumRows;
	u32 NumMemRows;
	u32 StartCol;
	u32 StartRow;
	u32 StartTileRow;
};

static struct AieArray AieInst = {
	.NocAddress = 0x20000000000U,
	.NumCols = 50U,
	.NumRows = 8U,
	.StartCol = 0U,
	.StartRow = 1U,
};

static struct AieArray Aie2Inst = {
	.NocAddress = 0x20000000000U,
	.NumCols = 38U,
	.NumRows = 10U,
	.NumMemRows = 2U,
	.StartCol = 0U,
	.StartRow = 1U,
	.StartTileRow = 3U,
};


static const struct AieArray ShimInst = {
	.NocAddress = 0x20000000000U,
	.NumCols = 50U,
	.NumRows = 1U,
	.StartCol = 0U,
	.StartRow = 0U,
};

/*****************************************************************************/
/**
 * This function is used to set/clear bits in AIE PCSR
 *
 * @param Mask Mask to be written into PCSR_MASK register
 * @param Value Value to be written into PCSR_CONTROL register
 * @return
 *****************************************************************************/
static XStatus AiePcsrWrite(u32 Mask, u32 Value)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	PmOut32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
	/* Check mask value again for blind write check */
	PmChkRegOut32(((BaseAddress + NPI_PCSR_MASK_OFFSET)), Mask, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_MASK;
		goto done;
	}

	PmOut32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);
	/* Check control value again for blind write check */
	PmChkRegMask32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Mask, Value, Status);
	if (XPM_REG_WRITE_FAILED == Status) {
		DbgErr = XPM_INT_ERR_REG_WRT_NPI_PCSR_CONTROL;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * This function provides a delay for specified duration
 *
 * @param MicroSeconds Duration in micro seconds
 * @return
 *****************************************************************************/
static inline void AieWait(u32 MicroSeconds)
{
	usleep(MicroSeconds);
}


/*****************************************************************************/
/**
 * This function is used to enable AIE Core
 *
 * @param Col Column index of the Core
 * @param Row Row index of the Core
 * @return
 *****************************************************************************/
static void AieCoreEnable(u32 Col, u32 Row)
{
    /* Release reset to the Core */
	AieWrite64(TILE_BASEADDRESS(Col, Row) + AIE_CORE_CONTROL_OFFSET, 0U);

	/* Enable the Core */
	AieWrite64(TILE_BASEADDRESS(Col, Row) + AIE_CORE_CONTROL_OFFSET, 1U);
}

/*****************************************************************************/
/**
 * This function waits for a Core's DONE bit to be set
 *
 * @param Col Column index of the Core
 * @param Row Row index of the Core
 * @return Status Code
 *****************************************************************************/
static XStatus AieWaitForCoreDone(u32 Col, u32 Row)
{
	u64 StatusRegAddr = TILE_BASEADDRESS(Col, Row) + AIE_CORE_STATUS_OFFSET;
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = XPlmi_UtilPollForMask64((u32)(StatusRegAddr>>32),
				(u32)(StatusRegAddr), AIE_CORE_STATUS_DONE_MASK, 10U);
	if (Status != XST_SUCCESS) {
		DbgErr = XPM_INT_ERR_AIE_CORE_STATUS_TIMEOUT;
		PmInfo("ERROR: Poll for Done timeout \r\n");
	}

	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * This function loads a core's program memory with zeroization elf
 *
 * @param Col Column index of the Core
 * @param Row Row index of the Core
 * @return Status Code
 *****************************************************************************/
static XStatus ProgramCore(u32 Col, u32 Row, const u32 *PrgData, u32 NumOfWords)
{
	u64 PrgAddr = TILE_BASEADDRESS(Col, Row) + AIE_PROGRAM_MEM_OFFSET;

	return XPlmi_DmaXfr((u64)(0U)|(u32)PrgData, PrgAddr, NumOfWords, XPLMI_PMCDMA_0);
}

/*****************************************************************************/
/**
 * This function is used to cycle reset to the entire AIE array
 *
 * @return
 *****************************************************************************/
static XStatus ArrayReset(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK,
					ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ARRAY_RESET;
		goto done;
	}

	/* Wait for reset to propagate (1us) */
	AieWait(1U);

	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ARRAY_RESET_RELEASE;
	}

	/* Wait for reset to propagate (1us) */
	AieWait(1U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to scrub ECC enabled memories in the entire AIE array
 * Parameter: Action: ECC_SCRUB_DISABLE - Disable ECC scrub (disable PMEM Scrub using False event in all the Tiles)
 *		      ECC_SCRUB_ENABLE  - Enable ECC scrub (enable PMEM Scrub using True event in all the Tiles)
 *
 * @return
 *****************************************************************************/
static void TriggerEccScrub(u32 Action)
{
	u32 row, col;

	for (col = AieInst.StartCol; col < (AieInst.StartCol + AieInst.NumCols); col++) {
		for (row = AieInst.StartRow; row < (AieInst.StartRow + AieInst.NumRows); row++) {
			AieWrite64(TILE_BASEADDRESS(col, row) + AIE_CORE_ECC_SCRUB_EVENT_OFFSET, Action);
		}
	}
}

/*****************************************************************************/
/**
 * This function clock gates ME tiles clock column-wise
 *
 * @return
 *****************************************************************************/
static void AieClkGateByCol(void)
{
	u32 row, col;

	for(row = ShimInst.StartRow; row < (ShimInst.StartRow + ShimInst.NumRows); ++row) {
		for(col = ShimInst.StartCol; col < (ShimInst.StartCol + ShimInst.NumCols); ++col) {
			AieRMW64(TILE_BASEADDRESS(col, row) + AIE_TILE_CLOCK_CONTROL_OFFSET,
				AIE_TILE_CLOCK_CONTROL_CLK_BUFF_EN_MASK,
				~AIE_TILE_CLOCK_CONTROL_CLK_BUFF_EN_MASK);
		}
	}
}

static XStatus AieCoreMemInit(void)
{
	u32 row = AieInst.StartRow;
	u32 col = AieInst.StartCol;
	XStatus Status;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	for (col = AieInst.StartCol; col < (AieInst.StartCol + AieInst.NumCols); col++) {
		for (row = AieInst.StartRow; row < (AieInst.StartRow + AieInst.NumRows); row++) {
			PmDbg("---------- (%d, %d)----------\r\n", col, row);
			Status = ProgramCore(col, row, &ProgramMem[0], ARRAY_SIZE(ProgramMem));
			if (XST_SUCCESS != Status) {
				DbgErr = XPM_INT_ERR_PRGRM_CORE;
				goto done;
			}

			AieCoreEnable(col, row);
		}
	}

	/**
	 * NOTE: In future, if contents of ProgramMem[] are changed due to an
	 * updated AIE elf generated by latest tools, then the below check for
	 * core DONE may not work. Latest tools use events instead of DONE bit.
	 */
	Status = AieWaitForCoreDone(col-1U, row-1U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_CORE_STATUS_TIMEOUT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static void Aie2ClockInit(u32 BaseAddress)
{
	u32 col;

	/* Enable privelaged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK);

	/* Enable all column clocks */
	for (col = Aie2Inst.StartCol; col < (Aie2Inst.StartCol + Aie2Inst.NumCols); col++) {
		AieWrite64(AIE2_TILE_BASEADDRESS(col, 0) +
				AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 1U);
	}

	/* Disable privelaged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);
}

static void Aie2ClockGate(u32 BaseAddress)
{
	u32 col;

	/* Enable privelaged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK);

	/* Disable all column clocks */
	for (col = Aie2Inst.StartCol; col < (Aie2Inst.StartCol + Aie2Inst.NumCols); col++) {
		AieWrite64(AIE2_TILE_BASEADDRESS(col, 0) +
				AIE2_PL_MODULE_COLUMN_CLK_CTRL_OFFSET, 0U);
	}

	/* Disable privelaged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK, 0U);

}

static XStatus AieInitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DisableMask;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Check for ME Power Status */
	if( (XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) !=
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Unlock ME PCSR */
	XPmAieDomain_UnlockPcsr(BaseAddress);

	/* Relelase IPOR */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto fail;
	}

	/* TODO: Configure TOP_ROW and ROW_OFFSET by reading from EFUSE */
	/* Hardcode ME_TOP_ROW value for XCVC1902 device */
	PmOut32((BaseAddress + ME_NPI_ME_TOP_ROW_OFFSET), 0x00000008U);

	/* Get houseclean disable mask */
	DisableMask = XPm_In32(PM_HOUSECLEAN_DISABLE_REG_2) >> HOUSECLEAN_AIE_SHIFT;

	/* Set Houseclean Mask */
	PwrDomain->HcDisableMask |= DisableMask;

	Status = XST_SUCCESS;
	goto done;

fail:
	/* Lock ME PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2InitStart(XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 DisableMask;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const Aie2Dev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == Aie2Dev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = Aie2Dev->Node.BaseAddress;

	/* Check for AIE2 Power Status */
	if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) !=
			ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) {
		DbgErr = XPM_INT_ERR_POWER_SUPPLY;
		goto done;
	}

	/* Unlock AIE PCSR */
	XPmAieDomain_UnlockPcsr(BaseAddress);

	/* Release IPOR */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_RST_RELEASE;
		goto fail;
	}

	/* TODO: This needs to be data driven to handle multiple devices */
	/* Change from AIE. Each device has unique ME_TOP_ROW value */
	/* Configure ME_TOP_ROW and ROW_OFFSET registers */
	PmOut32((BaseAddress + ME_NPI_ME_TOP_ROW_OFFSET), Aie2Inst.NumRows);

	/* Change from AIE to AIE2. AIE handles in CDO */
	/* De-assert INIT_STATE */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_INITSTATE_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_INITSTATE_RELEASE;
		goto fail;
	}

	/* Change from AIE to AIE2. AIE handles in CDO */
	/* De-assert AIE2 array reset */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ARRAY_RESET_RELEASE;
		goto fail;
	}

	/* Get houseclean disable mask */
	DisableMask = XPm_In32(PM_HOUSECLEAN_DISABLE_REG_2) >> HOUSECLEAN_AIE_SHIFT;

	/* Set Houseclean Mask */
	PwrDomain->HcDisableMask |= DisableMask;

fail:
	/* Lock AIE PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieInitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Set PCOMPLETE bit */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK,
				 ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_PCOMPLETE;
		goto fail;
	}

	/* Clock gate ME Array column-wise (except SHIM array) */
	AieClkGateByCol();

	Status = XST_SUCCESS;

fail:
	/* Lock ME PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2InitFinish(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Unlock AIE PCSR */
	XPmAieDomain_UnlockPcsr(BaseAddress);

	/* Change from AIE to AIE2. */
	/* Clock gate for each column */
	Aie2ClockGate(BaseAddress);

	/* Change from AIE to AIE2. PCOMPLETE should be set at the end of the
	 * sequence */
	/* Set PCOMPLETE bit */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK,
			ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_AIE_PCOMPLETE;
	}

	/* Lock AIE PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieScanClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* De-assert ODISABLE[1] */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ODISABLE_1_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ODISABLE_1_RELEASE;
		goto fail;
	}

	if (HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_SCAN_CLEAR_MASK)) {
		PmInfo("Triggering ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Trigger Scan Clear */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
						ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TRIGGER;
			goto fail;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Scan Clear complete...", __func__);

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
					 ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_DONE_MASK,
					 AIE_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TIMEOUT;
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto fail;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Scan Clear PASS */
		if( (XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
		     ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) !=
		    ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_PASS;
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Scan Clear FAILED\r\n", __func__);
			Status = XST_FAILURE;
			goto fail;
		}

		/* Unwrite trigger bits */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK, 0);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_SCAN_CLEAR_TRIGGER_UNSET;
			goto fail;
		}
	} else {
		/* ScanClear is skipped */
		PmInfo("Skipping ScanClear for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	/* De-assert ODISABLE[0] */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ODISABLE_0_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_ODISABLE_0_RELEASE;
		goto fail;
	}

	/* De-assert GATEREG */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_GATEREG_MASK, 0U);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_GATEREG_UNSET;
		goto fail;
	}

	goto done;

fail:
	/* Lock ME PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieBisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_ISO;
		goto fail;
	}

	if (HOUSECLEAN_DISABLE_BISR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_BISR_MASK)) {
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		Status = XPmBisr_Repair(MEA_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEA_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEB_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEB_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEC_BISR_REPAIR;
			goto fail;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	goto done;

fail:
	/* Lock ME PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2Bisr(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Unlock AIE PCSR */
	XPmAieDomain_UnlockPcsr(BaseAddress);

	/* Change from AIE to AIE2. AIE has clocks enabled by default whereas AIE2
	 * has then disabled by default. Clocks must be up from this point to
	 * continue the sequence */
	/* Enable all column clocks */
	Aie2ClockInit(BaseAddress);

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_PMC_SOC_ISO;
		goto fail;
	}

	if (HOUSECLEAN_DISABLE_BISR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_BISR_MASK)) {
		PmInfo("Triggering BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		Status = XPmBisr_Repair(MEA_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEA_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEB_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEB_BISR_REPAIR;
			goto fail;
		}

		Status = XPmBisr_Repair(MEC_TAG_ID);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEC_BISR_REPAIR;
		}
	} else {
		/* BISR is skipped */
		PmInfo("Skipping BISR for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

fail:
	/* Lock AIE PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieMbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	if (HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK)) {
		PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);

		/* Clear MEM_CLEAR_EN_ALL to minimize power during mem clear */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_EN;
			goto fail;
		}

		/* Set OD_MBIST_ASYNC_RESET_N bit */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK,
					ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MBIST_RESET;
			goto fail;
		}

		/* Assert OD_BIST_SETUP_1 */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK,
					ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BIST_RESET;
			goto fail;
		}

		/* Assert MEM_CLEAR_TRIGGER */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK,
			ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER;
			goto fail;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Mem Clear complete...", __func__);

		/* Wait for Mem Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
					ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					AIE_POLL_TIMEOUT);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto fail;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Mem Clear PASS */
		if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) !=
			ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Mem Clear FAILED\r\n", __func__);
			DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			Status = XST_FAILURE;
			goto fail;
		}

		/* Clear OD_MBIST_ASYNC_RESET_N bit */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MBIST_RESET_RELEASE;
			goto fail;
		}

		/* De-assert OD_BIST_SETUP_1 */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BIST_RESET_RELEASE;
			goto fail;
		}

		/* De-assert MEM_CLEAR_TRIGGER */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER_UNSET;
			goto fail;
		}
	} else {
		/* MBIST is skipped */
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	Status = XST_SUCCESS;
	goto done;

fail:
	/* Lock ME PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2MbistClear(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Unlock AIE PCSR */
	XPmAieDomain_UnlockPcsr(BaseAddress);

	if (HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK != (PwrDomain->HcDisableMask &
				HOUSECLEAN_DISABLE_MBIST_CLEAR_MASK)) {
		PmInfo("Triggering MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
		/* Change from AIE to AIE2. */
		/* TODO: In AIE this is set to low power mode to avoid failures. Need
		 * confirmation that for AIE2 low power mode is not required. */
		/* Assert MEM_CLEAR_EN_ALL */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK,
				ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_EN;
			goto fail;
		}

		/* Set OD_MBIST_ASYNC_RESET_N bit */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK,
				ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MBIST_RESET;
			goto fail;
		}

		/* Assert OD_BIST_SETUP_1 */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK,
				ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BIST_RESET;
			goto fail;
		}

		/* Assert MEM_CLEAR_TRIGGER */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK,
				ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER;
			goto fail;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Mem Clear complete...", __func__);

		/* Wait for Mem Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
				ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
				AIE_POLL_TIMEOUT);
		if (Status != XST_SUCCESS) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_DONE_TIMEOUT;
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto fail;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Mem Clear PASS */
		if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
					ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) !=
				ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Mem Clear FAILED\r\n", __func__);
			DbgErr = XPM_INT_ERR_MEM_CLEAR_PASS;
			Status = XST_FAILURE;
			goto fail;
		}

		/* Clear OD_MBIST_ASYNC_RESET_N bit */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MBIST_RESET_RELEASE;
			goto fail;
		}

		/* De-assert OD_BIST_SETUP_1 */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_BIST_RESET_RELEASE;
			goto fail;
		}

		/* De-assert MEM_CLEAR_TRIGGER */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK, 0U);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_MEM_CLEAR_TRIGGER_UNSET;
			goto fail;
		}
	} else {
		/* MBIST is skipped */
		PmInfo("Skipping MBIST for power node 0x%x\r\n", PwrDomain->Power.Node.Id);
	}

	Status = XST_SUCCESS;

fail:
	/* Lock AIE PCSR */
	XPmAieDomain_LockPcsr(BaseAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus AieMemInit(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* This function does not use the args */
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	PmDbg("---------- START ----------\r\n");

	/* Enable scrub, Scrub ECC protected memories */
	TriggerEccScrub(ECC_SCRUB_ENABLE);
	/* Wait for scrubbing to finish (1ms)*/
	AieWait(1000U);

	/* Disable scrub, Scrub ECC protected memories */
	TriggerEccScrub(ECC_SCRUB_DISABLE);

	/* Reset Array */
	Status = ArrayReset();
	if (XST_SUCCESS != Status) {
		PmErr("ERROR: Array reset failed\r\n");
	}
	/* Zeroize Data Memory */
	Status = AieCoreMemInit();
	if (Status != XST_SUCCESS) {
		PmInfo("ERROR: MemInit failed\r\n");
	}
	/* Reset Array */
	Status = ArrayReset();
	if (XST_SUCCESS != Status) {
		PmErr("ERROR: Array reset failed\r\n");
		/* Lock ME PCSR */
		XPmAieDomain_LockPcsr(BaseAddress);
	}
	PmDbg("---------- END ----------\r\n");

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus Aie2MemInit(const XPm_PowerDomain *PwrDomain, const u32 *Args,
		u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	XStatus CoreZeroStatus = XST_FAILURE;
	XStatus MemZeroStatus = XST_FAILURE;
	XStatus MemTileZeroStatus = XST_FAILURE;
	u32 AieZeroizationTime = 0U;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 BaseAddress;
	u32 row;
	u32 col;
	u32 mrow;

	/* This function does not use the args */
	(void)PwrDomain;
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Enable privelaged write access */
	XPm_RMW32(BaseAddress + AIE2_NPI_ME_PROT_REG_CTRL_OFFSET,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK,
			ME_PROT_REG_CTRL_PROTECTED_REG_EN_MASK);

	/* Enable memory zeroization for mem tiles. Mem tiles start at row 1. */
	for (col = Aie2Inst.StartCol; col < (Aie2Inst.StartCol + Aie2Inst.NumCols); col++) {
		for (row = 1U; row <= Aie2Inst.NumMemRows; row++) {
			AieRMW64(AIE2_TILE_BASEADDRESS(col, row) + AIE2_MEM_TILE_MODULE_MEM_CTRL_OFFSET,
					AIE2_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK,
					AIE2_MEM_TILE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
		}
	}

	/* Enable memory zeroization for all AIE2 tiles.
	 * Enable for core and memory modules. */
	for (col = Aie2Inst.StartCol; col < (Aie2Inst.StartCol + Aie2Inst.NumCols); col++) {
		for (row = Aie2Inst.StartTileRow; row < (Aie2Inst.StartRow + Aie2Inst.NumRows); row++) {
			AieWrite64(AIE2_TILE_BASEADDRESS(col, row) + AIE2_CORE_MODULE_MEM_CTRL_OFFSET,
					AIE2_CORE_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
			AieWrite64(AIE2_TILE_BASEADDRESS(col, row) + AIE2_MEM_MODULE_MEM_CTRL_OFFSET,
					AIE2_MEM_MODULE_MEM_CTRL_MEM_ZEROISATION_MASK);
		}
	}

	col = Aie2Inst.StartCol + Aie2Inst.NumCols - 1U;
	row = Aie2Inst.StartRow + Aie2Inst.NumRows - 1U;
	mrow = Aie2Inst.StartRow + Aie2Inst.NumMemRows - 1U;

	/* Poll the last cell for each tile type for memory zeroization complete */
	while ((XST_SUCCESS != MemTileZeroStatus) ||
	       (XST_SUCCESS != CoreZeroStatus) ||
	       (XST_SUCCESS != MemZeroStatus)) {

		if (0U == AieRead64(AIE2_TILE_BASEADDRESS(col, mrow) +
				AIE2_MEM_TILE_MODULE_MEM_CTRL_OFFSET)) {
			MemTileZeroStatus = XST_SUCCESS;
		}
		if (0U == AieRead64(AIE2_TILE_BASEADDRESS(col, row) +
				AIE2_CORE_MODULE_MEM_CTRL_OFFSET)) {
			CoreZeroStatus = XST_SUCCESS;
		}
		if (0U == AieRead64(AIE2_TILE_BASEADDRESS(col, row) +
				AIE2_MEM_MODULE_MEM_CTRL_OFFSET)) {
			MemZeroStatus = XST_SUCCESS;
		}

		AieZeroizationTime++;
		if (AieZeroizationTime > XPLMI_TIME_OUT_DEFAULT) {
			DbgErr = XPM_INT_ERR_AIE_MEMORY_ZEROISATION;
			Status = XST_FAILURE;
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static struct XPm_PowerDomainOps AieOps[XPM_AIE_OPS_MAX] = {
	[XPM_AIE_OPS] = {
		.InitStart = AieInitStart,
		.InitFinish = AieInitFinish,
		.ScanClear = AieScanClear,
		.Bisr = AieBisr,
		.Mbist = AieMbistClear,
		.MemInit = AieMemInit,
		/* Mask to indicate which Ops are present */
		.InitMask = (BIT16(FUNC_INIT_START) |
				BIT16(FUNC_INIT_FINISH) |
				BIT16(FUNC_SCAN_CLEAR) |
				BIT16(FUNC_BISR) |
				BIT16(FUNC_MBIST_CLEAR) |
				BIT16(FUNC_MEM_INIT))
	},
	[XPM_AIE2_OPS] = {
		.InitStart = Aie2InitStart,
		.InitFinish = Aie2InitFinish,
		.ScanClear = AieScanClear,
		.Bisr = Aie2Bisr,
		.Mbist = Aie2MbistClear,
		.MemInit = Aie2MemInit,
		/* Mask to indicate which Ops are present */
		.InitMask = (BIT16(FUNC_INIT_START) |
				BIT16(FUNC_INIT_FINISH) |
				BIT16(FUNC_SCAN_CLEAR) |
				BIT16(FUNC_BISR) |
				BIT16(FUNC_MBIST_CLEAR) |
				BIT16(FUNC_MEM_INIT))
	},
};

XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;
	u32 Platform = XPm_GetPlatform();
	u32 IdCode = XPm_GetIdCode();
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const struct XPm_PowerDomainOps *Ops = NULL;

	/* Set HC Ops based on AIE version */
	if (PM_POWER_ME == Id) {
		/* AIE1: Ops */
		Ops = &AieOps[XPM_AIE_OPS];
		/* AIE1: Array size for SPP/EMU (non-silicon platforms) */
		if (Platform != PLATFORM_VERSION_SILICON) {
			AieInst.NumCols = 7U;
			AieInst.NumRows = 5U;
			AieInst.StartCol = 6U;
			AieInst.StartRow = 1U;
		}
	} else if (PM_POWER_ME2 == Id) {
		/* AIE2: Ops */
		Ops = &AieOps[XPM_AIE2_OPS];
	} else {
		DbgErr = XPM_INT_ERR_INVALID_PWR_DOMAIN;
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	/* AIE2 Instance for VE2302 (TODO: Remove this when topology support is added) */
	if (PMC_TAP_IDCODE_DEV_SBFMLY_VE2302 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) {
		Aie2Inst.NumCols = 17U;
		Aie2Inst.NumRows = 3U;
		Aie2Inst.NumMemRows = 1U;
		Aie2Inst.StartCol = 0U;
		Aie2Inst.StartRow = 1U;
		Aie2Inst.StartTileRow = 2U;
	}

	/* AIE Instance for VC1702 (TODO: Remove this when topology support is added) */
	if ((PMC_TAP_IDCODE_DEV_SBFMLY_VC1702 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK)) ||
			(PMC_TAP_IDCODE_DEV_SBFMLY_VE1752 == (IdCode & PMC_TAP_IDCODE_DEV_SBFMLY_MASK))) {
		AieInst.NumCols = 38U;
		AieInst.NumRows = 8U;
		AieInst.StartCol = 0U;
		AieInst.StartRow = 1U;
	}

	/* NOP for HC on QEMU */
	if (Platform == PLATFORM_VERSION_QEMU) {
		Ops = NULL;
	}

	Status = XPmPowerDomain_Init(&AieDomain->Domain, Id, BaseAddress,
			Parent, Ops);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

	/* Clear AIE section of PMC RAM register reserved for houseclean disable */
	XPm_RMW32(PM_HOUSECLEAN_DISABLE_REG_2, PM_HOUSECLEAN_DISABLE_AIE_MASK, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
