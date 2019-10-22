/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xplmi_util.h"
#include "xplmi_dma.h"
#include "xpm_common.h"
#include "xpm_aie.h"
#include "xpm_regs.h"
#include "xpm_bisr.h"
#include "xpm_device.h"

#define AIE_POLL_TIMEOUT 0X1000000U

#define COL_SHIFT 23U
#define ROW_SHIFT 18U
#define TILE_BASEADDRESS(col, row) ((u64)0x20000000000U +\
			((u64)(col) << COL_SHIFT)+\
			((u64)(row) << ROW_SHIFT))

#define AIE_CORE_CONTROL_ENABLE_MASK (1U<<0U)
#define AIE_CORE_CONTROL_RESET_MASK (1U<<1U)
#define AIE_CORE_STATUS_DONE_MASK   (1UL<<20U)

#define AieWrite64(addr, val) swea(addr, val)

/* Buffer to hold AIE data memory zeroization elf*/
static u32 ProgramMem[] __attribute__ ((aligned(16))) = {
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
	u32 NpiAddress;
	u64 NocAddress;
	u32 NumCols;
	u32 NumRows;
	u32 StartCol;
	u32 StartRow;
	u8 IsSecure;
};

static struct AieArray AieInst = {
	.NpiAddress = 0xF70A0000U,
	.NocAddress = 0x20000000000U,
	.NumCols = 50U,
	.NumRows = 8U,
	.StartCol = 0U,
	.StartRow = 1U,
	.IsSecure = 0U,
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

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	PmOut32((BaseAddress + NPI_PCSR_MASK_OFFSET), Mask);
	PmOut32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), Value);

	Status = XST_SUCCESS;

done:
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

	Status = XPlmi_UtilPollForMask64((u32)(StatusRegAddr>>32),
				(u32)(StatusRegAddr), AIE_CORE_STATUS_DONE_MASK, 10U);
	if (Status != XST_SUCCESS) {
		PmInfo("ERROR: Poll for Done timeout \r\n");
	}

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
static XStatus ProgramCore(u32 Col, u32 Row, u32 *PrgData, u32 NumOfWords)
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

	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK,
					ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_ARRAY_RESET_MASK, 0U);

done:
	return Status;
}

/*****************************************************************************/
/**
 * This function is used to scrub ECC enabled memories in the entire AIE array
 *
 * @return
 *****************************************************************************/
static void TriggerEccScrub(void)
{
	u32 row, col;

	for (col = AieInst.StartCol; col < (AieInst.StartCol + AieInst.NumCols); col++) {
		for (row = AieInst.StartRow; row < (AieInst.StartRow + AieInst.NumRows); row++) {
			AieWrite64(TILE_BASEADDRESS(col, row) + AIE_CORE_ECC_SCRUB_EVENT_OFFSET, 1U);
		}
	}
}


static XStatus MemInit(void)
{
	u32 row = AieInst.StartRow;
	u32 col = AieInst.StartCol;
	int Status;

	for (col = AieInst.StartCol; col < (AieInst.StartCol + AieInst.NumCols); col++) {
		for (row = AieInst.StartRow; row < (AieInst.StartRow + AieInst.NumRows); row++) {
			PmDbg("---------- (%d, %d)----------\r\n", col, row);
			Status = ProgramCore(col, row, &ProgramMem[0], ARRAY_SIZE(ProgramMem));
			if (XST_SUCCESS != Status) {
				goto done;
			}

			AieCoreEnable(col, row);
		}
	}

	Status = AieWaitForCoreDone(col-1U, row-1U);

done:
	return Status;
}



static XStatus AieInitStart(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* Check for ME Power Status */
	if( (XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) !=
			 ME_NPI_REG_PCSR_STATUS_ME_PWR_SUPPLY_MASK) {
		goto done;
	}

	/* Unlock ME PCSR */
	PmOut32((BaseAddress + NPI_PCSR_LOCK_OFFSET), NPI_PCSR_UNLOCK_VAL);

	/* Relelase IPOR */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ME_IPOR_MASK, 0U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* TODO: Configure TOP_ROW and ROW_OFFSET by reading from EFUSE */
	/* Hardcode ME_TOP_ROW value for S80 device */
	PmOut32((BaseAddress + ME_NPI_ME_TOP_ROW_OFFSET), 0x00000008U);

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieInitFinish(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Set PCOMPLETE bit */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK,
				 ME_NPI_REG_PCSR_MASK_PCOMPLETE_MASK);
	if (XST_SUCCESS != Status) {
		goto done;
	}
	/* TODO: Check if we can lock PCSR registers here */

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieScanClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	/* De-assert ODISABLE[1] */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ODISABLE_1_MASK, 0U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (PLATFORM_VERSION_SILICON == Platform) {
		/* Trigger Scan Clear */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK,
						ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Scan Clear complete...", __func__);

		/* Wait for Scan Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
					 ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_DONE_MASK,
					 AIE_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto done;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Scan Clear PASS */
		if( (XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
		     ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) !=
		    ME_NPI_REG_PCSR_STATUS_SCAN_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Scan Clear FAILED\r\n", __func__);
			Status = XST_FAILURE;
			goto done;
		}

		/* Unwrite trigger bits */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_SCAN_CLEAR_TRIGGER_MASK, 0);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* De-assert ODISABLE[0] */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_ODISABLE_0_MASK, 0U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* De-assert GATEREG */
	Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_GATEREG_MASK, 0U);

done:
	return Status;
}

static XStatus AieBisr(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	/* Remove PMC-NoC domain isolation */
	Status = XPmDomainIso_Control((u32)XPM_NODEIDX_ISO_PMC_SOC, FALSE_VALUE);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPmBisr_Repair(MEA_TAG_ID);
	if (XST_SUCCESS != Status) {
                goto done;
        }
	Status = XPmBisr_Repair(MEB_TAG_ID);
	if (XST_SUCCESS != Status) {
                goto done;
        }
	Status = XPmBisr_Repair(MEC_TAG_ID);
	if (XST_SUCCESS != Status) {
                goto done;
        }
done:
	return Status;
}

static XStatus AieMbistClear(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	const XPm_Device * const AieDev = XPmDevice_GetById(PM_DEV_AIE);
	if (NULL == AieDev) {
		goto done;
	}

	BaseAddress = AieDev->Node.BaseAddress;

	if (Platform == PLATFORM_VERSION_SILICON) {
		/* Assert MEM_CLEAR_EN_ALL */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK,
					ME_NPI_REG_PCSR_MASK_MEM_CLEAR_EN_ALL_MASK);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* De-assert OD_MBIST_ASYNC_RESET_N */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK,
					ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* Assert OD_BIST_SETUP_1 */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK,
					ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* Assert MEM_CLEAR_TRIGGER */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK,
			ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		XPlmi_Printf(DEBUG_INFO, "INFO: %s : Wait for AIE Mem Clear complete...", __func__);

		/* Wait for Mem Clear DONE */
		Status = XPm_PollForMask(BaseAddress + NPI_PCSR_STATUS_OFFSET,
					ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_DONE_MASK,
					AIE_POLL_TIMEOUT);
		if (Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "ERROR\r\n");
			goto done;
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "DONE\r\n");
		}

		/* Check Mem Clear PASS */
		if ((XPm_In32(BaseAddress + NPI_PCSR_STATUS_OFFSET) &
			ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) !=
			ME_NPI_REG_PCSR_STATUS_MEM_CLEAR_PASS_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "ERROR: %s: AIE Mem Clear FAILED\r\n", __func__);
			Status = XST_FAILURE;
			goto done;
		}

		/* Assert OD_MBIST_ASYNC_RESET_N */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_MBIST_ASYNC_RESET_N_MASK, 0U);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* De-assert OD_BIST_SETUP_1 */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_OD_BIST_SETUP_1_MASK, 0U);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		/* De-assert MEM_CLEAR_TRIGGER */
		Status = AiePcsrWrite(ME_NPI_REG_PCSR_MASK_MEM_CLEAR_TRIGGER_MASK, 0U);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus AieMemInit(u32 *Args, u32 NumOfArgs)
{
	XStatus Status = XST_FAILURE;

	/* This function does not use the args */
	(void)Args;
	(void)NumOfArgs;

	PmDbg("---------- START ----------\r\n");

	/* Scrub ECC protected memories */
	TriggerEccScrub();
	/* Wait for scrubbing to finish (1ms)*/
	AieWait(1000U);
	/* Reset Array */
	Status = ArrayReset();
	if (XST_SUCCESS != Status) {
		PmErr("ERROR: Array reset failed\r\n");
	}
	/* Zeroize Data Memory */
	Status = MemInit();
	if (Status != XST_SUCCESS) {
		PmInfo("ERROR: MemInit failed\r\n");
	}
	/* Reset Array */
	Status = ArrayReset();
	if (XST_SUCCESS != Status) {
		PmErr("ERROR: Array reset failed\r\n");
	}
	PmDbg("---------- END ----------\r\n");

	return Status;
}

static struct XPm_PowerDomainOps AieOps = {
	.InitStart = AieInitStart,
	.InitFinish = AieInitFinish,
	.ScanClear = AieScanClear,
	.Bisr = AieBisr,
	.Mbist = AieMbistClear,
	.MemInit = AieMemInit,
};

/*****************************************************************************/
/**
 * @brief This funcction unlocks the AIE PCSR registers.
 *
 *****************************************************************************/
void XPmAieDomain_UnlockPcsr(u32 BaseAddress)
{
	u32 NpiPcsrLockReg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
	PmOut32(NpiPcsrLockReg, NPI_PCSR_UNLOCK_VAL);
}

/*****************************************************************************/
/**
 * @brief This funcction locks the AIE PCSR registers.
 *
 *****************************************************************************/
void XPmAieDomain_LockPcsr(u32 BaseAddress)
{
	u32 NpiPcsrLockReg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
	PmOut32(NpiPcsrLockReg, 0x00000000U);
}

XStatus XPmAieDomain_Init(XPm_AieDomain *AieDomain, u32 Id, u32 BaseAddress,
			   XPm_Power *Parent)
{
	XStatus Status = XST_FAILURE;

	/* Skip AIE Init for base QEMU without COSIM */
	if (Platform == PLATFORM_VERSION_QEMU) {
		AieOps.InitStart = NULL;
		AieOps.InitFinish = NULL;
		AieOps.ScanClear = NULL;
		AieOps.Bisr = NULL;
		AieOps.Mbist = NULL;
		AieOps.MemInit = NULL;
	}
	/* For SPP and EMU,  setup the array size */
	if (Platform != PLATFORM_VERSION_SILICON) {
		AieInst.NumCols = 7U;
		AieInst.NumRows = 5U;
		AieInst.StartCol = 6U;
		AieInst.StartRow = 1U;
	}

	Status = XPmPowerDomain_Init(&AieDomain->Domain, Id, BaseAddress, Parent, &AieOps);

	return Status;
}
